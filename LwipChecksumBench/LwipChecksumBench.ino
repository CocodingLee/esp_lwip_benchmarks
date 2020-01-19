//Simple benchmark of lwIP checksum algorithms.
//These are copied and renamed from inet_chksum.c in lwIP
//
//CodeSize comparisons with esp8266 2.6.3 compiler:
//lwip_standard_chksum1: 96 Bytes, 36 Instructions
//lwip_standard_chksum2: 104 Bytes, 38 Instructions
//lwip_standard_chksum3: 160 Bytes, 57 Instructions
//
//lwIP recommends tuning this: https://www.nongnu.org/lwip/2_1_x/optimization.html
//Format is: <micros spend in algo 1> <micros spent in algo 2> <micros spent in algo 3>

#include "lwip/opt.h"

#include "lwip/inet_chksum.h"
#include "lwip/def.h"
#include "lwip/ip_addr.h"

#include <string.h>

/**
 * lwip checksum
 *
 * @param dataptr points to start of data to be summed at any boundary
 * @param len length of data to be summed
 * @return host order (!) lwip checksum (non-inverted Internet sum)
 *
 * @note accumulator size limits summable length to 64k
 * @note host endianess is irrelevant (p3 RFC1071)
 */
u16_t
lwip_standard_chksum1(const void *dataptr, int len)
{
  u32_t acc;
  u16_t src;
  const u8_t *octetptr;

  acc = 0;
  /* dataptr may be at odd or even addresses */
  octetptr = (const u8_t *)dataptr;
  while (len > 1) {
    /* declare first octet as most significant
       thus assume network order, ignoring host order */
    src = (*octetptr) << 8;
    octetptr++;
    /* declare second octet as least significant */
    src |= (*octetptr);
    octetptr++;
    acc += src;
    len -= 2;
  }
  if (len > 0) {
    /* accumulate remaining octet */
    src = (*octetptr) << 8;
    acc += src;
  }
  /* add deferred carry bits */
  acc = (acc >> 16) + (acc & 0x0000ffffUL);
  if ((acc & 0xffff0000UL) != 0) {
    acc = (acc >> 16) + (acc & 0x0000ffffUL);
  }
  /* This maybe a little confusing: reorder sum using lwip_htons()
     instead of lwip_ntohs() since it has a little less call overhead.
     The caller must invert bits for Internet sum ! */
  return lwip_htons((u16_t)acc);
}

/*
 * Curt McDowell
 * Broadcom Corp.
 * csm@broadcom.com
 *
 * IP checksum two bytes at a time with support for
 * unaligned buffer.
 * Works for len up to and including 0x20000.
 * by Curt McDowell, Broadcom Corp. 12/08/2005
 *
 * @param dataptr points to start of data to be summed at any boundary
 * @param len length of data to be summed
 * @return host order (!) lwip checksum (non-inverted Internet sum)
 */
u16_t
lwip_standard_chksum2(const void *dataptr, int len)
{
  const u8_t *pb = (const u8_t *)dataptr;
  const u16_t *ps;
  u16_t t = 0;
  u32_t sum = 0;
  int odd = ((mem_ptr_t)pb & 1);

  /* Get aligned to u16_t */
  if (odd && len > 0) {
    ((u8_t *)&t)[1] = *pb++;
    len--;
  }

  /* Add the bulk of the data */
  ps = (const u16_t *)(const void *)pb;
  while (len > 1) {
    sum += *ps++;
    len -= 2;
  }

  /* Consume left-over byte, if any */
  if (len > 0) {
    ((u8_t *)&t)[0] = *(const u8_t *)ps;
  }

  /* Add end bytes */
  sum += t;

  /* Fold 32-bit sum to 16 bits
     calling this twice is probably faster than if statements... */
  sum = FOLD_U32T(sum);
  sum = FOLD_U32T(sum);

  /* Swap if alignment was odd */
  if (odd) {
    sum = SWAP_BYTES_IN_WORD(sum);
  }

  return (u16_t)sum;
}

/**
 * An optimized checksum routine. Basically, it uses loop-unrolling on
 * the checksum loop, treating the head and tail bytes specially, whereas
 * the inner loop acts on 8 bytes at a time.
 *
 * @arg start of buffer to be checksummed. May be an odd byte address.
 * @len number of bytes in the buffer to be checksummed.
 * @return host order (!) lwip checksum (non-inverted Internet sum)
 *
 * by Curt McDowell, Broadcom Corp. December 8th, 2005
 */
u16_t
lwip_standard_chksum3(const void *dataptr, int len)
{
  const u8_t *pb = (const u8_t *)dataptr;
  const u16_t *ps;
  u16_t t = 0;
  const u32_t *pl;
  u32_t sum = 0, tmp;
  /* starts at odd byte address? */
  int odd = ((mem_ptr_t)pb & 1);

  if (odd && len > 0) {
    ((u8_t *)&t)[1] = *pb++;
    len--;
  }

  ps = (const u16_t *)(const void *)pb;

  if (((mem_ptr_t)ps & 3) && len > 1) {
    sum += *ps++;
    len -= 2;
  }

  pl = (const u32_t *)(const void *)ps;

  while (len > 7)  {
    tmp = sum + *pl++;          /* ping */
    if (tmp < sum) {
      tmp++;                    /* add back carry */
    }

    sum = tmp + *pl++;          /* pong */
    if (sum < tmp) {
      sum++;                    /* add back carry */
    }

    len -= 8;
  }

  /* make room in upper bits */
  sum = FOLD_U32T(sum);

  ps = (const u16_t *)pl;

  /* 16-bit aligned word remaining? */
  while (len > 1) {
    sum += *ps++;
    len -= 2;
  }

  /* dangling tail byte remaining? */
  if (len > 0) {                /* include odd byte */
    ((u8_t *)&t)[0] = *(const u8_t *)ps;
  }

  sum += t;                     /* add end bytes */

  /* Fold 32-bit sum to 16 bits
     calling this twice is probably faster than if statements... */
  sum = FOLD_U32T(sum);
  sum = FOLD_U32T(sum);

  if (odd) {
    sum = SWAP_BYTES_IN_WORD(sum);
  }

  return (u16_t)sum;
}

static unsigned char packet[1450];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  for( unsigned i = 0; i < sizeof packet; ++i )
      packet[i] = rand(); 
}

uint32_t timerCounts[3];
uint32_t loopCount;

void loop() {
u32_t time0 = micros();
u16_t res1 = lwip_standard_chksum1(packet, sizeof packet);
u32_t time1 = micros();
u16_t res2 = lwip_standard_chksum2(packet, sizeof packet);
u32_t time2 = micros();
u16_t res3 = lwip_standard_chksum3(packet, sizeof packet);
u32_t time3 = micros();
memcpy( packet, packet + 256, rand()%256 );//Stir the packet around a bit
u32_t time4 = micros();

timerCounts[0] += time1-time0;
timerCounts[1] += time2-time1;
timerCounts[2] += time3-time2;
loopCount += 1;

if( loopCount % 10000 == 0 ) {
  Serial.printf( "%f %f %f\n",
    (float)timerCounts[0]/(float)loopCount,
    (float)timerCounts[1]/(float)loopCount,
    (float)timerCounts[2]/(float)loopCount
    );
  }
}
