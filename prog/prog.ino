#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>  // Library for LCD
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
RF24 radio(7, 8);
const byte address[6] = "122222";
#define nvalv 4
#define npar 3
int valv[nvalv][npar];
int tvalv[nvalv][npar];
bool states[nvalv];

int b1 = 4;
int b2 = 3;
int b3 = 2;

int h0 = 12;
int m0 = 5;

int pointer = 0;
int subdash = 0;

unsigned long int cron = 0;

byte off[] = {
  B11111,
  B10001,
  B10001,
  B10001,
  B10001,
  B10001,
  B11111,
  B00000
};

byte on[] = {
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B11111,
  B00000
};

byte up[] = {
  B00000,
  B00100,
  B01110,
  B11111,
  B01110,
  B01110,
  B01110,
  B00000
};

bool bb1 = false;
bool bb2 = false;
bool bb3 = false;

char rtime[8]="        ";
void setradio(bool r) {
  if (r) {
    radio.openReadingPipe(0, address);
    radio.startListening();
  } else {
    radio.stopListening();
  }
}
void setup() {
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
  pinMode(b1, INPUT_PULLUP);
  pinMode(b2, INPUT_PULLUP);
  pinMode(b3, INPUT_PULLUP);
  lcd.begin();      //initialize the lcd
  lcd.backlight();  //open the backlight
  lcd.createChar(0, off);
  lcd.createChar(1, on);
  lcd.createChar(2, up);
  Serial.begin(9600);

  int y = 0;
  for (int i = 0; i < nvalv; i++) {
    states[i] = false;
    for (int x = 0; x < npar; x++) {
      valv[i][x] = EEPROM.read(y);
      tvalv[i][x] = valv[i][x];
      y++;
    }
  }
}

void loop() {
  movepointer();
  setradio(true);
  if(radio.available()){
    radio.read(&rtime, sizeof(rtime));
    setradio(false);

  }
  

}
void disp() {
  bb1 = blogic(b1);
  bb2 = blogic(b2);
  bb3 = blogic(b3);

  if (pointer == 0) dash();

  else if (pointer == 1) {
    page("Mod. Manuale", "Menu", false);
  } else if (pointer == 2) {
    page("Mod. Manuale", "Menu", true);
  } else if (pointer == 3) {
    page("Reset", "", false);
  } else if (pointer == 10) {
    page("Valvola 1", "Valvola 2", false);
  } else if (pointer == 11) {
    page("Valvola 1", "Valvola 2", true);
  } else if (pointer == 12) {
    page("Valvola 3", "Valvola 4", false);
  } else if (pointer == 13) {
    page("Valvola 3", "Valvola 4", true);
  } else if (pointer == 14) {
    page("Esci", "", false);
  } else if (pointer == 20) {
    toggle("Valvola 1: ", "Valvola 2: ", false, states[0], states[1]);
  } else if (pointer == 21) {
    toggle("Valvola 1: ", "Valvola 2: ", true, states[0], states[1]);
  } else if (pointer == 22) {
    toggle("Valvola 3: ", "Valvola 4: ", false, states[2], states[3]);
  } else if (pointer == 23) {
    toggle("Valvola 3: ", "Valvola 4: ", true, states[2], states[3]);
  } else if (pointer == 24) {
    page("Esci", "", false);
  } else if (pointer == 100) {
    confirm("<   Salvare?   >");
  } else if (pointer == 101) {
    confirm("<  Resettare?  >");
  }
}
void movepointer() {
  if (bb1 || bb2 || bb3) lcd.clear();

  switch (pointer) {

    case 1 ... 3:
      if (bb1) {
        pointer++;
      }
      if (bb3) {
        pointer--;
      }
      pointer = limit(pointer, 3, 0);
      break;

    case 10 ... 14:
      if (bb1) {
        pointer++;
      }
      if (bb3) {
        pointer--;
      }
      pointer = rotate(pointer, 14, 10);
      break;

    case 20 ... 24:
      if (bb1) {
        pointer++;
      }
      if (bb3) {
        pointer--;
      }
      pointer = rotate(pointer, 24, 20);
      break;

    case 100:
      if (bb3) {
        save(true);
        pointer = 0;
      } else if (bb1) {
        save(false);
        pointer = 0;
      } else if (bb2) pointer = 10;
      bb1 = false;
      bb2 = false;
      bb3 = false;
      break;

    case 101:
      if (bb3) {
        res();
        pointer = 0;
      } else if (bb1) {

        pointer = 0;
      } else if (bb2) pointer = 0;
      bb1 = false;
      bb2 = false;
      bb3 = false;
  }
  enter();
  disp();
}
void enter() {
  String st = "Valvola ";
  if (bb2) {
    Serial.println(pointer);
    lcd.clear();
    switch (pointer) {
      case 0:
        pointer = 1;
        break;

      case 1:
        pointer = 20;
        break;

      case 2:
        pointer = 10;
        break;

      case 3:
        pointer = 101;
        break;

      case 10 ... 13:
        st.concat(pointer - 9);
        program(pointer - 10, tvalv[pointer - 10][0], tvalv[pointer - 10][1], tvalv[pointer - 10][2], st);
        break;

      case 14:
        pointer = 100;
        break;

      case 20 ... 23:
        states[pointer - 20] = !states[pointer - 20];
        sendState(pointer - 20, states[pointer - 20]);
        break;

      case 24:
        pointer = 0;
        sendradio("2  ");
        for(int i = 0; i<nvalv; i++)states[i]=false;
        break;
    }
  }
}
void sendradio(String tosend){
  char c [tosend.length()] = "";
  for(int i = 0; i<tosend.length(); i++)c[i]=tosend.charAt(i);
  setradio(false);

  radio.write(&c, sizeof(c));
}
void page(String st1, String st2, bool p) {
  lcd.setCursor(0, 0);
  lcd.print(st1);
  lcd.setCursor(0, 1);
  lcd.print(st2);
  if (!p) lcd.setCursor(15, 0);
  else lcd.setCursor(15, 1);
  lcd.print("<");
}
void toggle(String st1, String st2, bool p, bool state1, bool state2) {
  lcd.setCursor(11, 0);
  if (state1) lcd.write(1);
  else lcd.write(0);
  lcd.setCursor(11, 1);
  if (state2) lcd.write(1);
  else lcd.write(0);
  page(st1, st2, p);
}
void confirm(String msg) {
  lcd.setCursor(0, 0);
  lcd.print(msg);
  lcd.setCursor(0, 1);
  lcd.print("Si  Indietro  No");
}

