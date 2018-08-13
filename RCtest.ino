// First Example in a series of posts illustrating reading an RC Receiver with
// micro controller interrupts.
//
// Subsequent posts will provide enhancements required for real world operation
// in high speed applications with multiple inputs.
//

#include "RCtestLib.h"


// Initial start values for controls. Neutral stick os center value from transmitter
volatile int nZoomIn = NEUTRAL_STICK; // volatile, we set this in the Interrupt and read it in loop so it must be declared volatile
volatile int nXaxisIn = NEUTRAL_STICK;
volatile int nYaxisIn = NEUTRAL_STICK;
volatile int nRecordIn = NEUTRAL_STICK;

volatile unsigned long ulStartPeriod = 0; // set in the interrupt
volatile unsigned long ulStartPeriodX = 0; // set in the interrupt
volatile unsigned long ulStartPeriodY = 0; // set in the interrupt
volatile unsigned long ulStartPeriodRecord = 0; // set in the interrupt


volatile boolean bNewZoomSignal = false; // set in the interrupt and read in the loop
											 // we could use nThrottleIn = 0 in loop instead of a separate variable, but using bNewThrottleSignal to indicate we have a new signal 
											 // is clearer for this first example
volatile boolean bNewXSignal = false;
volatile boolean bNewYSignal = false;
volatile boolean bNewRecordSignal = false;

int zoomValue;
bool camerIsRecording = false;

//Begin LANC
int cmdRepeatCount;
int bitDuration = 104; //Duration of one LANC bit in microseconds. 


					   //LANC commands byte 0 + byte 1
					   //Tested with Canon XF300

					   //Start-stop video recording
boolean REC[] = { LOW,LOW,LOW,HIGH,HIGH,LOW,LOW,LOW,   LOW,LOW,HIGH,HIGH,LOW,LOW,HIGH,HIGH }; //18 33

																							  //Zoom in from slowest to fastest speed
boolean ZOOM_IN_0[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW }; //28 00
boolean ZOOM_IN_1[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,LOW,LOW,LOW,HIGH,LOW }; //28 02
boolean ZOOM_IN_2[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,LOW,LOW,HIGH,LOW,LOW }; //28 04
boolean ZOOM_IN_3[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,LOW,LOW,HIGH,HIGH,LOW }; //28 06
boolean ZOOM_IN_4[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,LOW,HIGH,LOW,LOW,LOW }; //28 08
boolean ZOOM_IN_5[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,LOW,HIGH,LOW,HIGH,LOW }; //28 0A
boolean ZOOM_IN_6[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,LOW,HIGH,HIGH,LOW,LOW }; //28 0C
boolean ZOOM_IN_7[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,LOW,HIGH,HIGH,HIGH,LOW }; //28 0E

																								   //Zoom out from slowest to fastest speed
boolean ZOOM_OUT_0[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,HIGH,LOW,LOW,LOW,LOW }; //28 10
boolean ZOOM_OUT_1[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,HIGH,LOW,LOW,HIGH,LOW }; //28 12
boolean ZOOM_OUT_2[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,HIGH,LOW,HIGH,LOW,LOW }; //28 14
boolean ZOOM_OUT_3[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,HIGH,LOW,HIGH,HIGH,LOW }; //28 16
boolean ZOOM_OUT_4[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,HIGH,HIGH,LOW,LOW,LOW }; //28 18
boolean ZOOM_OUT_5[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,HIGH,HIGH,LOW,HIGH,LOW }; //28 1A
boolean ZOOM_OUT_6[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,HIGH,HIGH,HIGH,LOW,LOW }; //28 1C
boolean ZOOM_OUT_7[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,LOW,LOW,HIGH,HIGH,HIGH,HIGH,LOW }; //28 1E

																									 //Focus control. Camera must be switched to manual focus
																									 //boolean FOCUS_NEAR[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,HIGH,LOW,LOW,LOW,HIGH,HIGH,HIGH }; //28 47
																									 //boolean FOCUS_FAR[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,HIGH,LOW,LOW,LOW,HIGH,LOW,HIGH }; //28 45

