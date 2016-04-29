#include <RFduinoGZLL.h>

// Initialization
#define MAX_DEVICES 4 // Sets the max amount of devices in the host-device-loop

//define the ports for the colors
#define RED 2
#define GREEN 3
#define BLUE 4 

// Sets he ID for the device
device_t role = DEVICE2;

int erHost = 0; // flag for host state

int flagCounterHost = 0; //flag for starting the counter thing, see next line
int startCounterHost = 0; //start time for host counter system thingy (explain later, promise)

int returnToSender = 70; //How long after first signal from host are you going to change to host

int soneInne = -36;
int soneBorte = -44;

int isRed = 0; // Here we flag over a low shoe, yes, yes
int isGreen = 0; // These are for fading and trying to make a more stable lightcombo
int isBlue = 0;

// RSSI total and count for each deveice for averaging
int rssi_total[MAX_DEVICES];
int rssi_count[MAX_DEVICES];

// Collect samples flag
int collect_samples = 0;

// Set up a structure for data packet transfer
struct transPacket {
  int red;
  int green;
  int blue;
  int hostDevice;
  int touchEnTo;
  int touchEnTre;
  int touchToTre;
} bokser, fraPot; // Create a structure named 'bokser';

void setup() {
  Serial.begin(9600);
  if (role != 1) {
    RFduinoGZLL.begin(role);
  } else {
    RFduinoGZLL.begin(HOST);
  }
  
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  analogWrite(RED, 255);
  analogWrite(GREEN, 255);
  analogWrite(BLUE, 255);
}

void loop() {
  // Sjekk om man skal skifte til host
  if (millis() - startCounterHost >= 1000) {
    flagCounterHost = 0; // Something is fucky, reset counter and all the shit... damn...
  }
  
  if (role != 1) {
    if (millis() - startCounterHost >= returnToSender+80 && flagCounterHost) {
      erHost = 1;
      flagCounterHost = 0;
    }
  } else {
    if (!flagCounterHost) {
      startCounterHost = millis();
      flagCounterHost = 1;
    } else {
      if (millis() - startCounterHost >= 750) {
        //Serial.println(millis() - startCounterHost);
        erHost = 1;
        flagCounterHost = 0;
      }
    }
  }
  
  if (erHost) {
    RFduinoGZLL.end(); // End device role and start as host
    RFduinoGZLL.begin(HOST);
    //digitalWrite(RED,LOW);
    
    //Serial.println("Jeg er host!");

    byttHostVar();
    resetRSSIavg();
    
    collect_samples = 1;
    delay(100);
    collect_samples = 0;

    //Serial.println(rssi_total[3]/rssi_count[3]);

    findTouch(); // Calculate and flag if two boxes are close
    //Serial.println(bokser.touchEnTo);
    delay(returnToSender);
    
    erHost = 0;
    //digitalWrite(RED, HIGH);
    RFduinoGZLL.end();
    RFduinoGZLL.begin(role);
  } else {
    delay(10);
    RFduinoGZLL.sendToHost(NULL, 0);
    //Serial.println("Jeg er device!");
    //Serial.println(millis() - startCounterHost);
  }
  //Serial.println(bokser.red);
  //Serial.println(bokser.green);
  changeColor(fraPot.red, fraPot.green, fraPot.blue);
}


void RFduinoGZLL_onReceive(device_t device, int rssi, char *data, int len) {
  // if collecting samples, update the RSSI total and count
  if (collect_samples) {
    rssi_total[device] += rssi;
    rssi_count[device]++;
    return;
  }
  
  if (device == 4) {
    memcpy(&fraPot, &data[0], len); // Copy the data sent from host to "bokser"
    
    bokser.red = fraPot.red;
    bokser.green = fraPot.green;
    bokser.blue = fraPot.blue;
    
    return;
  }
  
  // What to do if device is host and if not
  //Serial.println(device);
  if (erHost) {
    RFduinoGZLL.sendToDevice(device, (char *)&bokser, sizeof(bokser)); // Send updated data structure to devices
  } else if (len != 0) {
    memcpy( &bokser, &data[0], len); // Copy the data sent from host to "bokser"
    Serial.println(bokser.blue);
    //Serial.println(bokser.touchToTre);
  }
  
  if (role == bokser.hostDevice) {
    if (!flagCounterHost && len != 0) {
      //Serial.println("Du har kommet riktig");
      flagCounterHost = 1;
      startCounterHost = millis();
    }
  }
}

