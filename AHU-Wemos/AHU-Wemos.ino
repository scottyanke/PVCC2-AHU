/* ========================================================================================
 *  This specific program is for Wemos D1 Mini's used with Cypress PSOCs on the air handlers.
 *  It off loads all of the sensor processing to the PSOC, including talking to any modbus 
 *  devices and DS18B20s for temperature readings.  The only purpose this Wemos (ESP8266) device
 *  serves is to communicate through the wifi guest network in the building to an mqtt server.
 *  The Wemos has one serial connection to the PSOC, and the network connections to the buildng
 *  wifi, but that's it.  Look to the PSOC Air Handler programs (one for MAX31855 and the other
 *  for MAX31865) to see all of the sensor connections.  Both PSOC types support the same kind
 *  of connection to the mqtt server. 
 * ======================================================================================== */
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
// This is a small web server so that configuration changes can be made without attaching to any other network
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
// AsyncMqttClient is used instead of PubSub, because it allows for QOS 2, and seems to work more reliably
#include <AsyncMqttClient.h>

// The standard time functions, including one for timezones.
#include <TimeLib.h>
#include <Timezone.h>
tmElements_t tm, tm2;
int Year, Month, Day, Hour, Minute, Seconds, Wday ;
bool time_was_set = false;

// The EEPROM functions are used so that the Wemos can store configuration data in the event
// of power failures.
#include <EEPROM.h>
//========================================================================================
// This device includes the ability to update over-the-air, using the standard Arduino
// library.  It is normally not connected to the guest wifi, but it is an AP all on its own,
// and OTA changes can be done through that method.  Just make sure that the PC side that
// has the update is connected to the 192.168.4.1 address, just like looking at the status
// or configuring the device.  With the ESPOTA.PY program, just use the 192.168.4.1 address
// while connected to the AP, and it will do the update just like serving the web pages.
// There is no need to even have Arduino on the portable PC side, as ESPOTA.PY is a command
// line python program that will connect to the appropriate address (always 192.168.4.1 in
// this case) and upload the binary, which has been assumed to be copied from the master PC
// anyway.  The command line looks like "python3 espota.py -r -i 192.168.4.1 -f AHU-Mmqtt.ino.bin",
// which will also report the progress of the copy (using the -r) to the PC.  The bin file
// would have been copied from the /tmp directory on the master machine.
//========================================================================================
#include <ArduinoOTA.h>

// Common defines
const char VERSION[] = __DATE__ " " __TIME__;
#define WIFI_SSID "my_ssid"
#define WIFI_PASSWORD "the_ssid_password"
#define WEBNAME "http://pvcc-hvac.6te.net/index.html"

#define MQTT_HOST IPAddress(192, 168, 0, 104)
#define MQTT_PORT 1883
#define MQTT_NAME "AHU"
// Poll the PSOC every 5 minutes, plus a few seconds just to keep things from happening at the same time
#define TIME_BETWEEN_POLLING 303000
// 1800 seconds is half an hour
#define BLAST_INTERVAL 1800000

// The next few lines are used for the local timezone, since web servers supply world time
#define HTTP_TIME_SOURCE "http://mn.gov"
// US Central Time Zone (Chicago)
// This next line sets the correct time offset for the start of Central Daylight Time, on the second Sunday of March at 2am
TimeChangeRule myDST = {"CDT", Second, Sun, Mar, 2, -300};    // Daylight time = UTC - 5 hours
// and the mySTD variable is set for Central Standard Time, which starts the first Sunday in November
TimeChangeRule mySTD = {"CST", First, Sun, Nov, 2, -360};     // Standard time = UTC - 6 hours
Timezone myTZ(myDST, mySTD);

// Global class defines for some of the WiFi uses this device communicates through
HTTPClient http;
WiFiClient client;

ESP8266WebServer server(80);  // set the web server to port 80
AsyncMqttClient mqttClient;

