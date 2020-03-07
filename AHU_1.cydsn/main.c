/* ========================================
 *
 * Copyright Park View Care Center, 2020
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF PVCC.
 *
 * ========================================
*/
#include "project.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <build_defs.h>
#include <max31865.h>
#include <one_wire.h>
#include <ds18b20.h>
#include <ModbusRtu.h>

#define MY_ID '0'
// basically, poll the modbus about every 5 minutes
#define TIME_BETWEEN_POLLING 295000
// MODBUS_COUNT is the max number of modbus devices to poll
#define MODBUS_COUNT 10
#define MODBUS_BASE_ADDR 0x31

uint8_t wemos_buffer[256];
//uint8_t rs485_buffer[256];
char temp_message[128];
char myId;
uint8_t slave_id;
uint8_t wemos_pos = 0;    // where in the receive buffer are we?
//uint8_t rs485_pos;    // where in the receive buffer are we?
uint8_t esc;        // flag to indicate whether an escape character was received
uint32 Global_time; // constantly updated by the timer interrupt, used for exact delays
uint16_t last_modbus_time;  // At what point in the seconds counter was the modbus read
uint32 esc_time;    // used to help indicate how long an escape character is valid, compared to Global_time
char version[12];   // just used to hold to version date sent at boot time
// data array for modbus network sharing
uint16_t registers[MODBUS_COUNT][60];
uint16_t last_registers[MODBUS_COUNT][60];
uint8_t modbus_state;
uint8_t modbus_slave;
uint32 modbus_wait;
uint16_t temp_regs[40];
float last_Adc_result[4] = {0,0,0,0}; // the last____ variables are to cut down on sending unchanged information
float last31865[8] = {0,0,0,0,0,0,0,0};
float lastDS18B20[10] = {0,0,0,0,0,0,0,0,0,0};
uint8_t lastPins = 0;
/**
 * This is an structure which contains a query to an slave device
 */
modbus_t telegram;

#define millis() Global_time;
void printBinary(uint8_t inByte, char *ts);