boolean FOCUS_AUTO[] = { LOW,LOW,HIGH,LOW,HIGH,LOW,LOW,LOW,   LOW,HIGH,LOW,LOW,LOW,LOW,LOW,HIGH }; //28 41

																								   //boolean POWER_OFF[] = {LOW,LOW,LOW,HIGH,HIGH,LOW,LOW,LOW,   LOW,HIGH,LOW,HIGH,HIGH,HIGH,HIGH,LOW}; //18 5E
																								   //boolean POWER_ON[] = {LOW,LOW,LOW,HIGH,HIGH,LOW,LOW,LOW,   LOW,HIGH,LOW,HIGH,HIGH,HIGH,LOW,LOW}; //18 5C  Doesn't work because there's no power supply from the LANC port when the camera is off
																								   //boolean POWER_OFF2[] = {LOW,LOW,LOW,HIGH,HIGH,LOW,LOW,LOW,   LOW,LOW,HIGH,LOW,HIGH,LOW,HIGH,LOW}; //18 2A Turns the XF300 off and then on again
																								   //boolean POWER_SAVE[] = {LOW,LOW,LOW,HIGH,HIGH,LOW,LOW,LOW,   LOW,HIGH,HIGH,LOW,HIGH,HIGH,LOW,LOW}; //18 6C Didn't work



//END LANC






void setup()
{
	// tell the Arduino we want the function calcInput to be called whenever INT0 (digital pin 2) changes from HIGH to LOW or LOW to HIGH
	// catching these changes will allow us to calculate how long the input pulse is
	attachInterrupt(ZOOM_INTERRUPT, calcInput, CHANGE);
	attachInterrupt(XAXIS_INTERRUPT, calcXaxis, CHANGE);
	attachInterrupt(YAXIS_INTERRUPT, calcYaxis, CHANGE);
	attachInterrupt(RECORD_INTERRUPT, calcRecord, CHANGE);





	//LANC Setup
	pinMode(lancPin, INPUT); //listens to the LANC line
	pinMode(cmdPin, OUTPUT); //writes to the LANC line

	pinMode(recButton, INPUT); //start-stop recording button
	digitalWrite(recButton, HIGH); //turn on an internal pull up resistor
	pinMode(zoomOutButton, INPUT);
	digitalWrite(zoomOutButton, HIGH);
	pinMode(zoomInButton, INPUT);
	digitalWrite(zoomInButton, HIGH);
	pinMode(focusNearButton, INPUT);
	digitalWrite(focusNearButton, HIGH);
	pinMode(focusFarButton, INPUT);
	digitalWrite(focusFarButton, HIGH);

	digitalWrite(cmdPin, LOW); //set LANC line to +5V
	delay(5000); //Wait for camera to power up completly
	bitDuration = bitDuration - 8; //Writing to the digital port takes about 8 microseconds so only 96 microseconds are left for each bit





	Serial.begin(115200);
}


