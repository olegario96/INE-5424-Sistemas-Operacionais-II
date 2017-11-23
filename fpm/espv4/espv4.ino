#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <time.h>
#include "FS.h"

extern "C" {
  #include "user_interface.h"
  #include "c_types.h"
}

class IoTConnection {
static const int SSID_MAX_SIZE = 32;
static const int PASS_MAX_SIZE = 32;
static const int HOST_MAX_SIZE = 128;
static const int ROUTE_MAX_SIZE = 128;
static const int MAX_MESSAGE_SIZE = 1024;
static const int USER_MAX_SIZE = 64;
static const int CONNECT_MAX_TRIES = 3;
private:
  char ssidChar[SSID_MAX_SIZE+1];
  String ssid;
  int ssid_size;

  char passChar[PASS_MAX_SIZE+1];
  String pass;
  int pass_size;

  //char host[HOST_MAX_SIZE];
  String host;
  int host_size;

  //char route[ROUTE_MAX_SIZE];
  String route;
  int route_size;

  char username_char[USER_MAX_SIZE+1];
  String username;
  int username_size;

  int port;

  bool user_connect;

  File certificate;
  File key;
  char * certificateBuffer;
  char * keyBuffer;

  enum state_t { CONNECTED, DISCONNECTED, HELLODONE, HELLONOTDONE };
  int con_state;
  int server_con_state;

  unsigned long int last_time_recorded;
  unsigned long int last_response_time;

  HTTPClient http;
  ESP8266WiFiMulti WiFiMulti;
private:
  bool can_try_connection(){
    return (ssid_size > 0 && (user_connect || pass_size > 0));
  }

  bool is_all_setup_correctly(){
    if(host_size == 0 || port == 0)
      return false;
    
    return true;
  }

public:
  IoTConnection(){
    ssid_size = 0;
    pass_size = 0;
    host_size = 0;
    route_size = 0;
    port = 0;
    user_connect = false;

    con_state = DISCONNECTED;
    server_con_state = HELLONOTDONE;
  }

  String charToString(char * buf, int len){
    String ret = "";
    for(int i = 0; i < len; i++){
      ret += buf[i]; 
    }
    return ret;
  }

  bool connect_to_wifi(){
    int tries = 0;
    if(!can_try_connection())
      return false;
    
    //if wifi has username --> deal with certificate and key files
    if(user_connect){
      
      username.toCharArray(username_char, username_size+1);
      wifi_station_set_username((unsigned char*) username_char, username_size+1);

      SPIFFS.begin();
      certificate = SPIFFS.open("/cert.der", "r");
      key = SPIFFS.open("/key.der", "r");
      
      if(!key || !certificate)
        return false;

      certificateBuffer = new char[certificate.size()];
      keyBuffer = new char[key.size()];
      certificate.readBytes(certificateBuffer, certificate.size());
      key.readBytes(keyBuffer, key.size());
      
      if( wifi_station_set_cert_key((uint8 *) certificateBuffer, certificate.size(), (uint8 *) keyBuffer, key.size(), NULL, 0) != 0 ) {
        return false;
      }
      delete certificateBuffer;
      delete keyBuffer;
    }

    if(get_status())
      WiFi.disconnect();      
    ssid.toCharArray(ssidChar, ssid_size+1);
    pass.toCharArray(passChar, pass_size+1);
    
    WiFiMulti.APlistClean();
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(ssidChar, passChar);
    while(WiFiMulti.run() != WL_CONNECTED){
      delay(1000);
      if(tries >= CONNECT_MAX_TRIES)
        return false;
      tries++;
   }
    return true;
  }

  bool disconnect_from_wifi(){
     WiFi.disconnect();

     con_state = DISCONNECTED;
     return true;
  }

  bool post(unsigned char * data, int data_size){
    if(!is_all_setup_correctly() || data_size >= MAX_MESSAGE_SIZE)
      return false;
    
    http.begin(host, port, route);
    //Serial.print("[HTTP] POST ...\n");
    // start connection and send HTTP header
    int httpCode = http.POST(data, data_size);

     if(httpCode > 0) {
     // HTTP header has been send and Server response header has been handled
       //Serial.printf("[HTTP] POST... code: %d\n", httpCode);
       // file found at server
        if(httpCode == HTTP_CODE_OK) {
          return true;
        }
           
        } else {
            //Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
            return false;
        }
        
    last_time_recorded = millis();
  
    return true;
     
  }

  bool get(unsigned char * message, int message_size) {
    if(!is_all_setup_correctly() || message_size >= MAX_MESSAGE_SIZE)
      return false;

    http.begin(host, port, route + "/" + charToString((char *) message, message_size));
    //Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

     if(httpCode > 0) {
     // HTTP header has been send and Server response header has been handled
       //Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        int len = http.getSize();
       // file found at server and length is acceptable
        if(httpCode == HTTP_CODE_OK && len < MAX_MESSAGE_SIZE) {
         
          WiFiClient * stream = http.getStreamPtr();
          stream->readBytes(message, len);
          Serial.write(message, len);
        }
           
        } else {
            //Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            return false;
        }
        
    last_time_recorded = millis();
  
    return true;
  }

