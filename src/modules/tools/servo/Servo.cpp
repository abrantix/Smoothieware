/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/

#include "libs/Module.h"
#include "libs/Kernel.h"
#include "libs/SerialMessage.h"
#include <math.h>
#include "Servo.h"
//#include "libs/Pin.h"
#include "modules/robot/Conveyor.h"
#include "PublicDataRequest.h"
#include "ServoPublicAccess.h"
#include "SlowTicker.h"
#include "Config.h"
#include "Gcode.h"
#include "checksumm.h"
#include "ConfigValue.h"
#include "StreamOutput.h"
#include "StreamOutputPool.h"
#include "utils.h"

#include "PwmOut.h"

#include "MRI_Hooks.h"

#include <algorithm>

#define    command_checksum    CHECKSUM("command")

//#define ROUND2DP(x) (roundf(x * 1e2F) / 1e2F)

Servo::Servo() {}

Servo::Servo(uint16_t name)
{
    this->name_checksum = name;
    //this->dummy_stream = &(StreamOutput::NullStream);
}

// set the pin to the fail safe value on halt
void Servo::on_halt(void *arg)
{
}

void Servo::on_module_loaded()
{
    this->register_for_event(ON_GCODE_RECEIVED);
    this->register_for_event(ON_MAIN_LOOP);
    this->register_for_event(ON_GET_PUBLIC_DATA);
    this->register_for_event(ON_SET_PUBLIC_DATA);
    this->register_for_event(ON_HALT);

    // Settings
    this->on_config_reload(this);
}

// Get config
void Servo::on_config_reload(void *argument)
{
    std::string command = THEKERNEL->config->value(servo_checksum, this->name_checksum, command_checksum )->by_default("")->as_string();
    this->servo_command = THEKERNEL->config->value(servo_checksum, this->name_checksum, command_checksum )->by_default("")->as_string();

    // Set the on/off command codes, Use GCode to do the parsing
    command_letter = 0;

    if(!command.empty()) {
        Gcode gc(command, NULL);
        if(gc.has_g) {
            command_letter = 'G';
            command_code = gc.g;
        } else if(gc.has_m) {
            command_letter = 'M';
            command_code = gc.m;
        }
    }
	else
	{
		/* default to M250 */
		command_letter = 'M';
		command_code = 250;
	}
}

bool Servo::match_gcode(const Gcode *gcode) const
{
    bool b= ((command_letter == 'M' && gcode->has_m && gcode->m == command_code) ||
            (command_letter == 'G' && gcode->has_g && gcode->g == command_code));

    return b;
}

int Servo::hex_char_to_binary(char character)
{
	char c = toupper(character);
	if (c >= '0' && c <= '9')
	{
		return c - '0';
	}
	else if (c >= 'A' && c <= 'F')
	{
		return c - 'A' + 10;
	}
	else
	{
		//invalid character
		return -1;
	}
}

bool Servo::hex_string_to_binary(	
	const char*		hexstring,
	int				stringlength,
	char*			outbuffer,
	unsigned char*	outbufferlength
)
{
	int				maxlength = *outbufferlength;
	
	*outbufferlength = 0;

	//parse string - but omit newline
	for (int i = 0; i < stringlength && 10 != i[hexstring] && 13 != i[hexstring]; i+=2)
	{
		int upperNibble = hex_char_to_binary(hexstring[i]);
		int lowerNibble = hex_char_to_binary(hexstring[i + 1]);
		if (0 <= upperNibble && 0 <= lowerNibble)
		{
			outbuffer[(*outbufferlength)++] = static_cast<char>((upperNibble << 4) + lowerNibble);
			if (*outbufferlength > maxlength)
			{
				//buffer overflow
				printf("Servo: buffer overflow\n");
				return false;
			}
		}
		else
		{
			//illegal character(s) found
			printf("Servo: illegal hex character(s) '%d|%d'\n", hexstring[i], hexstring[i+1]);
			return false;
		}
	}

	return true;
}

void Servo::execute_command(Gcode* gcode, PServoControllerMessage servo_controller_message)
{
	printf("Servo: sending message with datalength %d to ServoController\n", servo_controller_message->DataLength);
	int i2cError = ServoControllerWriteMessage(servo_controller_message);
	int isBusy = 0;

	if (0 != i2cError)
	{
		gcode->stream->printf("error: I2C Error %d while sending message to ServoController\n", i2cError);
		printf("error: I2C %d while sending message to ServoController\n", i2cError);
	}
	else
	{
		//By default, the controller returns an empty response message
		//TODO: other messages with DataLength > 0 may be supported in future
		servo_controller_message->DataLength = 0;
		i2cError = ServoControllerReadMessage(servo_controller_message);
		if (0 != i2cError)
		{
			gcode->stream->printf("error: I2C Error %d while receiving message from ServoController\n", i2cError);
			printf("error: I2C %d while receiving message from ServoController\n", i2cError);
		}
		else
		{
			printf("Wait for ServoController being idle\n");
			do 
			{
				i2cError = ServoControllerIsBusy(&isBusy);
				if (isBusy)
				{
					safe_delay_ms(100);
				}
			} while (0 == i2cError && isBusy);

			if (0 != i2cError)
			{
				printf("error: I2C %d while checking ServoController from being busy\n", i2cError);
			}
		}
	}
}

void Servo::on_gcode_received(void *argument)
{
	ServoControllerMessage servo_controller_message;
    Gcode *gcode = static_cast<Gcode *>(argument);
    // Add the gcode to the queue ourselves if we need it
    if (!(match_gcode(gcode))) {
        return;
    }

	printf("Servo: GCode match\n");

    // we need to sync this with the queue, so we need to wait for queue to empty, however due to certain slicers
    // issuing redundant swicth on calls regularly we need to optimize by making sure the value is actually changing
    // hence we need to do the wait for queue in each case rather than just once at the start
    if(match_gcode(gcode))
	{
		//Servo GCODE format: M250.<subcode: command> <data (hex string)>

		

		//convert G-CODE command (HEX) to local binary buffer
		const char* command = gcode->get_command();
		//memory will be freed on execution on main thread;
		servo_controller_message.Command = gcode->subcode;
		
		bool success = true;
		if (0 < strlen(command))
		{
			//set maxLength
			servo_controller_message.DataLength = 30;

			printf("Servo: parse HEX string '%s'\n", command + 1);

			//First character of command is a space - omit
			success = hex_string_to_binary(command + 1, strlen(command + 1), servo_controller_message.Data, &servo_controller_message.DataLength);
		}
		else
		{
			servo_controller_message.DataLength = 0;
		}
		
		if (success)
		{
			// drain queue
			THEKERNEL->conveyor->wait_for_idle();
			execute_command(gcode, &servo_controller_message);
		}
		else
		{
			printf("error: Servo: couldn't parse HEX string '%s'\n", command);
			gcode->stream->printf("error: Servo: couldn't parse HEX string '%s'\n", command);
		}
    }
}

void Servo::on_get_public_data(void *argument)
{
}

void Servo::on_set_public_data(void *argument)
{
}

void Servo::on_main_loop(void *argument)
{
}

void Servo::send_gcode(std::string msg, StreamOutput *stream)
{
    struct SerialMessage message;
    message.message = msg;
    message.stream = stream;
    THEKERNEL->call_event(ON_CONSOLE_LINE_RECEIVED, &message );
}