void loop()
{
	// if a new throttle signal has been measured, lets print the value to serial, if not our code could carry on with some other processing
	if (bNewZoomSignal)
	{		
		//changeZoom(1500);
		Serial.print("Zoom: ");
	    Serial.println(nZoomIn);
		//lancCommand(ZoomAmount);
		// set this back to false when we have finished
		// with nThrottleIn, while true, calcInput will not update
		// nThrottleIn
		bNewZoomSignal = false;
		
		if ((nZoomIn > 1100) && (nZoomIn < 1250))
		{//zoomAmount = ZOOM_OUT_7;
			lancCommand(ZOOM_OUT_7);
		}
		if ((nZoomIn > 1250) && (nZoomIn < 1350))
		{	//zoomAmount = ZOOM_OUT_5;
			lancCommand(ZOOM_OUT_5);
		}
		if ((nZoomIn > 1350) && (nZoomIn < 1450))
		{
			lancCommand(ZOOM_OUT_3);
			//Serial.print("Zoom2: ");
			//Serial.println(nZoomIn);
		}


		if (nZoomIn > 1750)
		{//zoomAmount = ZOOM_OUT_7;
			lancCommand(ZOOM_IN_7);
		}
		if ((nZoomIn < 1750) && (nZoomIn > 1650))
		{	//zoomAmount = ZOOM_OUT_5;
			lancCommand(ZOOM_IN_5);
		}
		if ((nZoomIn < 1650) && (nZoomIn > 1550))
		{//ZoomAmount = ZOOM_OUT_3;
			lancCommand(ZOOM_IN_3);
		}
		
		//lancCommand(ZoomAmount);
		// set this back to false when we have finished
		// with nThrottleIn, while true, calcInput will not update
		// nThrottleIn

		
	}

    if (bNewXSignal)
	{
		//Serial.print(" Xaxis : ");
		//Serial.println(nXaxisIn);
		moveHead(pinToMoveRight, pinToMoveLeft, nXaxisIn, 1890, 1150, 1500, 20);
		bNewXSignal = false;
	}

    if (bNewYSignal)
	{
		//Serial.println(" Processing Y");
		//Serial.print(" Yaxis : ");
		//Serial.println(nYaxisIn);
		moveHead(pinToMoveUp, pinToMoveDown, nYaxisIn, 1890, 1150, 1500, 20);
		bNewYSignal = false;
	}

	if (bNewRecordSignal)
	{
		//Serial.print(" Record : ");
		//Serial.println(nRecordIn);
		if ((nRecordIn > 1700) && !camerIsRecording)
		{
			lancCommand(REC);
			camerIsRecording = true;
		}
		if ((nRecordIn < 1300) && camerIsRecording)
		{
			lancCommand(REC);
			camerIsRecording = false;
		}
		bNewRecordSignal = false;
	}

	//LANC Control
	if (!digitalRead(recButton)) {
		lancCommand(REC);
	}

	if (!digitalRead(zoomOutButton)) {
		lancCommand(ZOOM_OUT_4);
	}

	if (!digitalRead(zoomInButton)) {
		lancCommand(ZOOM_IN_4);
	}

	/*if (!digitalRead(focusNearButton)) {
	lancCommand(FOCUS_NEAR);
	}

	if (!digitalRead(focusFarButton)) {
	lancCommand(FOCUS_FAR);
	}
	*/

	// other processing ... 
}


void calcRecord()
{
	// if the pin is high, its the start of an interrupt
		if (digitalRead(RECORD_SIGNAL_PIN) == HIGH)
		{
			// get the time using micros - when our code gets really busy this will become inaccurate, but for the current application its 
			// easy to understand and works very well
			ulStartPeriodRecord = micros();
		}
		else
		{
			// if the pin is low, its the falling edge of the pulse so now we can calculate the pulse duration by subtracting the 
			// start time ulStartPeriod from the current time returned by micros()
			if (ulStartPeriodRecord && (bNewRecordSignal == false))
			{
				nRecordIn = (int)(micros() - ulStartPeriodRecord);
				ulStartPeriodRecord = 0;

				// tell loop we have a new signal on the throttle channel
				// we will not update nThrottleIn until loop sets
				// bNewThrottleSignal back to false
				bNewRecordSignal = true;

			}
		}
}

int convertInputToAnalogValue(int valueIn, device_type deviceType)
{
	// Converts input values from the receiver to output values compatible with the devices 
	// that can be controlled. 
	// Current devices
	// 0 = base
	// 1 = Bescor vertical
	// 2 = Bescor horizontal
	int returnValue;

	
	switch (deviceType)
	{
		case generic:
			returnValue = valueIn;
		case Bescor:
			//Conversion for bescor head
			//I am favoring this to actually move, providing my head around 9V
			//so adjust values accordingly. PWM under 128 doesnt work
			returnValue = abs(valueIn - NEUTRAL_STICK);
			// Normalize values toa  working range
				//return of over 255 causes issues with analogwrite
				//return of less than 128 causes issues with mechanics
			if (returnValue > 254)
			{
				//Keeps analogwrite happy
				returnValue = 255;
			}
			else if (returnValue < 128)
			{
				//keeps head actually moving instead of wasting power
				returnValue = 128;
			}
		break;
	}

	return returnValue;
	
}

