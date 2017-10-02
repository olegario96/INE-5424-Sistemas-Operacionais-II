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
  cout << ("---> 0x");
  cout<< std::hex << (uint32_t)(FINGERPRINT_STARTCODE >> 8);
  cout << (" 0x");
  cout<< std::hex << (uint32_t)FINGERPRINT_STARTCODE;
  cout << (" 0x");
  cout<< std::hex << (uint32_t)(addr >> 24);
  cout << (" 0x");
  cout<< std::hex << (uint32_t)(addr >> 16);
  cout << (" 0x");
  cout<< std::hex << (uint32_t)(addr >> 8);
  cout << (" 0x");
  cout<< std::hex << (uint32_t)(addr);
  cout << (" 0x");
  cout<< std::hex << (uint32_t)packettype;
  cout << (" 0x");
  cout<< std::hex << (uint32_t)(len >> 8);
  cout << (" 0x");
  cout<< std::hex << (uint32_t)(len);
#endif
if(mySerial->ready_to_put()){
  mySerial->put((uint8_t)(FINGERPRINT_STARTCODE >> 8));
  mySerial->put((uint8_t)FINGERPRINT_STARTCODE);
  mySerial->put((uint8_t)(addr >> 24));
  mySerial->put((uint8_t)(addr >> 16));
  mySerial->put((uint8_t)(addr >> 8));
  mySerial->put((uint8_t)(addr));
  mySerial->put((uint8_t)packettype);
  mySerial->put((uint8_t)(len >> 8));
  mySerial->put((uint8_t)(len));
}

  uint16_t sum = (len>>8) + (len&0xFF) + packettype;
  for (uint8_t i=0; i< len-2; i++) {
    mySerial->put((uint8_t)(packet[i]));
#ifdef FINGERPRINT_DEBUG
    cout << (" 0x");
    cout << std::hex << packet[i];
#endif
    sum += packet[i];
  }
#ifdef FINGERPRINT_DEBUG
  //Serial.print("Checksum = 0x"); Serial.println(sum);
  cout << '\n';
  cout << (" 0x"); 
  cout<< std::hex << (uint8_t)(sum>>8);
  Serial.print(" 0x"); 
  cout<< std::hex << (uint8_t)(sum);
#endif
  mySerial->put((uint8_t)(sum>>8));
  mySerial->put((uint8_t)sum);
}


uint32_t Adafruit_Fingerprint::getReply(uint32_t packet[], uint32_t timeout) {
  uint32_t reply[20], idx;
  uint32_t timer=0;

  idx = 0;
#ifdef FINGERPRINT_DEBUG
  cout << ("<--- ");
#endif
while (true) {
    while (!mySerial->ready_to_get()) {
      Machine::delay(1);
      timer++;
      if (timer >= timeout) return FINGERPRINT_TIMEOUT;
    }
    // something to read!
    reply[idx] = mySerial->get();
#ifdef FINGERPRINT_DEBUG
    cout << (" 0x"); 
    cout << std::hex << reply[idx];
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
      cout << '\n';
#endif
      return len;
    }
  }
}

/*
 * Available virou get
 * tudo virou 32 bits
 * tirei o print
 * uart
 * configu uart
 */
