//TCP Benchmark - meant to measure maximum bytes/second through lwIP
//Prints "IdleTicks:StackMicros:DeltaBytes" once per second
//
//Test Instructions:
//  Connect TCP TX device
//  Let the sketch run for 10 or so seconds to get a baseline for idle ticks.
//  Average the idle-ticks together.
//  Launch tcpSpam.py, let it run for a minute or so
//  For each printout during the test, compute:
//    CpuUsedPct =(100-(100*A2/AverageIdleTicks))
//    MegaBits/sec = 8 * DeltaBytes / 1000000
//    Mbps/CpuPct = CpuUsedPct/(Megabits/sec)
//  The Mbps/CpuPct should be close to repeatable over multiple seconds, except for the first and last few seconds
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>

const unsigned int TCP_PORT = 4120;  // local port to listen on
const unsigned int channel = 6; //Make sure to try 1/6/11 depending on local interference

//Comment this out to do RX-only testing.
//deltaBytes result is for one-side only.
#define DO_LOOPBACK 1

uint32_t total_bytes; //Bytes received
uint32_t timer;

 /* clients events */
static void handleError(void* arg, AsyncClient* client, int8_t error) {
  Serial.printf("\n connection error %s from client %s \n", client->errorToString(error), client->remoteIP().toString().c_str());
}

static void handleData(void* arg, AsyncClient* client, void *data, size_t len) {
  //Serial.printf("\n received %i bytes\n", len);
  total_bytes += len;
#if DO_LOOPBACK
  if( len != client->add( (const char*)data, len ) )
  {
    Serial.println("Dropped bytes due to not enough send buffer space. Test invalidated");
  }
#endif
}

static void handleDisconnect(void* arg, AsyncClient* client) {
  Serial.printf("\n client %s disconnected \n", client->remoteIP().toString().c_str());
}

static void handleTimeOut(void* arg, AsyncClient* client, uint32_t time) {
  Serial.printf("\n client ACK timeout ip: %s \n", client->remoteIP().toString().c_str());
}

/* server events */
static void handleNewClient(void* arg, AsyncClient* client) {
  // register events
  client->onData(&handleData, NULL);
  client->onError(&handleError, NULL);
  client->onDisconnect(&handleDisconnect, NULL);
  client->onTimeout(&handleTimeOut, NULL);
}

void setup() {
  Serial.begin(2000000);
  delay(20);
  
  // create access point
  WiFi.softAP("accesspoint",NULL,channel);

  AsyncServer* server = new AsyncServer(TCP_PORT);
  server->onClient(&handleNewClient, server);
  server->begin();
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
