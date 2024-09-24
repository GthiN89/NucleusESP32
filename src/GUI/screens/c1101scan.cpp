#include "screens.h"
//#include "modules/RF/subGhz.h"
#include <lvgl.h>
#include <iostream>
#include <cstring>
#include "playZero.h"
#include "c1101scan.h"
#include "GUI/screens/replay.h"

#include "../events.h"
#include "../menus/RFmenu.h"



lv_obj_t* C1101scanCont = nullptr;
lv_obj_t* c1101scanScreen = nullptr;
lv_obj_t* scanLabel = nullptr;
lv_obj_t* txtProtscanReceived;
lv_obj_t* txtProtscanBitLength;
lv_obj_t* txtProtscanBinary;
lv_obj_t* txtProtscanPulsLen;
lv_obj_t* txtProtscanTriState;
lv_obj_t* txtProtscanProtocol;
lv_obj_t* txtProtscanResults;



void create1101scanScreen();
void play_scanlyzer_btn_event_cb(lv_event_t* e);
void CC1101Scanloop();
void output(unsigned long decimal, unsigned int length, unsigned int pktdelay, unsigned int* raw, unsigned int protocol);
void fcnProtscanCancel(lv_event_t * e);
void fcnProtscanRxEn(lv_event_t * e);
void fcnProtscanRxOff(lv_event_t * e);
void fcnProtscanClear(lv_event_t * e);


// CC1101 - RCSW Apps Stuff

int ProtscanRxEn=0;






void create1101scanScreen() {
    // Serial.println("Initializing c1101 screen...");

    // c1101scanScreen = lv_obj_create(lv_scr_act());
    // lv_obj_set_size(c1101scanScreen, 240, 320);
    // lv_obj_set_flex_flow(c1101scanScreen, LV_FLEX_FLOW_COLUMN);
    // lv_obj_set_flex_align(c1101scanScreen, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    // lv_scr_load(c1101scanScreen);

    // Serial.println("Created c1101 screen");

    // C1101scanCont = lv_textarea_create(c1101scanScreen);
    // lv_obj_set_size(C1101scanCont, 215, 240);
    // lv_obj_set_flex_flow(C1101scanCont, LV_FLEX_FLOW_COLUMN);

    // scanLabel = lv_label_create(c1101scanScreen);
    // lv_label_set_text(scanLabel, "SubGhz scanlyzer");
    // lv_obj_set_style_align(scanLabel, LV_ALIGN_LEFT_MID, 0);

    // lv_obj_t* txtProtscanReceived = lv_textarea_create(C1101scanCont);
    // lv_obj_set_size(txtProtscanReceived, 180, 180);  // Adjust size as needed
    // lv_textarea_set_text(txtProtscanReceived, "");

    // lv_obj_t* scan_button_container = lv_obj_create(lv_scr_act());
    // lv_obj_set_size(scan_button_container, 230, 30);
    // lv_obj_set_flex_flow(scan_button_container, LV_FLEX_FLOW_ROW);
    // lv_obj_set_flex_align(scan_button_container, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // lv_obj_t* rxONBTN = lv_btn_create(scan_button_container);
    // lv_obj_add_event_cb(rxONBTN, fcnProtscanRxEn, LV_EVENT_CLICKED, nullptr);
    // lv_obj_set_size(rxONBTN, 40, 20);
    // lv_obj_t* rxONBTN_label = lv_label_create(rxONBTN);
    // lv_label_set_text(rxONBTN_label, "RX-ON");

    // lv_obj_t* rxOFFBTN = lv_btn_create(scan_button_container);
    // lv_obj_set_size(rxOFFBTN, 40, 20);
    // lv_obj_add_event_cb(rxOFFBTN, fcnProtscanRxOff, LV_EVENT_CLICKED, nullptr);
    // lv_obj_t* rxOFFBTN_label = lv_label_create(rxOFFBTN);
    // lv_label_set_text(rxOFFBTN_label, "RX-OFF");

    // lv_obj_t* clsBTN = lv_btn_create(scan_button_container);
    // lv_obj_set_size(clsBTN, 40, 20);
    // lv_obj_add_event_cb(clsBTN, fcnProtscanClear, LV_EVENT_CLICKED, nullptr);
    // lv_obj_t* clsBTN_label = lv_label_create(clsBTN);
    // lv_label_set_text(clsBTN_label, "CLS");

    // lv_obj_t* back_btn = lv_btn_create(scan_button_container);
    // lv_obj_set_size(back_btn, 40, 20);
    // lv_obj_add_event_cb(back_btn, btn_event_subGhzTools, LV_EVENT_CLICKED, NULL);
    // lv_obj_t* back_label = lv_label_create(back_btn);
    // lv_label_set_text(back_label, LV_SYMBOL_LEFT "Exit");
}

