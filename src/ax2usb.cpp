#include "ax2usb.h"
#include <Arduino.h>
#include <mutex>
#include <string>
#include "ax2usbmap.hpp"
#include "ps2code.hpp"
#include "util.h"

#define AX2USB_DEBUG 0
#include "debug.h"

namespace ax2usb {

namespace {

constexpr uint8_t REPORT_ID_KBD = 1;
constexpr uint8_t REPORT_ID_SYS = 2;
constexpr uint8_t REPORT_ID_CONSUMER = 3;
constexpr const uint8_t desc_hid_report[] = { TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(REPORT_ID_KBD)),
	                                            TUD_HID_REPORT_DESC_SYSTEM_CONTROL(HID_REPORT_ID(REPORT_ID_SYS)),
	                                            TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(REPORT_ID_CONSUMER)) };

constexpr uint8_t SYSTEM_CONTROL_POWER_OFF = 0x01;
constexpr uint8_t SYSTEM_CONTROL_STANDBY = 0x02;
constexpr uint8_t SYSTEM_CONTROL_WAKE_HOST = 0x03;

constexpr uint16_t DO_NOTHING = 0x00;
constexpr uint16_t TRANSPORT_CONTROL_PLAY = 0xb0;
constexpr uint16_t TRANSPORT_CONTROL_PAUSE = 0xb1;
constexpr uint16_t TRANSPORT_CONTROL_SCAN_NEXT_TRACK = 0xb5;
constexpr uint16_t TRANSPORT_CONTROL_SCAN_PREVIOUS_TRACK = 0xb6;
constexpr uint16_t TRANSPORT_CONTROL_PLAY_PAUSE = 0xcd;

constexpr uint16_t AUDIO_CONTROL_MUTE = 0xe2;
constexpr uint16_t AUDIO_CONTROL_VOLUME_INCREMENT = 0xe9;
constexpr uint16_t AUDIO_CONTROL_VOLUME_DECREMENT = 0xea;

constexpr uint32_t STATE_TIMEOUT_MSEC = 300;
constexpr int USB_SEND_RETRY_COUNT = 3;

}  // namespace

#if AX2USB_DEBUG
char
AX2USB::usb_mod_char(uint8_t mod_key) {
	switch (mod_key) {
		case HID_KEY_CONTROL_LEFT:
			return 'C';
		case HID_KEY_SHIFT_LEFT:
			return 'S';
		case HID_KEY_ALT_LEFT:
			return 'A';
		case HID_KEY_GUI_LEFT:
			return 'W';
		case HID_KEY_CONTROL_RIGHT:
			return 'c';
		case HID_KEY_SHIFT_RIGHT:
			return 's';
		case HID_KEY_ALT_RIGHT:
			return 'a';
		case HID_KEY_GUI_RIGHT:
			return 'w';
		default:
			return '*';
	}
}

std::string
AX2USB::usb_mods_str(const usb_mod_t& mod) {
	std::string ret;

	ret += mod.l_ctrl ? 'C' : 'c';
	ret += mod.l_shift ? 'S' : 's';
	ret += mod.l_alt ? 'A' : 'a';
	ret += mod.l_gui ? 'W' : 'w';
	ret += '|';
	ret += mod.r_ctrl ? 'C' : 'c';
	ret += mod.r_shift ? 'S' : 's';
	ret += mod.r_alt ? 'A' : 'a';
	ret += mod.r_gui ? 'W' : 'w';

	return ret;
}

std::string
AX2USB::usb_led_str() const {
	std::string ret;

	ret += usb_led.num ? 'N' : 'n';
	ret += usb_led.caps ? 'C' : 'c';
	ret += usb_led.scroll ? 'S' : 's';
	ret += usb_led.compose ? 'M' : 'm';
	ret += usb_led.kana ? 'K' : 'k';

	return ret;
}

static std::string
usb_key_str(uint8_t key) {
	if (key >= HID_KEY_0 && key <= HID_KEY_9) {
		return std::string{ static_cast<char>('0' + key - HID_KEY_0) };
	} else if (key >= HID_KEY_A && key <= HID_KEY_Z) {
		return std::string{ static_cast<char>('A' + key - HID_KEY_A) };
	} else if (key == HID_KEY_PRINT_SCREEN) {
		return "Prt";
	} else if (key == HID_KEY_PAUSE) {
		return "Pause";
	} else {
		return std::string{ util::hexchar((key & 0xf0) >> 4), util::hexchar(key & 0x0f) };
	}
}

