/*
 * Copyright (c) 2019, Abrantix
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack
 */

#include <stdio.h>
#include <string.h>

#include "uip.h"
#include "udpsvc.h"
#include "timer.h"
#include "pt.h"
#include "libs/ServoController.h"

#if UIP_CONF_UDP

#define ntohl(a) ((((a) >> 24) & 0x000000FF) | (((a) >> 8) & 0x0000FF00) | (((a) << 8) & 0x00FF0000) | (((a) << 24) & 0xFF000000))
static struct udpsvc_state s __attribute__ ((section ("AHBSRAM1")));


/* UDP Frame definition:
  { MAGIC[8]  Command[2] DataLength[2] Data[0..512]}
*/

/* Command defines */
typedef enum _COMMAND {
	COMMAND_INFO = 1,
	COMMAND_SET_MAC_ADDRESS,
	COMMAND_ERROR,
} COMMAND;

typedef enum _ERROR {
	ERROR_COMMAND_UNKNOWN = 1,
	ERROR_COMMAND_SYNTAX,
} ERROR;

typedef struct _MESSAGEHEADER {
	char Magic[8];
	unsigned short Command;
	unsigned short DataLength;
} MESSAGEHEADER, *PMESSAGEHEADER;


typedef struct _MESSAGE {
	MESSAGEHEADER Header;
	char Data[512];
} MESSAGE, *PMESSAGE;


#define UDPSVC_SERVER_PORT  8005

static const u8_t UDPSVC_MAGIC[8] = { 0xD9, 0xC5, 0xAE, 0x42, 0xF8, 0x9F, 0x71, 0x90 };

/* macros */
#define BUF ((struct uip_tcpip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UDPBUF ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])

/*---------------------------------------------------------------------------*/
static
PT_THREAD(handle_udp(void))
{
    PT_BEGIN(&s.pt);

	int setMacError;
	(void)PT_YIELD_FLAG; // avoid warning unused variable

	if (uip_newdata())
	{
		//printf("udpsvc: new data\n");
		PMESSAGE m = (PMESSAGE)uip_appdata;
		if (0 == memcmp(m->Header.Magic, UDPSVC_MAGIC, sizeof(UDPSVC_MAGIC)))
		{
			printf("udpsvc: received command %d\n", m->Header.Command);
			//received magic
			switch (m->Header.Command)
			{
			case COMMAND_INFO:
				//Version: 2 bytes
				m->Data[0] = MAJOR_VERSION;
				m->Data[1] = MINOR_VERSION;
				//MAC Address: 6 bytes
				memcpy(m->Data + 2, s.mac_addr, 6);
				//Local IP: 4 bytes
				memcpy(m->Data + 8, s.ipaddr, 4);

				m->Header.DataLength = 12;
				break;
			
			case COMMAND_SET_MAC_ADDRESS:
				setMacError = -1;
				if (6 == m->Header.DataLength)
				{
					setMacError = ServoControllerSetMacAddress((char*)(m->Data), 6);
				}
				else
				{
					printf("given MAC address must have length 6\n");
				}
				if (setMacError)
				{
					printf("I2C error %d while setting MAC address\n", setMacError);
					m->Header.Command = COMMAND_ERROR;
					m->Data[0] = ERROR_COMMAND_SYNTAX;
					m->Header.DataLength = 1;
				}
				break;
			
			default:
				m->Header.Command = COMMAND_ERROR;
				m->Data[0] = ERROR_COMMAND_UNKNOWN;
				m->Header.DataLength = 1;
				break;
			}

			uip_send(uip_appdata, sizeof(MESSAGEHEADER) + m->Header.DataLength);
		}
	}

    PT_END(&s.pt);
}
/*---------------------------------------------------------------------------*/
void
udpsvc_init(const void *mac_addr, int mac_len, char *hostname, const void *ipaddr)
{
	printf("Initialising udpsvc\n");

	s.mac_addr = mac_addr;
	s.mac_len = mac_len;
	s.hostname = hostname;
	s.ipaddr = ipaddr;

    s.conn = uip_udp_new(NULL, 0, udpsvc_appcall);
    if (s.conn != NULL) 
	{
		printf("Binding udpsvc\n");
        uip_udp_bind(s.conn, HTONS(UDPSVC_SERVER_PORT));
    }
	else
	{
		printf("Error while binding udpsvc\n");
	}
    PT_INIT(&s.pt);
}
/*---------------------------------------------------------------------------*/
void
udpsvc_appcall(void)
{
    handle_udp();
}
/*---------------------------------------------------------------------------*/

#endif
