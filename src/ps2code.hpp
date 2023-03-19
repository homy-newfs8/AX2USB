#pragma once
#include <cstdint>

namespace ax2usb {

namespace ps2cmd {

constexpr inline uint8_t MODE_IND = 0xed;
constexpr inline uint8_t ECHO = 0xee;
constexpr inline uint8_t SELECT_CODE_SET = 0xf0;
constexpr inline uint8_t READ_ID = 0xf2;
constexpr inline uint8_t SET_TYPEMATIC = 0xf3;
constexpr inline uint8_t ENABLE = 0xf4;
constexpr inline uint8_t DEFAULT_DISABLE = 0xf5;
constexpr inline uint8_t DEFAULT = 0xf5;
constexpr inline uint8_t RESEND = 0xfe;
constexpr inline uint8_t RESET = 0xff;

}  // namespace ps2cmd

namespace ps2ind {

constexpr inline uint8_t OVERRUN = 0x00;
constexpr inline uint8_t BAT_COMPLETED = 0xaa;
constexpr inline uint8_t E0 = 0xe0;
constexpr inline uint8_t E1 = 0xe1;
constexpr inline uint8_t ECHO_RESPONSE = 0xee;
constexpr inline uint8_t BREAK = 0xf0;
constexpr inline uint8_t ACK = 0xfa;
constexpr inline uint8_t BAT_FAILED = 0xfc;
constexpr inline uint8_t DIAG_FAILURE = 0xfd;
constexpr inline uint8_t RESEND = 0xfe;

}  // namespace ps2ind

namespace ps2key {

constexpr inline uint8_t L_SHIFT = 0x12;
constexpr inline uint8_t L_CTRL = 0x14;
constexpr inline uint8_t AX_MUHENKAN = 0x17;
constexpr inline uint8_t AX_HENKAN = 0x1f;
constexpr inline uint8_t AX = 0x27;
constexpr inline uint8_t CAPS = 0x58;
constexpr inline uint8_t R_SHIFT = 0x59;
constexpr inline uint8_t PAUSE = 0x77;
constexpr inline uint8_t BREAK = 0x7e;
constexpr inline uint8_t ALT_PRINT_SCREEN = 0x84;

}  // namespace ps2key

}  // namespace ax2usb
