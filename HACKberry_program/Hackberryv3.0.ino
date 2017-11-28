/*
 *  Arduino micro code for HACKberry.
 *  Origially created by exiii Inc.
 *  edited by Genta Kondo on 2017/6/11
 */
#include <Servo.h>

//Settings
const char Serialnum = "";
const boolean isRight = ;//right:1, left:0
const int outThumbMax = ;//right:open, left:close
const int outIndexMax = ;//right:open, left:close
const int outOtherMax = ;//right:open, left:close
const int outThumbMin = ;//right:close, left:open
const int outIndexMin = ;//right:close, left:open
const int outOtherMin = ;//right:close, left:open
const int speedMax = ;
const int speedMin = ;
const int speedReverse = ;
const int thSpeedReverse = ;//0-100
const int thSpeedZero = ;//0-100
const boolean onSerial = ;

//Hardware
Servo servoIndex; //index finger
Servo servoOther; //other three fingers
Servo servoThumb; //thumb
int pinCalib; //start calibration
int pinGrasp; // change grasp mode
int pinThumb; // open/close thumb
int pinOther; //lock/unlock other three fingers
int pinSensor = A0; //sensor input

//Software
boolean isThumbOpen = 1;
boolean isOtherLock = 0;
boolean isGrasp = 1;
int swCount0,swCount1,swCount2,swCount3 = 0;
int sensorValue = 0; // value read from the sensor
int sensorMax = 700;
int sensorMin = 0;
int speed = 0;
int position = 0;
const int positionMax = 100;
const int positionMin = 0;
int prePosition = 0;
int outThumb,outIndex,outOther = 90;
int outThumbOpen,outThumbClose,outIndexOpen,outIndexClose,outOtherOpen,outOtherClose;

void setup() {
  Serial.begin(9600);
  if(isRight){
    pinCalib =  A6;
    pinGrasp =  A5;
    pinThumb =  A4;
    pinOther =  A3;
    outThumbOpen=outThumbMax; outThumbClose=outThumbMin;
    outIndexOpen=outIndexMax; outIndexClose=outIndexMin;
    outOtherOpen=outOtherMax; outOtherClose=outOtherMin;
  }
  else{
    pinCalib =  11;
    pinGrasp =  10;
    pinThumb =  8;
    pinOther =  7;
    outThumbOpen=outThumbMin; outThumbClose=outThumbMax;
    outIndexOpen=outIndexMin; outIndexClose=outIndexMax;
    outOtherOpen=outOtherMin; outOtherClose=outOtherMax;
  }
  servoIndex.attach(3);//index
  servoOther.attach(5);//other
  servoThumb.attach(6);//thumb
  pinMode(pinCalib, INPUT);//A6
  digitalWrite(pinCalib, HIGH);
  pinMode(pinGrasp, INPUT);//A5
  digitalWrite(pinGrasp, HIGH);
  pinMode(pinThumb, INPUT);//A4
  digitalWrite(pinThumb, HIGH);
  pinMode(pinOther, INPUT);//A3
  digitalWrite(pinOther, HIGH);
}

