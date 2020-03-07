/* ========================================================================================
 *  This specific program is for Wemos D1 Mini's used with Cypress PSOCs on the air handlers.
 *  It off loads all of the sensor processing to the PSOC, including talking to any modbus 
 *  devices and DS18B20s for temperature readings.  The only purpose this Wemos (ESP8266) device
 *  serves is to communicate through the wifi guest network in the building to an mqtt server.
 *  The Wemos has one serial connection to the PSOC, and the network connections to the buildng
 *  wifi, but that's it.  Look to the PSOC Air Handler programs (one for MAX31855 and the other
 *  for MAX31865) to see all of the sensor connections.  Both PSOC types support the same kind
 *  of connection to the mqtt server. 
  *  
 *  The IDE options for this device are:
 *  Board: LOLIN(WENOS) D1 R2 & mini
 *  Upload Speed: "921600"
 *  CPU Frequency: "80 Mhz"
 *  Flash Size: "4MB (FS:1MB OTA:~1019KB)"
 *  Debug port: "Disabled"
 *  Debug Level: "None"
 *  LwIP Variant: "V2 Lower Memory"
 *  VTables: "Flash"
 *  Exceptions: "Legacy (new can return nullptr)"
 *  Erase Flash: "Only Sketch"
 *  SSL Support: "All SSL ciphers (most compatible)"
 *  
 *  The library versions used by this program are:
 *  Wire at version 1.0
 *  ESP8266WiFi at version 1.0
 *  ESP8266HTTPClient at version 1.2
 *  ESP8266WebServer at version 1.0
 *  async-mqtt-client at version 0.8.2
 *  ESPAsyncTCP at version 1.2.0
 *  Time at version 1.6 in folder
 *  Timezone at version 1.2.4
 *  EEPROM at version 1.0
 *  
 *  Executable segment sizes:
 *  IROM   : 294536          - code in flash         (default or ICACHE_FLASH_ATTR) 
 *  IRAM   : 27452   / 32768 - code in IRAM          (ICACHE_RAM_ATTR, ISRs...) 
 *  DATA   : 1268  )         - initialized variables (global, static) in RAM/HEAP 
 *  RODATA : 2952  ) / 81920 - constants             (global, static) in RAM/HEAP 
 *  BSS    : 26312 )         - zeroed variables      (global, static) in RAM/HEAP 
* ======================================================================================== */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
// This is a small web server so that configuration changes can be made without attaching to any other local network
#include <ESP8266WebServer.h>
// AsyncMqttClient is used instead of PubSub, because it allows for QOS 2, and seems to work more reliably
#include <AsyncMqttClient.h>

// The standard time functions, including one for timezones.
#include <TimeLib.h>
#include <Timezone.h>
bool time_was_set = false;
uint16_t time_offset;
/**
 * This system keeps the time in UTC, and converts to local time on demand.  That way it can handle
 * the Daylight Savings issues that pop up twice a year.  This works out well as it gets the time
 * in UTC anyway...
 */
// US Central Time Zone (Chicago)
// This next line sets the correct time offset for the start of Central Daylight Time, on the second Sunday of March at 2am
TimeChangeRule myDST = {"CDT", Second, Sun, Mar, 2, -300};    // Daylight time = UTC - 5 hours
// and the mySTD variable is set for Central Standard Time, which starts the first Sunday in November
TimeChangeRule mySTD = {"CST", First, Sun, Nov, 2, -360};     // Standard time = UTC - 6 hours
Timezone myTZ(myDST, mySTD);  // These are used so much that they might as well be global

// The EEPROM functions are used so that the Wemos can store configuration data in the event
// of power failures.  Yes, there is one specifically for the ESP, but it parks the data all
// over the place, and makes it difficult to read.
#include <EEPROM.h>

// Common defines
const char VERSION[] = __DATE__ " " __TIME__;
#define WIFI_SSID "my_ssid"
#define WIFI_PASSWORD "the_ssid_password"
#define ALT_WIFI_SSID "Poiuytrewq"
#define ALT_WIFI_PASSWORD "sparky050504"
#define WEBNAME "X"

#define MQTT_HOST IPAddress(192, 168, 0, 104)
#define ALT_MQTT_HOST IPAddress(192, 168, 0, 104)
#define MQTT_PORT 1883
#define MQTT_NAME "AHU"
// Poll the PSOC every 5 minutes, plus a few seconds just to keep things from happening at the same time
#define TIME_BETWEEN_POLLING 303000
// 1800 seconds is half an hour
#define BLAST_INTERVAL 1800000
// MILLIS_MAX is the trip point for rebooting this device, due to weird problems connecting in the production environment after a few days.
// It will boot itself if the current millis() value is higher than this number, and it's 00:30 in the morning.
#define MILLIS_MAX 172800000

// The next few lines are used for the local timezone, since web servers supply world time.  The state's web server is a reliable choice.
#define HTTP_TIME_SOURCE "http://mn.gov"
int last_hour;    //  Simply used to limit how often the address is checked for, and diagnostics are reported