void calcXaxis()
{
	volatile int amountToMove;

	//Hack to figure out how to move the head

	//Serial.println("top of Calc Xaxis");
	// if the pin is high, its the start of an interrupt
	if (digitalRead(XAXIS_SIGNAL_PIN) == HIGH)
	{
		// get the time using micros - when our code gets really busy this will become inaccurate, but for the current application its 
		// easy to understand and works very well
		ulStartPeriodX = micros();
	}
	else
	{
		// if the pin is low, its the falling edge of the pulse so now we can calculate the pulse duration by subtracting the 
		// start time ulStartPeriod from the current time returned by micros()
		if (ulStartPeriodX && (bNewXSignal == false))
		{
			nXaxisIn = (int)(micros() - ulStartPeriodX);
			ulStartPeriodX = 0;

			// tell loop we have a new signal on the throttle channel
			// we will not update nThrottleIn until loop sets
			// bNewThrottleSignal back to false
			bNewXSignal = true;

		}
	}
}

/* moveHead - takes abunch of parameters to move the head. The thinking is that this allows a way
to change values if necessary. 
todo - move to a structure based on head type
*/
void moveHead(int UpperRangeOutputPin, int LowerRangeOutputPin, int axisValue, int PWMUpperRange, int PWMLowerRange, int PWMNeutralValue, int PWMJitter)
{
	//volatile int amountToMove = convertInputToAnalogValue(axisValue, 1);
	int amountToMove = convertInputToAnalogValue(axisValue, 1);

	if (axisValue > (NEUTRAL_STICK + PWMJitter))
	{
		pinMode(UpperRangeOutputPin, OUTPUT);
		analogWrite(UpperRangeOutputPin, amountToMove);
		Serial.print(amountToMove);
		Serial.println(" right move complete");
	}
	else if (axisValue < (NEUTRAL_STICK - PWMJitter))
	{
		pinMode(LowerRangeOutputPin, OUTPUT);
		analogWrite(LowerRangeOutputPin, amountToMove);
		Serial.print(amountToMove);
		Serial.println(" LEFT Move Complete");
	}
	else
	{
		analogWrite(UpperRangeOutputPin, 0);
		analogWrite(LowerRangeOutputPin, 0);
		//Serial.println("---  Xaxis is centered");
	}
}

/*Process ZOOM Cammand
  This is used to send ZOOM commands to the attached video camera.
*/
void changeZoom(int zoomVALUE)
{
	if ((zoomValue < 1450))
	{//ZoomAmount = ZOOM_OUT_3;
		Serial.print("Zoom3: ");
		//Serial.println(nZoomIn);
		noInterrupts();
		lancCommand(ZOOM_OUT_3);
		interrupts();


	}
}
void calcYaxis()
{
	volatile int amountToMove;

	//Hack to figure out how to move the head
	//Serial.println("top of Calc Xaxis");
	// if the pin is high, its the start of an interrupt
	if (digitalRead(YAXIS_SIGNAL_PIN) == HIGH)
	{
		// get the time using micros - when our code gets really busy this will become inaccurate, but for the current application its 
		// easy to understand and works very well
		ulStartPeriodY = micros();
	}
	else
	{
		// if the pin is low, its the falling edge of the pulse so now we can calculate the pulse duration by subtracting the 
		// start time ulStartPeriod from the current time returned by micros()
		if (ulStartPeriodY && (bNewYSignal == false))
		{
			nYaxisIn = (int)(micros() - ulStartPeriodY);
			ulStartPeriodY = 0;

			// tell loop we have a new signal on the throttle channel
			// we will not update nThrottleIn until loop sets
			// bNewThrottleSignal back to false
			bNewYSignal = true;

		}
	}
}




