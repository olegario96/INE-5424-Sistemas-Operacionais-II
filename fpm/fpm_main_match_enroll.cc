/*
 * Example of main application
 */

#include "FPM.cc"
#include <uart.h>
#include <machine.h>
#include <alarm.h>
#include <utility/ostream.h>

#define TEMPLATES_PER_PAGE 256
int enroll; //define 0 to match and 1 to enroll
#define TEMPLATE_SIZE 768

int getFingerprintIDez();
uint32_t getFingerprintID();
bool get_free_id(int16_t * id);
int getFingerprintEnroll(int id);

// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)

UART *mySerial;
FPM *finger;
uint8_t * templateBuf;

uint8_t * templateSend;
// On Leonardo/Micro or others with hardware serial, use those! #0 is green wire, #1 is white
//Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);

void setup()
{

  cout << "Adafruit finger detect test" << "\n";

  // set the data rate for the sensor serial port
  while(!finger->begin(mySerial)){
    cout << "Falha na inicialização" << '\n';
    Delay(1000000);
  }

  /*if (finger->verifyPassword()) {
    cout << "Found fingerprint sensor!" << "\n";
  } else {
    cout << "Did not find fingerprint sensor :(" << "";
    while (1);
  }
  cout << "Waiting for valid finger->.." << "\n";
  */
}

void loopMatch()                     // run over and over again
{
  cout << "Put your finger on the sensor" << endl;
  getFingerprintIDez();
  Delay(500000);           //don't ned to run this at full speed.
}


uint32_t getFingerprintID() {
  uint32_t p = finger->getImage();
  switch (p) {
    case FINGERPRINT_OK:
      cout << "Image taken" << "\n";
      Delay(2000000);
      break;
    case FINGERPRINT_NOFINGER:
      cout << "No finger detected" << "\n";
      Delay(2000000);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      cout << "Communication error" << "\n";
      Delay(2000000);
      return p;
    case FINGERPRINT_IMAGEFAIL:
      cout << "Imaging error" << "\n";
      Delay(2000000);
      return p;
    default:
      cout << "Unknown error" << "\n";
      Delay(2000000);
      return p;
  }

  // OK success!

  p = finger->image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      cout << "Image converted" << "\n";
      Delay(2000000);
      break;
    case FINGERPRINT_IMAGEMESS:
      cout << "Image too messy" << "\n";
      Delay(2000000);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      cout << "Communication error" << "\n";
      Delay(2000000);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      cout << "Could not find fingerprint features" << "\n";
      Delay(2000000);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      cout << "Could not find fingerprint features" << "\n";
      Delay(2000000);
      return p;
    default:
      cout << "Unknown error" << "\n";
      Delay(2000000);
      return p;
  }

  // OK converted!
  p = finger->fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    cout << "Found a print match!" << "\n";
    Delay(2000000);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    cout << "Communication error" << "\n";
    Delay(2000000);
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    cout << "Did not find a match" << "\n";
    Delay(2000000);
    return p;
  } else {
    cout << "Unknown error" << "\n";
    Delay(2000000);
    return p;
  }

  // found a match!
  cout << "Found ID #" << finger->fingerID << "\n";
  cout << " with confidence of " << finger->confidence << "\n";
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint32_t p = finger->getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger->image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger->fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  cout << "Found ID #" << finger->fingerID << "\n";
  cout << " with confidence of " << finger->confidence << "\n";
  return finger->fingerID;
}

void loopEnroll()                     // run over and over again
{

  cout << ("Send any character to enroll a finger->..") << '\n';
  cout << ("Searching for a free slot to store the template...")<< '\n';
  int16_t id;
  if (get_free_id(&id))
    getFingerprintEnroll(id);
  else
    cout << ("No free slot in flash library!") << '\n';
  //while (Serial.get() != -1); // clear buffer just in case
  Delay(500000);           //don't ned to run this at full speed.
}

bool get_free_id(int16_t * id){
  int p = -1;
  for (int page = 0; page < (finger->capacity / TEMPLATES_PER_PAGE) + 1; page++){
    p = finger->getFreeIndex(page, id);
    switch (p){
      case FINGERPRINT_OK:
        if (*id != FINGERPRINT_NOFREEINDEX){
          cout << "Free slot at ID ";
          cout << *id << endl;
          return true;
        }
      case FINGERPRINT_PACKETRECIEVEERR:
        cout <<("Communication error!") << endl;
        return false;
      default:
        cout << ("Unknown error!") << endl;
        return false;
    }
}
}

