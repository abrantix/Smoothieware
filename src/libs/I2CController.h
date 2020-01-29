/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
	  
	  (c) Abrantix AG

	   Implementation for I2C devices attached by I2C to Smoothie
*/

#ifndef I2CCONTROLLER_H
#define I2CCONTROLLER_H

int I2CControllerWrite(char address, char* data, int length);
int I2CControllerRead(char address, char* data, int* length);

#endif
