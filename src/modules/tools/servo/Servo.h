/*
      this file is part of smoothie (http://smoothieware.org/). the motion control part is heavily based on grbl (https://github.com/simen/grbl).
      smoothie is free software: you can redistribute it and/or modify it under the terms of the gnu general public license as published by the free software foundation, either version 3 of the license, or (at your option) any later version.
      smoothie is distributed in the hope that it will be useful, but without any warranty; without even the implied warranty of merchantability or fitness for a particular purpose. see the gnu general public license for more details.
      you should have received a copy of the gnu general public license along with smoothie. if not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
//#include "Pin.h"

//#include <math.h>
#include <string>
#include "libs/ServoController.h"

class Gcode;
class StreamOutput;

//namespace mbed {
    //class PwmOut;
//}

class Servo : public Module {
    public:
		Servo();
		Servo(uint16_t name);

        void on_module_loaded();
        void on_main_loop(void *argument);
        void on_config_reload(void* argument);
        void on_gcode_received(void* argument);
        void on_get_public_data(void* argument);
        void on_set_public_data(void* argument);
        void on_halt(void *arg);

    private:
		void execute_command(Gcode* gcode, PServoControllerMessage servo_controller_message);
		void send_gcode(std::string msg, StreamOutput* stream);
        bool match_gcode(const Gcode* gcode) const;
		int hex_char_to_binary(char character);
		bool hex_string_to_binary(
			const char*		hexstring,
			int				stringlength,
			char*			outbuffer,
			unsigned char* outbufferlength
		);

        std::string    servo_command;
        struct {
            uint16_t	name_checksum:16;
            uint16_t	command_code:16;
            char		command_letter:8;
        };
};