int getFingerprintEnroll(int id) {
int p = -1;
while(p != FINGERPRINT_OK){
  p = finger->getImage();
  switch (p) {
    case FINGERPRINT_OK:
      cout << "Image taken" << "\n";
      break;
    case FINGERPRINT_NOFINGER:
      cout << "No finger detected" << "\n";
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      cout << "Communication error" << "\n";
      return p;
    case FINGERPRINT_IMAGEFAIL:
      cout << "Imaging error" << "\n";
      return p;
    default:
      cout << "Unknown error" << "\n";
      return p;
  }
}
  // OK success!

  p = finger->image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      cout << "Image converted" << "\n";
      Delay(2000000);
      break;
    case FINGERPRINT_IMAGEMESS:
      cout << "Image too messy" << "\n";
      Delay(2000000);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      cout << "Communication error" << "\n";
      Delay(2000000);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      cout << "Could not find fingerprint features" << "\n";
      Delay(2000000);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      cout << "Could not find fingerprint features" << "\n";
      Delay(2000000);
      return p;
    default:
      cout << "Unknown error" << "\n";
      return p;
  }

  cout << "Remova o dedo!" << '\n';

  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger->getImage();
  }

  p = -1;
  cout << "Coloque o mesmo dedo de novo!" << '\n';
  Delay(3000000);
  while(p != FINGERPRINT_OK){
  p = finger->getImage();
  switch (p) {
    case FINGERPRINT_OK:
      cout << "Image taken" << "\n";
      Delay(2000000);
      break;
    case FINGERPRINT_NOFINGER:
      cout << "No finger detected" << "\n";
      Delay(2000000);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      cout << "Communication error" << "\n";
      Delay(2000000);
      return p;
    case FINGERPRINT_IMAGEFAIL:
      cout << "Imaging error" << "\n";
      Delay(2000000);
      return p;
    default:
      cout << "Unknown error" << "\n";
      Delay(2000000);
      return p;
  }
}

  p = finger->image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      cout << "Image converted" << "\n";
      Delay(2000000);
      break;
    case FINGERPRINT_IMAGEMESS:
      cout << "Image too messy" << "\n";
      Delay(2000000);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      cout << "Communication error" << "\n";
      Delay(2000000);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      cout << "Could not find fingerprint features" << "\n";
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      cout << "Could not find fingerprint features" << "\n";
      return p;
    default:
      cout << "Unknown error" << "\n";
      return p;
  }


  // OK converted!
  p = finger->createModel();
  if (p == FINGERPRINT_OK) {
    cout << ("Prints matched!");
    Delay(2000000);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    cout << ("Communication error");
    Delay(2000000);
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    cout << ("Fingerprints did not match");
    Delay(2000000);
    return p;
  } else {
    cout << ("Unknown error");
    Delay(2000000);
    return p;
  } 

  cout << ("ID ") << id << " ";
  p = finger->storeModel(id);
  if (p == FINGERPRINT_OK) {
    cout << ("Stored!");
    Delay(2000000);
    return 0;
  }else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    cout << ("Communication error");
    Delay(2000000);
    return p;
  }else if (p == FINGERPRINT_BADLOCATION) {
    cout << ("Could not store in that location");
    Delay(2000000);
    return p;
  }else if (p == FINGERPRINT_FLASHERR) {
    cout << ("Error writing to flash");
      Delay(2000000);
    return p;
  } else {
    cout << ("Unknown error");
    Delay(2000000);
    return p;
  }
}

void getTemplate(uint16_t id){
  uint16_t pos = 0, count = 0, buflen = TEMPLATE_SIZE; 
  bool lastPacket = false, working;

  uint8_t p = finger->loadModel(id);
  switch (p) {
    case FINGERPRINT_OK:
      cout << "template " << id << " loaded" << endl;
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      cout << "Communication error" << endl;
      return;
    case 12:
      cout << "Error when reading template or inavlid!" << endl;
      return;
    default:
      cout << "Unknown error " << p << endl;
      return;
  }

  // OK success!
  p = finger->getModel();
  switch (p) {
    case FINGERPRINT_OK:
      break;
   default:
      cout << "Unknown error " << p << endl;
      return;
  }

  Delay(5000);
  while (true){
    while(!mySerial->ready_to_get());
    bool working = finger->readRaw(templateBuf + pos, ARRAY_TYPE, &lastPacket, &buflen);
    if (working){
      count++;
      pos += buflen;
      buflen = TEMPLATE_SIZE - pos;
      if (lastPacket){
        cout << "POS ATUAL: " << pos << endl;
        break;
      }
    }
    else {
      cout << "POS ATUAL: " << pos << endl;
      cout << ("Error receiving packet") << endl;
      return;
    }
  }
  
  cout << "WHILE EXECUTOU: " << count << endl;

  cout << ("---------------------------------------------") << endl;
  for (int i = 0; i < TEMPLATE_SIZE; ++i)
  {
    cout << "0x" << hex << templateBuf[i] << ", ";
  }
  cout << ("--------------------------------------------") << endl;
}

void sendTemplate(uint16_t id){
  int p = finger->uploadModel();
  switch (p) {
    case FINGERPRINT_OK:
      cout << ("Starting template upload") << endl;;
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      cout << ("Comms error") << endl;;
      return;
    case FINGERPRINT_PACKETRESPONSEFAIL:
      cout << ("Did not receive packet") << endl;;
      return;
    default:
      cout << ("Unknown error") << endl;;
      return;
  }

  finger->writeRaw(templateSend, TEMPLATE_SIZE);

  Delay(5000);
  p = finger->storeModel(id);
  switch (p) {
    case FINGERPRINT_OK:
      cout << ("Template stored at ID ") << (id) << endl;
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      cout << ("Comms error") << endl;
      break;
    case FINGERPRINT_BADLOCATION:
      cout << ("Could not store in that location") << endl;
      break;
    case FINGERPRINT_FLASHERR:
      cout << ("Error writing to flash") << endl;
      break;
    default:
      cout << ("Unknown error") << endl;
  }
  return;
}

int main()
{
  Delay(7000000);
  mySerial = new UART(0, 9600, 8, 0, 1);
  finger = new FPM();
  templateBuf = new uint8_t[TEMPLATE_SIZE];
  templateSend = new uint8_t[TEMPLATE_SIZE];
  setup();
  /*for (int i = 0; i < TEMPLATE_SIZE; ++i)
  {
    templateSend[i] = templateBuf[TEMPLATE_SIZE - 1 - i];
  }*/

  Delay(50000);
  getTemplate(0);
  finger->emptyDatabase();
  templateSend = templateBuf;
  sendTemplate(0);
  Delay(3000000);

  enroll = 0;
  if(enroll)
     while(true)
         loopEnroll();
  else
     while(true)
         loopMatch();
  return 0;
}
