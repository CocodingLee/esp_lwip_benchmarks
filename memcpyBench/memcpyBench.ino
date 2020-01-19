//Naive Benchmark to measure time to copy a UDP packet,
//with various alignements.
void setup() {
  Serial.begin(2000000);
}

#define COPY_SIZE 1460
unsigned char packet[4100] __attribute__ ((aligned (8)));

static uint32_t timerCounts[7];
static uint32_t loopCount;

void loop() {
uint32_t time0 = micros();
memcpy( packet, packet+2000, COPY_SIZE ); //Aligned source, aligned dest
uint32_t time1 = micros();
memcpy( packet, packet+2001, COPY_SIZE ); //Misaligned source, aligned dest
uint32_t time2 = micros();
memcpy( packet, packet+2002, COPY_SIZE ); //Misaligned source, aligned dest
uint32_t time3 = micros();
memcpy( packet, packet+2003, COPY_SIZE ); //Misaligned source, aligned dest
uint32_t time4 = micros();
memcpy( packet+1, packet+2100, COPY_SIZE ); //Aligned source, misaligned dest
uint32_t time5 = micros();
memcpy( packet+2, packet+2100, COPY_SIZE ); //Aligned source, misaligned dest
uint32_t time6 = micros();
memcpy( packet+3, packet+2100, COPY_SIZE ); //Aligned source, misaligned dest
uint32_t time7 = micros();

timerCounts[0] += time1-time0;
timerCounts[1] += time2-time1;
timerCounts[2] += time3-time2;
timerCounts[3] += time4-time3;
timerCounts[4] += time5-time4;
timerCounts[5] += time6-time5;
timerCounts[6] += time7-time6;
loopCount += 1;

if( loopCount % 10000 == 0 ) {
  Serial.printf( "%f %f %f %f %f %f %f\n",
    (float)timerCounts[0]/(float)loopCount,
    (float)timerCounts[1]/(float)loopCount,
    (float)timerCounts[2]/(float)loopCount,
    (float)timerCounts[3]/(float)loopCount,
    (float)timerCounts[4]/(float)loopCount,
    (float)timerCounts[5]/(float)loopCount,
    (float)timerCounts[6]/(float)loopCount
    );
  }
}
