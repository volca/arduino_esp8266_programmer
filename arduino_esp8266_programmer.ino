/**************************************************
* Simple pass-through serial flash programmer 
* programming the ESP8266 by an Arduino
*
* Any serial data is transfered as-is between the 
* two devices, with one exception:
* Sending data to the Arduino will trigger a reset
* of the ESP8266 into bootloader mode, if it is the 
* first connection attempt since 1 second.
* If no data is send for 1 second, the ESP is reset 
* again into normal mode.
* 
* This resembles the comfortable DTS controlled 
* programming mode one have with an FTDI or similiar
* serial connection cable, where no manual reset of
* the ESP is needed to upload code and run it.
* Unfortunately there is no RTS/DTS control on the
* Arduino Serial library, so we solely rely on timing.
*
* If the esptool does not wait or retry long enough, 
* you have to send some chars to trigger the reset 
* eg. add this to your Makefiles / scripts:
*
* echo 'RESET' >/dev/ttyACM0
* sleep 0.5 
* esptool .....
*
*
* Transmissions from the ESP are passed without any
* modification.
*
* You can not send serial commands (eg. to AT firmware)
* to the ESP with this tool, as it would frequently
* trigger resets into program mode.
*
* TODO: Provide control by magic keywords in the 
* serial data, like 'MAGIC_RESET_TO_BOOTLOADER'. 
* This way we can remove the timeouts and regain 
* interactive bidirectional serial console.
*
* TODO: tighten timings.
*
***************************************************/


/*
* Use an 3.3V Arduino, or TTL level shifter on RX / TX connections!
* The pin 9 / 10 connections are just pulled down, no level shifter is needed here.
* 
* connection table:
* ESP8266  Arduino
* GPIO0    5
* ENABLE   13
* RX       TX
* TX       RX
* GND      GND
* 
* Further connections to GND or VCC are needed depending on the ESP breakout module.
* 
* An LED on Arduino Pin 13 would indicate Program Mode.
*/

int program_pin = 5;
int enable_pin=13;
//int led_pin=13;

void setup()
{
	Serial1.begin(115000);
	Serial.begin(115000);
    pinMode(enable_pin, OUTPUT);
    pinMode(program_pin, OUTPUT);
	digitalWrite(program_pin, LOW);
	digitalWrite(enable_pin,HIGH);

	Serial.println("ESP8266 programmer ready.");
}

long last_send=0;

// resets the ESP8266 into normal or program / bootloader mode
void reset_target(bool program_mode)
{
    return;
	if(program_mode)
	{
		pinMode(program_pin,OUTPUT);
	}
	else
	{
		pinMode(program_pin,INPUT);
	}
	//digitalWrite(led_pin, program_mode);
	
	delay(100);
	pinMode(enable_pin,INPUT);	
	delay(100);
	pinMode(enable_pin,OUTPUT);
	digitalWrite(enable_pin,HIGH);
	delay(500);
}

bool program_mode=true;
void loop()
{
	// pass data from ESP to host, if any
	while(Serial1.available())
	{
		Serial.write((uint8_t)Serial1.read());
	}

	// pass data from host to ESP, if any
	if(Serial.available())
	{
		// if we are not in program mode, trigger reset to bootloader mode.
		if(!program_mode){
			//reset_target(true);
			program_mode=true;
		}			
		// pass data
		while(Serial.available())
		{
			Serial1.write((uint8_t)Serial.read());
		}
		last_send=millis();
	}

	// if the last transfer is more then one second ago,
	// trigger reset into normal mode.
	//if(last_send>0 && millis()-last_send>1000)
	if(0)
	{
		reset_target(false);
		last_send=0;
		program_mode=false;
	}
}


