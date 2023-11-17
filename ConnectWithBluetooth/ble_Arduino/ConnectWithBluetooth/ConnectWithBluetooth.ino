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

int COUNT = 0;
int COUNT_SUCCESS = 0;
bool COUNT_FLAG = false;
int COUNT_VALUE = 130;
int PREV_VALUE = 0;
int MAX_VALUE = 0;

unsigned long RANGE_TIME_LIMIT = 3;
int RANGE_COUNT = 0;
unsigned long RANGE_PREV_TIME = 0;

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

void resetStage(int time = 0){
  timer0_millis = time;
  COUNT = 0;
  COUNT_SUCCESS = 0;
  COUNT_FLAG = false;
  PREV_VALUE = 0;
  MAX_VALUE = 0;
  Serial.println("Reset Stage");
}

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));
  BTSerial.begin(9600);
  pinMode(LED,OUTPUT);
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
    result.sensorValueList[i] = value;
    result.tmpSensorValueList[i] = value;
  }
  result.sensorValueString = result.sensorValueString.substring(0, result.sensorValueString.length() - 1);
  Serial.println(result.sensorValueString);
  // float voltage = fsrVal * (5.0 / 1023.0);
  // int convertFsrVal = map(fsrVal, 0, 410, 0, 100); 
  // if(testMax < fsrVal){testMax = fsrVal;}
  // Serial.println("\nValue : " + String(fsrVal) + " Voltage : " + String(voltage) + "v  Pressure : " + String(convertFsrVal) + "kg  Max_val : " + String(testMax));
  // // delay(500);
  return result;
}

void quickSort(int arr[], int low, int high) {
    if (low < high) {
        // 분할(partitioning) 단계
        int pi = partition(arr, low, high);
        // 분할된 부분에서 각각 정렬 수행
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

int partition(int arr[], int low, int high) {
    int pivot = arr[high];  // 피벗을 배열의 마지막 요소로 선택
    int i = (low - 1);  // 작은 요소들의 마지막 인덱스
    for (int j = low; j <= high - 1; j++) {
        // 현재 요소가 피벗보다 작으면 교환
        if (arr[j] < pivot) {
            i++;
            swap(arr[i], arr[j]);
        }
    }
    // 피벗 위치 교환
    swap(arr[i + 1], arr[high]);
    return (i + 1);
}

void swap(int &a, int &b) {
    int temp = a;
    a = b;
    b = temp;
}

// int deepCopyArray(int originArr[]){
//   int size = sizeof(originArr) / sizeof(originArr[0]);
//   int result[size];
//   for(int i; i < size; i++){
//     result[i] = originArr[i];
//   }
//   return result;
// }

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



countValues p_count(int current_val){
  countValues result;
  if(current_val - PREV_VALUE > 6){ // UP
    COUNT_FLAG = true;
    MAX_VALUE = current_val;
  }
  else if((current_val - PREV_VALUE < -6) && (COUNT_FLAG == true)){ // DOWN
    if(MAX_VALUE >= COUNT_VALUE){
      COUNT_SUCCESS++;
    }
    COUNT_FLAG = false;
    MAX_VALUE = 0;
    COUNT++;
  }

  PREV_VALUE = current_val;
  result.count = COUNT;
  result.successCount = COUNT_SUCCESS;
  return result;
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

  if ((currentTime) > TIME_LIMIT * 1000){
    FLAG = false;
    dataSend("SF");
    // resetTimer();
  }

  while(FLAG == false){
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
            FLAG = true;
            resetStage();
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
  sensorValues fsrVals = chkFSR();
  int size = sizeof(fsrVals.sensorValueList) / sizeof(fsrVals.sensorValueList[0]);
  countValues countVals = p_count(fsrVals.tmpSensorValueList[0]);
  // double bpm;
  // int tmpArray[size];
  // tmpArray = deepCopyArray(fsrVals.sensorValueList);

  quickSort(fsrVals.sensorValueList, 0, size - 1);
  
  for(int i = 0; i < 5; i++){
    if(fsrVals.tmpSensorValueList[i] == fsrVals.sensorValueList[4]){
      fsrVals.sensorValue0 = i;
    }
    else if(fsrVals.tmpSensorValueList[i] == fsrVals.sensorValueList[3]){
      fsrVals.sensorValue1 = i;
    }
  }

  //// BPM with Time Limit ////
  // if(currentTime - RANGE_PREV_TIME > RANGE_TIME_LIMIT * 1000){
  //   double tmpCount = count - RANGE_COUNT;
  //   double tmpTime = RANGE_TIME_LIMIT;
  //   RANGE_PREV_TIME = currentTime;
  //   RANGE_COUNT = count;
  //   bpm = (tmpCount / tmpTime) * 60;
  //   Serial.println("bpm : " + String(bpm) + " tmpCount : " + String(tmpCount) + " tmpTime : " + String(tmpTime));
  //   if(bpm < 0){
  //     bpm = 0;
  //   }
  // }

  String data = String(cycle * sendInterval) + '_' + fsrVals.sensorValueString + "_" + String(fsrVals.sensorValue0) + "," + String(fsrVals.sensorValue1) + "_" + String(countVals.count) + "_" + String(int(bpm(countVals.count, currentTime))) + "_" + String(countVals.successCount);
  dataSend(data);
  Serial.println(data);

  // Serial.println("cycle : " + String(cycle) + ",  currentTime : " + String(currentTime));
  // freeMem();
}