void save(bool sc) {
  if (sc) {
    int y = 0;
    for (int i = 0; i < nvalv; i++) {
      for (int x = 0; x < npar; x++) {
        valv[i][x] = tvalv[i][x];
        EEPROM.write(y, valv[i][x]);
        y++;
      }
    }
    sendData();

  } else {
    for (int i = 0; i < nvalv; i++) {
      for (int x = 0; x < npar; x++) {
        tvalv[i][x] = valv[i][x];
      }
    }
  }
}
void dash() {
  if (bb1) subdash++;
  else if (bb3) subdash--;
  subdash = rotate(subdash, 4, 0);
  Serial.println(rtime);
  int s = 0;
  for (int i = 0; i < 4; i++) {
    lcd.setCursor(s, 1);
    lcd.print(i + 1);
    s++;
    if (subdash - 1 != i) lcd.print(":");
    else lcd.setCursor(s + 1, 1);
    if (states[i]) lcd.write(1);
    else lcd.write(0);
    s += 3;
  }
  if (subdash == 0) {
    lcd.setCursor(0, 0);
    lcd.print(rtime);
  } else {
    int pos = 0;
    switch (subdash) {
      case 1:
        pos = 1;
        break;
      case 2:
        pos = 5;
        break;
      case 3:
        pos = 9;
        break;
      case 4:
        pos = 13;
        break;
    }
    lcd.setCursor(pos, 1);
    lcd.write(2);

    dispprogram(valv[subdash - 1][0], valv[subdash - 1][1], valv[subdash - 1][2]);
  }
}
void dispprogram(int hh, int mm, int dd) {

  lcd.setCursor(0, 0);
  String str = "";
  str = righttime(hh, mm, true);

  str.concat(" => ");
  if (dd > 0) {
    str.concat(dd);
    str.concat(" min");
  } else str.concat("Off");
  lcd.print(str);
}
void program(int n, int hh, int mm, int dd, String name) {
  int step = 0;
  while (step < 3) {
    bb1 = blogic(b1);
    bb2 = blogic(b2);
    bb3 = blogic(b3);
    if (bb1 || bb2 || bb3) lcd.clear();
    if (bb2) step++;
    if (step != 2) print(hh, mm, name, ":");
    if (step == 0) {  //set h
      if (bb1) hh++;
      else if (bb3) hh--;
      hh = rotate(hh, 23, 0);
    } else if (step == 1) {  //set m
      print(hh, mm, name, ":");
      if (bb1) mm++;
      else if (bb3) mm--;
      mm = rotate(mm, 59, 0);
    } else if (step == 2) {  //set d
      if (dd > 0) print(dd, -1, name, "Durata min: ");
      else print(-1, -1, name, "Durata min: Off");
      if (bb1) dd++;
      else if (bb3) dd--;
      dd = rotate(dd, 59, 0);
    }
  }
  tvalv[n][0] = hh;
  tvalv[n][1] = mm;
  tvalv[n][2] = dd;
  lcd.clear();
}

