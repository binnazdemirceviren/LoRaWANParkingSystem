/*
 * LoRa E32-TTL-100
 *
 * E32-TTL-100----- Arduino UNO or esp8266
 * M0         ----- 3.3v (To config) GND (To send) 7 (To dinamically manage)
 * M1         ----- 3.3v (To config) GND (To send) 6 (To dinamically manage)
 * TX         ----- RX PIN 2 (PullUP)
 * RX         ----- TX PIN 3 (PullUP & Voltage divider)
 * AUX        ----- Not connected (5 if you connect)
 * VCC        ----- 3.3v/5v
 * GND        ----- GND
 *
 */
#include "Arduino.h"
#include "LoRa_E32.h"

// ---------------- Arduino pins -------------------
//LoRa_E32 e32ttl(2, 3, 5, 7, 6);
LoRa_E32 e32ttl(2, 3); // Config without connect AUX and M0 M1

//#include <SoftwareSerial.h>
//SoftwareSerial mySerial(2, 3); // Arduino RX <-- e32 TX, Arduino TX --> e32 RX
//LoRa_E32 e32ttl(&mySerial, 5, 7, 6);
// -------------------------------------------------

// ---------- Sensor pins & variables --------------
#define trig 8
#define echo 9

long distance = 0;
long deltaTime = 0;

int sensorStatus = 0;
int tempStatus = 1;
// -------------------------------------------------

// --------------- Adress Variables ----------------
byte brodcastAdr[2] = {0,244}; // adrH, adrL
//byte myAdr[2] = {2,56}; // adrH, adrL //568
//byte myAdr[2] = {0,251}; // adrH, adrL //251
byte myAdr[2] = {1,16}; // adrH, adrL //272
// -------------------------------------------------

void sendMessageToBrodcast(byte code, byte adrH, byte adrL);
int getSensorStatus();
int delayedSensorResult();
bool listenAck();

void setup(){
  	pinMode(trig, OUTPUT);
  	pinMode(echo, INPUT);
  
	Serial.begin(9600);
	delay(100);

	e32ttl.begin();
}

struct Message{
    byte code;
    byte adrH;
    byte adrL;
} message;


void loop(){
	sensorStatus = getSensorStatus();
	delay(100);

	if(tempStatus != sensorStatus){
		sensorStatus = delayedSensorResult();
		if(tempStatus != sensorStatus){
			//bool ackRecieved = false;
			sendMessageToBrodcast((byte)sensorStatus, myAdr[0], myAdr[1]);
			/*while(!ackRecieved){
				ackRecieved = listenAck();
				if(ackRecieved)
					Serial.println("XXX");
			}*/
			tempStatus = sensorStatus;
		}
	}
}

void sendMessageToBrodcast(byte code, byte adrH, byte adrL){  
  	Message msg = {code,adrH,adrL};
  
  	ResponseStatus rs = e32ttl.sendFixedMessage(brodcastAdr[0],brodcastAdr[1],23,&msg, sizeof(Message));
  	Serial.println(rs.getResponseDescription());
}

int getSensorStatus(){
	digitalWrite(trig, HIGH);
	delayMicroseconds(1000);
	digitalWrite(trig, LOW);
  
	deltaTime = pulseIn(echo, HIGH);
	distance = (deltaTime / 29.1) / 2;
	if(distance > 100)
		distance = 100;
	if(distance < 50)
		return 2; //Dolu
	else
    	return 1; //Boş
}

int delayedSensorResult(){
	int oneCount = 0;
	int twoCount = 0;
    while ((oneCount + twoCount) < 9){
		int temp = getSensorStatus();
		if (temp == 2)
            twoCount++;
        else if(temp == 1)
            oneCount++;
		delay(100);
	}
	
    if (twoCount > oneCount)
		return 2;
	else
		return 1;
}

bool listenAck(){
	if (e32ttl.available()>1){
		ResponseStructContainer rsc = e32ttl.receiveMessage(sizeof(Message));
		Message msg = *(Message*) rsc.data;

		if (rsc.status.code != 1){
    		rsc.status.getResponseDescription();
		}else if(msg.code == 0){ //ACK Type Message
    		Serial.println("Gönderilen mesaj yerine ulasti.");
    		return true;
    	}
  	}
  	return false;
}
