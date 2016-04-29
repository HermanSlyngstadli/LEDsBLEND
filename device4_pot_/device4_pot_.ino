#include <RFduinoGZLL.h>

#define potR 2
#define potG 3
#define potB 4

int sensorR = 0;
int sensorG = 0;
int sensorB = 0;

device_t role = DEVICE4;

// Set up a structure for data packet transfer
struct transPacket {
  int red;
  int green;
  int blue;
  int hostDevice;
  int touchEnTo;
  int touchEnTre;
  int touchToTre;
} fraPot; // Create a structure named 'bokser';

void setup() {
  // put your setup code here, to run once:
  RFduinoGZLL.begin(role);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  sensorR = analogRead(potR);
  sensorG = analogRead(potG);
  sensorB = analogRead(potB);

  Serial.println(sensorR);
  
  fraPot.red = 255-map(sensorR, 0, 1023, 10, 240);
  fraPot.green = 255-map(sensorG, 0, 1023, 10, 240);
  fraPot.blue = 255-map(sensorB, 0, 1023, 10, 240);

  RFduinoGZLL.sendToHost((char *)&fraPot, sizeof(fraPot));

  delay(30);
}
