/***************************************************
  This is a library for our optical Fingerprint sensor

  Designed specifically to work with the Adafruit Fingerprint sensor
  ----> http://www.adafruit.com/products/751

  These displays use TTL Serial to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ****************************************************/

#include "Adafruit_Fingerprint.h"
#include <uart.h>
#include <machine.h>
#include <utility/ostream.h>

using namespace EPOS;

//static SoftwareSerial mySerial = SoftwareSerial(2, 3);
Adafruit_Fingerprint::Adafruit_Fingerprint(UART *ss) {
  thePassword = 0;
  theAddress = 0xFFFFFFFF;

  mySerial = ss;
}

void Adafruit_Fingerprint::begin(uint32_t baud_rate) {
  Machine::delay(1000);  // one second delay to let the sensor 'boot up'

  mySerial->config(baud_rate, 8, 0, 1);
}

bool Adafruit_Fingerprint::verifyPassword(void) {
  uint32_t packet[] = {FINGERPRINT_VERIFYPASSWORD,
                      (thePassword >> 24), (thePassword >> 16),
                      (thePassword >> 8), thePassword};
  writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 7, packet);
  uint32_t len = getReply(packet);

  if ((len == 1) && (packet[0] == FINGERPRINT_ACKPACKET) && (packet[1] == FINGERPRINT_OK))
    return true;

/*
  Serial.print("\nGot packet type "); Serial.print(packet[0]);
  for (uint32_t i=1; i<len+1;i++) {
    Serial.print(" 0x");
    Serial.print(packet[i], HEX);
  }
  */
  return false;
}

uint32_t Adafruit_Fingerprint::getImage(void) {
  uint32_t packet[] = {FINGERPRINT_GETIMAGE};
  writePacket(theAddress, FINGERPRINT_COMMANDPACKET, 3, packet);
  uint32_t len = getReply(packet);

  if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
   return -1;
  return packet[1];
}

uint32_t Adafruit_Fingerprint::image2Tz(uint32_t slot) {
  uint32_t packet[] = {FINGERPRINT_IMAGE2TZ, slot};
  writePacket(theAddress, FINGERPRINT_COMMANDPACKET, sizeof(packet)+2, packet);
  uint32_t len = getReply(packet);

  if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
   return -1;
  return packet[1];
}


uint32_t Adafruit_Fingerprint::createModel(void) {
  uint32_t packet[] = {FINGERPRINT_REGMODEL};
  writePacket(theAddress, FINGERPRINT_COMMANDPACKET, sizeof(packet)+2, packet);
  uint32_t len = getReply(packet);

  if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
   return -1;
  return packet[1];
}


uint32_t Adafruit_Fingerprint::storeModel(uint32_t id) {
  uint32_t packet[] = {FINGERPRINT_STORE, 0x01, id >> 8, id & 0xFF};
  writePacket(theAddress, FINGERPRINT_COMMANDPACKET, sizeof(packet)+2, packet);
  uint32_t len = getReply(packet);

  if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
   return -1;
  return packet[1];
}

uint32_t Adafruit_Fingerprint::deleteModel(uint32_t id) {
    uint32_t packet[] = {FINGERPRINT_DELETE, id >> 8, id & 0xFF, 0x00, 0x01};
    writePacket(theAddress, FINGERPRINT_COMMANDPACKET, sizeof(packet)+2, packet);
    uint32_t len = getReply(packet);

    if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
        return -1;
    return packet[1];
}

uint32_t Adafruit_Fingerprint::emptyDatabase(void) {
  uint32_t packet[] = {FINGERPRINT_EMPTY};
  writePacket(theAddress, FINGERPRINT_COMMANDPACKET, sizeof(packet)+2, packet);
  uint32_t len = getReply(packet);

  if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
   return -1;
  return packet[1];
}

uint32_t Adafruit_Fingerprint::fingerFastSearch(void) {
  fingerID = 0xFFFF;
  confidence = 0xFFFF;
  // high speed search of slot #1 starting at page 0x0000 and page #0x00A3
  uint32_t packet[] = {FINGERPRINT_HISPEEDSEARCH, 0x01, 0x00, 0x00, 0x00, 0xA3};
  writePacket(theAddress, FINGERPRINT_COMMANDPACKET, sizeof(packet)+2, packet);
  uint32_t len = getReply(packet);

  if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
   return -1;

  fingerID = packet[2];
  fingerID <<= 8;
  fingerID |= packet[3];

  confidence = packet[4];
  confidence <<= 8;
  confidence |= packet[5];

  return packet[1];
}

