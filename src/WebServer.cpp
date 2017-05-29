/*
  Web Server

 A simple web server that shows the value of the analog input pins.
 using an Arduino Wiznet Ethernet shield.

 Circuit:
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)

 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe

 */

#include "WebServer.h"

#define URL_BUFFER_LENGTH 50

// L'adresse MAC du shield
byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0xF6, 0x91 };
// L'adresse IP que prendra le shield
IPAddress ip(192,168,0,143);

// Initialise notre serveur
// Ce dernier écoutera sur le port 4200
EthernetServer server(4200);

File webFile;

void setup()
{
	//Init pin5 state
	pinMode(5, OUTPUT);
	digitalWrite(5, LOW);

	  // Open serial communications and wait for port to open:
	  Serial.begin(9600);	//For debug purposes
	  while (!Serial) {
	    ; // wait for serial port to connect. Needed for native USB port only
	  }

	  char error = 0;
	  // On démarre le shield Ethernet SANS adresse ip (donc donnée via DHCP)
	  error = Ethernet.begin(mac);

	  if (error == 0) {
	    Serial.println("Parametrage avec ip fixe...");
	    // si une erreur a eu lieu cela signifie que l'attribution DHCP
	    // ne fonctionne pas. On initialise donc en forçant une IP
	    Ethernet.begin(mac, ip);
	  }
	  Serial.println("Init...");
	  // Donne une seconde au shield pour s'initialiser
	  delay(1000);
	  // On lance le serveur
	  server.begin();
	  Serial.println("Pret !");

	  //Init SD card
	  Serial.print("Initializing SD card...");
	  if (!SD.begin(4)) {
	    Serial.println("initialization failed!");
	    return;
	  }
	  Serial.println("initialization done.");

	  //Check if index.htm file exist
	  if(!SD.exists("index.htm"))
	  {
		  Serial.println("ERROR - can't find index.htm file !");
		  return; 	//can't find index file
	  }
	  Serial.println("Success - Found index.htm file.");

}

void loop() {
	get_url();

}

void get_url(void)
{
	EthernetClient client = server.available();	//try to get the client

	char url_buffer[URL_BUFFER_LENGTH];
	unsigned char write_url_ptr = 0;
	unsigned char read_url_ptr = 0;
	char url_buffer_length = 0;
	unsigned char i = 0;
	char tmp_buffer[5];
	unsigned char j = 0;
	char tmp = 0;
	bool token_json_update = false;

	if(client){
		boolean currentLineIsBlank = true;
		while (client.connected()) {
			if(client.available()) {	//Check if client data is available to read
				char c = client.read(); 	//read 1 byte (character) from client
				Serial.print(c); //used to debug url received
				url_buffer[write_url_ptr++] = c;	//fill up url buffer and increment write_url_ptr
				if(write_url_ptr >= URL_BUFFER_LENGTH)
					write_url_ptr = 0;
				url_buffer_length++; //increment length
				//last line of client request is blank and ends with \n
				//respond to client after last line received
				if(c == '\n' && currentLineIsBlank) {
					HTTP_standard_header(client); //send a standard http response header
					//look for ?p= on the string
					tmp = url_buffer_length; //temporary buffer length
					for(i = 0; i < tmp; i++) //look all buffer content if "?p5=" from URL is present
					{
						if(url_buffer[read_url_ptr] == '?') //first character found, look for other ones
						{
							for(j = 0; j < 5; j++) //check a five bytes string "?p5=x"
							{
								tmp_buffer[j] = url_buffer[read_url_ptr++]; //fill temporary buffer
								if(read_url_ptr >= URL_BUFFER_LENGTH)
									read_url_ptr = 0;
							}
							url_buffer_length -= 5; //decrease number of bytes read from url_buffer
							if(tmp_buffer[0] == '?' && tmp_buffer[1] == 'p' &&
									tmp_buffer[2] == '5' && tmp_buffer[3] == '=')
							{
								//change pin5 value. LOW is default value.
								if(tmp_buffer[4] == '1')
								{
									digitalWrite(5, HIGH);
									Serial.println("PIN 5 HIGH"); //used to debug url received
								}
								else
								{
									digitalWrite(5, LOW);
									Serial.println("PIN 5 LOW"); //used to debug url received
								}
								//Token pin5 used to update JSON answer
								token_json_update = true;
							}
							else
							{
								Serial.println("pin5 state failed"); //used to debug url received
							}
						}
						else
						{
							//if characters are different from '?', continue to read url_buffer
//							Serial.print(url_buffer[read_url_ptr]);
							read_url_ptr++;
							if(read_url_ptr >= URL_BUFFER_LENGTH)
								read_url_ptr = 0;
							url_buffer_length--;
						}
					}
					//send web page if no JSON update
					if(!token_json_update)
						send_SD_webFile(client);
					else
					{	//send JSON update once a ?p5= request is done
						JSON_answer(client);
						token_json_update = false;
					}
					//Once the server answered to the client, quit while loop.
					break;
				}
				//every line of text received from the client ends with \r\n
				if(c == '\n') {
					//a text character was received from client
					currentLineIsBlank = false;
				}
			} //end if(client.available())
		} //end while(client.connected())
		delay(1); //give the web browser time to received the data
		client.stop(); //close the connection
	} //end if(client)
}

void send_SD_webFile(EthernetClient client)
{
	webFile = SD.open("index.htm");	//open web page index
	if(webFile) {
		while(webFile.available()) {
			client.write(webFile.read()); //send web page to client
		}
		webFile.close();
	}
}

//JSON answer called after pin5 has been updated
void JSON_answer(EthernetClient client){
	client.println("{");
	client.print("\t\t\"pin5\": ");
	client.println(digitalRead(5));
	client.println(",");
	client.print("\t\"uptime\":");
	client.println(millis());
	client.println("}");
}

//call each time a client send a request to the Arduino
void HTTP_standard_header(EthernetClient client)
{
	client.println("HTTP/1.1 200 OK");
	client.println("Content-Type: text/html");
	client.println("Access-Control-Allow-Origin: *");
	client.println("Connection: close");
	client.println();
}