// Global variables used througout this program
bool ssidFound;     // Set if a scan finds the SSID we are expecting to connect to.
char my_ssid[30];           // The SSID is generated from the last 3 bytes of this devices MAC address, so it can be used as a web server
long request_wait = 0;   // This is used to control when polling takes place, and is a millisecond value that can go negative
uint8_t request_state = 0;
uint16_t retry_count = 0;
long blast_time;    // used to force mqtt data sends
uint32_t read_time; // Only used to show on the web status page how long ago the readings were taken
String wifi_ssid;   // The SSID of the local guest network in the building
String wifi_password; // The password for the above SSID
String WebName;     // The WebName variable has the name of the external web server used as a fallback for finding the mqtt address.
byte mac_address[8];  // The unique address of this device, used to tell who is sending data and as the SSID for web access.
IPAddress mqtt_host;  // This is used for the IP address of the mqtt server to connect to.
IPAddress apIP(192, 168, 4, 1);  // The local address of this device when accessed through the web interface.

// QUEUE_WIDTH is the maximum size of an mqtt packet, and is a fixed length for this table
// QUEUE_MAX is the maximum number of entries that can be queued to be transmitted via mqtt
// QUEUE_ARRAY is the byte size of the queue table used for all of the queue entries.
// Note that the size of this table really impacts dynamic memory in the Wemos, and reduces space for local variables.
// Think carefully about the maximum number of queued entries.
#define QUEUE_WIDTH 128
#define QUEUE_MAX 240
#define QUEUE_ARRAY QUEUE_WIDTH * QUEUE_MAX
char queue[QUEUE_ARRAY];  // The global queue where all messages end up before being sent to the mqtt server
uint16_t queue_pos = 0;
uint8_t queue_len = 0;

// The eeprom information is used to store the connection information in the EEPROM on this device, so it can
// survive after a reboot or firmware update.
uint eeprom_addr = 0;
struct {
  char validate[6] = "";  // will have the word valid when it is valid
  char ssid[20] = "";
  char password[20] = "";
  IPAddress mqtt = IPAddress(0, 0, 0, 0);
  char webname[64] = "";
} eeprom_data;

// The next two variables are used for obtaining the time from a trusted external web page.
const char * headerKeys[] = {"date", "server"} ;
const size_t numberOfHeaders = 2;

