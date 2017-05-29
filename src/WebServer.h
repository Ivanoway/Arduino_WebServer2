/*
 * WebServer.h
 *
 *  Created on: May 12, 2017
 *      Author: ivanoway
 */

#ifndef SRC_WEBSERVER_H_
#define SRC_WEBSERVER_H_


#include <SPI.h>
#include <Ethernet2.h>
#include <SD.h>

void get_url(void);
void send_SD_webFile(EthernetClient client);
void HTTP_standard_header(EthernetClient client);
void JSON_answer(EthernetClient client);

#endif /* SRC_WEBSERVER_H_ */