std::string
AX2USB::usb_codes_str() const {
	std::string ret;
	ret.reserve(24);
	for (size_t i = 0; i < std::size(usb_codes); i++) {
		ret += '[';
		ret += usb_key_str(usb_codes[i]);
		ret += ']';
	}

	return ret;
}

static char
mod_mark(bool make_break) {
	return make_break ? '#' : '~';
}

static char
key_mark(bool make_break) {
	return make_break ? '+' : '-';
}

static const char*
mb_str(bool make_break) {
	return make_break ? "make" : "break";
};
#endif

bool
AX2USB::begin(uint8_t ps2_data_pin, uint8_t ps2_clock_pin) {
	usb_hid.setBootProtocol(HID_ITF_PROTOCOL_KEYBOARD);
	usb_hid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
	usb_hid.setPollInterval(2);
	usb_hid.enableOutEndpoint(false);
	usb_hid.setReportCallback(nullptr, hid_report_callback);

#if defined(ARDUINO_ARCH_MBED) && defined(ARDUINO_ARCH_RP2040)
	// Manual begin() is required on core without built-in support for TinyUSB
	// such as mbed rp2040
	TinyUSB_Device_Init(0);
#endif

	if (!usb_hid.begin()) {
		return false;
	}

	ps2.set_recv_callback([this](auto code) { rx.put(code); });
	ps2.begin(ps2_data_pin, ps2_clock_pin);

	while (!TinyUSBDevice.mounted()) {
		delay(1);
	}

#if AX2USB_DEBUG
	for (size_t i = 0; i < std::size(map::ax2_usb); i++) {
		auto v = map::ax2_usb[i];
		if (v == 0)
			continue;
		for (size_t j = i + 1; j < std::size(map::ax2_usb); j++) {
			if (map::ax2_usb[j] == v) {
				DEBUG_PRINTLN("map[%u] and map[%u] are same", i, j);
			}
		}
	}
#endif

	return true;
}

static uint8_t
usb_key_to_mod_mask(uint8_t usb_key) {
	if (usb_key < HID_KEY_CONTROL_LEFT || usb_key > HID_KEY_GUI_RIGHT) {
		return 0;
	}
	return (1 << (usb_key - HID_KEY_CONTROL_LEFT));
}

bool
AX2USB::ps2_available() const {
	std::lock_guard<Mutex> lock(rx_mux);
	return rx.count() > 0;
}

int
AX2USB::ps2_read() {
	std::lock_guard<Mutex> lock(rx_mux);
	uint8_t v;
	if (rx.get(v)) {
		return v;
	}
	return -1;
}

void
AX2USB::wait_usb_ready() {
	while (!usb_hid.ready()) {
		delay(1);
	}
}

void
AX2USB::send_keyboard_report() {
	for (int i = 0; i < USB_SEND_RETRY_COUNT; i++) {
		if (usb_hid.keyboardReport(REPORT_ID_KBD, usb_mod.value, usb_codes)) {
			break;
		}
		wait_usb_ready();
	}
}

void
AX2USB::send_report16(uint8_t report_id, uint16_t usage) {
	for (int i = 0; i < USB_SEND_RETRY_COUNT; i++) {
		if (usb_hid.sendReport16(report_id, usage)) {
			break;
		}
		wait_usb_ready();
	}
}

void
AX2USB::send_usb_key(uint8_t usb, bool make_break) {
	if (auto mask = usb_key_to_mod_mask(usb); mask) {
		// modifiers
		if (update_usb_modifier(mask, make_break)) {
			send_keyboard_report();
			DEBUG_PRINTLN(">%c%c %s %s", mod_mark(make_break), usb_mod_char(usb), usb_mods_str(usb_mod).c_str(), usb_codes_str().c_str());
		}
	} else {
		// normal keys
		if (update_usb_codes(usb, make_break)) {
			send_keyboard_report();
			DEBUG_PRINTLN(">%c%s %s %s", key_mark(make_break), usb_key_str(usb).c_str(), usb_mods_str(usb_mod).c_str(),
			              usb_codes_str().c_str());
		}
	}
}

void
AX2USB::send_usb_key_mod(uint8_t usb, uint8_t usb_mod_key, bool make_break) {
	bool cod = update_usb_codes(usb, make_break);
	bool mod = update_usb_modifier(usb_key_to_mod_mask(usb_mod_key), make_break);
	if (cod || mod) {
		send_keyboard_report();
		DEBUG_PRINTLN(">%c%c%c%s %s %s", mod_mark(make_break), usb_mod_char(usb_mod_key), key_mark(make_break),
		              usb_key_str(usb).c_str(), usb_mods_str(usb_mod).c_str(), usb_codes_str().c_str());
	}
}

