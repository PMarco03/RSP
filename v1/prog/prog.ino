#include <EEPROM.h>
#include <LiquidCrystal_I2C.h> // Library for LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

int b1 = 4;
int b2 = 3;
int b3 = 2;

int h0 = 12;
int m0 = 5;

int h1 = 0;
int h2 = 0;
int h3 = 0;
int h4 = 0;

int m1 = 0;
int m2 = 0;
int m3 = 0;
int m4 = 0;

int d1 = 0;
int d2 = 0;
int d3 = 0;
int d4 = 0;

int th1 = 0;
int th2 = 0;
int th3 = 0;
int th4 = 0;

int tm1 = 0;
int tm2 = 0;
int tm3 = 0;
int tm4 = 0;

int td1 = 0;
int td2 = 0;
int td3 = 0;
int td4 = 0;

bool st1 = false;
bool st2 = false;
bool st3 = false;
bool st4 = true;

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

bool bb1 = false;
bool bb2 = false;
bool bb3 = false;

bool blink = false;
int temp [3];

void setup() {
  pinMode(b1, INPUT_PULLUP);
  pinMode(b2,INPUT_PULLUP);
  pinMode(b3,INPUT_PULLUP);
  lcd.begin(); //initialize the lcd
  lcd.backlight(); //open the backlight
  lcd.createChar(0, off);
  lcd.createChar(1, on);
  Serial.begin(9600);

  h1 = EEPROM.read(10);
  h2 = EEPROM.read(11);
  h3 = EEPROM.read(12);
  h4 = EEPROM.read(13);
  m1 = EEPROM.read(14);
  m2 = EEPROM.read(15);
  m3 = EEPROM.read(16);
  m4 = EEPROM.read(17);
  d1 = EEPROM.read(18);
  d2 = EEPROM.read(19);
  d3 = EEPROM.read(20);
  d4 = EEPROM.read(21);


  th1 = h1;
  th2 = h2;
  th3 = h3;
  th4 = h4;

  tm1 = m1;
  tm2 = m2;
  tm3 = m3;
  tm4 = m4;

  td1 = d1;
  td2 = d2;
  td3 = d3;
  td4 = d4;
}

void loop() {
  movepointer();
}
void disp(){
   bb1 = blogic(b1);
   bb2 = blogic(b2);
   bb3 = blogic(b3);

  if(pointer == 0)dash();

  else if(pointer == 1){
    page("Mod. Manuale","Menu",false);
  }
  else if(pointer == 2){
    page("Mod. Manuale","Menu",true);
  }
  else if(pointer == 3){
    page("Reset","",false);
  }
  else if(pointer == 10){
    page("Valvola 1","Valvola 2",false);
  }
  else if(pointer == 11){
    page("Valvola 1","Valvola 2",true);
  }
  else if(pointer == 12){
    page("Valvola 3","Valvola 4",false);
  }
  else if(pointer == 13){
    page("Valvola 3","Valvola 4",true);
  }
  else if(pointer == 14){
    page("Esci","",false);
  }
  else if(pointer == 20){
    toggle("Valvola 1: ","Valvola 2: ",false,st1,st2);
  }
  else if(pointer == 21){
    toggle("Valvola 1: ","Valvola 2: ",true,st1,st2);
  }
  else if(pointer == 22){
    toggle("Valvola 3: ","Valvola 4: ",false,st3,st4);
  }
  else if(pointer == 23){
    toggle("Valvola 3: ","Valvola 4: ",true,st3,st4);
  }
  else if(pointer == 24){
    page("Esci","",false);
  }
  else if(pointer == 100){
    confirm("<   Salvare?   >");
  }
  else if(pointer == 101){
    confirm("<  Resettare?  >");
  }

}
void movepointer(){
  if(bb1 || bb2 || bb3)lcd.clear();

  switch(pointer){

    case 1 ... 3:
      if(bb1){
        pointer++;
      }
      if(bb3){
        pointer--;
      }
      pointer = limit(pointer,3,0);
    break;

    case 10 ... 14:
      if(bb1){
        pointer++;
      }
      if(bb3){
        pointer--;
      }
      pointer = rotate(pointer,14,10);
      break;
    
     case 20 ... 24:
      if(bb1){
        pointer++;
      }
      if(bb3){
        pointer--;
      }
      pointer = rotate(pointer,24,20);
      break;

      case 100:
        if(bb3){
          save(true);
          pointer = 0;  
        }
        else if(bb1){
          save(false);
          pointer = 0;  
        }
        else if(bb2) pointer = 10;
        bb1 = false;
        bb2 = false;
        bb3 = false;
      break; 

      case 101:
        if(bb3){
          res();
          pointer = 0;  
        }
        else if(bb1){
          
        pointer = 0;  
        }
        else if(bb2) pointer = 0;
          bb1 = false;
          bb2 = false;
          bb3 = false;        
  }
  enter();
  disp();
}
void enter(){

  if(bb2){
    lcd.clear();
    switch(pointer){
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
        pointer=101;
        
      break;

      case 10:
        program(th1,tm1,td1,"Valvola 1");
        th1=temp[0];
        tm1=temp[1];
        td1=temp[2];
      break;

      case 11:
        program(th2,tm2,td2,"Valvola 2");
        th2=temp[0];
        tm2=temp[1];
        td2=temp[2];
      break;

      case 12:
        program(th3,tm3,td3,"Valvola 3");
        th3=temp[0];
        tm3=temp[1];
        td3=temp[2];
      break;

      case 13:
        program(th4,tm4,td4,"Valvola 4");
        th4=temp[0];
        tm4=temp[1];
        td4=temp[2];
      break;

      case 14:
        pointer = 100;
      break;
      
      case 20:
        st1=!st1;
      break;

      case 21:
        st2=!st2;
      break;

      case 22:
        st3=!st3;
      break;

      case 23:
        st4=!st4;
      break;

      case 24:
        pointer=0;
      break;
    }
  }
}
void page(String st1, String st2, bool p){
  lcd.setCursor(0,0);
  lcd.print(st1);
  lcd.setCursor(0,1);
  lcd.print(st2);
  if(!p)lcd.setCursor(15,0);
  else lcd.setCursor(15,1);
  lcd.print("<");
}
void toggle(String st1, String st2, bool p,bool state1,bool state2){
  lcd.setCursor(11,0);
  if(state1)lcd.write(1);
  else lcd.write(0);
  lcd.setCursor(11,1);
  if(state2)lcd.write(1);
  else lcd.write(0);
  page(st1,st2,p);
}
void confirm(String msg){
lcd.setCursor(0,0); 
lcd.print(msg);
lcd.setCursor(0,1);
lcd.print("Si  Indietro  No");
}