  bool set_user_connect(char * enable, int data_size){
    if(data_size == 1){
      if(enable[0] == '1')
        user_connect = true;
      else if(enable[0] == '0')
        user_connect = false; 
      else
        return false;
      return true;
    } else
      return false;
  }

  bool get_user_connect(){
    return user_connect;
  }

  bool set_user(char * user, int user_size){
    if(user_size <= USER_MAX_SIZE){
      username_size = user_size;
      username = charToString(user, user_size);
      return true;
    } else
      return false;
  }

  String get_user(){
    return username;
  }
  
  bool get_status(){
    if(WiFi.status() == WL_CONNECTED)
      con_state = CONNECTED;
    else
      con_state = DISCONNECTED;
    
    return (con_state == CONNECTED);
  }

  void get_ip(){
    Serial.println(WiFi.localIP());
  }

  bool set_ssid(char *newssid, int ssidsize) {
    if(ssidsize > SSID_MAX_SIZE || newssid == 0)
      return false;

    ssid_size = ssidsize;
    ssid = charToString(newssid, ssidsize);

    return true;
  }

  bool set_pass(char *newpass, int passsize) {
    if(passsize > PASS_MAX_SIZE || newpass == 0)
      return false;

    pass_size = passsize;
    pass = charToString(newpass, passsize);
    return true;
  }

  bool set_host(char *newhost, int hostsize) {
    if(hostsize > HOST_MAX_SIZE || newhost == 0)
      return false;

    host_size = hostsize;
    host = charToString(newhost, hostsize);

    return true;
  }

  bool set_route(char *newroute, int routesize) {
    if(routesize > ROUTE_MAX_SIZE || newroute == 0)
      return false;

    route_size = routesize;
    route = charToString(newroute, routesize);

    return true;
  }

  bool set_port(int newport) {
    port = newport;

    return true;
  }

  bool set_timeout(uint16_t timeout){
    http.setTimeout(timeout);
    return true;
  }
  String get_ssid() {
    return ssid;
  }

  int get_ssid_size(){
    return ssid_size;
  }

  String get_pass() {
    return pass;
  }

  int get_pass_size(){
    return pass_size;
  }

  String get_host() {
    return host;
  }

  int get_host_size(){
    return host_size;
  }

  String get_route() {
    return route;
  }

  int get_route_size(){
    return route_size;
  }

  int get_port(){
    return port;
  }
  
};

void act();

IoTConnection iot;

void setup() {
  Serial.begin(9600);
  for(uint8_t t = 7; t > 0; t--) {
      //Serial.printf("[SETUP] WAIT %d...\n", t);
      //Serial.flush();
      //delay(1000);
    }
}

char command[32];
int command_pos = 0;

char data[4096];
int data_pos = 0;

bool is_command = true; //alternates between command or data

void loop() {
  
  if(Serial.available() != 0){ //if received a command or data
    while(Serial.available() != 0){ //read until it finds an end
      
      if(is_command) {
        command[command_pos] = Serial.read();
        
        if(command[command_pos] == '=') {          
          is_command = false;
          
          continue;
        }

        if(command[command_pos] == '\n' && command[command_pos - 1] == '\r') {
          act();          

          command_pos = 0;
          break;          
        }

        if(command[command_pos] == '+' && command[command_pos-1] == '+' && command[command_pos-2] == '+') {
          is_command = true;
          data_pos = 0;
          command_pos = 0;

          Serial.read(); //remove \r\n
          Serial.read(); //remove \r\n

          Serial.print("OK");
          Serial.print('\r');
          Serial.print('\n');

          continue;
        }
        
        command_pos++;

        
      } else {
        data[data_pos] = Serial.read();

        if(data[data_pos] == '\n' && data[data_pos - 1] == '\r') {
          act();
          
          break;
        } 

        if(data[data_pos] == '+' && data[data_pos-1] == '+' && data[data_pos-2] == '+') {
          is_command = true;
          data_pos = 0;
          command_pos = 0;

          Serial.read(); //remove \r\n
          Serial.read(); //remove \r\n

          Serial.print("OK");
          Serial.print('\r');
          Serial.print('\n');

          continue;
        }

        data_pos++;
      }
      
    }
    
  }

}

