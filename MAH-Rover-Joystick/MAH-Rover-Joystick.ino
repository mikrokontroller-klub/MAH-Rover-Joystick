/*
 Name:		MAH_Rover_Joystick.ino
 Created:	3/29/2019 6:22:23 PM
 Author:	oliwerbig
*/

#include "WiFi.h"
#include "AsyncUDP.h"

#define UDPTIMEOUT 500

#define IN1 25
#define IN2 26
#define IN3 27
#define IN4 14

#define UDPort 1172

const char* ssid = "dlink";
const char* password = "";

long timeWentSinceLastPacket;

// Setting PWM properties
const int freq = 30000;
const int pwmChannel0 = 0;
const int pwmChannel1 = 1;
const int pwmChannel2 = 2;
const int pwmChannel3 = 3;
const int resolution = 8;
int dutyCycle0 = 0;
int dutyCycle1 = 0;
int dutyCycle2 = 0;
int dutyCycle3 = 0;

AsyncUDP udp;

void setup()
{
	ledcSetup(pwmChannel0, freq, resolution);
	ledcSetup(pwmChannel1, freq, resolution);
	ledcSetup(pwmChannel2, freq, resolution);
	ledcSetup(pwmChannel3, freq, resolution);
	ledcAttachPin(IN1, pwmChannel0);
	ledcAttachPin(IN2, pwmChannel1);
	ledcAttachPin(IN3, pwmChannel2);
	ledcAttachPin(IN4, pwmChannel3);
	Serial.begin(115200);
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	if (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.println("WiFi Failed");
		while (1) {
			delay(1000);
		}
	}
	if (udp.listen(UDPort)) {
		Serial.print("UDP Listening on IP: ");
		Serial.println(WiFi.localIP());
		udp.onPacket([](AsyncUDPPacket packet) {

			char receivedPayload[17];
			int receivedValues[4];

			Serial.print("UDP Packet Type: ");
			Serial.print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
			Serial.print(", From: ");
			Serial.print(packet.remoteIP());
			Serial.print(":");
			Serial.print(packet.remotePort());
			Serial.print(", To: ");
			Serial.print(packet.localIP());
			Serial.print(":");
			Serial.print(packet.localPort());
			Serial.print(", Length: ");
			Serial.print(packet.length());
			Serial.print(", Data: ");
			Serial.write(packet.data(), packet.length());
			Serial.println();

			for (int x = 0; x < packet.length(); x++) {
				receivedPayload[x] = byte(packet.data()[x]);
			}

			//Serial.println(receivedPayload);

			receivedValues[0] = String(receivedPayload).substring(0, 4).toInt();
			receivedValues[1] = String(receivedPayload).substring(4, 8).toInt();
			receivedValues[2] = String(receivedPayload).substring(8, 12).toInt();
			receivedValues[3] = String(receivedPayload).substring(12).toInt();

			/*Serial.println(receivedValues[0]);
			Serial.println(receivedValues[1]);
			Serial.println(receivedValues[2]);
			Serial.println(receivedValues[3]);*/

			doControllingStuff(receivedValues);
			});
	}
}

void loop()
{
	if (millis() - timeWentSinceLastPacket > UDPTIMEOUT)
	{
		controlLeftMotor('s', 0);
		controlRightMotor('s', 0);
	}
}

void doControllingStuff(int receivedValues[]) {
	if (receivedValues[1] < 1500) {
		controlLeftMotor('f', map(receivedValues[1], 1500, 1000, 150, 255));
	}
	else if (receivedValues[1] == 1500) {
		controlLeftMotor('s', 0);
	}
	else if (receivedValues[1] > 1500) {
		controlLeftMotor('b', map(receivedValues[1], 1500, 2000, 150, 255));
	}

	if (receivedValues[3] < 1500) {
		controlRightMotor('f', map(receivedValues[3], 1500, 1000, 150, 255));
	}
	else if (receivedValues[3] == 1500) {
		controlRightMotor('s', 0);
	}
	else if (receivedValues[3] > 1500) {
		controlRightMotor('b', map(receivedValues[3], 1500, 2000, 150, 255));
	}
}

void controlLeftMotor(char movingDirection, int pwmValue)
{
	switch (movingDirection)
	{
	case 'f':
		ledcWrite(pwmChannel2, 0);
		ledcWrite(pwmChannel3, pwmValue);
		break;
	case 's':
		ledcWrite(pwmChannel2, 0);
		ledcWrite(pwmChannel3, 0);
		break;
	case 'b':
		ledcWrite(pwmChannel2, pwmValue);
		ledcWrite(pwmChannel3, 0);
		break;
	}
}

void controlRightMotor(char movingDirection, int pwmValue)
{
	switch (movingDirection)
	{
	case 'f':
		ledcWrite(pwmChannel0, pwmValue);
		ledcWrite(pwmChannel1, 0);
		break;
	case 's':
		ledcWrite(pwmChannel0, 0);
		ledcWrite(pwmChannel1, 0);
		break;
	case 'b':
		ledcWrite(pwmChannel0, 0);
		ledcWrite(pwmChannel1, pwmValue);
		break;
	}
}