void print(int n1, int n2, String name, String str1) {
  lcd.setCursor(0, 0);
  lcd.print(name);
  if (n2 > -1) {
    String str = righttime(n1, n2, true);
    lcd.setCursor(0, 1);
    lcd.print(str);
  } else {
    lcd.setCursor(0, 1);
    String str = str1;
    if (n1 > -1) str.concat(n1);
    if (n1 > -1) str.concat("  ");
    lcd.print(str);
  }
}

bool timer(int limit) {
  //Serial.println(cron);
  if (millis() - cron >= limit) {
    cron = millis();
    return true;
  } else {
    cron++;
    return false;
  }
}

int limit(int n, int max, int min) {
  if (n > max) n = max;
  else if (n < min) n = min;
  return n;
}
int rotate(int n, int max, int min) {
  if (n > max) n = min;
  else if (n < min) n = max;
  return n;
}

bool blogic(int b) {
  if (!digitalRead(b)) {
    while (!digitalRead(b)) {}

    return true;
  } else return false;
}

String righttime(int hh, int mm, bool dp) {
  String str = "";
  if (hh > -1) {
    if (hh < 10) str = str + "0";
    str.concat(hh);
  }
  if (dp) str = str + ":";
  if (mm > -1) {
    if (mm < 10) str = str + "0";
    str.concat(mm);
  }
  return str;
}

void sendData() {
  setradio(false);
  char toSend[3] = "1  ";
  radio.write(&toSend, sizeof(toSend));
  for (int i = 0; i < nvalv; i++) {
    for (int x = 0; x < npar; x++) {
      toSend[0] = fillzero(valv[i][x]).charAt(0);
      toSend[1] = fillzero(valv[i][x]).charAt(1);
      toSend[2] = fillzero(valv[i][x]).charAt(2);
      radio.write(&toSend, sizeof(toSend));
      delay(10);
    }
  }
}
String fillzero(int n) {
  String ret = "";
  if (n < 10) ret.concat("0");
  if (n < 100) ret.concat("0");
  ret.concat(n);
  Serial.println(ret);
  return ret;
}
void realTime(int valv, bool state) {
  String send = "";
  send.concat(valv);
  send = send + "+";
  send.concat(state);
  char tosend[send.length()] = {};
  for (int i = 0; i < send.length(); i++) tosend[i] = send.charAt(i);
}

void res() {
  lcd.clear();
  int mapped = 0;
  int mapped2 = 0;
  int checker = 0;
  int checker2 = 0;
  String str = "";
  String str2 = "";
  for (int i = 0; i < EEPROM.length(); i++) {
    mapped = map(i, 0, EEPROM.length(), 0, 100);
    mapped2 = map(i, 0, EEPROM.length(), 0, 17);
    EEPROM.write(i, 0);
    str = "Rimanente: ";
    if (mapped != checker) {
      lcd.setCursor(0, 0);
      lcd.clear();
      checker = mapped;
      str.concat(mapped);
      str.concat("%");
      lcd.print(str);
      lcd.setCursor(0, 1);
      lcd.print(str2);
    }
    if (mapped2 != checker2) {
      checker2 = mapped2;
      str2.concat("#");
    }
  }
  for (int i = 0; i < nvalv; i++) {
    for (int x = 0; x < npar; x++) {
      valv[i][x] = 0;
      tvalv[i][x] = 0;
    }
  }
  pointer = 0;
  subdash = 0;
  Serial.print("reset");
  lcd.clear();
}
void sendState(int v, bool st) {
  setradio(false);
  char toSend[3] = "0  ";
  radio.write(&toSend, sizeof(toSend));
  String str = "";
  str.concat(v);
  str.concat(st);
  str.concat("    ");
  char tosend[str.length()];
  for (int i = 0; i < str.length(); i++) tosend[i] = str.charAt(i);
  radio.write(&tosend, sizeof(tosend));
}