void save(bool sc){
  if(sc){
    h1 = th1;
    h2 = th2;
    h3 = th3;
    h4 = th4;

    m1 = tm1;
    m2 = tm2;
    m3 = tm3;
    m4 = tm4;

    d1 = td1;
    d2 = td2;
    d3 = td3;
    d4 = td4;

    EEPROM.write(10, h1);
    EEPROM.write(11, h2);
    EEPROM.write(12, h3);
    EEPROM.write(13, h4);
    EEPROM.write(14, m1);
    EEPROM.write(15, m2);
    EEPROM.write(16, m3);
    EEPROM.write(17, m4);
    EEPROM.write(18, d1);
    EEPROM.write(19, d2);
    EEPROM.write(20, d3);
    EEPROM.write(21, d4);

  }
  else{
    th1 = h1;
    th2 = h2;
    th3 = h3;
    th4 = h4;

    tm1 = m1;
    tm2 = m2;
    tm3 = m3;
    tm4 = m4;

    td1 = d1;
    td2 = d2;
    td3 = d3;
    td4 = d4;
  }
}
void dash(){
  if(timer(1000))blink = !blink;
  if(bb1)subdash++;
  else if(bb3)subdash--;
  subdash=rotate(subdash,4,0);
  if(subdash==0){
    lcd.setCursor(0,0);
    lcd.print(righttime(h0,m0,true));
  }

  lcd.setCursor(0,1);
  lcd.print("1");
  lcd.setCursor(1,1);
  if(subdash == 1){
    if(blink)lcd.print(":");
    else lcd.print(" ");
    dispprogram(h1,m1,d1);
  }
  else lcd.print(":");
  lcd.setCursor(2,1);
  if(st1) lcd.write(1);
  else lcd.write(0);
  lcd.setCursor(4,1);
  lcd.print("2");
  lcd.setCursor(5,1);
  if(subdash == 2){
    if(blink)lcd.print(":");
    else lcd.print(" ");
    dispprogram(h2,m2,d2);
  }
  else lcd.print(":");
  lcd.setCursor(6,1);
  if(st2) lcd.write(1);
  else lcd.write(0);
  lcd.setCursor(8,1);
  lcd.print("3");
  lcd.setCursor(9,1);
  if(subdash == 3){
    if(blink)lcd.print(":");
    else lcd.print(" ");
    dispprogram(h3,m3,d3);
  }
  else lcd.print(":");
  lcd.setCursor(10,1);
  if(st3) lcd.write(1);
  else lcd.write(0);
  lcd.setCursor(12,1);
  lcd.print("4");
  lcd.setCursor(13,1);
  if(subdash == 4){
    if(blink)lcd.print(":");
    else lcd.print(" ");
    dispprogram(h4,m4,d4);
  }
  else lcd.print(":");
  lcd.setCursor(14,1);
  if(st4) lcd.write(1);
  else lcd.write(0);

}
void dispprogram(int hh, int mm, int dd){
  
  lcd.setCursor(0,0);
  String str = "";
  str = righttime(hh,mm,true);
  
  str.concat(" => ");
  if(dd>0){
    str.concat(dd);
    str.concat(" min");
  }
  else str.concat("Off");
  lcd.print(str);
}
void program(int hh, int mm, int dd, String name){
  int step = 0;
  while(step<3){
    bb1 = blogic(b1);
    bb2 = blogic(b2);
    bb3 = blogic(b3);
    if(bb1 || bb2 || bb3)lcd.clear();
    if(bb2)step++;
    if(step!=2)print(hh,mm,name,":");
    if(step==0){ //set h
      if(bb1)hh++;
      else if(bb3)hh--;
      hh=rotate(hh,23,0);
    }
    else if(step==1){ //set m
      print(hh,mm,name,":");
      if(bb1)mm++;
      else if(bb3)mm--;
      mm=rotate(mm,59,0);
    }
    else if(step==2){ //set d
      if(dd>0)print(dd,-1,name,"Durata min: ");
      else print(-1,-1,name,"Durata min: Off");
      if(bb1)dd++;
      else if(bb3)dd--;
      dd=rotate(dd,120,0);
      
    }
  }
  temp[0] = hh;
  temp[1] = mm;
  temp[2] = dd;
  lcd.clear();
}

