#include <ESP32Servo.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>



//######################## TIME #############################
#include <time.h>
#include <sys/time.h>

const char* ntpServer = "asia.pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;


#include <sys/time.h>
#include <time.h>

char lsuuu[30];
// strftime(buffer, 26, "%Y:%m:%d %H:%M:%S", tm_info);

#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

String printLocalTime() {
  char buffer[26];
  int millisec;
  struct tm* tm_info;
  struct timeval tv;

  gettimeofday(&tv, NULL);

  millisec = lrint(tv.tv_usec/1000.0); // Round to nearest millisec
  if (millisec>=1000) { // Allow for rounding up to nearest second
    millisec -=1000;
    tv.tv_sec++;
  }

  tm_info = localtime(&tv.tv_sec);

  strftime(buffer, 26, "%Y:%m:%d %H:%M:%S", tm_info);
  return buffer ;
}
// String printLocalTime()
// {
//   struct tm timeinfo;
//   if(!getLocalTime(&timeinfo)){
//     Serial.println("Failed to obtain time");
//     return;
//   }
//   char lsuuu[30];
//   strftime(lsuuu, 26, "%Y:%m:%d %H:%M:%S", tm_info);
//   return &timeinfo, "%A, %B %d %Y %H:%M:%S";
  


//#####################################################

// FIREBASE
#include<WiFi.h>;
#include<Firebase_ESP_Client.h>;
#include "addons/TokenHelper.H";
#include "addons/RTDBHelper.H";

#define WIFI_SSID "Sathappan_lap"
#define WIFI_PASSWORD "12345678"
#define API_KEY "AIzaSyB5bJqmsRRqf5OQpTGBtNFFhcrjGBXTEb4"
#define DATABASE_URL "https://smartparking-d872f-default-rtdb.firebaseio.com/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config; 

unsigned long sendDataPrevMillis = 0;
bool singupOK = false; 
// #######################################################################################

#define OLED_SDA 22
#define OLED_SCL 21

Adafruit_SH1106 display(22, 21);
Servo myservo; 
int pos = 0,IR_in = 4,IR_out = 23,Car_slot_1 = 26,Car_slot_2 = 27;    // variable to store the servo position

void setup() {
  Serial.begin(9600);
//############################## TIME ##########################
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

//##################################################################################
  WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
  Serial.print("Connecting to WiFi :");
  while(WiFi.status()!= WL_CONNECTED){
    Serial.print(" Not connetcted with a WIFI");
    delay(300);
  }  
  Serial.println();Serial.print("Connetcted with WIFI");
  Serial.println(WiFi.localIP());
  Serial.println();
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  if(Firebase.signUp(&config, &auth, "", "")){
    Serial.print("Singup OK");
    singupOK = true;    
  }
  else{
    Serial.printf("%s\n",config.signer.signupError.message.c_str());
  }
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config,&auth);
  Firebase.reconnectWiFi(true);      
//##################################################################################
  
  pinMode(IR_in,INPUT);
  pinMode(IR_out,INPUT);
  pinMode(Car_slot_2,INPUT);
  pinMode(Car_slot_1,INPUT);
  myservo.attach(5); 
  myservo.write(90);
  
  display.begin(SH1106_SWITCHCAPVCC, 0x3C); 
  display.println("IOT");
  display.println("Parking Slot");
  display.clearDisplay();
  }


void loop() {
  printLocalTime();  
  // to check avaiable slot
  if ((digitalRead(Car_slot_1)==0) and (digitalRead(Car_slot_2)==0)){
      display.println("Slots Filled");  
  }
  else if (((digitalRead(Car_slot_1)==0) or (digitalRead(Car_slot_2)==0))){
    display.println("Avaiable slots : 1");  
  }    
  else{
    display.println("Avaiable slots : 2");  
  }
  display.display();
  // delay(2000);
  display.clearDisplay();  

// to open servo  
  if ((digitalRead(IR_in)==0) or (digitalRead(IR_out)==0))
  {
    if ((digitalRead(Car_slot_1)==0) and (digitalRead(Car_slot_2)==0)) // 0 1 open 1 0 open 11 close
    {
      Serial.println("car parking full");
    }
    else
    {
      Serial.println("car parking free");
      myservo.write(0);
      delay(1000);
      Serial.println("Object Detected in");
    }
  }
  else
  {
    Serial.println("Object not Detected");    
    myservo.write(90);
  }  

// pirticular slot
  if (digitalRead(Car_slot_1)==0){
      display.println("Slot 1 : Full");  
  }
  else{
    display.println("Slot 1 : Free");
  }  
  if (digitalRead(Car_slot_2)==0){
      display.println("Slot 2 : Full");  
  }
  else{
    display.println("Slot 2 : Free");
  }  
//###############################################################################
  int carslot[2]={Car_slot_1,Car_slot_2};
  char park_str[][15] = {"Not Avaiable","Avaiable"}; 
  
  char path_slot[][50]={"Parking/Slot_1/Slot_One","Parking/Slot_2/Slot_Two"};
  char entry[][50]={"Parking/Slot_1/En_Time","Parking/Slot_2/En_Time"};

  for (int I=0;I<2;I++)
  {if(Firebase.ready() && singupOK){
      int park = digitalRead(carslot[I]);
       
      if(Firebase.RTDB.setString(&fbdo,path_slot[I],park_str[park])){  
        if(Firebase.RTDB.setString(&fbdo,entry[I],printLocalTime())){             
          Serial.println();               
        }
        else{
          Serial.println("Failed: "+fbdo.errorReason());
        }              
      Serial.println();               
      }
      else{
        Serial.println("Failed: "+fbdo.errorReason());
      }
       
    }
  }
//###############################################################################
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.setTextSize(1.95);
  display.println("Total Slots : 2");
} 

