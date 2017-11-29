#include <machine/cortex/FPM.h>
#include <uart.h>
#include <alarm.h>
#include <utility/ostream.h>
#include <machine/cortex/esp8266.h>
#include <display.h>
#include <gpio.h>
#include <string.h>
using namespace EPOS;
OStream cout;

int enroll; //define 0 to match and 1 to enroll

int getFingerprintIDez();
uint32_t getFingerprintID();
int getFingerprintEnroll(int id);
void sendTemplate(uint16_t id);

UART * wifiSerial;
ESP8266 * wifi;
UART * mySerial;
FPM * finger;
uint8_t * templateSend;

void setup()
{

  cout << "Inicializando sensor biometrico" << "\n";

  // set the data rate for the sensor serial port
  while(!finger->begin(mySerial)){
    cout << "Falha na inicialização" << '\n';
    Delay(1000000);
  }

  cout << "Sensor Inicializado" << endl;
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
  if (finger->getFreeIndex(&id)){
    cout << "Free slot at ID ";
    cout << id << endl;
    getFingerprintEnroll(id);
  }
  else
    cout << ("No free slot in flash library!") << '\n';
  //while (Serial.get() != -1); // clear buffer just in case
  Delay(500000);           //don't ned to run this at full speed.
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
  if(finger->getBufOneTemplate(templateSend)){
    //cout << ("---------------------------------------------") << endl;
    for (int i = 0; i < TEMPLATE_SIZE; ++i) {
      //cout << "0x" << hex << templateSend[i] << ", ";
    }
    //cout << ("--------------------------------------------") << endl;    
  } else {
    cout << "Error getting template!" << endl;
  }

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

  Delay(200000);
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
      break;
  }
  return;
}

void sendAllTemplates(){
	finger->getTemplateCount();
	cout << (uint8_t) (finger->templateCount >> 8);
	cout << (uint8_t) (finger->templateCount);
  char res[500];
  int size;
  finger->getTemplateCount();
	for (int i = 0; i < finger->templateCount; ++i)
	{
		getTemplate(i);
		size = wifi->post((char *) templateSend, TEMPLATE_SIZE, res);
	}
}

void sendImageCout(){
	uint32_t p = finger->getImage();
	while(p != FINGERPRINT_OK){
		p = finger->getImage();
		Delay(50000);
	}

	p = finger->downImage();
	cout << "Resposta downImage:" << p << endl;
	finger->sendImageSerial();
}

int main()
{
  Delay(5000000);
  mySerial = new UART(0, 9600, 8, 0, 1);
  wifiSerial = new UART(1, 9600, 8, 0, 1);
  GPIO rst('B', 3, GPIO::OUT);
  rst.set(true);
  wifi = new ESP8266(wifiSerial, &rst);
  finger = new FPM();
  templateSend = new uint8_t[TEMPLATE_SIZE+2];
  setup();
  Delay(1000000);
  finger->emptyDatabase();

  char host[] = "hammerfall.g.olegario.vms.ufsc.br";

  cout << "CONFIGURANDO ENDPOINT" << endl;
  wifi->config_endpoint(5000, host, sizeof(host) - 1, "/INE5424", 8);
  
  cout << "Tentando conectar" << endl;
  
  wifi->connect("LRG", 3, "$UFSC$LRG", 9);
  int vezes = 0;
  while(!wifi->connected()){
     vezes++;
     cout << "Tentando conectar " << vezes << endl;
     Delay(3000000);
     wifi->connect("LRG", 3, "$UFSC$LRG", 9);
  }
   
  cout << "CONNECTED" << endl;

  //Capturando biometrias
  enroll = 1;
  if(enroll){
  	  	 finger->getTemplateCount();
     while(finger->templateCount < 1){
     	 finger->getTemplateCount();
         loopEnroll();
     }
 }
  else
     while(true)
         loopMatch();

  //Enviando biometrias para base de dados
  sendAllTemplates();
  finger->emptyDatabase();
  Delay(5000000);
  //Processo de baixar as biometrias
  int size = 0, numBiometrics;
  char numBiometricsChar[7], biometricChar[7];

  size = wifi->get(numBiometricsChar, "0", 1);
  while(strncmp("ERR", numBiometricsChar, 3) == 0)
    size = wifi->get(numBiometricsChar, "0", 1);

  numBiometricsChar[size - 2] = '\0';
  cout << "Num biometrias CHAR: " << numBiometricsChar << endl;
  numBiometrics = atoi(numBiometricsChar);
  for (int i = 1; i <= numBiometrics; ++i){
    size = 0;
    while(size != TEMPLATE_SIZE + 2){
      itoa(i, biometricChar);
      cout << endl;
      cout << "Biometric Number String:" << biometricChar << endl;
      size = wifi->get((char *) templateSend, biometricChar,
       strlen(biometricChar));
      sendTemplate(i-1);
    }
  }
  
  while(true)
         loopMatch();
  return 0;
}
