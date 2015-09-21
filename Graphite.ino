#include "Stream.h"
#include <dht.h>
#include <Time.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>

#define DHT22_PIN 2

#ifndef PREFIX
#define PREFIX "org.btdev.sensors."
#endif

#ifndef NODE_NAME
#define NODE_NAME "basement"
#endif

#ifndef TIME_ZONE
#define TIME_ZONE -6
#endif

#ifndef GRAHPITE_SERVER
#define GRAPHITE_SERVER "graphite"
#endif

#ifndef GRAHPITE_PORT
#define GRAPHITE_PORT 2003
#endif

#ifndef INTERVAL
#define INTERVAL 60
#endif

dht DHT;
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE };
IPAddress ntpServer(208, 79, 218, 140);
const int timeZone = TIME_ZONE;

EthernetUDP Udp;

unsigned int localPort = 8888;
time_t prevDisplay = 0; // when the digital clock was displayed
time_t displayNext = 0;

/*-------- NTP code ----------*/
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets
// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0; // Stratum, or type of clock
  packetBuffer[2] = 6; // Polling Interval
  packetBuffer[3] = 0xEC; // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
    Serial.println("Transmit NTP Request");
    sendNTPpacket(ntpServer);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500) {
      int size = Udp.parsePacket();
      if (size >= NTP_PACKET_SIZE) {
        Serial.println("Receive NTP Response");
        Udp.read(packetBuffer, NTP_PACKET_SIZE); // read packet into the buffer
        unsigned long secsSince1900;
        // convert four bytes starting at location 40 to a long integer
        secsSince1900 = (unsigned long)packetBuffer[40] << 24;
        secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
        secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
        secsSince1900 |= (unsigned long)packetBuffer[43];
        return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
      }
    }
    Serial.println("No NTP Response :-(");
    return 0; // return 0 if unable to get the time
}

void sendData( time_t time,  ) {
  EthernetClient client;
  if( client.connect(GRAPHITE_SERVER, GRAPHITE_PORT) ) {
    client.print(PREFIX);
    client.print(NODE_NAME);
    client.print(".temperature ");
    client.print(DHT.temperature);
    client.print(" ");

  }
}

void loop( )
{
  // READ DATA
  if (now() >= displayNext) { //update the display only if time has changed
    int chk = DHT.read22(DHT22_PIN);
    prevDisplay = now();
    displayNext = prevDisplay + INTERVAL;
    Serial.print("DHT22, \t");

    switch (chk)
    {
      case DHTLIB_OK:  
        Serial.print("OK,\t"); 
        break;
      case DHTLIB_ERROR_CHECKSUM: 
        Serial.print("Checksum error,\t"); 
        break;
      case DHTLIB_ERROR_TIMEOUT: 
        Serial.print("Time out error,\t"); 
        break;
      default: 
        Serial.print("Unknown error,\t"); 
        break;
    }
    // DISPLAY DATA
    Serial.print(DHT.humidity, 1);
    Serial.print(",\t");
    Serial.println(DHT.temperature, 1);
  }
}

void setup( )
{
  Serial.begin(9600);
  delay(250);
  if (Ethernet.begin(mac) == 0) {
    // no point in carrying on, so do nothing forevermore:
    while (1) {
      Serial.println("Failed to configure Ethernet using DHCP");
      delay(10000);
    }
  }
  Serial.print("IP number assigned by DHCP is ");
  Serial.println(Ethernet.localIP());
  server.begin();
  Udp.begin(localPort);
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);

  Serial.println("DHT TEST PROGRAM ");
  Serial.print("LIBRARY VERSION: ");
  Serial.println(DHT_LIB_VERSION);
  Serial.println();
  Serial.println("Type,\tstatus,\tHumidity (%),\tTemperature (C)");
}