CY_ISR(Timer_ISR){  // interrupt routine to update the Global_time value
    Global_time++;
    Timer_Int_ClearPending();
}
/*
CY_ISR(Rx_ISR){     //ISR for 'On Byte Recd.'
	uint16 GetB;    //16bit var for storing GetByte's return.
	RS485_ReadRxStatus();   //Read it to clear interrupts.
	GetB=RS485_GetByte();   //MSB has RxStatus Reg,and LSB has the data.
  rs485_buffer[rs485_pos++] = (GetB & 0xff);  // put the character in the buffer
  if (rs485_pos > 254)
      rs485_pos = 254;
}
*/
CY_ISR(Wemos_Rx_ISR){     //ISR for 'On Byte Recd.'
	uint16 GetB;    //16bit var for storing GetByte's return.
	Wemos_ReadRxStatus();   //Read it to clear interrupts.
	GetB=Wemos_GetByte();   //MSB has RxStatus Reg,and LSB has the data.
  if((GetB & 0xff)==0x1b)
  {    // did an escape character get received?
    LED_Write(1);
		esc=1;              // set the flag
		wemos_pos=0;          // prepare for the next character
		esc_time = Global_time; // record when the escape was received
	}
  else if (esc)   // had an escape character already been received?
  {
    wemos_buffer[wemos_pos++] = (GetB & 0xff);  // put the character in the buffer
    if (wemos_pos > 100)
        wemos_pos = 0;
  }
}
//========================================================================================
// This simple function compares two values to see if they are different by more than maxDiff
//========================================================================================
_Bool checkDiff(float newValue, float prevValue, float maxDiff)
{
  return (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}
//========================================================================================
/*
 * Register mapping is
 *  0-3 DS18B20 Temp is degrees Centigrade times 100 in the first register, then 6 address bytes in the next 3 registers
 *  4-7 The second DS18B20 that was found (if any)
 *  8-11 The third DS18B20
 *  12-15 The fourth DS18B20
 *  16-20 The fifth DS18B20
 *  21-23 The sixth, and last DS18B20 found
 *  24 Version number of this program for diagnostics
 *  25 How many seconds have passed since the last sensor readings AKA how out of date are the holding registers
 *  26 The humidity as read by the HTU21D, times 10 to avoid floating points
 *  27 The compensated humidity from the HTU21D, also times 10
 *  28 The temperature in degrees centigrade from the HTU21D, times 100 to avoid floats
 *  29 If there is a PCF8574, then this register has its value.  Otherwise it is unused.
 *  30 Sent from the master, if non-zero the time is updated.  Set to 0 by this device after the time change has been done
 *  31-36 The time of day with the registers broken down to century, month, day, hour, minute and second
 *  37-40 Available
 */
//========================================================================================
void process_holding_registers(_Bool force)
{
  char ts[9];
  uint8_t i,j,k;    // It's amazing how the variables from fortran just hang around...
  uint16_t modbus_staleness;
  float newTemp;

  CyDelay(250);   // provides times for communications to settle
  // Note that modbus doesn't really handle negative values that well.  Everything is a positive integer.
  for (j = 0; j < MODBUS_COUNT; j++)  // for each unit that is polled we check the registers
  {
    CyDelay(250);
    k = 0;
    for (i = 0; i < 30; i++)
      if (registers[j][i] > 0)
        k = 1;    // all k does is indicate that there is a value other than 0 in one of the fields
    if (k || force)  // we got something from a device cause not everything is 0
    {
      modbus_staleness = (Global_time / 1000) - last_modbus_time + registers[j][25]; // get the real 'how out of date are we?' value

      // pull the humidity readings from the HTU21D sensor on each remote device
      if (registers[j][28] > 0)
      {
        newTemp = registers[j][28] / 100.0;
        if (force || checkDiff(newTemp, last_registers[j][28] / 100.0, 0.55) ||
            checkDiff(registers[j][26] / 10.0, last_registers[j][26] / 10.0, 0.55) ||
            checkDiff(registers[j][27] / 10.0, last_registers[j][27] / 10.0, 0.55)) // only update if it changed from the last time
        {
          last_registers[j][26] = registers[j][26];
          last_registers[j][27] = registers[j][27];
          last_registers[j][28] = registers[j][28];
          CyDelay(250);  // done to let the buffer clear
          sprintf(temp_message, "H|%03d|%06.2f|%05.2f|%05.2f|%c|%c\r\n", modbus_staleness,newTemp,registers[j][26] / 10.0, registers[j][27] / 10.0,MODBUS_BASE_ADDR + j,myId);
          Wemos_PutString(temp_message);  // send to mqtt later
        }
      }

      for (i = 0; i < 6; i++)  // there are up to six ds18b20s on each remote device
      {
        if (registers[j][i*4] > 0)  // If this DS18B20 has a temperature that isn't 0, then queue it up
        {
          newTemp = registers[j][i * 4] / 100.0;
          if (force || checkDiff(newTemp, last_registers[j][i * 4] / 100.0, 0.55))  // only update if something changed
          {
            last_registers[j][i * 4] = registers[j][i * 4];
            CyDelay(250);  // done to let the buffer clear
            sprintf(temp_message, "T|%03d|%06.2f|%04x%04x%04x|%c|%c\r\n", modbus_staleness,newTemp,registers[j][(i*4)+1], registers[j][(i*4)+2], registers[j][(i*4)+3],MODBUS_BASE_ADDR + j,myId);
            Wemos_PutString(temp_message);  // send to mqtt after queuing
          }
        }
      }

      // now pull the PCF8574 readings from the remote device
      if (registers[j][29] > 0)
      {
        if (force || registers[j][29] != last_registers[j][29])
        {
          last_registers[j][29] = registers[j][29];
          printBinary((uint8_t)(registers[j][29] & 0xff), ts);
          CyDelay(250);  // done to let the buffer clear
          sprintf(temp_message, "P|%03d|%s|%c|%c\r\n", modbus_staleness,ts,MODBUS_BASE_ADDR + j,myId);
          Wemos_PutString(temp_message);  // send to mqtt later
        }
      }
    }
  }
  // ------ End of processing the holding registers
}
//========================================================================================
//========================================================================================
void printBinary(uint8_t inByte, char *ts)
{
  for (int b = 7; b >= 0; b--)
  {
    if (bitRead(inByte, b))
    {
      *ts = '0';
      ts++;
    }
    else
    {
      *ts = '1';
      ts++;
    }
  }
  *ts = 0; // terminating null
}

//========================================================================================
//========================================================================================
int main(void)
{
  uint8_t i;
  uint8_t fault;  // used to check faults on the RTDs
  uint8_t forced;
  char s[20];   // essentially just used to report pin states
  char ts[200]; // a temporary string used for brief periods to send messages
  double f1;
  uint32 Adc_result;
  float fAdc;
  float temperature;    // temperature of the DS18B20s directly connected to this device
  ds18b20_devices devices;  // the structure of DS18B20s found on this device by a search
  one_wire_device address;  // The address of each DS18B20 to get the temperature from

  // TODO Convert the switches to an id
  myId = MY_ID;
  wemos_pos = 0;
  //rs485_pos = 0;
  Global_time = 0;
  forced = 0;
  sprintf(version,"%02d%02d%02d-%02d%02d", BUILD_YEAR - 2000, BUILD_MONTH, BUILD_DAY, BUILD_HOUR, BUILD_MIN);
  CyDelay(5000); // Just take a short pause to let the wemos start
  myId = DIP_Read();    // Find out what device this is supposed to be reported as
  myId += 0x40;         // This type of device starts at 'A', and works up to 'O'
  Modbus_begin();       // Initialize the modbus routines
  Modbus_setTimeOut( 2000 ); // if there is no answer in 2 seconds, roll over
  modbus_wait = Global_time + 10000;  // wait 10 seconds to do the first poll
  modbus_state = 0;     // 0 is idle, 1 is sending a request, and 2 is getting the reponse
  modbus_slave = 0;     // What slave we are requesting and getting information from
  MAX31865_CS_Write(0xff);    // deselect all of the MAX31865 spi devices

  CYGlobalIntDisable;//Disable interrupts to avoid triggering during setup.

  SPIM_Start();

  Timer_Start();        // This starts our counter for Global_time, which is in milliseconds
  Timer_Int_Start();
  Timer_Int_SetVector(Timer_ISR);

  RS485_Start();              // all communications are done using RS485 for modbus
//  Rx_Int_Start();             //Start the Interrupt.
//  Rx_Int_SetVector(Rx_ISR);   //Set the Vector to the ISR.

  CyGlobalIntEnable; /* Enable global interrupts. */
  max31865_begin(MAX31865_3WIRE); // tell all of the MAX31865 that they use 3-wire connections to the PTCs

  Wemos_Start();              // all communications are done via serial to the Wemos
  Wemos_Rx_Int_Start();             //Start the Interrupt.
  Wemos_Rx_Int_SetVector(Wemos_Rx_ISR);   //Set the Vector to the ISR.

  wemos_pos = 0;        // The position of any received bytes from the wemos
  Wemos_ClearRxBuffer();    // Make sure the firmware has nothing hanging

  AMux_Start();
  ADC_Start();

  CyDelay(5000);
  //=============================================
  // Everything up to this point was setup
  // The normal processing occurs after this point
  //=============================================
  for(;;)     // this program loops forever
  {
    // check to see if both the escape character and the requested id is in the receive buffer
    if (esc && wemos_pos)
    {   // going into this routine means we got an escape character, and at least one other character
      esc = 0;                    //reset the escape flag
      wemos_buffer[wemos_pos] = 0;    //terminate the input at the end of the data
      wemos_pos = 0;
      if (wemos_buffer[0] == '1' || wemos_buffer[0] == '2')  // process requests
      {
        CyDelay(500);           // Needed to let the other side turn around and listen
        LED_Write(1);           // light the led just to show we received a request for this id, for debugging
        one_wire_reset_pulse(); // do this for all of the DS18B20's
        // do this here because of timing, the DS18B20's can convert while other devices report
        devices = ds18b20_get_devices(1);
        ds18b20_convert_temperature_all();  // because this take 750ms, might as well start it now

        sprintf(ts,"R|000|%s|%c\r\n",version,myId);
        Wemos_PutString(ts);  // done just to clear everything
        if (wemos_buffer[0] == '1') // A 1 just requests whatever has changed
        {
          forced = 0;
          process_holding_registers(false);    // this queues up the changed data for mqtt
        }
        else  // A 2 forces everthing to report, in case a change is missing
        {
          forced = 1;
          process_holding_registers(true);    // this queues up all the data for mqtt
          CyDelay(250);
          for (i = 0; i < 8; i++) // clear all last values so we get past the checkDiff function
          {
            last31865[i] = -20.0;
            lastDS18B20[i] = 0.0;
            if (i < 4)
              last_Adc_result[i] = 1;
            lastPins = 0;
          }
        }
        i = DINP_Read() << DINP_WIDTH; // get the values from the digital pins
        printBinary(i,s);
        s[DINP_WIDTH] = 0; // the WIDTH is the actual number of pins being looked at
        sprintf(ts,"P|000|%s|%c\r\n",s,myId);    // report on the status of the air conditioners
        if (i != lastPins || forced)
        {
          lastPins = i;
          CyDelay(20);
          Wemos_PutString(ts);
        }


        for (i = 0; i < 4; i++) // read the analog pins
        {
          AMux_Select(i); // pick which analog pin to read
          CyDelay(20);
          Adc_result = ADC_Read32();  // This function starts/stops and reads the conversion
          if (Adc_result > 1000000 || Adc_result < 80000) // used to eliminate the extreme edges
            Adc_result = 0;
          fAdc = round((ADC_CountsTo_Volts(Adc_result) * 1.052) * 10) / 10;
          if (last_Adc_result[i] != fAdc || forced) // only show what has changed
          {
            sprintf(ts,"A|000|%06.2lf|%06lu|%d|%c\r\n",fAdc,Adc_result,i,myId);
            CyDelay(20);
            Wemos_PutString(ts);
            last_Adc_result[i] = fAdc;
          }
        }

        for (i = 0; i < 8; i++)     // get the temperatures from up to 8 devices
        {
          fault = max31865_readFault(i);
          CyDelay(25);
          if (fault > 1)
          {

            if (fault & MAX31865_FAULT_HIGHTHRESH)
            {
              sprintf(ts,"E|000|Fault - RTD High Threshold|%d|%c\r\n",i,myId);
              Wemos_PutString(ts);
            }
            if (fault & MAX31865_FAULT_LOWTHRESH)
            {
              sprintf(ts,"E|000|Fault - RTD Low Threshold|%d|%c\r\n",i,myId);
              Wemos_PutString(ts);
            }
            if (fault & MAX31865_FAULT_REFINLOW)
            {
              sprintf(ts,"E|000|Fault - REFIN- > 0.85 x Bias|%d|%c\r\n",i,myId);
              Wemos_PutString(ts);
            }
            if (fault & MAX31865_FAULT_REFINHIGH)
            {
              sprintf(ts,"E|000|Fault - REFIN- < 0.85 x Bias - FORCE- open|%d|%c\r\n",i,myId);
              Wemos_PutString(ts);
            }
            if (fault & MAX31865_FAULT_RTDINLOW)
            {
              sprintf(ts,"E|000|Fault - RTDIN- < 0.85 x Bias - FORCE- open|%d|%c\r\n",i,myId);
              Wemos_PutString(ts);
            }
            if (fault & MAX31865_FAULT_OVUV)
            {
              sprintf(ts,"E|000|Fault - Under/Over voltage|%d|%c\r\n",i,myId);
              Wemos_PutString(ts);
            }
            max31865_clearFault(i);
            CyDelay(10);
          }
          else
          {
            f1 = max31865_temperature(RNOMINAL, RREF, i); // pass all temperature values in celsius
            if (f1 < 100 && f1 > -30 && f1 != 0.0)  // account for bad readings
            {
              sprintf(ts,"M|000|%06.2lf|%d|%c\r\n",f1,i,myId);
              if (forced || (checkDiff(f1,last31865[i],0.5) && f1 != 0.0))
              {
                last31865[i] = f1;
                CyDelay(20);
                Wemos_PutString(ts);
              }
              CyDelay(50);
            }
          }
        }

        for (i = 0; i < devices.size; ++i) // get the temperatures from all of the local DS18B20s that responded
        {
          temperature = ds18b20_read_temperature(devices.devices[i]);

          address = devices.devices[i];
          sprintf(ts,"T|000|%06.2lf|%02x%02x%02x%02x%02x%02x|%c\r\n",
              temperature,address.address[2],address.address[3],
              address.address[4],address.address[5],address.address[6],
              address.address[7],myId);
          if (forced || checkDiff(temperature,lastDS18B20[i],0.5))
          {
            CyDelay(20);
            Wemos_PutString(ts);
            lastDS18B20[i] = temperature;
          }
          CyDelay(50);
        }
        CyDelay(200);
        Wemos_PutString("DONE\r\n");  // Nothing more to send
        wemos_buffer[0] = 0;  // Clear the received character buffer
        forced = 0;     // clear the forced flag
        CyDelay(50);
        LED_Write(0);   // turn the communications indicator light off
    }
  } // end of communicating with wemos
  if (esc && (Global_time - esc_time > 2000))  // give it 2 seconds for a character to follow the escape
  {
    esc = 0;  // reset the esc flag, since it should have been followed by the id within 2 seconds
    wemos_pos = 0;
    wemos_buffer[0] = 0;
    Wemos_PutString("E|000|esc timed out\r\n");
    Wemos_ClearRxBuffer();
    //CySoftwareReset();
    LED_Write(0);
  }

  switch( modbus_state )
  {
    case 0:
      if (Global_time > modbus_wait)
      {
        modbus_state++; // wait state
      }
      break;
    case 1:
      LED_Write(1);   // turn the communications indicator light on
      for (i = 0; i < 60; i++)
        registers[modbus_slave][i] = 0;
      telegram.u8id = MODBUS_BASE_ADDR + modbus_slave; // slave address
      telegram.u8fct = 3; // function code (this one is registers read)
      telegram.u16RegAdd = 0; // start address in slave
      telegram.u16CoilsNo = 40; // number of elements (coils or registers) to read
      telegram.au16reg = &registers[modbus_slave][0]; // pointer to a memory array in the Arduino
      Modbus_query( telegram ); // send query (only once)
      modbus_state++;   // now looking for responses
      modbus_slave++;   // set up for the next slave
      break;
    case 2:
  while (RS485_GetRxBufferSize() > 0) // something has been received
    Wemos_PutChar(RS485_ReadRxData());
      Modbus_poll(); // check incoming messages
      if (Modbus_getState() == COM_IDLE)  // COM_IDLE is after we've received our response
      {
        modbus_state = 0;
        if (modbus_slave > (MODBUS_COUNT - 1))
        {
          last_modbus_time = Global_time / 1000;  // how many seconds since the last modbus read was done?
          modbus_wait = Global_time + TIME_BETWEEN_POLLING;  // reset for the next loop
          modbus_slave = 0; // setup for the next time we wake up
          LED_Write(0);   // Turn the light back off
        }
      }
      //break;
    }  // end of waking up on modbus_state
  }   // end of looping forever
}

/* [] END OF FILE */
