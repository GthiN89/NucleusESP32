
#include "ELECHOUSE_CC1101_SRC_DRV.h"
#include "SPI.h"
//#include "XPT2046_Bitbang.h"
#include "IRrecv.h"
#include <assert.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRtext.h>
#include <IRutils.h>
#include <Wire.h>

#include "GUI/logo.h"
#include <XPT2046_Bitbang.h>
#include "GUI/ScreenManager.h"
#include <esp32_smartdisplay/src/esp32_smartdisplay.h>
#include "GUI/events.h"

#include "modules/ETC/SDcard.h"
#include <FFat.h>
#include "lv_fs_if.h"
#include "modules/dataProcessing/SubGHzParser.h"
#include "modules/ir/TV-B-Gone.h"
#include "modules/ir/WORLD_IR_CODES.h"
#include "modules/IR/ir.h"
#include <SPI.h>
#include <IRsend.h>
#include "globals.h"

//extern XPT2046_Bitbang touchscreen;
extern decode_results lastResults;
//extern SPIClass SPI;