// Global class defines for some of the WiFi uses this device communicates through
HTTPClient http;
WiFiClient client;

ESP8266WebServer server(80);  // set the web server to port 80
AsyncMqttClient mqttClient;

// Global variables used througout this program
char my_ssid[12];           // The SSID is generated from the last 3 bytes of this devices MAC address, so it can be used as a web server
uint32_t request_wait;      // This is used to control when polling takes place, and is a millisecond value that can go negative
uint8_t request_state;
uint16_t retry_count;     // The number of tries attempted to get a response from the PSoC
uint32_t blast_time;      // used to force mqtt data sends
uint32_t read_time;     // Only used to show on the web status page how long ago the readings were taken
String wifi_ssid;       // The SSID of the local guest network in the building
String wifi_password;   // The password for the above SSID
String WebName;         // The WebName variable has the name of the external web server used as a fallback for finding the mqtt address.
byte mac_address[8];    // The unique address of this device, used to tell who is sending data and as the SSID for web access.
IPAddress mqtt_host;    // This is used for the IP address of the mqtt server to connect to.
IPAddress apIP(192, 168, 4, 1);  // The local address of this device when accessed through the web interface.

// QUEUE_WIDTH is the maximum size of an mqtt packet, and is a fixed length for this table
// QUEUE_MAX is the maximum number of entries that can be queued to be transmitted via mqtt
// QUEUE_ARRAY is the byte size of the queue table used for all of the queue entries.
// Note that the size of this table really impacts dynamic memory in the Wemos, and reduces space for local variables.
// Think carefully about the maximum number of queued entries.
#define QUEUE_WIDTH 128
// Warning - If QUEUE_MAX is too big, then the heap isn't big enough for OTA updates.  For example, 250 fails, but 200 works.
#define QUEUE_MAX 100
#define QUEUE_ARRAY QUEUE_WIDTH * QUEUE_MAX
// For the queue, malloc is used instead of reserving space here, or the heap just gets too big
char *queue;  // The global queue where all messages end up before being sent to the mqtt server
uint16_t queue_pos;
uint8_t queue_len;

// The eeprom information is used to store the connection information in the EEPROM on this device, so it can
// survive after a reboot or firmware update.
struct EEPROMStruct {
  char validate[6] = "";  // will have the word 'valid' when it is valid
  char ssid[20] = "";
  char password[20] = "";
  IPAddress mqtt = IPAddress(0, 0, 0, 0);
  char webname[64] = "";
  char debugging;
} eeprom_data;

uint8_t debugging_enabled;
// The next two variables are used for obtaining the time from a trusted external web page.
const char * headerKeys[] = {"date", "server"} ;
const size_t numberOfHeaders = 2;
// And this string is used for OTA updates of this program.
const char* serverIndex = "<form method='POST' action='/updateOTA' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
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
// anyway.  The command line looks like "python3 espota.py -r -i 192.168.4.1 -f AHU-Wemos.ino.bin",
// which will also report the progress of the copy (using the -r) to the PC.  The bin file
// would have been copied from the /tmp directory on the master machine.  Using a phone works great.
//========================================================================================

//========================================================================================
// A small procedure to try to clean up memory leaks
//========================================================================================
struct tcp_pcb;
extern struct tcp_pcb* tcp_tw_pcbs;
extern "C" void tcp_abort (struct tcp_pcb* pcb);
void tcpCleanup ()
{
  while (tcp_tw_pcbs != NULL)
  {
    tcp_abort(tcp_tw_pcbs);
  }
}

