#include <Wire.h>
#include <BH1750.h>
#include <Servo.h>
// #include <TimerOne.h>

// PCA9548A I2C address
#define PCA9548A_ADDRESS 0x70

// BH1750 I2C address
#define BH1750_ADDRESS 0x23

// BH1750 object
BH1750 lightMeter1(BH1750_ADDRESS);
BH1750 lightMeter2(BH1750_ADDRESS);
BH1750 lightMeter3(BH1750_ADDRESS);
BH1750 lightMeter4(BH1750_ADDRESS);

// BH1750 Channel
int lightMetersCh[4] = {0,1,3,7};
// BH1750 sensor variables
float lightLevels[4];

// Servo motor variables
Servo servoZ;
Servo servoX;
int servoZPos = 65;
int servoXPos = 45;
int lastservoZPos = 65;
int lastservoXPos = 45;

// Servo motor speed
int defstep = 20;
int step = 20;

// Control variables
int maxLightLevel = 0;
int maxLightSensor = 0;
float average = 0;
int flag = 0;

// variables for distance
float dist[4];

// variables for position
float diff = 150;
float diffLR = 0;
float diffUD = 0;

// Interval between readings
unsigned long interval = 500000;
unsigned long previousMillis = 0;

void setup() {

  // Initialize serial communication
  Serial.begin(9600);

  // Initialize I2C communication
  Wire.begin();

  // Initialize BH1750 light sensors and PCA9548A
  initBH1750(lightMeter1, lightMetersCh[0]);
  initBH1750(lightMeter2, lightMetersCh[1]);
  initBH1750(lightMeter3, lightMetersCh[2]);
  initBH1750(lightMeter4, lightMetersCh[3]);

  // Initialize the servo motors
  servoZ.attach(2);
  servoX.attach(3);
  servoZ.write(servoZPos);
  servoX.write(servoXPos);

  Serial.println(" Calibration's Done! ");

  delay(3000);

  // Initialize Timer1 attachInterrupt
  // Timer1.initialize(interval);
  // Timer1.attachInterrupt(Control);
}

void loop(){
  Control();
  delay(100);
}

void Control(){

  // Read the light levels from the BH1750 sensors
  lightLevels[0] = readLightLevels(lightMeter1,lightMetersCh[0]);
  lightLevels[1] = readLightLevels(lightMeter2,lightMetersCh[1]);
  lightLevels[2] = readLightLevels(lightMeter3,lightMetersCh[2]);
  lightLevels[3] = readLightLevels(lightMeter4,lightMetersCh[3]);

  dist[0] = (lightLevels[0]+lightLevels[1])/2;  //Left
  dist[1] = (lightLevels[0]+lightLevels[3])/2;  //UP
  dist[2] = (lightLevels[2]+lightLevels[3])/2;  //Right
  dist[3] = (lightLevels[1]+lightLevels[2])/2;  //Down

  // Find the sensor with the highest light level
  maxLightLevel = 0;
  maxLightSensor = 0;
  average = 0;

  for (int i = 0; i < 4; i++) {
    // average = average + lightLevels[i];
    if (dist[i] > maxLightLevel) {
      maxLightLevel = dist[i];
      maxLightSensor = i;
    }
  }
  // average = average/4;
  step = defstep;
  
  diffLR = abs(dist[0]-dist[2]);
  diffUD = abs(dist[1]-dist[3]);
  if ((diffLR > diff ))
    step = 50;
  if ((diffUD > diff ))
    step = 50;
  
  WriteServo();

}

void WriteServo(){
  // Determine the position of the light source
  switch (maxLightSensor) {
    case 0:
      // sensor 1 is highest, up and right
      servoZPos = servoZPos + diffLR/step;
      break;
    case 1:
      // sensor 2 is highest, down and right
      servoXPos = servoXPos + 4*diffUD/step;
      break;
    case 2:
      // sensor 3 is highest, down and left
      servoZPos = servoZPos - diffLR/step;
      break;
    case 3:
      // sensor 4 is highest, up and left
      servoXPos = servoXPos - 4*diffUD/step;
      break;
    case 4:
      // safe zone
      servoXPos = servoXPos ;
      servoZPos = servoZPos ;
      break;
  }

  // limit the servo positions to the maximum allowed range
  servoZPos = constrain(servoZPos, 10, 125);
  servoXPos = constrain(servoXPos, 5, 85);
  // write the new servo positions
  servoZ.write(servoZPos);
  servoX.write(servoXPos);

  Serial.print(" LR: ");
  Serial.print(diffLR);
  Serial.print(" | UD: ");
  Serial.println(diffUD);
}

void initBH1750(BH1750 &lightMeter, uint8_t channel) {
  // Initialize BH1750 (Sensor 1)
  Wirebegins(channel);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE_2);
  Serial.println("BH1750 Sensors : CHECK");
}

void Wirebegins(uint8_t channel) {
  // Select channel on PCA9548A
  Wire.beginTransmission(PCA9548A_ADDRESS);
  Wire.write(1 << channel);
  Wire.endTransmission();
  Serial.println("Wires : CHECK");
}

float readLightLevels(BH1750 &lightMeter, uint8_t channel) {
  // Select channel on PCA9548A
  Wire.beginTransmission(PCA9548A_ADDRESS);
  Wire.write(1 << channel);
  Wire.endTransmission();
  // Read light level
  return lightMeter.readLightLevel();
}