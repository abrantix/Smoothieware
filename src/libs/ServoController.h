/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
	  
	  (c) Abrantix AG

	   Implementation for Abrantix ServoController attached by I2C to Smoothie
*/

#ifndef SERVOCONTROLLER_H
#define SERVOCONTROLLER_H

#define ServoControllerI2CAddress 0x10

/* ServoControll Command defines */
typedef enum _SERVO_CONTROLLER_COMMAND {
	SERVO_CONTROLLER_COMMAND_INFO = 1,
	SERVO_CONTROLLER_COMMAND_SET_MAC_ADDRESS,
	SERVO_CONTROLLER_COMMAND_ERROR,
	SERVO_CONTROLLER_COMMAND_GET_MAC_ADDRESS,
} SERVO_CONTROLLER_COMMAND;

/* ServoController I2C Frame definition:
  { Command[1] DataLength[1] Data[0..30]}
*/
typedef struct _ServoControllerMessage {
	char Command;
	char DataLength;
	char Data[30];
} ServoControllerMessage, * PServoControllerMessage;


#ifdef __cplusplus
extern "C" {
#endif

	int ServoControllerSetMacAddress(char* data, int length);
	int ServoControllerGetMacAddress(char* data, int length);

#ifdef __cplusplus
}
#endif

#endif