//========================================================================================
// The setup() procedure takes care of reading the eeprom for any stored data, like the
// connection information and the address of the mqtt server, along with setting up the
// time, intializing the variables used during the loop() procedure, and getting the server
// side setup in case anything needs to be changed on the fly.
//========================================================================================
void setup()
{
  char ts[128];   // Primarily used for temporary strings of data, though not String data.
  uint8_t i,j;    // Small integers just used for counting

  queue_pos = 0;
  queue_len = 0;
  request_state = 0;
  pinMode(LED_BUILTIN, OUTPUT);    // This is really just an LED on the Wemos D1 Mini
  digitalWrite(LED_BUILTIN, HIGH); // Turn the light off
  Serial.begin(9600);     // All serial communications are to the PSOC, and 9600 is fine for what we are doing.

  EEPROM.begin(512);  // try to get the connection info from EEPROM
  EEPROM.get(eeprom_addr,eeprom_data);
  if (!strcmp(eeprom_data.validate,"valid"))  // Is there a valid entry written in the EEPROM of this device
  {
    wifi_ssid = String(eeprom_data.ssid);
    wifi_password = String(eeprom_data.password);
    mqtt_host = eeprom_data.mqtt;
    WebName = String(eeprom_data.webname);
  }
  else  // Default to the defines is the EEPROM has not yet been programmed.
  {
    wifi_ssid = WIFI_SSID;
    wifi_password = WIFI_PASSWORD;
    mqtt_host = MQTT_HOST;
    WebName = WEBNAME;
  }
  mqttClient.setServer(mqtt_host, MQTT_PORT); // Set the name of the mqtt server.

  ScanForWifi();  // Look for the defined wifi AP, and get the time if found

  blast_time = millis() + 60000;  // do the first blast in 1 minute, after the PSOC is up and running
  request_wait = millis() + TIME_BETWEEN_POLLING;  // The first regular polling is done after the first blast
  time_was_set = false;
  read_time = millis(); // the time we started

  WiFi.macAddress(mac_address);     // get the mac address of this chip to make it unique in the building
  sprintf(my_ssid,"%02X%02X%02X",mac_address[3],mac_address[4],mac_address[5]); // Use the hex version of the MAC address as our SSID
  WiFi.softAP(my_ssid, "87654321");             // Start the access point with password of 87654321

  delay(250);
  while (Serial.available())  // flush out the input buffer
  {
    Serial.read();
    delay(10);  // dumb delay, but needed for some odd reason
  }

  // Setup what to do with the various web pages used only for configuring/testing this device
  server.on("/config", handleConfig); // if we are told to change the configuration, jump to that web page
  server.on("/update", handleUpdate); // The handleUpdate() procedure is for program replacements done via OTA
  server.on("/", handleStatus);       // The most basic web request returns the status information
  server.on("/updateOTA", HTTP_POST, []()   // This chunk of code comes directly from the WebUpdate.ino example supplied under ESP8266WebServer.
  {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.setDebugOutput(true);
      WiFiUDP::stopAll();
      Serial.printf("Update: %s\n", upload.filename.c_str());
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      if (!Update.begin(maxSketchSpace)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
      Serial.setDebugOutput(false);
    }
    yield();
  });
  // End of setting up the web server pages.

  server.begin();   // Start listening for connections to this devices AP.  Most of the time it's not needed, but helpful...
  delay((mac_address[5] + (mac_address[4] << 8)) & 0xBBF);  // do a random (based on mac address) delay just so everything doesn't try to talk at once

  // Another method of doing OTA updates.  It will probably be removed.
  ArduinoOTA.setHostname(my_ssid);
  ArduinoOTA.onError([](ota_error_t error) {  // the OTA process will call this next set of lines if needed.
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();   // Listen for OTA updates, which will probably come in over the local 192.168.4.1 address

  // This is the end of the setup routine.  Posting the VERSION helps identify that everything really is at the same version throughout the building.
  sprintf(ts,"R|000|Running v.%s",VERSION);
  add_to_queue(ts);   // tell the mqtt server that this device is up and running.  It isn't needed, but helpful.
  xmit_the_queue();   // xmit_the_queue() actually connects to the network, connects to the mqtt server, and sends the queued up data.
}

//========================================================================================
// Search for the desired access point, and if it is found, connect and get the time in
// the getTimeFromHttp() procedure.
//========================================================================================
void ScanForWifi()
{
  ssidFound = false;
  int n = WiFi.scanNetworks();  // Do a simple network check to see what's available.
  if (n == 0)
  {
    ; // No wifi networks were found, which can happen in the tunnels.
  }
  else
  {
    for (int i = 0; i < n; ++i)
    {
      if (wifi_ssid == WiFi.SSID(i))  // Check to see if the SSID we are supposed to connect to has been found by the scan
      {
        ssidFound = true;
        getTimeFromHttp();    // If the SSID was found then use it and get the time of day from the internet
        break;  // It was found, so stop looking.
      }
    }
  }
}

//========================================================================================
// Pull the date and time using an HTTP request.  This is because the NTP port is blocked
// on the guest network that this system runs on.  It is annoying, but easier just to use
// the method of getting the time from a trusted web page.  This procedure will in turn
// call the timeElements() procedure with the timestring it got, which will compensate for
// the location and daylight savings.
//========================================================================================
void getTimeFromHttp() {
  int httpCode;
  String headerDate;

  connectToWifi();    // Connect to whatever guest network we are supposed to be on.
  if (WiFi.status() != WL_CONNECTED)  // If we couldn't connect through WiFi, then there's no point in continuing.
  {
    WiFi.disconnect();
    return;
  }
  http.begin(client,HTTP_TIME_SOURCE);   // A generally reliable place to get the date and time
  http.collectHeaders(headerKeys, numberOfHeaders);   // Just look at the headers
  httpCode = http.GET();
  if (httpCode > 0)
  {
    headerDate = http.header("date"); // We only want the date and time from the headers
    // headerDate looks like Sat, 19 Oct 2019 06:29:57 GMT
    timeElements(headerDate.c_str());   // set the date and time using what we got from the http request
  }
  http.end();   // We are done with our HTTP request for about 24 hours
  WiFi.disconnect();
}

//========================================================================================
// Parse the printable version of the date and time we got through HTTP into the appropriate
// format for setting the date and time within the Arduino program.  This takes a string
// value from an HTTP header.
//========================================================================================
void timeElements(const char *str)
{
  char t[80];
  int j;
  // Sat, 19 Oct 2019 06:29:57 GMT
  char delimiters[] = " :,";    // Used to separate the date and time fields
  char* valPosition;

  strcpy(t,str);
  valPosition = strtok(t, delimiters);
  j = 0;
  while(valPosition != NULL){
    switch(j) {
      case 0:   // Convert day of week into the proper Wday format.
        if (!strcmp(valPosition,"Sun")) Wday = 1;
        if (!strcmp(valPosition,"Mon")) Wday = 2;
        if (!strcmp(valPosition,"Tue")) Wday = 3;
        if (!strcmp(valPosition,"Wed")) Wday = 4;
        if (!strcmp(valPosition,"Thu")) Wday = 5;
        if (!strcmp(valPosition,"Fri")) Wday = 6;
        if (!strcmp(valPosition,"Sat")) Wday = 7;
        break;
      case 1: // Convert the day of the month into an integer.
        Day = atoi(valPosition);
        break;
      case 2: // Convert the month name into a simple integer.
        if (!strcmp(valPosition,"Jan")) Month = 1;
        if (!strcmp(valPosition,"Feb")) Month = 2;
        if (!strcmp(valPosition,"Mar")) Month = 3;
        if (!strcmp(valPosition,"Apr")) Month = 4;
        if (!strcmp(valPosition,"May")) Month = 5;
        if (!strcmp(valPosition,"Jun")) Month = 6;
        if (!strcmp(valPosition,"Jul")) Month = 7;
        if (!strcmp(valPosition,"Aug")) Month = 8;
        if (!strcmp(valPosition,"Sep")) Month = 9;
        if (!strcmp(valPosition,"Oct")) Month = 10;
        if (!strcmp(valPosition,"Nov")) Month = 11;
        if (!strcmp(valPosition,"Dec")) Month = 12;
        break;
      case 3: // Convert the year being passed into an integer.
        Year = atoi(valPosition);
        break;
      case 4: // Convert the printed hour into an integer.
        Hour = atoi(valPosition);
        break;
      case 5: // Convert the minute.
        Minute = atoi(valPosition);
        break;
      case 6: // An finally, convert the seconds passed into an integer.
        Seconds = atoi(valPosition);
        break;
    }
    j++;  // Look at the next position in the string
    //Here we pass in a NULL value, which tells strtok to continue working with the previous string
    valPosition = strtok(NULL, delimiters); // Use the strtok function to break the string into pieces
  }

  tm.Year = Year - 1970;  // Use the UNIX epoch since it is an offset
  tm.Month = Month;       // Setup the rest of the time values
  tm.Day = Day;
  tm.Hour = Hour;
  tm.Minute = Minute;
  tm.Second = Seconds;
  tm.Wday = Wday;

  setTime(myTZ.toLocal(makeTime(tm)));  // Call the myTX class to convert the date and time to the correct timezone, including daylight savings.

}

//========================================================================================
// Connect to the local guest network for long enough to either get the time, or to transmit
// a mqtt packet to the server.  Realistically, this program does a connection about every
// 30 minutes, maybe more often if it is monitoring something that changes frequently.
//========================================================================================
void connectToWifi()
{
  uint8_t i;
  if (!ssidFound)  // Check to see if the scan done at boot time actually found the SSID we want
    return;
  WiFi.disconnect();    // Sometimes we just need to reset everything.
  delay(50);
  WiFi.begin(wifi_ssid, wifi_password);
  i = 0;
  while (WiFi.status() != WL_CONNECTED && i < 120)   // Need to keep looking for about 15 seconds, because, yes, it can take that long to connect
  {
    i++;
    delay(125);
  }
}

//========================================================================================
// The getNewMqtt() procedure connects to the guest network, then gets the address of the
// mqtt server from a well known (to this program) website, just in case it changed.  Normally
// it wouldn't change, but because everything is running on the guest wifi network, anything
// could change at any time.
//========================================================================================
void getNewMqtt()   // gets a new mqtt server address if it changes by checking a web site
{
  char ts[128];
  IPAddress ip;

  connectToWifi();

  if (http.begin(client,WebName))
  {  // HTTP connection to the public server and page that has the mqtt address.
    // start connection and send HTTP header
    int httpCode = http.GET();
        // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been sent and Server response header has been handled
      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY || httpCode == HTTP_CODE_FOUND)
      {
        String payload = http.getString();
        payload.trim();   // get rid of the trailing newline
        ip.fromString(payload);
          sprintf(ts,"R|000|New mqtt address is going to be %d.%d.%d.%d",ip[0], ip[1], ip[2], ip[3]);
          add_to_queue(ts);
        if (mqtt_host != ip)  // check to see if the ip address changed
        {
          mqtt_host = ip;
          mqttClient.setServer(mqtt_host, MQTT_PORT);
          strcpy(eeprom_data.validate,"valid");
          wifi_ssid.toCharArray(eeprom_data.ssid,wifi_ssid.length() + 1);
          wifi_password.toCharArray(eeprom_data.password,wifi_password.length() + 1);
          eeprom_data.mqtt = IPAddress(ip[0], ip[1], ip[2], ip[3]);
          WebName.toCharArray(eeprom_data.webname,WebName.length() + 1);
          eeprom_addr = 0;
          EEPROM.put(eeprom_addr,eeprom_data);
          EEPROM.commit();
          add_to_queue((char *)"R|000|EEPROM has been changed");
          sprintf(ts,"R|000|New mqtt address is %d.%d.%d.%d",ip[0], ip[1], ip[2], ip[3]);
          add_to_queue(ts);
        }
      }
    } else {
      add_to_queue((char *)"E|[HTTP] GET... failed");
    }
  }
  http.end();
  delay(500);   // this is important.  Don't remove this delay
  WiFi.disconnect();
}