uint32_t Adafruit_Fingerprint::getTemplateCount(void) {
  templateCount = 0xFFFF;
  // get number of templates in memory
  uint32_t packet[] = {FINGERPRINT_TEMPLATECOUNT};
  writePacket(theAddress, FINGERPRINT_COMMANDPACKET, sizeof(packet)+2, packet);
  uint32_t len = getReply(packet);

  if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
   return -1;

  templateCount = packet[2];
  templateCount <<= 8;
  templateCount |= packet[3];

  return packet[1];
}



void Adafruit_Fingerprint::writePacket(uint32_t addr, uint32_t packettype,
				       uint32_t len, uint32_t *packet) {
#ifdef FINGERPRINT_DEBUG
  Serial.print("---> 0x");
  Serial.print((uint32_t)(FINGERPRINT_STARTCODE >> 8), HEX);
  Serial.print(" 0x");
  Serial.print((uint32_t)FINGERPRINT_STARTCODE, HEX);
  Serial.print(" 0x");
  Serial.print((uint32_t)(addr >> 24), HEX);
  Serial.print(" 0x");
  Serial.print((uint32_t)(addr >> 16), HEX);
  Serial.print(" 0x");
  Serial.print((uint32_t)(addr >> 8), HEX);
  Serial.print(" 0x");
  Serial.print((uint32_t)(addr), HEX);
  Serial.print(" 0x");
  Serial.print((uint32_t)packettype, HEX);
  Serial.print(" 0x");
  Serial.print((uint32_t)(len >> 8), HEX);
  Serial.print(" 0x");
  Serial.print((uint32_t)(len), HEX);
#endif

/*  mySerial->print((uint32_t)(FINGERPRINT_STARTCODE >> 8), BYTE);
  mySerial->print((uint32_t)FINGERPRINT_STARTCODE, BYTE);
  mySerial->print((uint32_t)(addr >> 24), BYTE);
  mySerial->print((uint32_t)(addr >> 16), BYTE);
  mySerial->print((uint32_t)(addr >> 8), BYTE);
  mySerial->print((uint32_t)(addr), BYTE);
  mySerial->print((uint32_t)packettype, BYTE);
  mySerial->print((uint32_t)(len >> 8), BYTE);
  mySerial->print((uint32_t)(len), BYTE);*/

  uint32_t sum = (len>>8) + (len&0xFF) + packettype;
  for (uint32_t i=0; i< len-2; i++) {
    //mySerial->print((uint32_t)(packet[i]), BYTE);
#ifdef FINGERPRINT_DEBUG
    Serial.print(" 0x"); Serial.print(packet[i], HEX);
#endif
    sum += packet[i];
  }
#ifdef FINGERPRINT_DEBUG
  //Serial.print("Checksum = 0x"); Serial.println(sum);
  /*Serial.print(" 0x"); Serial.print((uint32_t)(sum>>8), HEX);
  Serial.print(" 0x"); Serial.println((uint32_t)(sum), HEX);*/
#endif
  /*mySerial->print((uint32_t)(sum>>8), BYTE);
  mySerial->print((uint32_t)sum, BYTE);*/
}


uint32_t Adafruit_Fingerprint::getReply(uint32_t packet[], uint32_t timeout) {
  uint32_t reply[20], idx;
  uint32_t timer=0;

  idx = 0;
#ifdef FINGERPRINT_DEBUG
  Serial.print("<--- ");
#endif
while (true) {
    while (!mySerial->get()) {
      Machine::delay(1);
      timer++;
      if (timer >= timeout) return FINGERPRINT_TIMEOUT;
    }
    // something to read!
    reply[idx] = mySerial->get();
#ifdef FINGERPRINT_DEBUG
    Serial.print(" 0x"); Serial.print(reply[idx], HEX);
#endif
    if ((idx == 0) && (reply[0] != (FINGERPRINT_STARTCODE >> 8)))
      continue;
    idx++;

    // check packet!
    if (idx >= 9) {
      if ((reply[0] != (FINGERPRINT_STARTCODE >> 8)) ||
          (reply[1] != (FINGERPRINT_STARTCODE & 0xFF)))
          return FINGERPRINT_BADPACKET;
      uint32_t packettype = reply[6];
      //Serial.print("Packet type"); Serial.println(packettype);
      uint32_t len = reply[7];
      len <<= 8;
      len |= reply[8];
      len -= 2;
      //Serial.print("Packet len"); Serial.println(len);
      if (idx <= (len+10)) continue;
      packet[0] = packettype;
      for (uint32_t i=0; i<len; i++) {
        packet[1+i] = reply[9+i];
      }
#ifdef FINGERPRINT_DEBUG
      Serial.println();
#endif
      return len;
    }
  }
}
