#pragma once
#include <class/hid/hid.h>  // from Adafruit TinyUSB
#include <cstdint>

namespace ax2usb::map {

// clang-format off
constexpr inline uint8_t ax2_usb[] = {
	//       00                  01                  02                  03                  04                  05                  06                  07
	//       08                  09                  0a                  0b                  0c                  0d                  0e                  0f
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	//                           F9                                      F5                  F3                  F1                  F2                  F12
	/* 00 */ 0x00,               HID_KEY_F9,         0x00,               HID_KEY_F5,         HID_KEY_F3,         HID_KEY_F1,         HID_KEY_F2,         HID_KEY_F12,
	//                           F10                 F8                  F6                  F4                  TAB                 `~
	/* 08 */ 0x00,               HID_KEY_F10,        HID_KEY_F8,         HID_KEY_F6,         HID_KEY_F4,         HID_KEY_TAB,        HID_KEY_GRAVE,      0x00,
	//                           LAlt                Shift               ひらがなカタカナ(JP)LCtrl               Q                   1!                  無変換(AX)
	/* 10 */ 0x00,               HID_KEY_ALT_LEFT,   HID_KEY_SHIFT_LEFT, HID_KEY_KANJI2,     HID_KEY_CONTROL_LEFT, HID_KEY_Q,        HID_KEY_1,          HID_KEY_GUI_LEFT,
	//                                               Z                   S                   A                   W                   2@                  変換(AX)
	/* 18 */ 0x00,               0x00,               HID_KEY_Z,          HID_KEY_S,          HID_KEY_A,          HID_KEY_W,          HID_KEY_2,          HID_KEY_GUI_RIGHT,
	//                           C                   X                   D                   E                   4$                  3#                  AX
	/* 20 */ 0x00,               HID_KEY_C,          HID_KEY_X,          HID_KEY_D,          HID_KEY_E,          HID_KEY_4,          HID_KEY_3,          HID_KEY_KANJI6,
	//                           SPC                 V                   F                   T                   R                   5%
	/* 28 */ 0x00,               HID_KEY_SPACE,      HID_KEY_V,          HID_KEY_F,          HID_KEY_T,          HID_KEY_R,          HID_KEY_5,          0x00,
	//                           N                   B                   H                   G                   Y                   6^
	/* 30 */ 0x00,               HID_KEY_N,          HID_KEY_B,          HID_KEY_H,          HID_KEY_G,          HID_KEY_Y,          HID_KEY_6,          0x00,
	//                                               M                   J                   U                   7&                  8*
	/* 38 */ 0x00,               0x00,               HID_KEY_M,          HID_KEY_J,          HID_KEY_U,          HID_KEY_7,          HID_KEY_8,          0x00,
	//                           ,<                  K                   I                   O                   0)                  9)
	/* 40 */ 0x00,               HID_KEY_COMMA,      HID_KEY_K,          HID_KEY_I,          HID_KEY_O,          HID_KEY_0,          HID_KEY_9,          0x00,
	//                           .>                  /?                  L                   ;:                  P                   -_
	/* 48 */ 0x00,               HID_KEY_PERIOD,     HID_KEY_SLASH,      HID_KEY_L,          HID_KEY_SEMICOLON,  HID_KEY_P,          HID_KEY_MINUS,      0x00,
	//                           \|                  '"                                      [{                  =+
	/* 50 */ 0x00,               HID_KEY_KANJI1,     HID_KEY_APOSTROPHE, 0x00,               HID_KEY_BRACKET_LEFT, HID_KEY_EQUAL,    0x00,               0x00,
	//       Caps                RShift              Enter               ]}                                      \|
	/* 58 */ HID_KEY_CAPS_LOCK,  HID_KEY_SHIFT_RIGHT,HID_KEY_ENTER,      HID_KEY_BRACKET_RIGHT, 0x00,            HID_KEY_BACKSLASH,  0x00,               0x00,
	//                           \|(下)                                                      変換(JP)                                BS                  無変換(JP)
	/* 60 */ 0x00,               HID_KEY_EUROPE_2,   0x00,               0x00,               HID_KEY_KANJI4,     0x00,               HID_KEY_BACKSPACE,  HID_KEY_KANJI5,
	//                           1End                yen|                4←                 7Home
	/* 68 */ 0x00,               HID_KEY_KEYPAD_1,   HID_KEY_KANJI3,     HID_KEY_KEYPAD_4,   HID_KEY_KEYPAD_7,   0x00,               0x00,               0x00,
	//       0                   Del.                2↓                 5                   6→                 8↑                 Esc                 Num
	/* 70 */ HID_KEY_KEYPAD_0,   HID_KEY_KEYPAD_DECIMAL, HID_KEY_KEYPAD_2, HID_KEY_KEYPAD_5, HID_KEY_KEYPAD_6,   HID_KEY_KEYPAD_8,   HID_KEY_ESCAPE,     HID_KEY_NUM_LOCK,
	//       F11                 +                   3PgDn               -                   *                   9PgUp               ScrLk
	/* 78 */ HID_KEY_F11,        HID_KEY_KEYPAD_ADD, HID_KEY_KEYPAD_3,   HID_KEY_KEYPAD_SUBTRACT, HID_KEY_KEYPAD_MULTIPLY, HID_KEY_KEYPAD_9, HID_KEY_SCROLL_LOCK, 0x00,
	//                                                                   F7
  /* 80 */ 0x00,               0x00,               0x00,               HID_KEY_F7
};
// clang-format on

struct ax2usb_e0_ent_t {
	uint8_t ps2;
	uint8_t usb;
};

// E0 12(L_SFT)... と来たときは別途特別に扱うこと
constexpr inline ax2usb_e0_ent_t ax2e0_usb[] = {
	{ 0x11, HID_KEY_ALT_RIGHT },     { 0x14, HID_KEY_CONTROL_RIGHT }, { 0x5a, HID_KEY_KEYPAD_ENTER }, { 0x1f, HID_KEY_GUI_LEFT },
	{ 0x27, HID_KEY_GUI_RIGHT },     { 0x2f, HID_KEY_APPLICATION },   { 0x6c, HID_KEY_HOME },         { 0x75, HID_KEY_ARROW_UP },
	{ 0x7d, HID_KEY_PAGE_UP },       { 0x6b, HID_KEY_ARROW_LEFT },    { 0x74, HID_KEY_ARROW_RIGHT },  { 0x69, HID_KEY_END },
	{ 0x72, HID_KEY_ARROW_DOWN },    { 0x7a, HID_KEY_PAGE_DOWN },     { 0x70, HID_KEY_INSERT },       { 0x71, HID_KEY_DELETE },
	{ 0x4a, HID_KEY_KEYPAD_DIVIDE }, { 0x7c, HID_KEY_PRINT_SCREEN }
};

// E1 で始まるのは PAUSE

// https://bsakatu.net/doc/scancode/ の *1,*2,*3,*4 を参照

}  // namespace ax2usb::map