//========================================================================================
// The add_to_queue() procedure adds a character string to the mqtt queue.  As part of that
// process, it will also timestamp and format the added line.  It doesn't send anything,
// it just queues it internally to the Wemos.
//========================================================================================
void add_to_queue(char* str)
{
  char ts[128];

  // Everything added to the queue has the same prefix and suffix
  // The my_ssid part is simply diagnostics, except in the case of the boiler monitor.
  sprintf(ts, "%-6s|%04d-%02d-%02d %02d:%02d:%02d|%s|", my_ssid, year(), month(), day(), hour(), minute(), second(),str);
  ts[QUEUE_WIDTH - 1] = '\0';   // This is a safety thing, to make sure each queue entry isn't longer than it should be
  if (queue_len < (QUEUE_MAX))  // add to the queue if there is room
  {
    strcpy(queue+queue_pos,ts);
    queue_pos += QUEUE_WIDTH;
    queue_len++;
  }
}
//========================================================================================
// The xmit_the_queue() procedure takes all of the queued entries (added by add_to_queue())
// and sends them to the mqtt server.  It does this by connecting to the guest network, then
// sending the queued items in first-in-first-out order to the mqtt system.  After the queue
// has been cleared, it disconnects from the mqtt server, waits, then disconnects from the
// wifi guest network.
//========================================================================================
void xmit_the_queue()
{
  char ts[128];
  uint8_t j;
  uint16_t packetIdPub2;
  uint16_t queue_position;

  if (!queue_len)  // no sense connecting if there's nothing to send
    return;
  connectToWifi();
  if (WiFi.status() != WL_CONNECTED)
    return;
  mqttClient.connect();
  j = 0;
  while (!mqttClient.connected() && j < 200)  // give it up to 4 seconds to connect to mqtt
  {
    delay(20);
    j++;
  }
  if (mqttClient.connected() && queue_len)
  {
    queue_position = 0; // Used for getting info out of the queue FIFO, versus pulling from the end
    do
    {
      queue_len--;
      strcpy(ts,queue + queue_position);
      queue_position += QUEUE_WIDTH;
      if (strlen(ts) > 0)
      {
        // Message is being published with the 'Clean' session under QOS 2.
        packetIdPub2 = mqttClient.publish(MQTT_NAME, 2, true, ts);  // topic, qos, retain, payload, length=0, dup=false, message_id=0
        delay(250);   // We do have to wait for it to clear
      }
    } while (queue_len > 0);
    queue_pos = 0;    // reset the queue_pos for the next entries to be added to the queue in the future
    mqttClient.disconnect();
  };
  delay(500);   // this is important.  Don't remove this delay
  WiFi.disconnect();
}

