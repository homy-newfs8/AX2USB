#pragma once
#include <Adafruit_TinyUSB.h>
#include <libps2.h>
#include <sq.hpp>
#include <string>
#include "ax2usbmap.hpp"
#include "mutex.hpp"

namespace ax2usb {

using namespace libps2;

class AX2USB {
 public:
	AX2USB() { theInstance = this; }
	bool begin(uint8_t ps2_data_pin, uint8_t ps2_clock_pin);
	void loop();

 private:
	enum state_t { base, brk_received, e0_received, e1_received, led_wait_ack, e0_break_received, e1_break_received, wait_ack };
	union __attribute__((packed)) usb_mod_t {
		struct __attribute__((packed)) {
			bool l_ctrl : 1;
			bool l_shift : 1;
			bool l_alt : 1;
			bool l_gui : 1;
			bool r_ctrl : 1;
			bool r_shift : 1;
			bool r_alt : 1;
			bool r_gui : 1;
		};
		uint8_t value;
	} usb_mod = { 0 };
	union __attribute__((packed)) usb_led_t {
		struct __attribute__((packed)) {
			bool num : 1;
			bool caps : 1;
			bool scroll : 1;
			bool compose : 1;
			bool kana : 1;
		};
		uint8_t value;
	} usb_led = { 0 };
	union __attribute__((packed)) ps2_led_t {
		struct __attribute__((packed)) {
			bool scroll : 1;
			bool num : 1;
			bool caps : 1;
			bool kana : 1;
		};
		uint8_t value;
	} ps2_led = { 0 };
	union __attribute__((packed)) fn_flags_t {
		struct __attribute__((packed)) {
			bool fn_left;
			bool fn_right;
		};
		uint8_t value;
	} fn_flags = { 0 };

	PS2 ps2;
	Adafruit_USBD_HID usb_hid;
	uint32_t timeout_state_started = 0;
	state_t state = state_t::base;
	uint8_t usb_codes[6]{};
	bool should_send_led = false;
	bool caps_sent = false;
	mutable Mutex rx_mux;
	SQ<uint8_t, 10> rx;
	bool consumer_control_active = false;

	// PS/2 読み出し
	bool ps2_available() const;
	int ps2_read();

	void handle_hid_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize);
	bool update_usb_codes(uint8_t code, bool make_break);
	bool update_usb_modifier(uint8_t mask, bool make_break);
	bool update_ps2_led();
	std::string usb_codes_str() const;
	std::string usb_led_str() const;

	/* 入力処理状態関数群 */
	state_t state_base(uint8_t ps2);
	state_t state_brk_received(uint8_t ps2);
	state_t state_e0_received(uint8_t ps2);
	state_t state_e1_received(uint8_t ps2);
	state_t state_led_wait_ack(uint8_t ps2);
	state_t state_e0_break_received(uint8_t ps2);
	state_t state_e1_break_received(uint8_t ps2);
	state_t state_wait_ack(uint8_t ps2);
	bool is_timeout_state() { return state == state_t::led_wait_ack || state == state_t::wait_ack; }

	/**
	 * @brief Fn+キーを処理する
	 *
	 * @param usb Fnと同時に押されたキーのUSB_HIDキーコード
	 * @param make_break
	 */
	void handle_fn_key(uint8_t usb, bool make_break);
	/**
	 * @brief 特殊キー(Caps/英数カナ Fn+キー)の処理を行う
	 *
	 * @param usb USB_HIDキーコード
	 * @param make_break make処理ならtrue、break処理ならfalse
	 * @return true 処理完了。
	 * @return false 処理しなかった or 通常キーとして処理する必要あり
	 */
	bool handle_special_key(uint8_t usb, bool make_break);

	/**
	 * @brief USBへキーを送信する
	 *
	 * @param usb USB_HIDキーコード
	 * @param make_break make or break
	 */
	void send_usb_key(uint8_t usb, bool make_break);
	/**
	 * @brief USBへキーの make,break を送信する
	 *
	 * @param usb USB_HIDキーコード
	 */
	void send_usb_key_oneshot(uint8_t usb);
	/**
	 * @brief USBへキーとモディファイアをまとめて送信する
	 *
	 * @param usb キーのUSB_HIDキーコード
	 * @param usb_mod_key モディファイアのUSB_HIDキーコード
	 * @param make_break make or break
	 */
	void send_usb_key_mod(uint8_t usb, uint8_t usb_mod_key, bool make_break);
	/**
	 * @brief トグルスイッチ用のコード送信
	 *
	 * @param report_id
	 * @param usage
	 */
	void send_report16_oneshot(uint8_t report_id, uint16_t usage);
	/**
	 * @brief リトライ付きキーボードコードの送信
	 */
	void send_keyboard_report();
	/**
	 * @brief リトライ付き16ビットレポートの送信
	 */
	void send_report16(uint8_t report_id, uint16_t usage);
	/**
	 * @brief USBが送信可能になるまでビジーウェイトする
	 */
	void wait_usb_ready();

	static const map::ax2usb_e0_ent_t* find_e0_map(uint8_t scan_code);
	static void hid_report_callback(uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize);
	static std::string usb_mods_str(const usb_mod_t& mod);
	static char usb_mod_char(uint8_t mod_key);
	static inline AX2USB* theInstance;
};

}  // namespace ax2usb