/*{
	// if the pin is high, its the start of an interrupt
	if (digitalRead(YAXIS_SIGNAL_PIN) == HIGH)
	{
		// get the time using micros - when our code gets really busy this will become inaccurate, but for the current application its 
		// easy to understand and works very well
		ulStartPeriod = micros();
	}
	else
	{
		// if the pin is low, its the falling edge of the pulse so now we can calculate the pulse duration by subtracting the 
		// start time ulStartPeriod from the current time returned by micros()
		if (ulStartPeriod && (bNewThrottleSignal == false))
		{
			nYaxisIn = (int)(micros() - ulStartPeriod);
			ulStartPeriod = 0;

			// tell loop we have a new signal on the throttle channel
			// we will not update nThrottleIn until loop sets
			// bNewThrottleSignal back to false
			bNewThrottleSignal = true;
		}
	}
}
*/

void calcInput()
{
	// if the pin is high, its the start of an interrupt
	if (digitalRead(ZOOM_SIGNAL_PIN) == HIGH)
	{
		// get the time using micros - when our code gets really busy this will become inaccurate, but for the current application its 
		// easy to understand and works very well
		ulStartPeriod = micros();
	}
	else
	{
		// if the pin is low, its the falling edge of the pulse so now we can calculate the pulse duration by subtracting the 
		// start time ulStartPeriod from the current time returned by micros()
		if (ulStartPeriod && (bNewZoomSignal == false))
		{
			nZoomIn = (int)(micros() - ulStartPeriod);
			ulStartPeriod = 0;

			// tell loop we have a new signal on the throttle channel
			// we will not update nThrottleIn until loop sets
			// bNewThrottleSignal back to false

			if ((bNewZoomSignal > (NEUTRAL_STICK + 10)) || (bNewZoomSignal < (NEUTRAL_STICK + 10)))
			{
				bNewZoomSignal = true;
			}
			else
			{
				bNewZoomSignal = true;
			}
		}
	}
}



void lancCommand(boolean lancBit[]) {

	cmdRepeatCount = 0;
	Serial.println("In Lanc Commoand");

	while (cmdRepeatCount < 5) {  //repeat 5 times to make sure the camera accepts the command

		while (pulseIn(lancPin, HIGH) < 5000) {
			//"pulseIn, HIGH" catches any 0V TO +5V TRANSITION and waits until the LANC line goes back to 0V 
			//"pulseIn" also returns the pulse duration so we can check if the previous +5V duration was long enough (>5ms) to be the pause before a new 8 byte data packet
			//Loop till pulse duration is >5ms
		}

		//LOW after long pause means the START bit of Byte 0 is here
		delayMicroseconds(bitDuration);  //wait START bit duration

										 //Write the 8 bits of byte 0 
										 //Note that the command bits have to be put out in reverse order with the least significant, right-most bit (bit 0) first
		for (int i = 7; i>-1; i--) {
			digitalWrite(cmdPin, lancBit[i]);  //Write bits. 
			delayMicroseconds(bitDuration);
		}

		//Byte 0 is written now put LANC line back to +5V
		digitalWrite(cmdPin, LOW);
		delayMicroseconds(10); //make sure to be in the stop bit before byte 1

		while (digitalRead(lancPin)) {
			//Loop as long as the LANC line is +5V during the stop bit
		}

		//0V after the previous stop bit means the START bit of Byte 1 is here
		delayMicroseconds(bitDuration);  //wait START bit duration

										 //Write the 8 bits of Byte 1
										 //Note that the command bits have to be put out in reverse order with the least significant, right-most bit (bit 0) first
		for (int i = 15; i>7; i--) {
			digitalWrite(cmdPin, lancBit[i]);  //Write bits 
			delayMicroseconds(bitDuration);
		}

		//Byte 1 is written now put LANC line back to +5V
		digitalWrite(cmdPin, LOW);

		cmdRepeatCount++;  //increase repeat count by 1

						   /*Control bytes 0 and 1 are written, now don�t care what happens in Bytes 2 to 7
						   and just wait for the next start bit after a long pause to send the first two command bytes again.*/


	}//While cmdRepeatCount < 5
}