//========================================================================================
// This is the main processing loop for the system.
// The loop itself is really small.  Essentially it sends requests to the PSOC at certain
// intervals, and queues up the resulting responses.
//========================================================================================
void loop()
{
  char ts[128];   // ts is a temporary array of characters, basically a string without the issues.

  if (millis() < 1000)   // handle millis() rollovers that happen every 49.7 days
  {   // basically, just reset everything to startup values
    request_wait = millis() + TIME_BETWEEN_POLLING;
    blast_time = millis() + BLAST_INTERVAL;
    delay(1000);  // just to get past millis being under 1000
  }
  ArduinoOTA.handle();      // One of the ways that over-the-air updates can be made.
  server.handleClient();    // used to handle configuration changes through the web interface

  switch(request_state)
  {
    case 0:
      if (millis() > request_wait || millis() > blast_time)
      {
        request_state++; // wait state is switched to do the request
        if (millis() > blast_time)
        {
          ScanForWifi();
          getNewMqtt();   // only do this before the larger blast interval
        }
      }
      break;
    case 1:
      read_time = millis();
      digitalWrite(D4, LOW);
      Serial.write(0x1b); // Send the escape character out the serial port to the PSOC
      delay(10);  // just to try and resolve something...
      if (millis() > blast_time)  // force an output about once an hour
      {
        blast_time = millis() + BLAST_INTERVAL;
        Serial.write(0x32);  // Forced request, just means once an hour send a 2 instead of a 1 to get all data from the PSOC
      }
      else
      {
        Serial.write(0x31);  // send the standard, limited request to the PSOC by using a 1
      }
      request_wait = millis() + TIME_BETWEEN_POLLING;   // Do the next one in however many minutes
      request_state++;  // update to show we are waiting for a response
      break;
    case 2:
      if (!getLine(ts))   // if getLine returns false then it timed out
      { // timed out, so abandon any further reads
        retry_count++;
        if (retry_count < 5)
        {
          if (retry_count > 2)  // only say something if it keeps happening repeatedly
          {
            sprintf(ts,"E|%03d|Polling the PSOC timed out",retry_count);
            add_to_queue(ts);
          }
          request_wait = millis() + 20000;  // try again in 20 seconds
        }
        request_state = 0;
        digitalWrite(LED_BUILTIN, HIGH);
      }
      else
      {
        retry_count = 0;  // reset this because we successfully got an answer from the PSOC.
        if (strncmp(ts,"DONE",4))   // Remember the strncmp returns non-zero if the strings DONT match.
          add_to_queue(ts);   // We did NOT get the DONE, so add the returned string to the queue.
        else
        { // the DONE line is what triggers this next section - Remember that strncmp returns 0 if the strings match.
          ts[0] = 0;  // part of handling timeouts
          request_state = 0;
          xmit_the_queue();   // send anything that was received after we get the DONE
          digitalWrite(LED_BUILTIN, HIGH);  // Turn off the indicator light, which is just a diagnostic
        }
      }
      break;
  }

  // Things get interesting at 2am.
  if (hour() == 2 && !time_was_set) // Get the time at 2am-ish, so we aren't too far off as time passes
  {                                 // The time_was_set variable is used so we only get the time once a day.
    ScanForWifi();  // this gets the time as part of its function
    time_was_set = true;  // say we got the 2am time so we don't loop around and try again immediately
  }
  if (hour() > 2)
    time_was_set = false; // after 2am it is ok to reset this flag.
}