void fcnProtscanClear(lv_event_t * e)
{
	// Your code here
//   lv_textarea_set_text(txtProtscanReceived,"-"); // Binary
//   lv_textarea_set_text(txtProtscanReceived,"-"); //Pulse Length
//   lv_textarea_set_text(txtProtscanReceived,"-"); //TriState
//   lv_textarea_set_text(txtProtscanReceived,"-"); //Protocol
//   lv_textarea_set_text(txtProtscanReceived,"");
//   lv_textarea_set_text(txtProtscanReceived,"-");
//   lv_textarea_set_text(txtProtscanReceived,"");
}

void fcnProtscanCancel(lv_event_t * e)
{
//     Serial.println("scanCancel");
//   lv_event_code_t code = lv_event_get_code(e);
//     if (code == LV_EVENT_CLICKED) {
//         ProtscanRxEn=0;
//       //  RFMenu();
//     }
}



void fcnProtscanRxEn(lv_event_t * e)
{
//     Serial.println("scanEnable");
//     lv_event_code_t code = lv_event_get_code(e);
//     if (code == LV_EVENT_CLICKED) {
//          ProtscanRxEn=1;
//          CC1101_MHZ=433.92;
//          Serial.println("setfreq");
//         initCC1101();
//    //     mySwitch.enableReceive(CCGDO2A);  // Receiver on
//        lv_label_set_text(scanLabel,"RX ON");
//        Serial.println("rxOn");
//        ELECHOUSE_cc1101.SetRx(CC1101_MHZ);
//        Serial.println("setrx");
//        CC1101_TX=false;
//        CC1101ScanEn=1;
//     }
}

void fcnProtscanRxOff(lv_event_t * e)
{
    // Serial.println("scanOff");
	// lv_event_code_t code = lv_event_get_code(e);
    // if (code == LV_EVENT_CLICKED) {
    //     lv_label_set_text(scanLabel,"RX OFF");
    //     ELECHOUSE_cc1101.SetTx(CC1101_MHZ);
    //     CC1101_TX=true;
    //     ProtscanRxEn=0;  
    // }
}






void CC1101Scanloop() {
//   mySwitch.enableReceive(CCGDO2A);
//   Serial.println("scanloopenabled");
//  // lv_textarea_set_cursor_click_pos(ui_txtScannerData, false);
//  // int rxBW = lv_arc_get_value(ui_arcScanBW);
//   ELECHOUSE_cc1101.setRxBW(58); //orig = 58
//   ELECHOUSE_cc1101.SetRx(freq);
// Serial.println("2");
//   ELECHOUSE_cc1101.setMHZ(freq);
//   Serial.println("3");
//   rssi = ELECHOUSE_cc1101.getRssi();
//   Serial.println("4");

//   // Serial.print( rssi);
//   // Serial.print( ",");

//   if (rssi > -200)
//   {
//     Serial.println("if1");
//     if (rssi > mark_rssi)
//     {
//         Serial.println("if2");
//       mark_rssi = rssi;
//       mark_freq = freq;
//     }
//   }

//   freq += 0.01;

//   if (freq > stop_freq)
//   {
//     Serial.println("if3");
//     //lv_textarea_add_text(ui_txtScannerData,".");
//     //Serial.print(".");
//     freq = start_freq;
    
//     int thVal = 40;
    
//     //map(threshVal,-40,-80,40,80);
//     if (mark_rssi > thVal)
//     {
// Serial.println("if 5");
//       long fr = mark_freq * 100;

//       if (fr == compare_freq)
//       {
//         Serial.println("if 6");
//        // lv_textarea_add_text(ui_txtScannerData," ");
//         //Serial.println();
//       //  lv_textarea_add_text(ui_txtScannerData,"Freq: ");
//     ///    lv_textarea_add_text(ui_txtScannerData,String(mark_freq).c_str());
//       //  lv_textarea_add_text(ui_txtScannerData," Rssi: ");
//    //     lv_textarea_add_text(ui_txtScannerData,String(mark_rssi).c_str());
//         //lv_obj_t * ui_txtScannerData;
//         char test_msg_buf[] = "\n";
//     //    lv_textarea_add_text(ui_txtScannerData, test_msg_buf);
//         mark_rssi = -100;
//         compare_freq = 0;
//         mark_freq = 0;
//       }
//       else
//       {
//         Serial.println("else");
//         compare_freq = mark_freq * 100;
//         freq = mark_freq - 0.10;
//         mark_freq = 0;
//         mark_rssi = -100;
//       }
//     }
//   }
}