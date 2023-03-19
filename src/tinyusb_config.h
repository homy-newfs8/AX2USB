#pragma once
#ifdef ARDUINO_ARCH_RP2040
#include <arduino/ports/rp2040/tusb_config_rp2040.h>
#endif

#undef CFG_TUD_MSC
#define CFG_TUD_MSC 0
#undef CFG_TUD_MIDI
#define CFG_TUD_MIDI 0
#undef CFG_TUD_VENDOR
#define CFG_TUD_VENDOR 0