void loop() {
//==waiting for calibration==
  if(onSerial) Serial.println("======Waiting for Calibration======");
  while (1) {
    servoIndex.write(outIndexOpen);
    servoOther.write(outOtherOpen);
    servoThumb.write(outThumbOpen);
    if(onSerial) serialMonitor();
    delay(10);
    if (digitalRead(pinCalib) == LOW) {
      calibration();
      break;
    }
  }
 //==control==
  position = positionMin;
  prePosition = positionMin;
  while (1) {
    // ==Read Switch State==
    if (digitalRead(pinCalib) == LOW) swCount0 += 1;
    else swCount0 = 0;
    if (swCount0 == 10) {
      swCount0 = 0;
      calibration();
    }
    if (digitalRead(pinGrasp) == LOW) swCount1 += 1;
    else swCount1 = 0;
    if (swCount1 == 10) {
      swCount1 = 0;
      isGrasp = !isGrasp;
      while (digitalRead(pinGrasp) == LOW) delay(1);
    }
    if (digitalRead(pinThumb) == LOW) swCount2 += 1;
    else swCount2 = 0;
    if (swCount2 == 10) {
      swCount2 = 0;
      isThumbOpen = !isThumbOpen;
      while (digitalRead(pinThumb) == LOW) delay(1);
    }
    if (digitalRead(pinOther) == LOW) swCount3 += 1;//A3
    else swCount3 = 0;
    if (swCount3 == 10) {
      swCount3 = 0;
      isOtherLock = !isOtherLock;
      while (digitalRead(pinOther) == LOW) delay(1);
    }

    // ==Read Sensor Value==
    sensorValue = readSensor();
    delay(25);
    if(sensorValue<sensorMin) sensorValue=sensorMin;
    else if(sensorValue>sensorMax) sensorValue=sensorMax;
    sensorToPosition();

    // ==Drive servos==
    if (isGrasp) {
        outIndex = map(position, positionMin, positionMax, outIndexOpen, outIndexClose);
        servoIndex.write(outIndex);
    } else {
        outIndex = map(position, positionMin, positionMax, outIndexClose, outIndexOpen);
        servoIndex.write(outIndex);
    }
    if (!isOtherLock){
        if (isGrasp) {
            outOther = map(position, positionMin, positionMax, outOtherOpen, outOtherClose);
            servoOther.write(outOther);
        } else {
            outOther = map(position, positionMin, positionMax, outOtherClose, outOtherOpen);
            servoOther.write(outOther);
        }
    }
    if(isThumbOpen) servoThumb.write(outThumbOpen);
    else servoThumb.write(outThumbClose);
    if(onSerial) serialMonitor();
  }
}

/*
 * functions
 */
int readSensor() {
  int i, sval;
  for (i = 0; i < 10; i++) {
    sval += analogRead(pinSensor);
  }
  sval = sval/10;
  return sval;
}

void sensorToPosition(){
  int tmpVal = map(sensorValue, sensorMin, sensorMax, 100, 0);
  if(tmpVal<thSpeedReverse) speed=speedReverse;
  else if(tmpVal<thSpeedZero) speed=speedMin;
  else speed=map(tmpVal,40,100,speedMin,speedMax);
  position = prePosition + speed;
  if (position < positionMin) position = positionMin;
  if (position > positionMax) position = positionMax;
  prePosition = position;
}

void calibration() {
  outIndex=outIndexOpen;
  servoIndex.write(outIndexOpen);
  servoOther.write(outOtherClose);
  servoThumb.write(outThumbOpen);
  position=positionMin;
  prePosition=positionMin;

  delay(200);
  if(onSerial) Serial.println("======calibration start======");

  sensorMax = readSensor();
  sensorMin = sensorMax - 50;
  unsigned long time = millis();
  while ( millis() < time + 4000 ) {
    sensorValue = readSensor();
    delay(25);
    if ( sensorValue < sensorMin ) sensorMin = sensorValue;
    else if ( sensorValue > sensorMax )sensorMax = sensorValue;

    sensorToPosition();
    outIndex = map(position, positionMin, positionMax, outIndexOpen, outIndexClose);
    servoIndex.write(outIndex);

    if(onSerial) serialMonitor();
  }
  if(onSerial)  Serial.println("======calibration finish======");
}

void serialMonitor(){
  Serial.print("Serialnum="); Serial.print(Serialnum);
  Serial.print("Min="); Serial.print(sensorMin);
  Serial.print(",Max="); Serial.print(sensorMax);
  Serial.print(",sensor="); Serial.print(sensorValue);
  Serial.print(",speed="); Serial.print(speed);
  Serial.print(",position="); Serial.print(position);
  Serial.print(",outIndex="); Serial.print(outIndex);
  Serial.print(",isThumbOpen="); Serial.print(isThumbOpen);
  Serial.print(",isOtherLock="); Serial.println(isOtherLock);
}