void
AX2USB::send_usb_key_oneshot(uint8_t usb) {
	if (update_usb_codes(usb, true)) {
		send_keyboard_report();
		wait_usb_ready();
	}
	update_usb_codes(usb, false);
	// send break code even if make code was not sent, to be sure
	send_keyboard_report();
}

void
AX2USB::send_report16_oneshot(uint8_t report_id, uint16_t usage) {
	send_report16(report_id, usage);
	wait_usb_ready();
	send_report16(report_id, DO_NOTHING);
}

void
AX2USB::handle_code(uint8_t code, bool make_break) {
	if (code == ps2key::ALT_PRINT_SCREEN) {
		send_usb_key_mod(HID_KEY_PRINT_SCREEN, HID_KEY_ALT_LEFT, make_break);
	} else if (code < std::size(map::ax2_usb)) {
		if (auto usb = map::ax2_usb[code]; usb) {
			if (!handle_special_key(usb, make_break)) {
				send_usb_key(usb, make_break);
			}
		} else {
			DEBUG_PRINTLN("%s %02x is not mapped to usb_key", mb_str(make_break), code);
		}
	} else {
		DEBUG_PRINTLN("%s %02x not handled", mb_str(make_break), code);
	}
}

AX2USB::state_t
AX2USB::state_base(uint8_t code) {
	if (code == ps2ind::BREAK) {
		return state_t::brk_received;
	} else if (code == ps2ind::E0) {
		return state_t::e0_received;
	} else if (code == ps2ind::E1) {
		return state_t::e1_received;
	} else if (code == ps2ind::BAT_COMPLETED) {
		should_send_led = true;
	} else {
		handle_code(code, true);
	}
	return state_t::base;
}

AX2USB::state_t
AX2USB::state_brk_received(uint8_t code) {
	handle_code(code, false);
	return state_t::base;
}

void
AX2USB::handle_e0_code(uint8_t code, bool make_break) {
	if (code == ps2key::L_SHIFT || code == ps2key::R_SHIFT) {
		DEBUG_PRINTLN("simply ignore %cshift after E0", key_mark(make_break));
	} else if (code == ps2key::BREAK) {  // [Pause/Break] key
		if (!handle_special_key(HID_KEY_PAUSE, make_break)) {
			send_usb_key_mod(HID_KEY_PAUSE, HID_KEY_CONTROL_LEFT, make_break);
		}
	} else {
		if (auto* ent = find_e0_map(code); ent) {
			if (!handle_special_key(ent->usb, make_break)) {
				send_usb_key(ent->usb, make_break);
			}
		} else {
			DEBUG_PRINTLN("%02x: Unexpected code after E0 %s", code, mb_str(make_break));
		}
	}
}

AX2USB::state_t
AX2USB::state_e0_received(uint8_t code) {
	if (code == ps2ind::BREAK) {  // key release
		return e0_break_received;
	}
	handle_e0_code(code, true);
	return state_t::base;
}

AX2USB::state_t
AX2USB::state_e0_break_received(uint8_t code) {
	handle_e0_code(code, false);
	return state_t::base;
}

AX2USB::state_t
AX2USB::state_e1_received(uint8_t code) {
	if (code == ps2ind::BREAK) {
		return state_t::e1_break_received;
	} else if (code == ps2key::L_CTRL) {
		// wait for pause, keep state
		return state_t::e1_received;
	} else if (code == ps2key::PAUSE) {
		if (!handle_special_key(HID_KEY_PAUSE, true)) {
			send_usb_key(HID_KEY_PAUSE, true);
		}
	} else {
		DEBUG_PRINTLN("%02x: Unexpected code after E1 make", code);
	}
	return state_t::base;
}

AX2USB::state_t
AX2USB::state_e1_break_received(uint8_t code) {
	if (code == ps2key::L_CTRL) {
		// wait for pause, keep state
		return state_t::e1_received;
	} else if (code == ps2key::PAUSE) {
		if (!handle_special_key(HID_KEY_PAUSE, false)) {
			send_usb_key(HID_KEY_PAUSE, false);
		}
	} else {
		DEBUG_PRINTLN("%02x: Unexpected code after E1 break", code);
	}
	return state_t::base;
}

