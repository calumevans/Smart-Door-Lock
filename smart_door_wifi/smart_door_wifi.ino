/*
 * -my ESP8266's serial monitor just spits out garbage (unusable)
 * -this is why there are no serial outputs (much more difficult for troubleshooting!)
 * -this is the first usable version of the smart lock with wifi, will be a made  a lot nicer in the future
 * 
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Servo.h>

Servo motor;
#define degUnlock 100
#define degLock 0
 
const char* ssid = "XXXXXXXX";    //enter the WiFi SSID and password 
const char* password = "XXXXXXXX";

ESP8266WebServer server(80); //Declaring a global object variable from the ESP8266WebServer on port 80

//---------------------------------------------------------------HTML SOURCE CODE
const char MAIN_page[] PROGMEM = R"=====(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">

  <title>Smart Door Lock</title>
  <meta name="description" content="Smart Door Lock">
  <meta name="author" content="Calum Evans">
  <link rel="stylesheet" href="css/styles.css?v=1.0">
</head>
<body>
  <script src="js/scripts.js"></script>  
  <center>
  <h1>CALUM'S AMAZING SMART HOUSE!!</h1><br>
  Click to lock the door <a href="lock">LOCK</a><br>
  Click to unlock the door <a href="unlock">UNLOCK</a><br>
  <hr>
  </center>  
</body>
</html>
)=====";
 
void handleRoot(){                       //when the ESP8266's IP address is opened
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}
 
void handleLOCK(){                        //when the lock button is pressed
 lock();
 server.send(200, "text/html", "DOOR LOCKED"); //Send ADC value only to client ajax request
}
 
void handleUNLOCK(){                     //when the unlock button is pressed
 unlock();
 server.send(200, "text/html", "DOOR UNLOCKED");
}

void unlock(){                  //unlocking function
  motor.write(degUnlock);
  delay(500);
}

void lock(){                    //locking function
  motor.write(degLock);
  delay(500);
}

//---------------------------------------------------------------SETUP
void setup(void){
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  motor.attach(2);               // Connects the servo
  
  while (WiFi.status() != WL_CONNECTED) {     // Wait for connection
    delay(500);
  }
 
  server.on("/", handleRoot);      //these are the dirrent webpages (buttons)
  server.on("/lock", handleLOCK);
  server.on("/unlock", handleUNLOCK);
 
  server.begin();                  //Start server
}

//---------------------------------------------------------------LOOP
void loop(void){
  server.handleClient();          //Handle client requests
}