void act(){

  if(command[0] != 'A' || command[1] != 'T' || command[2] != '+')
    return;

  int action_size = command_pos - 4;
  int data_size = data_pos - 1;

  if(action_size < 1)
    return;
  
  char action[action_size];

  for(int i = 3; i < command_pos; i++){
    action[i - 3] = command[i];
  }

  if(strncmp(action,"SETSSID",action_size) == 0){
    if(data_size == 0)
      return;

    bool result = iot.set_ssid(data, data_size);

    if(result)
      Serial.print("OK");      
    else
      Serial.print("ERR");

  } else if(strncmp(action,"SETPASSWORD",action_size) == 0){
    if(data_size == 0)
      return;

    bool result = iot.set_pass(data, data_size);

    if(result)
      Serial.print("OK");      
    else
      Serial.print("ERR");

  } else if(strncmp(action,"SETHOST",action_size) == 0){
    if(data_size == 0)
      return;

    bool result = iot.set_host(data, data_size);

    if(result)
      Serial.print("OK");      
    else
      Serial.print("ERR");

  } else if(strncmp(action,"SETROUTE",action_size) == 0){
    if(data_size == 0)
      return;

    bool result = iot.set_route(data, data_size);

    if(result)
      Serial.print("OK");      
    else
      Serial.print("ERR");

  } else if(strncmp(action,"SETPORT",action_size) == 0){
    if(data_size == 0)
      return;

      String temp;

      for(int i = 0; i < data_size; i++)
        temp += data[i];

      bool result = iot.set_port(temp.toInt());

      if(result)
        Serial.print("OK");      
      else
        Serial.print("ERR");

  } else if(strncmp(action,"SETTIMEOUT",action_size) == 0) {
     String temp;
     int timeout;
     bool result = false;
     for(int i = 0; i < data_size; i++)
        temp += data[i];

     if(timeout = temp.toInt() != 0)
        result = iot.set_timeout(timeout);

      if(result)
        Serial.print("OK");      
      else
        Serial.print("ERR");
  } else if(strncmp(action,"GETSSID",action_size) == 0){

    //char temp[iot.get_ssid_size()];

    //memcpy(temp, iot.get_ssid(), iot.get_ssid_size());
    Serial.print(iot.get_ssid());

  } else if(strncmp(action,"GETPASS",action_size) == 0){

    //char temp[iot.get_pass_size() + 1];

    //memcpy(temp, iot.get_pass(), iot.get_pass_size());
    //temp[iot.get_pass_size()] = '\0';
    
    Serial.print(iot.get_pass());

  } else if(strncmp(action,"GETHOST",action_size) == 0){
    
    //char temp[iot.get_host_size()];

    //memcpy(temp, iot.get_host(), iot.get_host_size());
    Serial.print(iot.get_host());

  } else if(strncmp(action,"GETROUTE",action_size) == 0){
    
    //char temp[iot.get_route_size()];

    //memcpy(temp, iot.get_route(), iot.get_route_size());
    
    Serial.print(iot.get_route());

  } else if(strncmp(action,"GETPORT",action_size) == 0){
    
    Serial.print(iot.get_port());

  } else if(strncmp(action,"RESPONSETIME",action_size) == 0){
    
    Serial.println("OK");

  } else if(strncmp(action,"CONNECTWIFI",action_size) == 0){
    bool result = iot.connect_to_wifi();

    if(result)
      Serial.print("OK");
    else
      Serial.print("ERR");

  } else if(strncmp(action,"DISCONNECTWIFI",action_size) == 0){
    
    bool result = iot.disconnect_from_wifi();

    if(result)
      Serial.print("OK");
    else
      Serial.print("ERR");

  } else if(strncmp(action,"GETIP",action_size) == 0){
    
    iot.get_ip();

  } else if(strncmp(action,"CONNECTIONSTATUS",action_size) == 0){
    
    bool is_connected = iot.get_status();
    Serial.print(is_connected);

  } else if(strncmp(action,"GETTIMESTAMP",action_size) == 0){

    if(!iot.get_status()) {
      Serial.print("ERR");
    } else {
      configTime(8 * 3600, 0, "pool.ntp.org", "time.nist.gov");
      time_t now = time(nullptr);
      while (now < 1000) {
        delay(2);
        now = time(nullptr);
      }
      Serial.print(now);  
    }

  } else if(strncmp(action,"GETHEAPSIZE",action_size) == 0){
    
    Serial.print(ESP.getFreeHeap());

  } else if(strncmp(action,"SENDPOST",action_size) == 0){
    if(data_size == 0) {
      Serial.print("ERR");
    } else {
      bool result = iot.post((unsigned char *) data, data_size);
      if(result)
        Serial.print("OK");
      else
        Serial.print("ERR");  
    }

  } else if(strncmp(action,"SENDGET",action_size) == 0){
      bool res = iot.get((unsigned char *) data, data_size);
      
      if(!res)
        Serial.print("ERR"); 
  } else if(strncmp(action,"SETUSER",action_size) == 0){
      bool res = iot.set_user(data, data_size);
      
      if(res)
        Serial.print("OK");
      else
        Serial.print("ERR");   
  } else if(strncmp(action,"GETUSER",action_size) == 0){
      Serial.print(iot.get_user());
      
  } else if(strncmp(action,"SETUSERCONNECT",action_size) == 0){
      bool res = iot.set_user_connect(data, data_size);  
        
      if(res)
        Serial.print("OK");
      else
        Serial.print("ERR");  
  } else if(strncmp(action,"GETUSERCONNECT",action_size) == 0){
      Serial.print(iot.get_user_connect());   
  } else {
    Serial.print("INVALIDCOMMAND");
  }

  Serial.print('\r');
  Serial.print('\n');

}