AX2USB::state_t
AX2USB::state_led_wait_ack(uint8_t code) {
	if (code == ps2ind::ACK) {
		ps2.send(ps2_led.value);
		should_send_led = false;
		timeout_state_started = millis();
		return state_t::wait_ack;
	}
	return state_t::led_wait_ack;
}

AX2USB::state_t
AX2USB::state_wait_ack(uint8_t code) {
	if (code == ps2ind::ACK) {
		return state_t::base;
	}
	return state_t::wait_ack;
}

void
AX2USB::handle_fn_key(uint8_t usb, bool make_break) {
	if (make_break) {
		switch (usb) {
			case HID_KEY_PAUSE:
				usb_hid.sendReport8(REPORT_ID_SYS, SYSTEM_CONTROL_STANDBY);
				break;
			case HID_KEY_KANJI6:  // AX
				usb_hid.sendReport8(REPORT_ID_SYS, SYSTEM_CONTROL_POWER_OFF);
				break;
			case HID_KEY_KEYPAD_0:
				send_report16_oneshot(REPORT_ID_CONSUMER, AUDIO_CONTROL_MUTE);
				break;
			case HID_KEY_ARROW_DOWN:
			case HID_KEY_KEYPAD_2:
				if (!consumer_control_active) {
					send_report16(REPORT_ID_CONSUMER, AUDIO_CONTROL_VOLUME_DECREMENT);
					consumer_control_active = true;
				}
				break;
			case HID_KEY_ARROW_UP:
			case HID_KEY_KEYPAD_8:
				if (!consumer_control_active) {
					send_report16(REPORT_ID_CONSUMER, AUDIO_CONTROL_VOLUME_INCREMENT);
					consumer_control_active = true;
				}
				break;
			case HID_KEY_ARROW_LEFT:
			case HID_KEY_KEYPAD_4:
			case HID_KEY_PAGE_UP:
				send_report16_oneshot(REPORT_ID_CONSUMER, TRANSPORT_CONTROL_SCAN_PREVIOUS_TRACK);
				break;
			case HID_KEY_ARROW_RIGHT:
			case HID_KEY_KEYPAD_6:
			case HID_KEY_PAGE_DOWN:
				send_report16_oneshot(REPORT_ID_CONSUMER, TRANSPORT_CONTROL_SCAN_NEXT_TRACK);
				break;
			case HID_KEY_SPACE:
			case HID_KEY_KEYPAD_5:
				send_report16_oneshot(REPORT_ID_CONSUMER, TRANSPORT_CONTROL_PLAY_PAUSE);
				break;
			case HID_KEY_HOME:
			case HID_KEY_KEYPAD_7:
				send_report16_oneshot(REPORT_ID_CONSUMER, TRANSPORT_CONTROL_PLAY);
				break;
			case HID_KEY_END:
			case HID_KEY_KEYPAD_1:
				send_report16_oneshot(REPORT_ID_CONSUMER, TRANSPORT_CONTROL_PAUSE);
				break;
			default:
				// NOOP
				break;
		}
	} else {
		switch (usb) {
			case HID_KEY_ARROW_DOWN:
			case HID_KEY_KEYPAD_2:
			case HID_KEY_ARROW_UP:
			case HID_KEY_KEYPAD_8:
				if (consumer_control_active) {
					send_report16(REPORT_ID_CONSUMER, DO_NOTHING);
					consumer_control_active = false;
				}
				break;
			default:
				// NOOP
				break;
		}
	}
}

bool
AX2USB::handle_special_key(uint8_t usb, bool make_break) {
	if (usb == HID_KEY_CAPS_LOCK) {
		if (make_break) {
			if (usb_mod.l_shift || usb_mod.r_shift) {
				send_usb_key(HID_KEY_CAPS_LOCK, true);
				send_usb_key(HID_KEY_CAPS_LOCK, false);
				// send_usb_key_oneshot(HID_KEY_CAPS_LOCK);
				return true;
			} else {
				fn_flags.fn_left = true;
			}
		} else {
			if (fn_flags.fn_left) {
				fn_flags.fn_left = false;
			}
		}
		DEBUG_PRINTLN("%cLFn", mod_mark(make_break));
		return true;
	} else if (usb == HID_KEY_CONTROL_RIGHT) {
		DEBUG_PRINTLN("%cRFn", mod_mark(make_break));
		fn_flags.fn_right = make_break;
		return true;
	} else if (fn_flags.fn_left || fn_flags.fn_right) {
		handle_fn_key(usb, make_break);
		if (make_break) {
			return true;
		}
		// break 時は常に通常キーの break も処理する
		// 通常キー(make) → Fn(make) →通常キー(break) となった場合に通常キーのbreakを処理しないと通常キーが押されっぱなしになってしまう
		return false;
	} else if (!make_break) {
		// Fn(make) → ↑キー(make:ボリューム+) → Fn(break) → ↑キー(break) となった場合に備えて、Fnが押されてなくてもFnキーのbreak処理をする
		handle_fn_key(usb, make_break);
	}

	return false;
}

