//UDP RX Benchmark - meant to measure maximum bytes/second through lwIP
//Prints "IdleTicks:StackMicros:DeltaBytes" once per second
//
//Test Instructions:
//  Connect UDP TX device
//  Let the sketch run for 10 or so seconds to get a baseline for idle ticks.
//  Average the idle-ticks together.
//  Launch udpSpam.py, let it run for a minute or so
//  For each printout during the test, compute:
//    CpuUsedPct =(100-(100*A2/AverageIdleTicks))
//    MegaBits/sec = 8 * DeltaBytes / 1000000
//    Mbps/CpuPct = CpuUsedPct/(Megabits/sec)
//  The Mbps/CpuPct should be close to repeatable over multiple seconds
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "ESPAsyncUDP.h"

AsyncUDP udp;
const unsigned int localUdpPort = 4120;  // local port to listen on
const unsigned int channel = 6; //Make sure to try 1/6/11 depending on local interference

volatile uint32_t total_bytes;
uint32_t timer;
void setup()
{
  Serial.begin(2000000);//Use fast baud rate in hopes of not blocking too much
  Serial.println();
  WiFi.softAP("accesspoint",NULL,6);

  if(udp.listen(localUdpPort)) {
      Serial.println("UDP Listening");
      udp.onPacket([](AsyncUDPPacket packet) {
        total_bytes += packet.length();
        //UDP application packet processing goes here.
      });
  }
  timer = micros();
}

void loop()
{
  uint32_t new_timer = micros();
  uint32_t time_in_stack = new_timer - timer;
  uint32_t worst_case_in_stack = max( time_in_stack, worst_case_in_stack );
  uint32_t my_total_bytes = total_bytes;
  static uint32_t last_second_micros;
  static uint32_t last_bytes;
  static uint32_t idle_ticks;

  idle_ticks ++;
  #define ONE_SECOND_IN_MICROS 1000000
  if( new_timer - last_second_micros > ONE_SECOND_IN_MICROS )
    {
    //Format "IdleTicks:StackMicros:DeltaBytes"
    Serial.print(idle_ticks);Serial.print(":");
    Serial.print(worst_case_in_stack);Serial.print(":");
    Serial.println( my_total_bytes - last_bytes );
    last_second_micros = new_timer;
    last_bytes = my_total_bytes;
    idle_ticks = 0;
    }
  //Reset the timer instead of timer=new_timer, to avoid measuring print overhead
  timer = micros();
}
