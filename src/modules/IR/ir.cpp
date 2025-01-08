#include <IRremoteESP8266.h>
#include <IRsend.h>
#include "ir.h"
#include "main.h"

// Define the IR transmitter pin
#define IR_TX_PIN IR_TX
IRsend irsend(IR_TX_PIN); // IRsend object for transmitting IR signals
IRState IRCurrentState;
decode_results results;  


   

void sendReceived() {

        irsend.begin();
        irsend.send(lastResults.decode_type, lastResults.value, lastResults.bits);


}

bool txIrFile(File32 *fs, String filepath) {
  // SPAM all codes of the file

  int total_codes = 0;
  String line;
  
  File32 databaseFile = fs->open(filepath, FILE_READ);

  pinMode(IR_TX, OUTPUT);
  //digitalWrite(IrTx, LED_ON);

  if (!databaseFile) {
    Serial.println("Failed to open database file.");
    displayError("Fail to open file");
    delay(2000);
    return false;
  }
  Serial.println("Opened database file.");
  
  bool endingEarly;
  int codes_sent=0;
  uint16_t frequency = 0;
  String rawData = "";
  String protocol = "";
  String address = "";
  String command = "";
  String value = "";
  String bits = "32";
    
  databaseFile.seek(0); // comes back to first position
  
  // count the number of codes to replay
  while (databaseFile.available()) {
    line = databaseFile.readStringUntil('\n');
    if(line.startsWith("type:")) total_codes++;
  }

  Serial.printf("\nStarted SPAM all codes with: %d codes", total_codes);
  // comes back to first position, beggining of the file
  databaseFile.seek(0);
  while (databaseFile.available()) {
    progressHandler(codes_sent,total_codes);
    line = databaseFile.readStringUntil('\n');
    if (line.endsWith("\r")) line.remove(line.length() - 1);

    if (line.startsWith("type:")) {
      codes_sent++;
      String type = line.substring(5);
      type.trim();
      Serial.println("Type: "+type);
      if (type == "raw") {
        Serial.println("RAW code");
        while (databaseFile.available()) {
          line = databaseFile.readStringUntil('\n');
          if (line.endsWith("\r")) line.remove(line.length() - 1);

          if (line.startsWith("frequency:")) {
            line = line.substring(10);
            line.trim();
            frequency = line.toInt();
            Serial.println("Frequency: " + String(frequency));
          } else if (line.startsWith("data:")) {
            rawData = line.substring(5);
            rawData.trim();
            Serial.println("RawData: "+rawData);
          } else if ((frequency != 0 && rawData != "") || line.startsWith("#")) {
            sendRawCommand(frequency, rawData);
            rawData = "";
            frequency = 0;
            type = "";
            line = "";
            break;
          }
        }
      } else if (type == "parsed") {
        Serial.println("PARSED");
        while (databaseFile.available()) {
          line = databaseFile.readStringUntil('\n');
          if (line.endsWith("\r")) line.remove(line.length() - 1);

          if (line.startsWith("protocol:")) {
            protocol = line.substring(9);
            protocol.trim();
            Serial.println("Protocol: "+protocol);
          } else if (line.startsWith("address:")) {
            address = line.substring(8);
            address.trim();
            Serial.println("Address: "+address);
          } else if (line.startsWith("command:")) {
            command = line.substring(8);
            command.trim();
            Serial.println("Command: "+command);
          } else if (line.startsWith("value:") || line.startsWith("state:")) {
            value = line.substring(6);
            value.trim();
            Serial.println("Value: "+value);
          } else if (line.startsWith("bits:")) {
            bits = line.substring(strlen("bits:"));
            bits.trim();
            Serial.println("bits: "+bits);
          } else if (line.indexOf("#") != -1) {  // TODO: also detect EOF
            if (protocol.startsWith("NEC")) {
              sendNECCommand(address, command);
            } else if (protocol.startsWith("RC5")) {
              sendRC5Command(address, command);
            } else if (protocol.startsWith("RC6")) {
              sendRC6Command(address, command);
            } else if (protocol.startsWith("Samsung")) {
              sendSamsungCommand(address, command);
            } else if (protocol.startsWith("SIRC")) {
              sendSonyCommand(address, command);
            //} else if (protocol.startsWith("Panasonic")) {
            //  sendPanasonicCommand(address, command);
            } else if (protocol!="" && value!="") {
              sendDecodedCommand(protocol, value, bits);
            }
            protocol = "";
            address = "";
            command = "";
            protocol = "";
            value = "";
            type = "";
            line = "";
            break;
          }
        }
      }
    }
    // if user is pushing (holding down) TRIGGER button, stop transmission early
    if (checkSelPress()) // Pause TV-B-Gone
    {
      while (checkSelPress()) yield();
      displayRedStripe("Paused", TFT_WHITE, BGCOLOR);

      while (!checkSelPress()){ // If Presses Select again, continues
        if(checkEscPress()) {
          endingEarly= true;
          break;
        }
      }
      while (checkSelPress()){
        yield();
      }
      if (endingEarly) break; // Cancels  custom IR Spam
      displayRedStripe("Running, Wait", TFT_WHITE, FGCOLOR);
    }
  } // end while file has lines to process
  databaseFile.close();
  Serial.println("closed");
  Serial.println("EXTRA finished");

  resetCodesArray();
  digitalWrite(IrTx, LED_OFF);
  return true;
}