void
AX2USB::loop() {
	if (TinyUSBDevice.suspended()) {
		if (ps2_available()) {
			TinyUSBDevice.remoteWakeup();
		} else {
			delay(1000);
		}
		return;
	}
	// sending
	if (!usb_hid.ready()) {
		return;
	}
	if (state == state_t::base && should_send_led) {
		ps2.send(ps2cmd::MODE_IND);
		state = state_t::led_wait_ack;
		timeout_state_started = millis();
	}
	if (auto code = ps2_read(); code >= 0) {
		uint8_t k = code;
		DEBUG_PRINTLN("<%02x", k);
		state_t next_state;
		switch (state) {
			case state_t::led_wait_ack:
				next_state = state_led_wait_ack(k);
				break;
			case state_t::base:
				next_state = state_base(k);
				break;
			case state_t::brk_received:
				next_state = state_brk_received(k);
				break;
			case state_t::e0_received:
				next_state = state_e0_received(k);
				break;
			case state_t::e0_break_received:
				next_state = state_e0_break_received(k);
				break;
			case state_t::e1_received:
				next_state = state_e1_received(k);
				break;
			case state_t::e1_break_received:
				next_state = state_e1_break_received(k);
				break;
			case state_t::wait_ack:
				next_state = state_wait_ack(k);
				break;
			default:
				DEBUG_PRINTLN("%u: unhandled state, reverted to base", static_cast<uint8_t>(state));
				next_state = state_t::base;
				break;
		}
		state = next_state;
		if (is_timeout_state() && millis() - timeout_state_started > STATE_TIMEOUT_MSEC) {
			DEBUG_PRINTLN("ACK receive timeout, reverted to base");
			state = state_t::base;
		}
	}
}

void
AX2USB::handle_hid_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
	if (report_id != REPORT_ID_KBD || report_type != HID_REPORT_TYPE_OUTPUT) {
		return;
	}
	usb_led.value = buffer[0];
	DEBUG_PRINTLN("USB< %s", usb_led_str().c_str());
	if (update_ps2_led()) {
		should_send_led = true;
	}
}

bool
AX2USB::update_usb_codes(uint8_t code, bool make_break) {
	bool modified = false;
	if (make_break) {
		for (size_t i = 0; i < std::size(usb_codes); i++) {
			if (usb_codes[i] == code) {
				break;
			} else if (usb_codes[i] == 0) {
				usb_codes[i] = code;
				modified = true;
				break;
			}
		}
	} else {
		for (size_t i = 0; i < std::size(usb_codes); i++) {
			if (usb_codes[i] == code) {
				if (i == std::size(usb_codes) - 1 || usb_codes[i + 1] == 0) {
					usb_codes[i] = 0;
				} else {
					std::copy(&usb_codes[i + 1], std::end(usb_codes), &usb_codes[i]);
					usb_codes[std::size(usb_codes) - 1] = 0;
				}
				modified = true;
				break;
			}
		}
	}
	return modified;
}

bool
AX2USB::update_usb_modifier(uint8_t mask, bool make_break) {
	auto prev = usb_mod.value;
	if (make_break) {
		usb_mod.value |= mask;
	} else {
		usb_mod.value &= ~mask;
	}
	return usb_mod.value != prev;
}

bool
AX2USB::update_ps2_led() {
	auto prev = ps2_led.value;
	ps2_led.caps = usb_led.caps;
	ps2_led.num = usb_led.num;
	ps2_led.scroll = usb_led.scroll;
	ps2_led.kana = usb_led.kana;

	return ps2_led.value != prev;
}

const map::ax2usb_e0_ent_t*
AX2USB::find_e0_map(uint8_t scan_code) {
	for (size_t i = 0; i < std::size(map::ax2e0_usb); i++) {
		if (scan_code == map::ax2e0_usb[i].ps2) {
			return &map::ax2e0_usb[i];
		}
	}
	return nullptr;
}

void
AX2USB::hid_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
	AX2USB::theInstance->handle_hid_report(report_id, report_type, buffer, bufsize);
}

}  // namespace ax2usb
