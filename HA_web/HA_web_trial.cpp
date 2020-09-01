 /*
    Copyright (C) 2011  Andrew Richards
    
    Part of home automation suite
       
		Contains the HA_web class for handling dialogue with browser

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*

#include "HA_web.h"


EthernetServer server(80);


void HA_web::init() {
  server.begin();
}

EthernetClient HA_web::available() {
	return server.available();
}

void HA_web::processHTTP(EthernetClient client)  {
	SAVE_CONTEXT("Web1");
	
  char clientLine[HTTP_BUFSIZE];
  char URLline[HTTP_BUFSIZE];
  int index = 0;
  char *dataStart;
  char mode = ' ';    // ' ' = normal, 'G' = GET, 'P' = waiting for blank line in POST, 'D' = waiting for data line in POST, 'X' = request complete
  long int contLen;
  
  while (client.connected() && mode != 'X') {
    if (client.available()) { 
      char c = client.read();

      switch (c) {
        case '\n':
          clientLine[index] = 0;  
          
          if (index==0) {      // Blank line indicates end of request, unless it was a POST (in which case next line contains data, so keep alive)
            switch (mode) {
              case 'G':        // Was a GET; data is in the saved URLline
                if (dataStart = strstr(URLline, "ajax!")) {/*
	                SENDLOG('D', "Ajax get = ", URLline);
	                handleAjaxGet (client, dataStart + 6, dataStart[5]);    // Ajax Get, either 'R'eading or 'T'ime
	                */ /*
                }
                else if ((dataStart = strstr(URLline,"?")) != 0) handleHTTPCmd(client,dataStart+1);               // Was a GET after a Form submit - handle the submitted text                }
                else {
	                SENDLOG('I', "Client line = ", URLline);
                  serveFile(URLline);        // Normal GET
                  stopClient(client);                // Close connection if non-Ajax GET, otherwise keep alive
                }
                mode = 'X';
                break;
              case 'P':                                         // Finished POST headers, expect data in next line
                mode = 'D';
                break;
              default:                           // Shouldn't happen?
                SENDLOG('W', "Mode not recognised ", mode)         
                client.println("HTTP/1.1 200 OK");
                client.println();
                stopClient(client);
                mode = 'X';
            }
          }
          else {                                      // Not a blank line, check what's on it
            if (strstr(clientLine,"GET /") != 0) {              
              mode = 'G';                            
              strncpy (URLline,clientLine,HTTP_BUFSIZE);    // Save line for later 
            }
            else if (strstr(clientLine,"POST /") != 0) {
              mode = 'P';                            // Got a POST; set mode & remember URL
              strncpy (URLline,clientLine,HTTP_BUFSIZE);
	            SENDLOG('I', "Client line = ", URLline);
            }
            else if (mode =='P' && (dataStart = strstr(clientLine,"Content-Length:")) != 0) {    // Got the data length for a POST; remember it
              dataStart += 16;      // Step over "Content-Length: " to start of numbers
              contLen = strtol(dataStart,NULL,10);        // Get the content length
              if (contLen > HTTP_BUFSIZE) contLen = HTTP_BUFSIZE;        // Can't cope with huge POSTs
            }
            
            index = 0;
          }
          break;
        case '\r':    // Ignore CR
          break;
        default:      // Add a character to the input buffer; if too large just truncate (enough info in first HTTP_BUFSIZE chars)
          clientLine[index] = c;
          if (index < (HTTP_BUFSIZE-1)) index++;
      }    // Switch (c)
    }    // Client.available
  }    // Client.connected
  
  RESTORE_CONTEXT
}


void HA_web::serveFile(char *clientLine) {
	
	SAVE_CONTEXT("Web2")
  
  char homePage[] = "index.htm"; 
  char *fileName;
  
  // Housekeeping to standardise processing
  if (strstr(clientLine,"GET / ")) memcpy(clientLine+5,homePage,strlen(homePage));  // If no file specified, then serve home page
  strstr(clientLine," HTTP/")[0] = '\0';      // Place terminator after path/filename  
  fileName = clientLine + 5;            // Step over the "GET /"  
  
  // Open the file for reading:
  
  if (File myFile = SD.open(fileName)) {
    char extn[4];
    unsigned long prevTime;
		char buffer[UDP_TX_PACKET_MAX_SIZE]; 
		  
		timeToText(now(), buffer, UDP_TX_PACKET_MAX_SIZE);              // Get current time
    
    memcpy(extn,strstr(fileName,".")+1,3);
    extn[3] = '\0';
    stolower(extn); 
      
    prevTime = millis();
        
    server.print("HTTP/1.1 200 OK\r\n");  
    server.print("Server: Arduino\r\n");   
    server.print("Date: ");
    server.print(buffer);
    server.write("\r\n");
    server.print("Content-Type: ");
  
    if (strstr(extn, "htm") != 0)         server.print("text/html\r\n");
    else if (strstr(extn, "css") != 0)    server.print("text/css\r\n");
    else if (strstr(extn, "jpg") != 0)    server.print("image/jpeg\r\n");
    else if (strstr(extn, "png") != 0)    server.print("image/png\r\n");
    else if (strstr(extn, "gif") != 0)    server.print("image/gif\r\n");
    else if (strstr(extn, "pdf") != 0)    server.print("image/pdf\r\n");
    else if (strstr(extn, "ico") != 0)    server.print("image/x-icon\r\n");
    else if (strstr(extn, "xml") != 0)    server.print("application/xml\r\n");
    else if (strstr(extn, "jso") != 0)    server.print("application/json\r\n");
    else if (strstr(extn, "js") != 0)     server.print("application/javascript\r\n");
    else                                  server.print("text\r\n");
    
    server.print("Content-Length: ");
    server.print(myFile.size());
    server.write("\r\n");
    server.write("\r\n");
    

    
    // read from the file until there's nothing else in it:
    int size;
    byte buf[64];
    while (size = myFile.available()) {
	    if (size > 64) size = 64;
	    myFile.read(buf, size);
	    server.write(buf, size);
    }
    
    SENDLOG('I', "Serve time = ", (unsigned int)(millis() - prevTime))
    
    // close the file:
    myFile.close();
  } 
  else SENDLOG('W', "Error opening file ", fileName); 
  
  RESTORE_CONTEXT
}

void HA_web::handleAjaxGet(EthernetClient client, char* actionline, char type){        // Used to process Ajax GET; actionline points to first char after 'R', 'T' or 'P' - gets overwritten
	SAVE_CONTEXT("Web3")

const byte offsetDay = 11;
const byte offsetHour = 6;


  char responseText[100];
  char element[ELEM_BUFSIZE] = "";
  char *newVal;
  const char str1[] = "{\"id\": \"";
  const char str2[] = "\", \"reading\": \"";
  const char str3[] = "\", \"status\": \"";
  const char str4[] = "\"}";
  byte deviceIdx, devStatus, timeHr, timeMin;
  unsigned int devReading;
  unsigned long timestarted = millis();
  
  *strstr(actionline," HTTP") = 0;                      // Terminate string before " HTTP/1.1"
  if ((newVal = strstr(actionline,"=")) != 0) {          // See if there is an assignment (type = 'P')
    devReading = atoi(newVal+1);                          // Save the new value
    *newVal = 0;                                        // And set new termination point
  }
  strncpy (element, actionline, ELEM_BUFSIZE);    // Start of Actionline contains an element id, shared with browser
  
  switch (type) {      
    case 'H':                                            // Heartbeat required
      sprintf (responseText, "%s%s%s%u%s%d%s", str1, element, str2, heartBeat, str3, 2, str4);
      break;    
    case 'R':                                            // Reading required
/*      deviceIdx = getDeviceIdx(element);                    // Get index of element in device or variable array
      devReading = deviceMap.get(deviceIdx, valCurr);
      devStatus = deviceMap.get(deviceIdx, VAL_STATUS); */
