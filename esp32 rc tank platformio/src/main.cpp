#include <Arduino.h>

#include "AsyncTCP.h"
#include "ESPAsyncWebServer.h"
#include <TFT_eSPI.h>  

#include "networkCredentials.h"

#include "FastAccelStepper.h"

#define dirPinStepper_right   	2 
#define enablePinStepper_right	32
#define stepPinStepper_right	15

#define dirPinStepper_left		13 
#define enablePinStepper_left	32
#define stepPinStepper_left		12

AsyncWebServer server(80);
AsyncWebSocket ws("/ws"); // access at ws://[esp ip]/ws
AsyncEventSource events("/events"); // event source (Server-Sent events)

#define IWIDTH  240
#define IHEIGHT 135

TFT_eSPI tft = TFT_eSPI(135, 240);

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepper_right = NULL;
FastAccelStepper *stepper_left = NULL;


void onRequest(AsyncWebServerRequest *request)
{
	//Handle Unknown Request
	request->send(404);
}

void onBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
	//Handle body
}

void onUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
	//Handle upload
}


int topHz = 15000;
int reducedHz = 7000;

int topAcceleration = 50000;
int reducedAcceleration = 19000;

int maxSpeed = topHz;
int deadZone = 7;

int applyDeadZone(int input, int deadZone)
{
	if(input < deadZone && input > -deadZone)
		return 0;
	return input > 0 ? input - deadZone : input + deadZone;
}

int getAccelerationForSpeed(int hz)
{
	if(maxSpeed == reducedHz)
		return reducedAcceleration;

	return topAcceleration;
}

void setStepperTarget(FastAccelStepper* stepper, int target)
{
	target = applyDeadZone(target, deadZone);
	if(target == 0)
	{
		stepper->stopMove();
		return;
	}
	
	bool right = stepper == stepper_right;

	//stepper->applySpeedAcceleration()
	//stepper->setAcceleration(getAccelerationForSpeed(stepper->getCurrentSpeedInMilliHz()/1000));
	stepper->setSpeedInHz(maxSpeed*abs(target)/100);

	if(target > 0)
	{
		if(right)
			stepper->runForward();
		else
			stepper->runBackward();
	}
	else
	{
		if(right)
			stepper->runBackward();
		else
			stepper->runForward();
	}
}

void setNewTarget(bool arrowUp, bool arrowRight, bool arrowDown, bool arrowLeft, int8_t *joystickStates, int8_t mode)
{
	int target_right = 0;
	int target_left = 0;

	maxSpeed = mode == 1 ? reducedHz : topHz;

	if(mode == 2)
	{
		target_right = -joystickStates[4];
		target_left = -joystickStates[1];
	}

	if(mode == 0 || mode == 1)
	{
		maxSpeed = mode == 0 ? topHz : reducedHz;

		int diff = applyDeadZone(joystickStates[0], 7);
		int drive = +(joystickStates[5]+100)/2 - (joystickStates[2]+100)/2;

		if(drive < 0)
		{
			diff = -diff;
		}

		tft.drawString(String(diff), 5, 50);
		float ratio = (101 - abs(diff)) / 101.0;

		target_right = drive*(diff > 0 ? ratio : 1);
		target_left = drive*(diff < 0 ? ratio: 1);
	}

	if(mode == 3)
	{
		// forward
		if(arrowUp && !arrowRight && !arrowLeft && !arrowDown)
		{
			target_right = 100;
			target_left = 100;
		}
		// turn right
		if(arrowUp && arrowRight && !arrowLeft && !arrowDown)
		{
			target_right = 0;
			target_left = 100;
		}
		// turn left
		if(arrowUp && !arrowRight && arrowLeft && !arrowDown)
		{
			target_right = 100;
			target_left = 0;
		}
		// rotate right
		if(!arrowUp && arrowRight && !arrowLeft && !arrowDown)
		{
			target_right = -100;
			target_left = 100;
		}
		// rotate left
		if(!arrowUp && !arrowRight && arrowLeft && !arrowDown)
		{
			target_right = 100;
			target_left = -100;
		}
		// reverse
		if(!arrowUp && !arrowRight && !arrowLeft && arrowDown)
		{
			target_right = -100;
			target_left = -100;
		}
		// reverse right
		if(!arrowUp && arrowRight && !arrowLeft && arrowDown)
		{
			target_right = 0;
			target_left = -100;
		}
		// reverse left
		if(!arrowUp && !arrowRight && arrowLeft && arrowDown)
		{
			target_right = -100;
			target_left = 0;
		}
		// stop
		if(!arrowUp && !arrowRight && !arrowLeft && !arrowDown)
		{
			target_right = 0;
			target_left = 0;
		}
	}

	if(mode == 4)
	{
		target_right = (joystickStates[5]+100)/2;
		target_left = (joystickStates[2]+100)/2;
	}

	if(mode == 5)
	{
		target_right = -joystickStates[5]+100;
		target_left = -joystickStates[2]+100;
	}

	setStepperTarget(stepper_right, target_right);
	setStepperTarget(stepper_left, target_left);

	tft.drawString(String(target_right), 5, 20);
	tft.drawString(String(target_left), 5, 35);
}

void onEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
	//Handle WebSocket event

	Serial.print("received event, len: ");
	Serial.print(len);

	Serial.print("   type: ");
	Serial.print(type);

	if(type != WS_EVT_DATA)
		return;

	tft.fillScreen(TFT_BLACK);


	Serial.print("\t  ");
	for(int i = 0; i < len; i++)
	{
		Serial.print(data[i]-128*(i>=4));
		Serial.print("\t");
	}
	Serial.print("\t   ");
	for(int i = 0; i < len; i++)
	{
		int value = data[i]-128*(i>=4);
		Serial.print((char)(value == 10? 11 : value));
		Serial.print("\t");
	}
	Serial.println();

	int joystickOffset = 30;
	int radius = 10;

	int8_t joysticks[6] = 
	{
		(int8_t)data[4]-(int8_t)128, 
		data[5]-128,
		data[6]-128,
		data[7]-128,
		data[8]-128,
		data[9]-128
	};

	setNewTarget(data[0], data[1], data[2], data[3], joysticks, data[10]);

	if(len == 11)
	{
		tft.drawLine(tft.width()/2, tft.height()/2, tft.width()/2 + (data[1] - data[3])*radius, tft.height()/2 + (data[0] - data[2])*radius, TFT_BLUE);
		tft.drawCircle(tft.width()/2 - joystickOffset*2 + (data[4]-128)/4, tft.height()/2 + (data[5]-128)/4, 2, TFT_GREEN);
		tft.drawCircle(tft.width()/2 + joystickOffset*2 + (data[7]-128)/4, tft.height()/2 + (data[8]-128)/4, 2, TFT_GREEN);

		tft.drawCircle(tft.width()/2 - joystickOffset*3, tft.height()/2 - (data[6]-128)/4, 2, TFT_RED);
		tft.drawCircle(tft.width()/2 + joystickOffset*3, tft.height()/2 - (data[9]-128)/4, 2, TFT_RED);
		
		tft.drawString(String(data[10]), 5, 5);
	}
}

void wifiSetup()
{
	WiFi.mode(WIFI_STA);
	int current = -1;
	int atemptsRemaining = ssids_count*2;

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED && atemptsRemaining-->0) {
		current = (current+1)%ssids_count;
		WiFi.begin(ssids[current], passwords[current]);
		Serial.print("\nAttempting ");
		Serial.println(ssids[current]);
		delay(3000);
		if (WiFi.waitForConnectResult() != WL_CONNECTED) {
			Serial.printf("%s Failed!\n", ssids[current]);
		}
	}

	Serial.printf("connection result %d\n", (int)WiFi.status());

	Serial.println(WiFi.localIP());
	tft.drawString(WiFi.localIP().toString(), 10, 10);
	Serial.printf("ip is %s\n", WiFi.localIP().toString());

	ws.onEvent(onEvent);
	server.addHandler(&ws);

	// attach AsyncEventSource
	server.addHandler(&events);

	// respond to GET requests on URL /heap
	server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request){
		request->send(200, "text/plain", String(ESP.getFreeHeap()));
	});

	server.onNotFound(onRequest);
	//server.onFileUpload(onUpload);
	server.onRequestBody(onBody);

	server.begin();
}

void stepperSetup()
{
	engine.init();

	stepper_right = engine.stepperConnectToPin(stepPinStepper_right);
	if (stepper_right) {
		stepper_right->setDirectionPin(dirPinStepper_right);
		stepper_right->setEnablePin(enablePinStepper_right);
		stepper_right->setAutoEnable(true);

		stepper_right->setSpeedInHz(0);
		stepper_right->setAcceleration(topAcceleration);

		stepper_right->runForward();
	}

	stepper_left = engine.stepperConnectToPin(stepPinStepper_left);
	if (stepper_left) {
		stepper_left->setDirectionPin(dirPinStepper_left);
		stepper_left->setEnablePin(enablePinStepper_left);
		stepper_left->setAutoEnable(true);

		stepper_left->setSpeedInHz(0);
		stepper_left->setAcceleration(topAcceleration);

		stepper_left->runBackward();
	}
/*
		stepper_right->setSpeedInHz(10000);
		stepper_right->moveByAcceleration(-5000);
*/
}

void setup() 
{
	stepperSetup();

	tft.init();
	tft.setRotation(1);
	tft.fillScreen(TFT_BLUE);

	Serial.begin(115200);
	Serial.println("asdf");
	wifiSetup();

	// put your setup code here, to run once:
}

void loop() 
{
	/*
	if(shouldReboot){
		Serial.println("Rebooting...");
		delay(100);
		ESP.restart();
	}
	*/
	static char temp[128];
	sprintf(temp, "Seconds since boot: %u", millis()/1000);
	events.send(temp, "time"); //send event "time"
}