void resetRSSIavg() {
  int i;

  // reset the RSSI averaging for each device
  for (i = 0; i < MAX_DEVICES; i++) {
    rssi_total[i] = 0;
    rssi_count[i] = 0;
  }
}

void findTouch(){
  int i;

  // calculate the RSSI averages for each devies
  int average[MAX_DEVICES];

  for (i = 1; i < MAX_DEVICES; i++) {
    // no samples received, set to lowest RSSI
    // does also prevent dividing by zero
    if (rssi_count[i] == 0) {
      average[i] = -128;
    } else {
      average[i] = rssi_total[i] / rssi_count[i];
    }
  }
  // Register if close enough and handeling
  //Serial.println(average[2]);
  if (role == 1) {
      //Serial.println(average[3]);
    
    if (average[2] > soneInne) {
      bokser.touchEnTo = 1;
    } else if (average[2] < soneBorte) {
      bokser.touchEnTo = 0;    }

    if (average[3] > soneInne) {
      bokser.touchEnTre = 1;
    } else if (average[3] < soneBorte) {
      bokser.touchEnTre = 0;
    }
    
  } else if (role == 2) {
    if (average[1] > soneInne) {
      bokser.touchEnTo = 1;
    } else if (average[1] < soneBorte) {
      bokser.touchEnTo = 0;
    }

    if (average[3] > soneInne) {
      bokser.touchToTre = 1;
    } else if (average[3] < soneBorte) {
      bokser.touchToTre = 0;
    }
    
  } else if (role == 3) {
    
    if (average[1] > soneInne) {
      bokser.touchEnTre = 1;
    } else if (average[1] < soneBorte) {
      bokser.touchEnTre = 0;
    }

    if (average[2] > soneInne) {
      bokser.touchToTre = 1;
    } else if (average[2] < soneBorte) {
      bokser.touchToTre = 0;
    }
  }
}

void byttHostVar() {
  if (role == 1) {
    bokser.hostDevice = 2;
  } else if (role == 2) {
    bokser.hostDevice = 3;
  } else {
    //Serial.println("Venter på ny loop! Fucker.");
  }
}

void changeColor(int redValue, int greenValue, int blueValue) {
  int av = 255;
  int colors[] = {redValue, greenValue, blueValue};
  
  analogWrite(role+1, colors[role-1]);

  if (bokser.touchEnTo && bokser.touchEnTre || bokser.touchEnTo && bokser.touchToTre || bokser.touchEnTre && bokser.touchToTre) {
    analogWrite(2, colors[0]);
    analogWrite(3, colors[1]);
    analogWrite(4, colors[2]);

    return;
  }

  //Skru på riktig lys
  if (bokser.touchEnTo && role != 3) {
    analogWrite(4-role, colors[2-role]);
  } else if (role != 3) {
    analogWrite(4-role, av);
  }

  if (bokser.touchEnTre && role != 2) {
    analogWrite(5-role, colors[3-role]);
  } else if (role != 2) {
    analogWrite(5-role, av);
  }

  if (bokser.touchToTre && role != 1) {
    analogWrite(6-role, colors[4-role]);
  } else if (role != 1) {
    analogWrite(6-role, av);
  }
}

void fadeTo (int pin, int verdi) {
  for (int i = 0; i <= verdi; i += 10) {
    //analogWrite(pin, i);
    Serial.println(i);
    delay(5);
  }
}

