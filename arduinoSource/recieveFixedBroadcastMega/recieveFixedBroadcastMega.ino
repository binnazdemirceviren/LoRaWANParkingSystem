#include "Arduino.h"
#include "LoRa_E32.h"

#include <Ethernet.h>
#include "secrets.h"
#include "ThingSpeak.h" 

// ---------------- Arduino pins -------------------

LoRa_E32 e32ttl(2, 3); // Config without connect AUX and M0 M1

// -------------------------------------------------

// --------------- Adress Variables ----------------

byte brodcastAdr[2] = {0,244}; //adrH adrL

// -------------------------------------------------

// --------------- Ethernet Variables ----------------

byte mac[] = SECRET_MAC;

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);

EthernetClient client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;

// ---------------------------------------------------


int MsgToCode(byte msgCode, int adr);

void setup() {
  Serial.begin(9600);

  // Startup all pins and UART
  e32ttl.begin();

  Serial.println();
  Serial.println("Start listening!"); 

  Ethernet.init(10);  // Most Arduino Ethernet hardware
      
  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }
  // give the Ethernet shield a second to initialize:
  delay(1000);
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  Serial.println("Ready to go!");
}

struct Message {
    byte code;
    byte adrH;
    byte adrL;
} message;

void loop() {
  // If something available
  if (e32ttl.available()>1) {

    Serial.println("Lora is Available to recive");
    // read the String message
    ResponseStructContainer rsc = e32ttl.receiveMessage(sizeof(Message));
    Message msg = *(Message*) rsc.data;

    // Is something goes wrong print error
      if (rsc.status.code!=1){ 
        rsc.status.getResponseDescription();
      }else{

        // Print the data received
        /*Message responseMsg = {0,brodcastAdr[0],brodcastAdr[1]};
      
        ResponseStatus rs = e32ttl.sendFixedMessage(msg.adrH,msg.adrL,23,&responseMsg, sizeof(Message));
        Serial.print("Mesaj tanindi mi? : ");
        Serial.println(rs.getResponseDescription());
        */
        //Geri kalanı gelen mesajı yorumlamak.

        Serial.print("Mesaj kodu : ");
        Serial.println(msg.code);
        Serial.print("Mesaj Adressi : ");
        int msgAdress = (int)msg.adrH * 256 + msg.adrL;
        Serial.println(msgAdress);

        int code = MsgToCode(msg.code, msgAdress);

        ThingSpeak.setField(1, code);
        int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
          if(x == 200){
            Serial.println("Channel update successful.");
          }
          else{
            Serial.println("Problem updating channel. HTTP error code " + String(x));
          }
        
        rsc.close();
    }
  }
}

int MsgToCode(byte msgCode, int adr ){
  int code = msgCode * 1000;
  code = code + adr;
  return code;
}