/*
      sprintf (responseText, "%s%s%s%d%s%d%s", str1, element, str2, (unsigned int)random(32000), str3, 2, str4); 
      break;
    case 'T':                                            // Time required in format hh:mm
      if (timeStatus() == timeSet) { 
	      devReading = (weekday() * pow(2, offsetDay)) + (hour() * pow(2, offsetHour)) + minute(); 
	      devStatus = STATUS_STABLE; 
	    }
      else { 
	      timeHr = millis()/1000/SECS_PER_HOUR; 
	      timeMin = millis()/1000/SECS_PER_MIN; 
	      devStatus = STATUS_UNAVAILABLE; 
	    }
      sprintf (responseText, "%s%s%s%d%s%d%s", str1, element, str2, devReading, str3, devStatus, str4); 
      break;
    case 'P':                                          // Put required
/*      deviceIdx = getDeviceIdx(element);                    // Get index of element in device or variable array
      deviceMap.put(deviceIdx, valCurr, devReading);
      deviceMap.put(deviceIdx, VAL_STATUS, devStatus = VAL_STATUS_STABLE);
      sprintf (responseText, "%s%s%s%d%s%d%s", str1, element, str2, devReading, str3, devStatus, str4); */
/*      
      break;
    default:      Serial.println("Unrecognised ajax GET");
  }

  client.println("HTTP/1.1 200 OK");
  client.print("Server: Arduino/");
  client.println(arduinoMe);
  client.println("Content-Type: text");
  client.print("Content-Length: ");
  client.println(strlen(responseText));    
  client.println();
  client.print(responseText); 
  
  RESTORE_CONTEXT
}



void HA_web::handleHTTPCmd(EthernetClient client, char* actionline){        // Used to process POST and GET /? strings
	SENDLOG('I', "Action line = ", actionline);
  /*
  if (strstr(actionline,"=On")) {
    digitalWrite (ledPin,HIGH);
    Serial.println("Switching heating on");
    ledState = 1;
  }
  else if (strstr(actionline,"=Off")) {
    digitalWrite (ledPin,LOW);
    Serial.println("Switching heating off");
    ledState = 0;
  } 
  */
  /*
}


void HA_web::stopClient(EthernetClient client) {
//  delay(2);
//  client.stop();
}

// Create global object
HA_web web;

*/


