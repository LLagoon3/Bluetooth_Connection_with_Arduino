#include <SoftwareSerial.h>
#include <MemoryFree.h>

extern volatile unsigned long timer0_millis;
// SoftwareSerial(RX, TX)
SoftwareSerial BTSerial(4, 5);
int SWITCH = 5;
int FSR[5] = {0, 1, 2, 3, 4};
int led = 13;
unsigned long time_limit = 60;
bool flag = false;
// int CNT = 0;

int t[100];
int cnt = 0;
int testMax = 0;

unsigned long lastSendTime = 0;
int sendInterval = 50;

void debug(){
  while(true){
    int tmp = analogRead(FSR[1]);
    // if(tmp > 0){
    //   t[cnt] = tmp;
    //   cnt += 1;
    // }
    // Serial.print("nValue array : ");
    // for(int i = 0; i < cnt + 1; i++){
    //   Serial.print(String(t[i]) + " ");
    // }
    Serial.println("");
    float voltage = tmp * (5.0 / 1023.0);
    float pressure_ratio = voltage / 5;
    float resistance = (10000 * pressure_ratio) / (1 - pressure_ratio);
    Serial.println("\nValue : " + String(tmp) + "  Voltage : " + String(voltage) + "v  R : " + String(resistance) + "ohm");
    delay(30);
  }
}

void freeMem(){
  Serial.print("Memory : ");
  Serial.println(freeMemory());
}

float calRandNum(int time){
  long randNumber = random(-50, 50);
  float sinx = (800 * sin(time / 80));
  if(sinx < 0){return 0;}
  else if(randNumber > sinx){ return sinx;}
  else{ return sinx - randNumber;}
}

void dataSend(String data){

  byte *temp = new byte[data.length()+1];
  data.getBytes(temp, data.length()+1);

  ////////////////////////////////////////////////////////////////
  // Serial.println("data : " + data);
  // Serial.print("Byte data : ");
  // for(int i=0;i<data.length();i++){Serial.print(byte(temp[i]));}
  // Serial.println();////
  // Serial.print("byte size : " );
  // Serial.println(data.length());
  ////////////////////////////////////////////////////////////////

  for (int i = 0 ; i < data.length() + 1 ; i++) {
      BTSerial.write((byte)data[i]);
    }
  delete[] temp;
  // delay(1000);
}

void resetTimer(int time = 0){
  timer0_millis = time;
  Serial.println("Reset Timer");////
}

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));
  BTSerial.begin(9600);
  pinMode(led,OUTPUT);
}

bool chkSwitch(){
  int i = analogRead(SWITCH);
  Serial.println("\n\nanalogRead : " + String(i));
  if (i > 800){return true;}
  else{return false;}
}

String chkFSR(){
  int fsrVal;
  String fsrVals = "";
  // if(int(analogRead(FSR[0])) > 50){ CNT = CNT + 1;}
  for(int i = 0;i < sizeof(FSR) / sizeof(int); i++){
    fsrVals = fsrVals + String(analogRead(FSR[i])) + ",";
  }
  fsrVals = fsrVals.substring(0, fsrVals.length() - 1);
  Serial.println(fsrVals);
  // float voltage = fsrVal * (5.0 / 1023.0);
  // int convertFsrVal = map(fsrVal, 0, 410, 0, 100); 
  // if(testMax < fsrVal){testMax = fsrVal;}
  // Serial.println("\nValue : " + String(fsrVal) + " Voltage : " + String(voltage) + "v  Pressure : " + String(convertFsrVal) + "kg  Max_val : " + String(testMax));
  // // delay(500);
  return fsrVals;
}

void loop() {

  // debug();


  // if (BTSerial.available()) {
  //     while(BTSerial.available() > 0){
  //         Serial.println(BTSerial.read());
  //     }
  //     flag = false;
  //   }

  unsigned long currentTime = (millis());

  if ((currentTime) > time_limit * 1000){
    flag = false;
    dataSend("SF");
    // resetTimer();
  }

  while(flag == false){
    // chkSwitch();
    if(chkSwitch() == true){
      dataSend("SO");
      while(true){
        if (BTSerial.available()) {
          char buf[20];
          Serial.print("recv: ");
          BTSerial.readBytes(buf, 20);
          Serial.println(String(buf));
          // while(Serial.available() > 0){
          //   Serial.read();
          // }
          Serial.println(String(buf).indexOf("ST"));
          if(String(buf).indexOf("ST") == 0){
            dataSend("ST");
            flag = true;
            resetTimer();
            break;
          }
        }
      }
      break;
	  }
    // if (flag == true){
    //   dataSend("SO");
    //   Serial.println("SO");
    //   delay(6000);
    //   resetTimer();
    //   }
  }
  // long randNumber = random(300);
  // float randNumber = calRandNum(int(millis()));
  // data = String(millis()) + '_' + String(randNumber);
  // float bpm = (CNT / (time.toInt() / 1000)) * 60;
  // String time = String(millis());

  
  // if(currentTime - lastSendTime <= sendInterval){
  //   int cycle = (currentTime / sendInterval);
  //   String data = String(cycle) + '_' + chkFSR();
  //   dataSend(data);
  //   Serial.println("cycle : " + String(cycle) + ",  currentTime : " + String(currentTime) + ",  lastSendTime :" + String(lastSendTime));
  // }

  int32_t cycle = int32_t(currentTime / sendInterval);
  String data = String(cycle * sendInterval) + '_' + chkFSR();
  dataSend(data);
  Serial.println("cycle : " + String(cycle) + ",  currentTime : " + String(currentTime));
  // freeMem();

}
