#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <DS3231.h>
#include <EEPROM.h>
DS3231 RTC;
RF24 radio(7, 8);  // CE, CSN
const byte address[6] = "122222";
int valv[4][3];
int oldsec = 0;
bool h12 = false;
bool hPM = false;
bool states[4];
long unsigned int timers[4];
bool overwrite = false;
void setup() {
  Wire.begin();
  Serial.begin(9600);
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  radio.openWritingPipe(address);
  radio.stopListening();
  setradio(true);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  for (int i = 0; i < 4; i++) {
    states[i] = false;
    timers[i] = 0;
  }
  bool mode12 = false; // use 12-hour clock mode


}

void loop() {
  if(!overwrite)checker();
  if (oldsec != RTC.getSecond()) {
    oldsec = RTC.getSecond();

    setradio(false);
    String time = righttime(int(RTC.getHour(h12, hPM)), int(RTC.getMinute()), int(RTC.getSecond()));
    char sender[8] = "        ";
    for (int i = 0; i < 8; i++) sender[i] = time.charAt(i);
    radio.write(&sender, sizeof(sender));
    setradio(true);
  }
  char request[3] = "";
  if (radio.available()) {
    setradio(true);
    char c[3] = "   ";
    radio.read(&c, sizeof(c));
    Serial.println(c);
    if (c[0] == '1') {
      for (int i = 0; i < 4; i++) {
        for (int x = 0; x < 3; x++) {
          while (!radio.available()) {}
          radio.read(&c, sizeof(c));
          String converted = "";
          converted.concat(c[0]);
          converted.concat(c[1]);
          converted.concat(c[2]);
          valv[i][x] = converted.toInt();
    
        }
  
      }
      int y = 0;
      for (int i = 0; i < 4; i++) {
        for (int x = 0; x < 3; x++) {

          Serial.print(valv[i][x]);
          EEPROM.write(y, valv[i][x]);
          y++;
        }
      }

    } else if (c[0] == '0') {
      overwrite=true;
      while (!radio.available()) {}

      if (radio.available()) {
        char text[6] = "";
        radio.read(&text, sizeof(text));
        String str = "";
        str.concat(text[0]);
        char ch = str.charAt(0);  
        int nvalv = ch - '0';
        bool state = false;
        if (text[1] == '1') state = true;
        Serial.print("Valvola ");
        Serial.print(nvalv);
        Serial.print(": ");
        Serial.println(state);
        digitalWrite(nvalv + 2, !state);
        //   Serial.println(str2);
      }
    }
    else if(c[0]=='2') overwrite=false;
  }
}
void setradio(bool r) {
  if (r) {
    radio.openReadingPipe(0, address);
    radio.startListening();
  } else {
    radio.stopListening();
  }
}

String righttime(int hh, int mm, int ss) {
  String str = "";

  if (hh < 10) str = str + "0";
  str.concat(hh);

  str.concat(":");
  if (mm < 10) str = str + "0";
  str.concat(mm);

  str.concat(":");
  if (ss < 10) str = str + "0";
  str.concat(ss);

  return str;
}
void checker() {
  int y = 0;
  int totalm = 0;
  int realm = 0;
  for (int i = 0; i < 4; i++) {
    totalm = EEPROM.read(y) * 60 + EEPROM.read(y + 1);
    realm = RTC.getHour(h12, hPM) * 60 + RTC.getMinute();
   //Serial.println("total:\treal\tdur\t ");
  
    if (realm >= totalm && realm < totalm + EEPROM.read(y + 2) && EEPROM.read(y+2)>0) states[i]=false;
    else states[i]=true;
    y += 3;

  } 

  for (int i = 0; i < 4; i++) {
    digitalWrite(i + 2, states[i]);
  }
}
