#pragma once
#include <Adafruit_TinyUSB.h>
#include <string>

namespace hid_util {

class HidUtil {
 public:
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
	};
	usb_mod_t usb_mod = { 0 };

	HidUtil(Adafruit_USBD_HID& usb_hid, uint8_t report_id_kbd) : usb_hid(usb_hid), report_id_kbd(report_id_kbd) {}

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
	bool update_usb_codes(uint8_t code, bool make_break);
	bool update_usb_modifier(uint8_t mask, bool make_break);

 private:
	Adafruit_USBD_HID& usb_hid;
	uint8_t usb_codes[6]{};
	const uint8_t report_id_kbd;

	std::string usb_codes_str() const;
};

}  // namespace hid_util
