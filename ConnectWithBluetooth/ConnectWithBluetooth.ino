#include <SoftwareSerial.h>
extern volatile unsigned long timer0_millis;
// SoftwareSerial(RX, TX)
SoftwareSerial BTSerial(4, 5);
int led = 13;
int time_limit = 60;
bool flag = false;

#include <MemoryFree.h>

void freeMem(){
  Serial.print("Memory : ");
  Serial.println(freeMemory());
}

void dataSend(String data){

  byte *temp = new byte[data.length()+1];
  data.getBytes(temp, data.length()+1);


  ////////////////////////////////////////////////////////////////
  Serial.println("data : " + data);
  Serial.print("Byte data : ");
  for(int i=0;i<data.length();i++){Serial.print(byte(temp[i]));}
  Serial.println();////
  Serial.print("byte size : " );
  Serial.println(data.length());
  ////////////////////////////////////////////////////////////////

  for (int i = 0 ; i < data.length() + 1 ; i++) {
			BTSerial.write((byte)data[i]);
		}
  delete[] temp;
  delay(1000);
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

void chkSwitch(){
  int i = analogRead(2);
  Serial.println("\n\nanalogRead : " + String(i));
  if (i > 800){flag = true;}
  else{flag = false;}
}

void loop() {

  freeMem();
  
  String data;

  // if (BTSerial.available()) {
  //     while(BTSerial.available() > 0){
  //         Serial.println(BTSerial.read());
  //     }
  //     flag = false;
  //   }

  unsigned long tmpTime = int(millis());
  if (tmpTime > time_limit * 1000){
    flag = false;
    // resetTimer();
  }

  while(flag == false){
    chkSwitch();
    if (flag == true){
      // dataSend("SO");
      // delay(5000);
      resetTimer();
      }
  }
  
  long randNumber = random(300);
  data = String(millis()) + '_' + String(randNumber);
  dataSend(data);

}

