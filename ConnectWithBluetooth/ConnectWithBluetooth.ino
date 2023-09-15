#include <SoftwareSerial.h>
extern volatile unsigned long timer0_millis;
// SoftwareSerial(RX, TX)
SoftwareSerial BTSerial(4, 5);

#include <MemoryFree.h>

void freeMem(){
  Serial.print("Memory : ");
  Serial.println(freeMemory());
}

void dataSend(){
  long randNumber = random(300);
  String data = String(millis()) + '_' + String(randNumber);

  byte *temp = new byte[data.length()+1];
  data.getBytes(temp, data.length()+1);

  ////////////////////////////////////////////////////////////////
  Serial.println("data : " + data);
  Serial.print("Byte data : ");
  for(int i=0;i<data.length();i++){Serial.print(byte(temp[i]));}
  Serial.println();////
  Serial.print("byte size : " );
  Serial.println(sizeof(data) / sizeof(byte));
  ////////////////////////////////////////////////////////////////

  for (int i = 0 ; i < data.length() + 1 ; i++) {
			BTSerial.write((byte)data[i]);
		}
  delete[] temp;
  delay(1000);
}

void setup() {
	Serial.begin(9600);
  randomSeed(analogRead(0));
	BTSerial.begin(9600);
}

void loop() {

  freeMem();

	if (BTSerial.available()) {
    while(BTSerial.available() > 0){
        Serial.println(BTSerial.read());
    }
    timer0_millis = 0;
    Serial.println("Reset Timer");////
	}

  dataSend();

}

