/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.

	  (c) Abrantix AG

	  Implementation for Abrantix ServoController attached by I2C to Smoothie
*/

#include "libs/I2CController.h"
#include "libs/ServoController.h"
#include <stdio.h>
#include <string.h>


int ServoControllerSetMacAddress(char* data, int length)
{
	printf("Write MAC address to ServoController\n");
	if (length != 6)
	{
		printf("MAC address must be 6 bytes\n");
		return -1;
	}

	ServoControllerMessage message;

	message.Command = SERVO_CONTROLLER_COMMAND_SET_MAC_ADDRESS;
	memcpy(message.Data, data, 6);
	message.DataLength = 6;

	int i2cError = ServoControllerWriteMessage(&message);
	if (0 == i2cError)
	{
		message.DataLength = 0;
		i2cError = ServoControllerReadMessage(&message);
	}
	return i2cError;
}

int ServoControllerGetMacAddress(char* data, int length)
{
	printf("Read MAC address from ServoController\n");

	if (length != 6)
	{
		printf("MAC address must be 6 bytes\n");
		return -1;
	}

	ServoControllerMessage message;

	message.Command = SERVO_CONTROLLER_COMMAND_GET_MAC_ADDRESS;
	message.DataLength = 0;

	int i2cError = ServoControllerWriteMessage(&message);
	if (0 == i2cError)
	{
		message.DataLength = 6;
		i2cError = ServoControllerReadMessage(&message);
	}
	else
	{
		printf("Error while getting MAC address");
	}

	memcpy(data, message.Data, 6);

	return i2cError;
}

int ServoControllerIsBusy(int * isBusy)
{
	*isBusy = 0;

	ServoControllerMessage message;

	message.DataLength = 0;
	message.Command = SERVO_CONTROLLER_COMMAND_IS_BUSY;

	int i2cError = ServoControllerWriteMessage(&message);
	if (0 == i2cError)
	{
		message.DataLength = 1;
		i2cError = ServoControllerReadMessage(&message);
	}
	if (0 == i2cError)
	{
		*isBusy = !!message.Data[0];
	}
	return i2cError;
}

int ServoControllerWriteMessage(PServoControllerMessage message)
{
	printf("Write message with length %d to ServoController\n", message->DataLength);
	int bufferLength = 2 + message->DataLength;
	int i2cError = I2CControllerWrite(ServoControllerI2CAddress, (char*)(&message->Command), bufferLength);
	return i2cError;
}

int ServoControllerReadMessage(PServoControllerMessage message)
{
	char requestedCommand = message->Command;
	int bufferLength = 2 + message->DataLength;
	int i2cError = I2CControllerRead(ServoControllerI2CAddress, (char*)(&message->Command), &bufferLength);

	if (i2cError)
	{
		printf("Error %d while reading I2C\n", i2cError);
		return -1;
	}

	if (requestedCommand != message->Command)
	{
		printf("Expected command %d but got %d\n", requestedCommand, message->Command);
		return -99;
	}

	return i2cError;
}