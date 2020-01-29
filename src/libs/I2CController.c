/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.

	  (c) Abrantix AG

	  Implementation for I2C devices attached by I2C to Smoothie
*/

#include "libs/I2CController.h"
#include <stdio.h>
#include <string.h>

#include "i2c_api.h" //mbed c lib

I2CControllerConfigure(i2c_t * _i2c)
{
	i2c_init(_i2c, P0_27, P0_28);
	i2c_frequency(_i2c, 20000);

	return _i2c;
}

int I2CControllerWrite(char address, char* data, int length)
{
	i2c_t _i2c;
	I2CControllerConfigure(&_i2c);

	printf("Send I2C data length %d to 0x%02X\n", length, address);
	int result = i2c_write(&_i2c, address << 1, data, length, 1);
	return result;
}

int I2CControllerRead(char address, char* data, int *length)
{
	i2c_t _i2c;
	I2CControllerConfigure(&_i2c);

	printf("Receive I2C data length %d from 0x%02X\n", *length, address);
	return i2c_read(&_i2c, address << 1, data, length, 1);
}
