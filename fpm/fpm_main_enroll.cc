/*
 * Example of main application
 */

#include "FPM.cc"
#include <uart.h>
#include <machine.h>
#include <utility/ostream.h>
#define TEMPLATES_PER_PAGE 256

bool get_free_id(int16_t * id);
int getFingerprintEnroll(int id);


// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
OStream cout;
UART *mySerial;
FPM *finger;

// On Leonardo/Micro or others with hardware serial, use those! #0 is green wire, #1 is white
//Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);

void setup()
{

  cout << "Adafruit finger detect test" << "\n";

  // set the data rate for the sensor serial port
  if(!finger->begin(mySerial)){
    cout << "Falha na inicialização" << '\n';
    while(1);
  }

  cout << "Found fingerprint sensor!" << "\n";
  cout << "Capacity: " << finger->capacity <<"\n";
  cout << "Packet length: " << finger->packetLen <<"\n";
  /*if (finger->verifyPassword()) {
    cout << "Found fingerprint sensor!" << "\n";
  } else {
    cout << "Did not find fingerprint sensor :(" << "";
    while (1);
  }
  cout << "Waiting for valid finger->.." << "\n";
  */
}

void loop()                     // run over and over again
{

  cout << ("Send any character to enroll a finger->..") << '\n';
  while (!mySerial->ready_to_get());
  cout << ("Searching for a free slot to store the template...")<< '\n';
  int16_t id;
  if (get_free_id(&id))
    getFingerprintEnroll(id);
  else
    cout << ("No free slot in flash library!") << '\n';
  //while (Serial.get() != -1); // clear buffer just in case
  Machine::delay(50);           //don't ned to run this at full speed.
}

bool get_free_id(int16_t * id){
  int p = -1;
  for (int page = 0; page < (finger->capacity / TEMPLATES_PER_PAGE) + 1; page++){
    p = finger->getFreeIndex(page, id);
    switch (p){
      case FINGERPRINT_OK:
        if (*id != FINGERPRINT_NOFREEINDEX){
          cout << "Free slot at ID ";
          cout << *id;
          return true;
        }
      case FINGERPRINT_PACKETRECIEVEERR:
        cout <<("Communication error!");
        return false;
      default:
        cout << ("Unknown error!");
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
      break;
    case FINGERPRINT_IMAGEMESS:
      cout << "Image too messy" << "\n";
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      cout << "Communication error" << "\n";
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

  cout << "Remova o dedo!" << '\n';
  Machine::delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger->getImage();
  }
  

  p = -1;
  cout << "Coloque o mesmo dedo de novo!" << '\n';
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

  p = finger->image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      cout << "Image converted" << "\n";
      break;
    case FINGERPRINT_IMAGEMESS:
      cout << "Image too messy" << "\n";
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      cout << "Communication error" << "\n";
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
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    cout << ("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    cout << ("Fingerprints did not match");
    return p;
  } else {
    cout << ("Unknown error");
    return p;
  } 

  cout << ("ID ") << id << " ";
  p = finger->storeModel(id);
  if (p == FINGERPRINT_OK) {
    cout << ("Stored!");
    return 0;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    cout << ("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    cout << ("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    cout << ("Error writing to flash");
    return p;
  } else {
    cout << ("Unknown error");
    return p;
} 
}

int main()
{
  mySerial = new UART(0, 57600, 8, 0, 1);
  finger = new FPM();
  setup();
  while(true){
  	loop();
  }
  return 0;
}