void print(int n1, int n2, String name, String str1){
  lcd.setCursor(0,0);
  lcd.print(name);
  if(n2>-1){
    String str = righttime(n1,n2,true);
    lcd.setCursor(0,1);
    lcd.print(str);
  }
  else {
    lcd.setCursor(0,1);
    String str = str1;
    if(n1>-1)str.concat(n1);
    if(n1>-1)str.concat("  ");
    lcd.print(str);
    }
}

bool timer(int limit){
  //Serial.println(cron);
  if(millis()-cron>=limit){
    cron = millis();
    return true;
  }
  else{
    cron++;
    return false;
  }
}

int limit(int n, int max, int min){
  if(n>max)n=max;
  else if(n<min)n=min;
  return n;
}
int rotate(int n, int max, int min){
  if(n>max)n=min;
  else if(n<min)n=max;
  return n;
}

bool blogic(int b){
  if(!digitalRead(b)){
    while(!digitalRead(b)){}
  
    return true;
  }
  else return false;
}

String righttime(int hh, int mm, bool dp){
  String str = "";
  if(hh>-1){
    if(hh<10)str=str+"0";
    str.concat(hh);
  }
 if(dp)str=str+":";
 if(mm>-1){
  if(mm<10)str=str+"0";
  str.concat(mm);
 }
  return str;
}

void sendData(){
  String send = "";
  send.concat(h1);
  send=send+"+";
  send.concat(m1);
  send=send+"+";
  send.concat(d1);
  send=send+"+";
  send.concat(h2);
  send=send+"+";
  send.concat(m2);
  send=send+"+";
  send.concat(d2);
  send=send+"+";
  send.concat(h3);
  send=send+"+";
  send.concat(m3);
  send=send+"+";
  send.concat(d3);
  send=send+"+";
  send.concat(h4);
  send=send+"+";
  send.concat(m4);
  send=send+"+";
  send.concat(d4);
  char tosend [send.length()]={};
  for(int i = 0; i<send.length(); i++)tosend[i]=send.charAt(i);
}

void realTime(int valv, bool state){
  String send = "";
  send.concat(valv);
  send = send+"+";
  send.concat(state);
  char tosend [send.length()]={};
  for(int i = 0; i<send.length(); i++)tosend[i]=send.charAt(i);
}

void res(){
  lcd.clear();
  int mapped = 0;
  int mapped2 = 0;
  int checker = 0;
  int checker2 = 0;
  String str = ""; 
  String str2 = "";
  for (int i = 0 ; i < EEPROM.length() ; i++){
    mapped=map(i,0,EEPROM.length(),0,100);
    mapped2=map(i,0,EEPROM.length(),0,17);
    EEPROM.write(i, 0);
    str = "Rimanente: ";
    if(mapped!=checker){
      lcd.setCursor(0,0); 
      lcd.clear();
      checker = mapped;
      str.concat(mapped);
      str.concat("%");
      lcd.print(str);
      lcd.setCursor(0,1); 
      lcd.print(str2);
      }
    if(mapped2!=checker2){
      checker2=mapped2;
      str2.concat("#");
      } 
    }
  h0 = 0;
  m0 = 0;

  h1 = 0;
  h2 = 0;
  h3 = 0;
  h4 = 0;

  m1 = 0;
  m2 = 0;
  m3 = 0;
  m4 = 0;

  d1 = 0;
  d2 = 0;
  d3 = 0;
  d4 = 0;

  th1 = 0;
  th2 = 0;
  th3 = 0;
  th4 = 0;

  tm1 = 0;
  tm2 = 0;
  tm3 = 0;
  tm4 = 0;

  td1 = 0;
  td2 = 0;
  td3 = 0;
  td4 = 0;

  st1 = false;
  st2 = false;
  st3 = false;
  st4 = true;

  pointer = 0;
  subdash = 0;
  Serial.print("reset");
  lcd.clear();
}