//========================================================================================
// The setup() procedure takes care of reading the eeprom for any stored data, like the
// connection information and the address of the mqtt server, along with setting up the
// time, intializing the variables used during the loop() procedure, and getting the server
// side setup in case anything needs to be changed on the fly.
//========================================================================================
void setup()
{
  char ts[128];   // Primarily used for temporary strings of data, though not String data.

  request_state = 0;
  pinMode(LED_BUILTIN, OUTPUT);    // This is really just an LED on the Wemos D1 Mini
  digitalWrite(LED_BUILTIN, HIGH); // Turn the light off
  Serial.begin(9600);     // All serial communications are to the PSOC, and 9600 is fine for what we are doing.
  Serial.setTimeout(2000);  // Use a 2 second timeout to talk to the PSoC.

  // Allocate memory for the queue at this time.  The queue is a global variable.
  queue_pos = 0;
  queue_len = 0;
  queue = (char *)malloc(QUEUE_ARRAY * sizeof(char));
  if (queue == NULL)
  {
    Serial.print("Failed to allocate the queue.\r\n");
  }
  add_to_queue((char *)"R|000|Booting........");  // Let the other end know we had to boot.

  EEPROM.begin(512);  // try to get the connection info from EEPROM
  EEPROM.get(0,eeprom_data);
  if (!strcmp(eeprom_data.validate,"valid"))  // Is there a valid entry written in the EEPROM of this device
  {
    wifi_ssid = String(eeprom_data.ssid);
    wifi_password = String(eeprom_data.password);
    mqtt_host = eeprom_data.mqtt;
    WebName = String(eeprom_data.webname);
    debugging_enabled = eeprom_data.debugging;
  }
  else  // Default to the defines if the EEPROM has not yet been programmed.
  {
    wifi_ssid = WIFI_SSID;
    wifi_password = WIFI_PASSWORD;
    mqtt_host = MQTT_HOST;
    WebName = WEBNAME;
    debugging_enabled = 0;
  }
  EEPROM.end();
  if (!ScanForWifi()) // This is a fallback because constantly remembering to change the EEPROM is a pain.
  {
    wifi_ssid = ALT_WIFI_SSID;
    wifi_password = ALT_WIFI_PASSWORD;
    mqtt_host = ALT_MQTT_HOST;
  }
  mqttClient.setServer(mqtt_host, MQTT_PORT); // Set the name of the mqtt server.

  WiFi.macAddress(mac_address);     // get the mac address of this chip to make it unique in the building
  sprintf(my_ssid,"%02X%02X%02X",mac_address[3],mac_address[4],mac_address[5]); // Use the hex version of the MAC address as our SSID
  WiFi.softAP(my_ssid, "87654321");             // Start the access point with password of 87654321
  WiFi.persistent(false); // Only change the current in-memory Wi-Fi settings, and does not affect the Wi-Fi settings stored in flash memory.

  time_offset = mac_address[4] * 128;   // Create a random interval for this device, based on its MAC address
  blast_time = millis() + 60000;  // do the first blast in 1 minute, after the PSOC is up and running
  request_wait = millis() + TIME_BETWEEN_POLLING + time_offset;  // The first regular polling is done after the first blast
  time_was_set = false;
  read_time = millis(); // the time we started

  ESP.wdtFeed();
  delay(250);
  while (Serial.available())  // flush out the input buffer
  {
    Serial.read();
    delay(10);  // dumb delay, but needed for some odd reason
  }

  // Setup what to do with the various web pages used only for configuring/testing this device
  server.on(F("/config"), handleConfig); // if we are told to change the configuration, jump to that web page
  server.on(F("/"), handleStatus);       // The most basic web request returns the status information

  // The update code is for doing OTA updates of this program.  Only /update is called by users.  The /updateOTA
  // code is called in the background, never directly by the users.
  server.on(F("/update"), HTTP_GET, []() 
  {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  server.on(F("/updateOTA"), HTTP_POST, []() 
  {
    server.sendHeader(F("Connection"), "close");
    server.send(200, F("text/plain"), (Update.hasError()) ? "FAIL" : "OK");
    delay(1000);    // Show it for 1 second, then restart the ESP8266 using a software command
    ESP.restart();
  }, []() 
  {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) 
    {
      uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
      Update.begin(maxSketchSpace);
    } 
    else 
      if (upload.status == UPLOAD_FILE_WRITE) 
        Update.write(upload.buf, upload.currentSize);
      else 
        if (upload.status == UPLOAD_FILE_END)
          Update.end(true);
    delay(1);
  }); 
  server.begin();   // Start listening for connections to this devices AP.  Most of the time it's not needed, but helpful...
  ESP.wdtFeed();

  getTimeFromHttp();  // get the time if we can.  It's done after we have our MAC address
  last_hour = 25; // Used at the end of the loop() procedure

  retry_count = 0;    // Used for tracking attempts at getting a response from the PSoC
  
  // This is the end of the setup routine.  Posting the VERSION helps identify that everything really is at the same version throughout the building.
  sprintf(ts,"R|%03ld|Booted V.%s",(millis() + 499) / 1000,VERSION);
  add_to_queue(ts);   // tell the mqtt server that this device is up and running.  It isn't needed, but helpful.
  if (debugging_enabled)
  {
    char ts1[45];
    uint32_t heap_free;
    uint16_t heap_max;
    uint8_t heap_frag;
    sprintf(ts,"R|000|Boot reason was:%s",ESP.getResetReason().c_str());
    add_to_queue(ts);
    sprintf(ts1,"R|000|Core Version %s",ESP.getCoreVersion().c_str());
    add_to_queue(ts1);
    sprintf(ts1,"R|000|SDK Version %s",ESP.getSdkVersion());
    add_to_queue(ts1);
    sprintf(ts1,"R|000|Boot Version %d",ESP.getBootVersion());
    add_to_queue(ts1);
    sprintf(ts1,"R|000|Boot mode %d",ESP.getBootMode());
    add_to_queue(ts1);
    sprintf(ts1,"R|000|CPU Freq %dMhz",ESP.getCpuFreqMHz());
    add_to_queue(ts1);
    sprintf(ts1,"R|000|VCC is %4.2f",((double)ESP.getVcc()/1000) * 1.1);
    add_to_queue(ts1);
    ESP.getHeapStats(&heap_free, &heap_max, &heap_frag);
    sprintf(ts1,"R|000|Free heap is %zu",heap_free);
    add_to_queue(ts1);
    sprintf(ts1,"R|000|Heap fragmentation is %d%%",heap_frag);
    add_to_queue(ts1);
    sprintf(ts1,"R|000|MaxFreeBlockSize is %d",heap_max);
    add_to_queue(ts1);
    sprintf(ts1,"R|000|getFreeContStack is %zu",ESP.getFreeContStack());
    add_to_queue(ts1);
  }
  xmit_the_queue();   // xmit_the_queue() actually connects to the network, connects to the mqtt server, and sends the queued up data.
}

//========================================================================================
// Pull the date and time using an HTTP request.  This is because the NTP port is blocked
// on the guest network that this system runs on.  It is annoying, but easier just to use
// the method of getting the time from a trusted web page.  This procedure will in turn
// call the timeElements() procedure with the timestring it got, which will compensate for
// the location and daylight savings.
// The returned value should be the time in seconds from 1970.  Or 0 if something went wrong.
//========================================================================================
time_t getTimeFromHttp() 
{
  int httpCode;
  String headerDate;
  time_t tt;

  tt = 0;
  if (debugging_enabled)
    add_to_queue((char *)"R|000|Getting the time");
  connectToWifi();    // Connect to whatever guest network we are supposed to be on.
  if (WiFi.status() != WL_CONNECTED)  // If we couldn't connect through WiFi, then there's no point in continuing.
  {
    WiFi.disconnect();
    return 0;
  }
  ESP.wdtFeed();
  http.begin(client,HTTP_TIME_SOURCE);   // A generally reliable place to get the date and time
  http.setReuse(false);                  // Don't try to hang onto old web pages
  http.collectHeaders(headerKeys, numberOfHeaders);   // Just look at the headers
  httpCode = http.GET();
  ESP.wdtFeed();
  if (httpCode > 0)
  {
    headerDate.reserve(35);
    headerDate = http.header("date"); // We only want the date and time from the headers
    // headerDate looks like Sat, 19 Oct 2019 06:29:57 GMT
    tt = timeElements((char *)headerDate.c_str());   // set the date and time using what we got from the http request
    if (debugging_enabled)
      add_to_queue((char *)"R|000|Got the time");
  }
  http.end();   // We are done with our HTTP request for about 24 hours
  delay(100);   // The delays are necessary
  WiFi.disconnect();
  delay(100);
  tcpCleanup(); // Close any open connections
  return tt;
}

//========================================================================================
// Parse the printable version of the date and time we got through HTTP into the appropriate
// format for setting the date and time within the Arduino program.  This takes a string
// value from an HTTP header.
//========================================================================================
time_t timeElements(char *str)
{
  tmElements_t tm;
  int Year, Month, Day, Hour, Minute, Seconds, Wday ;
  int j;
  // An example of a time string to convert is - Sat, 19 Oct 2019 06:29:57 GMT
  char delimiters[] = " :,";    // Used to separate the date and time fields
  char* valPosition;
  Year = 1970;    // Set some base default values
  Month = 1;
  Day = 1;
  Hour = 0;
  Minute = 0;
  Seconds = 0;
  Wday = 1;

  valPosition = strtok(str, delimiters);
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

  ESP.wdtFeed();
  setTime(makeTime(tm));  // Set the time as UTC
  return makeTime(tm);
}

//========================================================================================
// Search for the desired access point, and if it is found, return 1
//========================================================================================
uint8_t ScanForWifi()
{
  int n = WiFi.scanNetworks();  // Do a simple network check to see what's available.
  if (n == 0)
  {
    ; // No wifi networks were found, which can happen in the tunnels.
    return(0);
  }
  else
  {
    for (int i = 0; i < n; ++i)
      if (wifi_ssid == WiFi.SSID(i))  // Check to see if the SSID we are supposed to connect to has been found by the scan
        return(1);
  }
  return(0);
}

//========================================================================================
// Connect to the local guest network for long enough to either get the time, or to transmit
// a mqtt packet to the server.  Realistically, this program does a connection about every
// 30 minutes, maybe more often if it is monitoring something that changes frequently.
//========================================================================================
void connectToWifi()
{
  uint8_t i;
  if (WiFi.status() == WL_CONNECTED)
    return;    // No point in connecting if we already are...
  delay(50);

  WiFi.begin(wifi_ssid, wifi_password);
  i = 0;
  while (WiFi.status() != WL_CONNECTED && i < 240)   // Need to keep looking for about 15 seconds, because, yes, it can take that long to connect
  {
    i++;
    delay(62);
    ESP.wdtFeed();
  }

  delay(100);
}

//========================================================================================
// The getNewMqtt() procedure connects to the guest network, then gets the address of the
// mqtt server from a well known (to this program) website, just in case it changed.  Normally
// it wouldn't change, but because everything is running on the guest wifi network, anything
// could change at any time.
// While this section of the code works, at the time of the writing the hosting website
// dropped supporting the page, so there is nothing currently in place actually using this.
//========================================================================================
void getNewMqtt()   // gets a new mqtt server address if it changes by checking a web site
{

  if (WebName.equalsIgnoreCase("X"))
    return;   // Nothing to get an address from, so return
  connectToWifi();
  if (WiFi.status() != WL_CONNECTED)  // If we couldn't connect through WiFi, then there's no point in continuing.
  {
    WiFi.disconnect();
    return;
  }

  delay(1);
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
        IPAddress ip;
        ip.fromString(payload);
        if (ip[0] == 0 || ip[3] == 0) // is it a valid ip address?  Starting and ending with 0 generally aren't valid.
        {
          http.end();
          delay(500);   // this is important.  Don't remove this delay.  Things go bad otherwise.
          WiFi.disconnect();
          return;
        }
        if (mqtt_host != ip)  // check to see if the ip address changed
        {
          char ts[64];
          sprintf(ts,"R|000|Changing the mqtt address to be %d.%d.%d.%d",ip[0], ip[1], ip[2], ip[3]);
          add_to_queue(ts);
          xmit_the_queue(); // Actually tell the old address
          mqtt_host = ip;   // From this point forward the address is changed.
          mqttClient.setServer(mqtt_host, MQTT_PORT);
          strcpy(eeprom_data.validate,"valid");
          wifi_ssid.toCharArray(eeprom_data.ssid,wifi_ssid.length() + 1);
          wifi_password.toCharArray(eeprom_data.password,wifi_password.length() + 1);
          eeprom_data.mqtt = IPAddress(ip[0], ip[1], ip[2], ip[3]);
          WebName.toCharArray(eeprom_data.webname,WebName.length() + 1);
          EEPROM.begin(512);
          EEPROM.put(0,eeprom_data);
          EEPROM.commit();
          EEPROM.end();
          sprintf(ts,"R|000|EEPROM mqtt address is now %d.%d.%d.%d",ip[0], ip[1], ip[2], ip[3]);
          add_to_queue(ts);
        }
      }
    } 
    else
      add_to_queue((char *)"E|[HTTP] GET... failed");
  }
  http.end();
  delay(500);   // this is important.  Don't remove this delay
  WiFi.disconnect();
  delay(250);
  tcpCleanup();
}

