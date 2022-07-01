//include the required libraries 
//LIBRARY FILES

#include <Wire.h>       // this library allows you to communicate using I2C protocol.
#include "FS.h"                       // FS.h is files library which allows to use file system to store sketch data.
#include "MAX30100_PulseOximeter.h"  // This is a sensor library 
#define CAYENNE_PRINT Serial    // defining the cayenne serial print varible.
#include <CayenneMQTTESP8266.h>  //This library belongs to cayenne IoT platform
#define REPORTING_PERIOD_MS 10000         //  This global variable is set to 10secs.

//INITIALIZATIONS
PulseOximeter pox;    // initialization of sensor
float BPM, SpO2;    // initialization of sensor value variables 
uint32_t tsLastReport = 0;  // initialization of variable used in the loop
String hrInfo = "";   // inilialization of string variable
unsigned long timemsec =0;          // initialization of variable for time of reading in milliseconds.
 
                                
// WiFi Network Information
char ssid[] = "POCO X3";
char wifiPassword[] = "zxcvbnm@11";

// Information for Cayenne authentication. This is obtained from the Cayenne Dashboard.
char username[] = "fe6cdd50-b1e4-11ec-8c44-371df593ba58";
char password[] = "aec28d166375ec0e7f84ba151c993a813a9aeba0";
char clientID[] = "9432d200-b202-11ec-8da3-474359af83d7";


// This function is called as soon as the beat is detected.
void onBeatDetected()
{
    Serial.println("Beat Detected!");
}

void setup() {
  Serial.begin(115200); // Serial Monitor setup
  pinMode(2, OUTPUT);  // GPIO2 pin on ESP8266 is set high
  Cayenne.begin(username, password, clientID, ssid, wifiPassword); // Cayenne Setup
  Serial.print("Initializing Pulse Oximeter..");
 //pinMode(16, OUTPUT);
    if (!pox.begin())
    {
         Serial.println("Initialization FAILED");
         for(;;);
    }
    else
    {
         Serial.println("Initialization SUCCESSFUL");
         digitalWrite(2, HIGH);   //Turn off the in-built LED
     }
   pox.setOnBeatDetectedCallback(onBeatDetected);
                                //50mA is the default current for the IR LED. 
                                //Below statement can be changed according to the skin color of the patient and other parameters.
   pox.setIRLedCurrent(MAX30100_LED_CURR_24MA);

    if(!SPIFFS.begin()){
    Serial.println("There is an error while mounting SPIFFS");
    return;
  }
 
}
 
void loop() {

  pox.update();   // This statement updates the value which is done by the sensor.
  
  BPM = pox.getHeartRate();   //Heart rate value is store in the BPM variable by calling the sensor function.
  SpO2 = pox.getSpO2();     //SpO2% value is stored in the SpO2 variable by calling the sensor function. 

  if (BPM < 40 || SpO2 == 0) {                                      // Neglects low readings and starts loop again.
  //it returns if the BPM and SpO2 are low
  return;
  }
  
   if (millis() - tsLastReport > REPORTING_PERIOD_MS)
  {
  digitalWrite(2, LOW); // In-Built LED is ON while reading the values.                                             // Turn ON LED everytime reading is saved
   // Cayenne.loop();
  Serial.print("Heart rate: ");  Serial.print(BPM);
  timemsec = millis();
  hrInfo = String(timemsec) + String(",") + String(BPM) + String(",") + String(SpO2);                  //convert variable from integer to string
  Serial.print(" bpm , SpO2:");  Serial.print(SpO2);  Serial.println(" %");
  Serial.println (hrInfo);

  File file = SPIFFS.open("/HR_test.txt", "a");                    // replace by your file name. Open file as w-overwrite/ a-append
  Cayenne.virtualWrite(0, BPM, "counter","null" );    // Heart Rate value is sent to the Cayenne IoT platform.
  Cayenne.virtualWrite(1, SpO2, "O2", "p");     //Spo2 value is sent to the cayenne IoT platform
  
  if(!file){
    Serial.println("Failed to open file for writing");
    return;
  }
  file.println (hrInfo);    // BPM are SpO2 values are printed in file
  file.close();   // file is closed
 
  digitalWrite(2, HIGH);    // in-built LED is turned off                                         //Turn OFF
  
  tsLastReport = millis();
  }
  
}
