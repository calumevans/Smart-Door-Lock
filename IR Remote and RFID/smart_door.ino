/*
The servo was powered using a relay to avoid the constant sound of servo whine
*/
#include <Servo.h>
#include "IRremote.h"
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 53
#define reciever 11
#define power 10
#define RST_PIN 5
Servo motor;

#define degUnlock 100
#define degLock 0

int calumIdentity[] = {49,50,51,52,53,54,55,56,57};      //a unique number for user 'Calum'
int denied = 0;
char stopper = '$';

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
IRrecv irrecv(reciever);            // create instance of 'irrecv'
decode_results results;             // create instance of 'decode_results'


void translateIR(){                 //IR remote instructions
  switch(results.value){
    case 0xFFA25D: Serial.println("POWER button pressed:"); lock(); break;        //locks door
    case 0xFF629D: Serial.println("VOL+ button pressed:"); unlock(); break;           //unlocks door
    case 0xFFE21D: Serial.println("FUNC/STOP"); test(); break;    //puts into test sequence

    default: 
      break;

  }
  delay(200); // Do not get immediate repeat
}

void unlock(){                  //unlocking function
  digitalWrite(power,LOW);
  motor.write(degUnlock);
  delay(250);
  digitalWrite(power,HIGH);
  Serial.println("Door unlocked");
}

void lock(){                    //locking function
  digitalWrite(power,LOW);
  motor.write(degLock);
  delay(250);
  digitalWrite(power,HIGH);
  Serial.println("Door locked");
}

void test(){                   //testing servo sequence
    lock();
    delay(3000);
    unlock();
    delay(2000);
    
    Serial.println("Door in testing sequence");
}
void setup() {
  Serial.begin(9600);
  SPI.begin(); 
  Serial.println("-----------------------------------");
  Serial.println("Smart Door Lock 2019 by Calum Evans");
  Serial.println("-----------------------------------");
  Serial.println("PWR to lock || VOL+ to unlock");
  Serial.println("The correct RFID tag allows access");
  Serial.println(" ");

  irrecv.enableIRIn();            // Start the IR receiver
  mfrc522.PCD_Init();             // Init MFRC522 card
  motor.attach(12);               // Connects the servo
  pinMode(power, OUTPUT);  
}

void loop() {
  digitalWrite(power,HIGH);
  if (irrecv.decode(&results)){             // if there is an IR signal
    translateIR(); 
    irrecv.resume();
  }

//--------------------------------------------------REQUIRED RFID CODE
MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  byte block;
  byte len;
  MFRC522::StatusCode status;

  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

//------------------------------------------- READING NAME
  byte buffer1[18];
  block = 4;
  len = 18;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  status = mfrc522.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

//---------------------------------------- READING IDENTITY NUMBER
  byte buffer2[18];
  block = 1;

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(mfrc522.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
  status = mfrc522.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

//----------------------------------------------PRINTING DATA
  for (uint8_t i = 0; i >=0 ; i++){      //printing name -using an infinite for loop with a break when it reads '$' in the name
    if ((buffer1[i] != 32) && (buffer1[i] != stopper))      //same thing could be done with a while loop and a counter
      Serial.write(buffer1[i]);
    else
      break;
  }
  Serial.println(" requested access");
  Serial.print("Identity Number: ");
  for (uint8_t i = 0; i < 9; i++) {
    Serial.write(buffer2[i] );
  }
  Serial.println(" ");

//----------------------------------------------ALLOWING ACCESS OR NOT
  for(int i=0;i<9;i++){
    if(buffer2[i] != calumIdentity[i]){
      denied = denied + 1;
      break;
    }
  }

  if(denied){
    Serial.println("Access denied!");
  }else{
      Serial.print("Access granted: ");
      unlock();
      Serial.println(" ");
  }
  Serial.println(" ");
  denied = 0;

  delay(200);
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