//========================================================================================
// The add_to_queue() procedure adds a character string to the mqtt queue.  As part of that
// process, it will also timestamp and format the added line.  It doesn't send anything,
// it just queues it internally to the Wemos.
//========================================================================================
void add_to_queue(char* str)
{
  // Everything added to the queue has the same prefix and suffix
  // The my_ssid part is simply diagnostics, except in the case of the boiler monitor.
  if (strlen(str) > 90) // Just to keep the string within the required length
    return;
  if (queue_len < (QUEUE_MAX - 1))  // add to the queue if there is room
  {
    Timezone myTZ(myDST, mySTD);
    time_t utc = now();
    time_t t = myTZ.toLocal(utc);
    sprintf(queue+queue_pos, "%-6s|%04d-%02d-%02d %02d:%02d:%02d|%s|", my_ssid, year(t), month(t), day(t), hour(t), minute(t), second(t),str);
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
  uint8_t j;
  uint16_t queue_position;

  if (!queue_len)  // no sense connecting if there's nothing to send
    return;
  connectToWifi();
  if (WiFi.status() != WL_CONNECTED)
    return;
  ESP.wdtFeed();
  mqttClient.connect();   // Get connected to the mqtt server
  j = 0;
  while (!mqttClient.connected() && j < 200)  // give it up to 4 seconds to connect to mqtt
  {
    delay(25);
    j++;
  }
  if (mqttClient.connected() && queue_len)
  {
    ESP.wdtFeed();
    queue_position = 0; // Used for getting info out of the queue FIFO, versus pulling from the end
    do
    {
      queue_len--;    // queue_len is the number of entries in the queue
      if (strlen(queue + queue_position) > 0)
      {
        // Message is being published with the 'Clean' session under QOS 2.
        mqttClient.publish(MQTT_NAME, 2, true, queue + queue_position);  // topic, qos, retain, payload, length=0, dup=false, message_id=0
        delay(250);   // We do have to wait for it to clear
      }
      queue_position += QUEUE_WIDTH;
    } while (queue_len > 0);
    queue_pos = 0;    // reset the queue_pos for the next entries to be added to the queue in the future
    mqttClient.disconnect();
  };
  ESP.wdtFeed();
  delay(500);   // this is important.  Don't remove this delay
  WiFi.disconnect();
  delay(250);
  tcpCleanup();
}

//========================================================================================
// This is the main processing loop for the system.
// The loop itself is really small.  Essentially it sends requests to the PSOC at certain
// intervals, and queues up the resulting responses.
//========================================================================================
void loop()
{
  char ts[64];   // ts is a temporary array of characters, basically a string without the issues.

  server.handleClient();    // used to handle configuration changes through the web interface
  if (hour() != last_hour && minute() > 56)   // At the end of each hour, do nothing for 3 minutes
  {
    ESP.wdtFeed();
    return;   // Just loop around for the server side of things for three minutes
  }           

  switch(request_state)
  {
    case 0:
      if (millis() > request_wait || millis() > blast_time)
      {
        request_state++; // wait state is switched to do the request
        if (millis() > blast_time)
        {
          getNewMqtt();   // only do this before the larger blast interval
        }
      }
      break;
    case 1:
      read_time = millis();
      digitalWrite(D4, LOW);  // turn the indicator light on to show we are talking
      Serial.write(0x1b); // Send the escape character out the serial port to the PSOC
      ESP.wdtFeed();      // just to try and resolve something...
      if (millis() > blast_time)  // force an output about once an hour
      {
        Serial.write(0x32);  // Forced request, just means once an hour send a 2 instead of a 1 to get all data from the PSOC
      }
      else
      {
        Serial.write(0x31);  // send the standard, limited request to the PSOC by using a 1
      }
      request_state++;  // update to show we are waiting for a response
      break;
    case 2:
      if (!getLine(ts))   // if getLine returns false then it timed out
      {                   // timed out, so abandon any further reads
        ESP.wdtFeed();
        retry_count++;
        if (retry_count < 5)  // try up to 5 times
        {
          if (retry_count > 3)  // only say something if it keeps happening repeatedly
          {
            add_to_queue((char *)"E|000|Polling the PSOC timed out");
            xmit_the_queue();   // Tell someone right away
            digitalWrite(LED_BUILTIN, HIGH);    // turn the light back off, we are done talking (and not hearing anything back)
          }
          request_wait += 20000;  // try again in 20 seconds
        }
        else  // give up, and try in half an hour, or whatever the TIME_BETWEEN_POLLING interval is
        {
          request_state = 0;
          retry_count = 0;
          request_wait += TIME_BETWEEN_POLLING;   // Do the next one in however many minutes
          blast_time += BLAST_INTERVAL;
          digitalWrite(LED_BUILTIN, HIGH);  // Turn off the indicator light
        }
        // At this point retry_count is less than 5, so we can reset to try in 20 seconds
        ESP.wdtFeed();
        if (millis() > blast_time)
          blast_time += 20000;    // offset this by 20 seconds due to the serial timeout
        request_state = 0;
        digitalWrite(LED_BUILTIN, HIGH);    // turn the light back off, we are done talking (and not hearing anything back)
      }
      else    // we got some kind of response on the serial line
      {
        retry_count = 0;  // reset this because we successfully got an answer from the PSOC.
        if (strncmp(ts,"DONE",4) && strlen(ts))   // Remember the strncmp returns non-zero if the strings DONT match.
          add_to_queue(ts);   // We did NOT get the DONE, so add the returned string to the queue.
        else
        { // the DONE line is what triggers this next section - Remember that strncmp returns 0 if the strings match.
          ESP.wdtFeed();
          ts[0] = 0;      // Zero it out for the next usage
          request_state = 0;
          xmit_the_queue();   // send anything that was received after we get the DONE
          digitalWrite(LED_BUILTIN, HIGH);  // Turn off the indicator light, which is just a diagnostic
        }
        request_wait += TIME_BETWEEN_POLLING;   // Do the next one in however many minutes
        if (millis() > blast_time)  // force an output about once an hour
          blast_time += BLAST_INTERVAL;
      }
      break;
  }
  if (request_state == 0)
  {
    // Things get interesting at 4am-ish.  It would be 2am, but then I'd have to deal with daylight savings at the same time
    time_t utc = now();
    time_t t = myTZ.toLocal(utc);
    if (hour(t) == 4 && !time_was_set) // Get the time at 4am-ish, so we aren't too far off as time passes
    {  // The time_was_set variable is used so we only get the time once a day.
      ESP.wdtFeed();
      delay(time_offset);       // Do this so everyone doesn't try to get the time at exactly the same time.
      if (!getTimeFromHttp())  // this gets the time
      {
        ESP.wdtFeed();
        add_to_queue((char *)"E|000|Failed to get the time from HTTP");
        xmit_the_queue();
        delay(10000);
        if (year() < 2019)
          ESP.restart();    // reboot because something is seriously wrong
      }
      time_was_set = true;  // say we got the 4am time so we don't loop around and try again immediately
    }
    if (queue_len > 50 || (hour(t) < 1 && minute(t) > 30 && millis() > MILLIS_MAX)) // Reboot every two or three days, due to a weird connection problem with the local network
    { // Two or three days means "only boot between 12:30am and 1am", which can stretch things out a bit.
      add_to_queue((char *)"Booting due to connection issues");
      xmit_the_queue(); // flush out any hanging queued entries, just in case we can
      delay(5000);     // make sure everything clears, then reboot
      ESP.restart();    // As part of the restart, millis is reset to 0.
    }
    if (hour(t) > 4)
      time_was_set = false; // after 4am (really 5am) it is ok to reset this flag.
     
    if (hour(t) != last_hour)    // Once an hour report the memory statistics of this device.  It's only a diagnostic.
    {
      last_hour = hour();
      delay(time_offset << 2);
      char ts1[45];
      uint32_t heap_free;
      uint16_t heap_max;
      uint8_t heap_frag;
      ESP.wdtFeed();
      ESP.getHeapStats(&heap_free, &heap_max, &heap_frag);
      add_to_queue((char *)"R|000|______________________");
      sprintf(ts1,"R|000|Free heap is %zu",heap_free);
      add_to_queue(ts1);
      sprintf(ts1,"R|000|Heap fragmentation is %d%%",heap_frag);
      add_to_queue(ts1);
      sprintf(ts1,"R|000|MaxFreeBlockSize is %d",heap_max);
      add_to_queue(ts1);
      sprintf(ts1,"R|000|getFreeContStack is %zu",ESP.getFreeContStack());
      add_to_queue(ts1);
      xmit_the_queue();
    }
  }
}

//========================================================================================
// Get a line of data from the serially connected PSOC.  It allows for timeouts of the
// serial process, as the PSOC might be too busy to respond.
//========================================================================================
bool getLine(char * buffer)
{
  uint8_t idx = 0;  // The index of where the to put the character in the buffer.
  ESP.wdtFeed();
  idx = Serial.readBytesUntil('\n', buffer, 127); // idx is one past the character just received.
  if (idx)
  {
    buffer[idx - 1] = 0;    // Terminate the string received so far
    return(true);
  }
  else
    return(false);
  /*
  char c; // The character we received through the serial port.
  uint32_t maxwait = millis() + 2000;   // wait up to two seconds for something
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
  buffer[idx - 1] = 0;    // Terminate the returned buffer.
  return(true);
  */
}

//========================================================================================
// handleConfig() is a procedure called by the web server.  It allows for changes to the
// network, the address of the mqtt server, and the public name of the web site that also
// has the mqtt server address.  Without this procedure the device will never connect to
// the local wifi network, unless everything was setup properly in eeprom.
//========================================================================================
void handleConfig() 
{
  char ts[80];
  time_t utc = now();
  time_t t = myTZ.toLocal(utc);
  // note that embedded in this form are lots of variable fields pre-filled with the sprintf
  if (server.hasArg("ssid")&& server.hasArg("Password")&& server.hasArg("MQTT_IP")) //If all form fields contain data call handleSubmit()
    handleSubmit();
  else // Display the form
  {
    ESP.wdtFeed();
    server.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
    server.sendHeader(F("Pragma"), F("no-cache"));
    server.sendHeader(F("Expires"), "-1");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    // here begin chunked transfer
    server.send(200, "text/html");
    server.sendContent(F("<!DOCTYPE HTML><html><head><meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">" \
        "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"));
    server.sendContent(F("<title>Configuration</title><style>\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\"</style></head>"));
    server.sendContent(F("<body><h1>Configuration</h1><FORM action=\"/config\" method=\"post\">"));
    sprintf(ts,"<P><label>SSID:&nbsp;</label><input maxlength=\"30\" value=\"%s\" name=\"ssid\"><br>",wifi_ssid.c_str());
    server.sendContent(ts);
    sprintf(ts,"<label>Password:&nbsp;</label><input maxlength=\"30\" value=\"%s\" name=\"Password\"><br>",wifi_password.c_str());
    server.sendContent(ts);
    sprintf(ts,"<label>MQTT IP:&nbsp;</label><input maxlength=\"15\" value=\"%d.%d.%d.%d\" name=\"MQTT_IP\"><br> ",mqtt_host[0],mqtt_host[1],mqtt_host[2],mqtt_host[3]);
    server.sendContent(ts);
    sprintf(ts,"<label>HTTP Web Name:&nbsp;</label><input maxlength=\"63\" value=\"%s\" name=\"WebName\"><br> ",WebName.c_str());
    server.sendContent(ts);
    server.sendContent(F("<INPUT type=\"submit\" value=\"Send\"> <INPUT type=\"reset\"></P>"));
    sprintf(ts,"<P><br>The time is %04d-%02d-%02d %02d:%02d:%02d</P></FORM></body></html>",year(t), month(t), day(t), hour(t), minute(t), second(t));
    server.sendContent(ts);
    server.sendContent(""); // this closes out the send
    ESP.wdtFeed();
    server.client().stop();
  }
}

//========================================================================================
// Report on the status of everthing through the web interface, mainly from 192.168.4.1.
// This routine is very helpfull for ensuring everthing is setup properly.
//========================================================================================
void handleStatus()
{
  char ts[72];
  char serial_buffer[128];
  uint16_t queue_length;
  uint16_t queue_position;
  int16_t last_read = (millis() - read_time) / 1000;

  time_t utc = now();
  time_t t = myTZ.toLocal(utc);
  ESP.wdtFeed();
  server.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
  server.sendHeader(F("Pragma"), F("no-cache"));
  server.sendHeader(F("Expires"), F("-1"));
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  // here begin chunked transfer a line at a time
  server.send(200, "text/html");
  ESP.wdtFeed();
  server.sendContent(F("<!DOCTYPE HTML><html><head><meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">" \
      "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"));
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
  sprintf(ts,"Free heap is currently %ld<br>",(long int)ESP.getFreeHeap());
  server.sendContent(ts);
  sprintf(ts,"Heap fragmentation is %d<br>",ESP.getHeapFragmentation());
  server.sendContent(ts);
  sprintf(ts,"MaxFreeBlockSize is %ld<br>",(long int)ESP.getMaxFreeBlockSize());
  server.sendContent(ts);
  sprintf(ts,"I think the time is %04d-%02d-%02d %02d:%02d:%02d</P>", year(t), month(t), day(t), hour(t), minute(t), second(t));
  server.sendContent(ts);
  ESP.wdtFeed();

  ts[0] = 0;
  server.sendContent("Reading PSOC devices...<br>");
  request_state = 1;
  while (request_state > 0)
  {
    switch(request_state)
    {
      case 1:
        digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on
        Serial.write(0x1b); // Send the escape character
        delay(100);  // just to try and resolve something...
        Serial.write(0x32);  // Forced request
        request_state++;  // update to show we are waiting for a response
        break;
      case 2:
        if (!getLine(serial_buffer))   // if getLine returns false then it timed out
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
          if (strncmp(serial_buffer,"DONE",4))   // Remember the strncmp returns non-zero if the strings DONT match
          {
            server.sendContent(serial_buffer);
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

  server.sendContent("All sensors have been read<br>");
  server.sendContent("__________________________<br>");
  server.sendContent("Reading the mqtt queue<br>");
  
  if (queue_len)
  {
    queue_length = queue_len;   // queue_length is just the copy within this procedure, vs queue_len, which is the global value
    queue_position = 0;         // Used for getting info out of the queue FIFO, versus pulling from the end.  This is the local copy.
    do
    {
      queue_length--;
      sprintf(ts,"%s<br>",queue + queue_position);
      queue_position += QUEUE_WIDTH;
      server.sendContent(ts);
      delay(1);
    } while (queue_length > 0);
  }
  server.sendContent(F("Finished reading the mqtt queue<br>"));
  
  server.sendContent(F("</body></html>"));
  server.sendContent(""); // this closes out the send
  ESP.wdtFeed();
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
  
  server.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
  server.sendHeader(F("Pragma"), F("no-cache"));
  server.sendHeader(F("Expires"), F("-1"));
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  // here begin chunked transfer a line at a time
  server.send(200, "text/html");
  delay(1);
  server.sendContent(F("<!DOCTYPE HTML><html><head><meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">" \
      "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\"><html><head><title>"));
  server.sendContent(my_ssid);
  server.sendContent(F(" Status</title></head><body><p>The ssid is "));
  server.sendContent(server.arg("ssid"));
  server.sendContent(F("<br>And the password is "));
  server.sendContent(server.arg("Password"));
  server.sendContent(F("<br>And the MQTT IP Address is "));
  server.sendContent(server.arg("MQTT_IP"));
  server.sendContent(F("<br>And the Web Name is "));
  server.sendContent(server.arg("WebName"));
  if (server.arg("enable_debug") == "Debug")
    server.sendContent(F("<br>Debugging enabled"));
  server.sendContent(F("</P><BR><H2><a href=\"/\">go home</a></H2><br>"));
  server.sendContent(F("</body></html>"));
  server.sendContent(""); // this closes out the send
  server.client().stop();

  // write data to EEPROM memory
  strcpy(eeprom_data.validate,"valid");
  i = server.arg("ssid").length() + 1;  // needed to make sure the correct length is used for the values
  server.arg("ssid").toCharArray(eeprom_data.ssid,i);
  i = server.arg("Password").length() + 1;
  server.arg("Password").toCharArray(eeprom_data.password,i);
  ip.fromString(server.arg("MQTT_IP"));
  eeprom_data.mqtt = IPAddress(ip[0], ip[1], ip[2], ip[3]);
  i = server.arg("WebName").length() + 1;
  server.arg("WebName").toCharArray(eeprom_data.webname,i);
  if (server.arg("enable_debug") == "Debug")
    eeprom_data.debugging = 1;
  else
    eeprom_data.debugging = 0;
  ESP.wdtFeed();

  EEPROM.begin(512);
  EEPROM.put(0,eeprom_data);
  ESP.wdtFeed();
  EEPROM.commit();    // This is what actually forces the data to be written to EEPROM.
  EEPROM.end();
  delay(500);   // Wait for the eeprom to acually be written
  // It is simpler to just restart the Wemos at this time, than try to reset all values.
  ESP.restart();
}

//========================================================================================
// A very simple web page is produced by handleNotFound() when an invalid web page is
// requested of this device.
//========================================================================================
void handleNotFound()
{
  server.sendHeader(F("Cache-Control"), F("no-cache, no-store, must-revalidate"));
  server.sendHeader(F("Pragma"), F("no-cache"));
  server.sendHeader(F("Expires"), F("-1"));
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  // here begin chunked transfer a line at a time
  server.send(404, "text/html");
  delay(1);
  server.sendContent(F("File Not Found<br><br>URI:"));
  server.sendContent(server.uri());
  server.sendContent(F("</P><BR><H2><a href=\"/\">go home</a></H2><br>"));
  server.sendContent(F("</body></html>"));
  server.sendContent(""); // this closes out the send
  server.client().stop();
}

//========================================================================================
// End of the program.
//========================================================================================
