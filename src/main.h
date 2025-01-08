
#include "ELECHOUSE_CC1101_SRC_DRV.h"
#include "modules/RF/CC1101.h"
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


extern CC1101_CLASS CC1101;
//extern XPT2046_Bitbang touchscreen;
extern decode_results lastResults;
//extern SPIClass SPI;
