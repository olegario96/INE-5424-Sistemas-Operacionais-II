/*
 * Example of main application
 */

#include "Adafruit_Fingerprint.cc"
#include <uart.h>
#include <machine.h>
#include <utility/ostream.h>

using namespace EPOS;

int getFingerprintIDez();

// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
OStream cout;
UART *mySerial;
Adafruit_Fingerprint *finger;

// On Leonardo/Micro or others with hardware serial, use those! #0 is green wire, #1 is white
//Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial1);

void setup()
{

  cout << "Adafruit finger detect test" << "\n";

  // set the data rate for the sensor serial port
  finger->begin(57600);

  if (finger->verifyPassword()) {
    cout << "Found fingerprint sensor!" << "\n";
  } else {
    cout << "Did not find fingerprint sensor :(" << "";
    while (1);
  }
  cout << "Waiting for valid finger->.." << "\n";
}

void loop()                     // run over and over again
{
  getFingerprintIDez();
  Machine::delay(50);           //don't ned to run this at full speed.
}

uint32_t getFingerprintID() {
  uint32_t p = finger->getImage();
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

  // OK success!

  p = finger->image2Tz();
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
  p = finger->fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    cout << "Found a print match!" << "\n";
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    cout << "Communication error" << "\n";
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    cout << "Did not find a match" << "\n";
    return p;
  } else {
    cout << "Unknown error" << "\n";
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

int main()
{
  mySerial = new UART(0, 9600, 8, 0, 1);
  finger = new Adafruit_Fingerprint(mySerial);
  setup();
  loop();
  return 0;
}