//========================================================================================
// Get a line of data from the serially connected PSOC.  It allows for timeouts of the
// serial process, as the PSOC might be too busy to respond.
//========================================================================================
bool getLine(char * buffer)
{
  uint8_t idx = 0;  // The index of where the put the character in the buffer.
  char c; // The character we received through the serial port.
  long maxwait = millis() + 2000;   // wait up to two seconds for something
  do
  {
    while (Serial.available() == 0)  // wait for a char, this causes the blocking
    {
      if (millis() > maxwait) // if millis() is greater then maxwait then we timed out
        return(false);
    }
    if (Serial.available() > 0)
    {
      c = Serial.read();
      if (c == '\r')
        c = 0;
      buffer[idx++] = c;
      if (idx > 127)
        idx = 127;    // limit the size of the receive buffer
    }
  }
  while (c != '\n');
  buffer[idx] = 0;    // Terminate the returned buffer.
  return(true);
}


//========================================================================================
// handleConfig() is a procedure called by the web server.  It allows for changes to the
// network, the address of the mqtt server, and the public name of the web site that also
// has the mqtt server address.  Without this procedure the device will never connect to
// the local wifi network, unless everything was setup properly in eeprom.
//========================================================================================
void handleConfig() {
  char ts[1000];
  // note that embedded in this form are lots of variable fields pre-filled with the sprintf
  if (server.hasArg("ssid")&& server.hasArg("Password")&& server.hasArg("MQTT_IP")) //If all form fields contain data call handleSubmit()
    handleSubmit();
  else // Display the form
  {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "-1");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    // here begin chunked transfer
    server.send(200, "text/html");
    server.sendContent("<!DOCTYPE HTML><html><head><meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">" \
        "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">");
    server.sendContent("<title>Configuration</title><style>\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\"</style></head>");
    server.sendContent("<body><h1>Configuration</h1><FORM action=\"/config\" method=\"post\">");
    sprintf(ts,"<P><label>SSID:&nbsp;</label><input maxlength=\"30\" value=\"%s\" name=\"ssid\"><br>",wifi_ssid.c_str());
    server.sendContent(ts);
    sprintf(ts,"<label>Password:&nbsp;</label><input maxlength=\"30\" value=\"%s\" name=\"Password\"><br>",wifi_password.c_str());
    server.sendContent(ts);
    sprintf(ts,"<label>MQTT IP:&nbsp;</label><input maxlength=\"15\" value=\"%d.%d.%d.%d\" name=\"MQTT_IP\"><br> ",mqtt_host[0],mqtt_host[1],mqtt_host[2],mqtt_host[3]);
    server.sendContent(ts);
    sprintf(ts,"<label>HTTP Web Name:&nbsp;</label><input maxlength=\"63\" value=\"%s\" name=\"WebName\"><br> ",WebName.c_str());
    server.sendContent(ts);
    server.sendContent("<INPUT type=\"submit\" value=\"Send\"> <INPUT type=\"reset\"></P>");
    sprintf(ts,"<P><br>I think the time is %04d-%02d-%02d %02d:%02d:%02d</P></FORM></body></html>",year(), month(), day(), hour(), minute(), second());
    server.sendContent(ts);
  }
}

