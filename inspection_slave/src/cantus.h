#pragma once

#ifdef __GNUG__ // The GNU C++ compiler defines this. Testing it is equivalent to testing (__GNUC__ && __cplusplus). 
extern "C"{
#endif
#include "typedef.h"
#include "cantus/cache.h"	
#include "cantus/gdma.h"
#include "cantus/gpio.h"
#include "cantus/i2s.h"
#include "cantus/interrupt.h"
#include "cantus/keyscan.h"
#include "cantus/nandctrl.h"
#include "cantus/norflash.h"
#include "cantus/pmu.h"
#include "cantus/paf.h"
#include "cantus/rtc.h"
#include "cantus/spi.h"
#include "cantus/sdcard.h"
#include "cantus/twi.h"
#include "cantus/timer.h"	
#include "cantus/uart.h"
#include "cantus/usb.h"
#include "cantus/voice_codec.h"
#include "fatfs/ff.h"
#include "command.h"
#ifdef __GNUG__
}
#endif
