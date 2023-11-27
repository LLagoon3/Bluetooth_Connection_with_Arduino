#include <SoftwareSerial.h>
#include <MemoryFree.h>

extern volatile unsigned long timer0_millis;
// SoftwareSerial(RX, TX)
SoftwareSerial BTSerial(4, 5);
int SWITCH = 5;
int FSR[5] = {0, 1, 2, 3, 4};
int LED = 13;
unsigned long TIME_LIMIT = 60;
bool FLAG = false;
// int CNT = 0;
// unsigned long lastSendTime = 0;
int sendInterval = 50;

int COUNT[5] = {0, 0, 0, 0, 0};
int COUNT_SUCCESS[5] = {0, 0, 0, 0, 0};
int COUNT_FLAG[5] = {0, 0, 0, 0, 0};
int COUNT_VALUE = 130;
int PREV_VALUE[5] = {0, 0, 0, 0, 0};
int MAX_VALUE[5] = {0, 0, 0, 0, 0};
int MIN_VALUE[5] = {300, 300, 300, 300, 300};
int THRESHOLDS[5] = {100,200, 95,95,145};

unsigned long RANGE_TIME_LIMIT = 3;
int RANGE_COUNT = 0;
unsigned long RANGE_PREV_TIME = 0;

struct defaultValue{
  int max = 0;
  int min = 0;
  int max_cnt = 0;
  int min_cnt = 0;
  float max_mean;
  float min_mean;
};

defaultValue defaultSensorValues[5];

struct sensorValues {
    int sensorValue0 = 0;
    int sensorValue1 = 0;
    int sensorValueList[5];
    int tmpSensorValueList[5];
    String sensorValueString = "";
};

struct countValues{
  int successCount = 0;
  int count = 0;
};

void debug(){
  while(true){
    int tmp = analogRead(FSR[1]);
    Serial.println("");
    float voltage = tmp * (5.0 / 1023.0);
    float pressure_ratio = voltage / 5;
    float resistance = (10000 * pressure_ratio) / (1 - pressure_ratio);
    Serial.println("\nValue : " + String(tmp) + "  Voltage : " + String(voltage) + "v  R : " + String(resistance) + "ohm");
    delay(30);
  }
}

void dataSend(String data){

  byte *temp = new byte[data.length()+1];
  data.getBytes(temp, data.length()+1);

  for (int i = 0 ; i < data.length() + 1 ; i++) {
      BTSerial.write((byte)data[i]);
    }
  delete[] temp;
}

void resetStage(int time = 0){
  timer0_millis = time;
  for(int i = 0; i < 5; i++){
    COUNT[i] = 0;
    COUNT_SUCCESS[i] = 0;
    COUNT_FLAG[i] = 0;
    PREV_VALUE[i] = 0;
    MAX_VALUE[i] = 0;
    MIN_VALUE[i] = 300;
  }
  Serial.println("Reset Stage");
}



bool chkSwitch(){
  int i = analogRead(SWITCH);
  Serial.println("\n\nanalogRead : " + String(i));
  if (i > 800){return true;}
  else{return false;}
}

sensorValues chkFSR(){
  sensorValues result;
  // if(int(analogRead(FSR[0])) > 50){ CNT = CNT + 1;}
  for(int i = 0; i < sizeof(FSR) / sizeof(int); i++){
    
    int value = analogRead(FSR[i]);
    result.sensorValueString = result.sensorValueString + String(value) + ",";
    result.sensorValueList[i] = map(constrain(value, 0, THRESHOLDS[i]), 0, THRESHOLDS[i], 0, 100);
    result.tmpSensorValueList[i] = value;
  }
  result.sensorValueString = result.sensorValueString.substring(0, result.sensorValueString.length() - 1);
  return result;
}

void quickSort(int arr[], int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

int partition(int arr[], int low, int high) {
    int pivot = arr[high]; 
    int i = (low - 1);
    for (int j = low; j <= high - 1; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(arr[i], arr[j]);
        }
    }
    swap(arr[i + 1], arr[high]);
    return (i + 1);
}

void swap(int &a, int &b) {
    int temp = a;
    a = b;
    b = temp;
}

void printArray(int arr[]) {
    for (int i = 0; i < 5; i++) {
        Serial.print(arr[i]);
        Serial.print(" ");
    }
    Serial.println();
}

double bpm(int count, unsigned long time){
  double tmpCount = count;
  double tmpTime = time / 1000;
  Serial.println("time : " + String(time) + " count : " + String(count) + " val : " + String(double(tmpCount / tmpTime)));
  return (tmpCount / tmpTime) * 60;
}


countValues p_count(int current_val, int index){
  countValues result;
  if(current_val - PREV_VALUE[index] > 6){ // UP
    COUNT_FLAG[index] = true;
    MAX_VALUE[index] = current_val;
  }
  else if((current_val - PREV_VALUE[index] < -6) && (COUNT_FLAG[index] == 1)){ // DOWN
    if(MAX_VALUE[index] >= COUNT_VALUE){
      COUNT_SUCCESS[index]++;
    }

    defaultSensorValues[index].max = defaultSensorValues[index].max + MAX_VALUE;
    defaultSensorValues[index].max_cnt++;
    defaultSensorValues[index].min = defaultSensorValues[index].min + MIN_VALUE;
    defaultSensorValues[index].min_cnt++;
  
    COUNT_FLAG[index] = 0;
    MAX_VALUE[index] = 0;
    MIN_VALUE[index] = 300;
    COUNT[index]++;
  }
  if((current_val != 0) && (current_val < MIN_VALUE[index])){
    MIN_VALUE[index] = current_val;
  }

  PREV_VALUE[index] = current_val;
  result.count = COUNT[index];
  result.successCount = COUNT_SUCCESS[index];
  return result;
}



void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));
  BTSerial.begin(9600);
  pinMode(LED,OUTPUT);
}

void loop() {

  unsigned long currentTime = (millis());

  if ((currentTime) > TIME_LIMIT * 1000){
    FLAG = false;
    dataSend("SF");
  }

  while(FLAG == false){
    if(chkSwitch() == true){
      dataSend("SO");
      while(true){
        if (BTSerial.available()) {
          char buf[20];
          Serial.print("recv: ");
          BTSerial.readBytes(buf, 20);
          Serial.println(String(buf));
          Serial.println(String(buf).indexOf("ST"));
          if(String(buf).indexOf("ST") == 0){
            dataSend("ST");
            FLAG = true;
            resetStage();
            break;
          }
        }
      }
      break;
	  }
  }
  

  int32_t cycle = int32_t(currentTime / sendInterval);
  sensorValues fsrVals = chkFSR();
  int size = sizeof(fsrVals.sensorValueList) / sizeof(fsrVals.sensorValueList[0]);
  countValues countVals = p_count(fsrVals.tmpSensorValueList[0], 0);
  String tmp = "";
  for(int i = 0; i< 5; i++){
    tmp = tmp + String(fsrVals.sensorValueList[i]) + ",";
  }
  printArray(fsrVals.sensorValueList);
  String data = String(cycle * sendInterval) + '_' + tmp + "_" + String(fsrVals.sensorValue0) + String(fsrVals.sensorValue1) + "_" + String(countVals.count) + "_" + String(int(bpm(countVals.count, currentTime))) + "_" + String(countVals.successCount);

  dataSend(data); 
}
