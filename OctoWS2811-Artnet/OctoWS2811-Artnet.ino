// Receive multiple universes via Artnet and control a strip of ws2811 leds via OctoWS2811
//
// This example may be copied under the terms of the MIT license, see the LICENSE file for details
//  https://github.com/natcl/Artnet
// 
// http://forum.pjrc.com/threads/24688-Artnet-to-OctoWS2811?p=55589&viewfull=1#post55589

#include <Artnet.h>
// DON'T FORGET TO CHANGE ANY REFERENCES OF Ethernet.h and EthernetUdp.h in Arnet.h to NativeEthernet.h and NativeEthernetUdp.h
#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
#include <SPI.h>
#include <OctoWS2811.h>

// Ideas for improving performance with WIZ820io / WIZ850io Ethernet:
// https://forum.pjrc.com/threads/45760-E1-31-sACN-Ethernet-DMX-Performance-help-6-Universe-Limit-improvements

// OctoWS2811 settings
const int ledsPerStrip = 300; // change for your setup
const int numStrips= 20; // change for your setup
const int bytesPerLED = 3;  // change to 4 if using RGBW
DMAMEM int displayMemory[ledsPerStrip * numStrips * bytesPerLED / 4];
unsigned int drawingMemory[ledsPerStrip * numStrips * bytesPerLED / 4];
const int config = WS2811_GRB | WS2811_800kHz;
//const int numPins = 1;
// byte pinList[numStrips] = {
// 23, 22, 21, 20, 19};

byte pinList[numStrips] = {
23, 22, 21, 20, 19, 
18, 17, 16, 15, 14,

 13, 41, 40, 39, 38,
 37, 36, 35, 34, 33};
OctoWS2811 leds(ledsPerStrip, displayMemory, drawingMemory, config,  numStrips, pinList);

// Artnet settings
Artnet artnet;
const int startUniverse = 1; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as zero.
const int numberOfChannels = ledsPerStrip * numStrips * 3; // Total number of channels you want to receive (1 led = 3 channels)
byte channelBuffer[numberOfChannels]; // Combined universes into a single array

// Check if we got all universes
// const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
// bool universesReceived[maxUniverses];

const int maxUniverses = 512;
bool universesReceived[512];
bool sendFrame = 1;

// Change ip and mac address for your setup
byte ip[] = {192, 168, 1, 3};
byte mac[] = {0x04, 0xE9, 0xE5, 0x00, 0x69, 0xEC};

void setup()
{
  Serial.begin(115200);
  artnet.begin(mac, ip);
  leds.begin();
  initTest();

  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop()
{
  // we call the read function inside the loop
  artnet.read();
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  sendFrame = 1;
  Serial.print(universe);
  Serial.print("/");
  Serial.print(length);
  Serial.print("/");
  Serial.print(sequence);
  Serial.print("/");
  Serial.print(*data);
  Serial.println();

  // Store which universe has got in
  if (universe < maxUniverses)
    universesReceived[universe] = 1;

  // for (int i = 0 ; i < maxUniverses ; i++)
  // {
  //   if (universesReceived[i] == 0)
  //   {
  //     //Serial.println("Broke");
  //     sendFrame = 0;
  //     break;
  //   }
  // }

  // read universe and put into the right part of the display buffer
  for (int i = 0 ; i < length ; i++)
  {
    int bufferIndex = i + ((universe - startUniverse) * length);
    if (bufferIndex < numberOfChannels) // to verify
      channelBuffer[bufferIndex] = byte(data[i]);
  }      

  // send to leds
  for (int i = 0; i < ledsPerStrip * numStrips; i++)
  {
    leds.setPixel(i, channelBuffer[(i) * 3], channelBuffer[(i * 3) + 1], channelBuffer[(i * 3) + 2]);
  }      
  
  if (sendFrame)
  {
    leds.show();
    // Reset universeReceived to 0
    memset(universesReceived, 0, maxUniverses);
  }
}

void initTest()
{
  for (int i = 0 ; i < ledsPerStrip * numStrips ; i++)
    leds.setPixel(i, 127, 0, 0);
  leds.show();
  delay(500);
  for (int i = 0 ; i < ledsPerStrip * numStrips  ; i++)
    leds.setPixel(i, 0, 127, 0);
  leds.show();
  delay(500);
  for (int i = 0 ; i < ledsPerStrip * numStrips  ; i++)
    leds.setPixel(i, 0, 0, 127);
  leds.show();
  delay(500);
  for (int i = 0 ; i < ledsPerStrip * numStrips  ; i++)
    leds.setPixel(i, 0, 0, 0);
  leds.show();
}