//========================================================================================
// Report on the status of everthing through the web interface, mainly from 192.168.4.1.
// This routine is very helpfull for ensuring everthing is setup properly, from the wifi
// network to the connection to the PSOC.
//========================================================================================
void handleStatus()
{
  char ts[200];
  int16_t last_read = (millis() - read_time) / 1000;

  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  // here begin chunked transfer
  server.send(200, "text/html");
  server.sendContent("<!DOCTYPE HTML><html><head><meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">" \
      "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">");
  sprintf(ts,"<html><head><title>%s Status</title>",my_ssid);
  server.sendContent(ts);
  server.sendContent("</head><body>");
  sprintf(ts,"<P><h1>%s Status</h1><br>Version %s<br>",my_ssid, VERSION);
  server.sendContent(ts);
  sprintf(ts,"WiFi SSID is %s<br>",wifi_ssid.c_str());
  server.sendContent(ts);
  sprintf(ts,"WiFi password is %s<br>",wifi_password.c_str());
  server.sendContent(ts);
  sprintf(ts,"MQTT host is %d.%d.%d.%d<br>",mqtt_host[0],mqtt_host[1],mqtt_host[2],mqtt_host[3]);
  server.sendContent(ts);
  sprintf(ts,"Fallback name is %s<br>",WebName.c_str());
  server.sendContent(ts);
  sprintf(ts,"Last reading was %d seconds ago.<br>",last_read);
  server.sendContent(ts);
  sprintf(ts,"There are %d queued entries.<br>",queue_len);
  server.sendContent(ts);
  sprintf(ts,"I think the time is %04d-%02d-%02d %02d:%02d:%02d</P>", year(), month(), day(), hour(), minute(), second());
  server.sendContent(ts);

  server.sendContent("Reading PSOC devices...<br>");
  request_state = 1;
  while (request_state > 0)
  {
    switch(request_state)
    {
      case 1:
        digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on
        Serial.write(0x1b); // Send the escape character
        delay(10);  // just to try and resolve something...
        Serial.write(0x32);  // Forced request
        request_state++;  // update to show we are waiting for a response
        break;
      case 2:
        if (!getLine(ts))   // if getLine returns false then it timed out
        { // timed out, so abandon any further reads
          retry_count++;
          if (retry_count < 4)
          {
            if (retry_count > 2)  // only say something if it keeps happening repeatedly
            {
              server.sendContent("Polling the PSOC timed out<br>");
              request_state = 0;
              digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off
            }
          }
          if (retry_count > 3)
          {
            request_state = 0;
            retry_count = 0;
            digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off
          }
        }
        else  // We didn't time out, so report what we received
        {
          retry_count = 0;  // reset this because we successfully got an answer from the PSOC
          if (strncmp(ts,"DONE",4))   // Remember the strncmp returns non-zero if the strings DONT match
          {
            server.sendContent(ts);
            server.sendContent("<br>");
          }
          else
          { // the DONE line is what triggers this next section - Remember that strncmp returns 0 if the strings match
            request_state = 0;
            server.sendContent("Finished with response from PSOC.<br>");
            digitalWrite(LED_BUILTIN, HIGH);  // Turn off the indicator light
          }
        }
    }
  }   // end of while loop

  server.sendContent("All sensors have been read");
  server.sendContent("</body></html>");
  server.sendContent(""); // this closes out the send
  server.client().stop();
}
//========================================================================================
// The handleSubmit() procedure takes whatever values were submitted by the /config page and
// updates the eeprom and global variables to the new ssid, password, mqtt server, etc.
// A restart is NOT needed after this procedure, as all of the communications is done on an
// as-needed basis using the global values that this updated.
//========================================================================================
void handleSubmit()
{//display values and write to memmory
  int i;
  IPAddress ip;
  String response="<!DOCTYPE HTML><html><head><meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">" \
      "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">";
  response += "<p>The ssid is ";
  response += server.arg("ssid");
  response +="<br>";
  response +="And the password is ";
  response +=server.arg("Password");
  response +="<br>";
  response +="And the MQTT IP Address is ";
  response +=server.arg("MQTT_IP");
  response +="<br>";
  response +="And the Web Name is ";
  response +=server.arg("WebName");
  response +="</P><BR>";
  response +="<H2><a href=\"/\">go home</a></H2><br>";

  server.send(200, "text/html", response);
  // write data to EEPROM memory
  strcpy(eeprom_data.validate,"valid");
  i = server.arg("ssid").length() + 1;
  server.arg("ssid").toCharArray(eeprom_data.ssid,i);
  i = server.arg("Password").length() + 1;
  server.arg("Password").toCharArray(eeprom_data.password,i);
  ip.fromString(server.arg("MQTT_IP"));
  eeprom_data.mqtt = IPAddress(ip[0], ip[1], ip[2], ip[3]);
  i = server.arg("WebName").length() + 1;
  server.arg("WebName").toCharArray(eeprom_data.webname,i);

  eeprom_addr = 0;
  EEPROM.put(eeprom_addr,eeprom_data);
  EEPROM.commit();
  wifi_ssid = String(eeprom_data.ssid);
  wifi_password = String(eeprom_data.password);
  WebName = String(eeprom_data.webname);
  mqtt_host = eeprom_data.mqtt;
  mqttClient.setServer(mqtt_host, MQTT_PORT);
  ScanForWifi();
  ESP.restart();
}
//========================================================================================
// The handleUpdate() procedure sets up a simple web page with a couple of really, really,
// small buttons.  One button is for the selection of a binary file to replace the firmware
// of this device.  The other does the actual upload, update, and reboots the device.
//========================================================================================
void handleUpdate()
{
  String response="<html><body><form method='POST' action='/updateOTA' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
  server.send(200, "text/html", response);
  server.sendContent("</body></html>");
  server.sendContent(""); // this closes out the send
  server.client().stop();

}

//========================================================================================
// A very simple web page is produced by handleNotFound() when an invalid web page is
// requested of this device.
//========================================================================================
void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  message +="<H2><a href=\"/\">go home</a></H2><br>";
  server.send(404, "text/plain", message);
}

//========================================================================================
// End of the program.
//========================================================================================
