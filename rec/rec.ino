#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <DS3231.h>
#include <EEPROM.h>
DS3231 RTC;
#define v1 2
#define v2 3
#define v3 4
#define v4 5
#define power1 9   //d2, d3
#define power2 10  //d4, d5
const int del = 100;
RF24 radio(7, 8);  // CE, CSN
const byte address[6] = "122222";
int valv[4][3];
int oldsec = 0;
bool h12 = false;
bool hPM = false;
bool states[4];
bool oldstates[4];
long unsigned int timers[4];
bool overwrite = false;
void setup() {

  pinMode(v1, OUTPUT);
  pinMode(v2, OUTPUT);
  pinMode(v3, OUTPUT);
  pinMode(v4, OUTPUT);
  pinMode(power1, OUTPUT);
  pinMode(power2, OUTPUT);
  

  digitalWrite(v1, true);
  digitalWrite(v2, true);
  digitalWrite(v3, true);
  digitalWrite(v4, true);
  digitalWrite(power1, false);
  digitalWrite(power2, false);

changeState(true, true, true, true);
changeState(false, false, false, false);

  Wire.begin();
  Serial.begin(9600);
  radio.begin();
  radio.setPALevel(RF24_PA_MIN);
  radio.openWritingPipe(address);
  radio.stopListening();
  setradio(true);


  
  for (int i = 0; i < 4; i++) {
    states[i] = false;
    oldstates[i] = false;
    timers[i] = 0;
  }

  bool mode12 = false;  // use 12-hour clock mode

}

void loop() {
  if (!overwrite) checker();
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
    if (c[0] == '1') {  //config
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

    } else if (c[0] == '0') {  //manual
      overwrite = true;
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
        // digitalWrite(nvalv + 2, !state);
        states[nvalv] = !state;
        sendvalv();
      }
    } else if (c[0] == '2') overwrite = false;
    else if (c[0] == '3') {
      while (!radio.available()) {}
      char hhmm[] = "     ";
      radio.read(&hhmm, sizeof(hhmm));
      String strhh = "";
      String strmm = "";
      strhh.concat(hhmm[0]);
      strhh.concat(hhmm[1]);
      strmm.concat(hhmm[2]);
      strmm.concat(hhmm[3]);
      int hh = strhh.toInt();
      int mm = strmm.toInt();
      RTC.setMinute(mm);
      RTC.setHour(hh);
      RTC.setSecond(0);
      Serial.println(hh);
      Serial.println(mm);
    }
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
    if (realm >= totalm && realm < totalm + EEPROM.read(y + 2) && EEPROM.read(y + 2) > 0) states[i] = false;
    else states[i] = true;
    y += 3;
  }

  sendvalv();
}

void changeState(bool s1, bool s2, bool s3, bool s4) {
  digitalWrite(v1, !s1);
  digitalWrite(v2, !s2);
  delay(del/4);

  digitalWrite(power1, true);
  delay(del);
  digitalWrite(power1, false);
  delay(del/4);

  digitalWrite(v1, !false);
  digitalWrite(v2, !false);
  delay(del/4);

  digitalWrite(v3, !s3);
  digitalWrite(v4, !s4);
  delay(del/4);

  digitalWrite(power2, true);
  delay(del);
  digitalWrite(power2, false);
  delay(del/4);

  digitalWrite(v3, !false);
  digitalWrite(v4, !false);
}
void sendvalv() {
  bool control = false;
  for (int i = 0; i < 4; i++) {
    if (oldstates[i] != states[i]) {
      oldstates[i] = states[i];
      control = true;
    }
  }
  if (control) changeState(states[0], states[1], states[2], states[3]);
}