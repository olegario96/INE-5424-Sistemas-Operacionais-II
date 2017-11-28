// EPOS Cortex ESP8266 Wi-Fi Board Mediator Implementation

#include <machine/cortex/esp8266.h>

__BEGIN_SYS
OStream out;
// Class attributes

// Methods
RTC::Microsecond ESP8266::now()
{
    flush_serial();

    command_mode();

    const char cmd[] = "AT+GETTIMESTAMP";
    send_no_wait(cmd, strlen(cmd));

    _uart->put('\r');
    _uart->put('\n');

    while(!_uart->ready_to_get()){
        Machine::delay(100);
    }

    char time[11];
    int count = 0;
    while(_uart->ready_to_get() && count < 10){
        Machine::delay(20000);

        time[count] = _uart->get();
        count++;
    }

    time[10] = '\0';

    return atoi(time) * 1000;
}

void ESP8266::reset()
{
    off();
    Machine::delay(500000); // 500ms delay recommended by the manual
    on();
}

void ESP8266::connect(const char *ssid, int ssid_size, const char *pass, int pass_size)
{
    flush_serial();

    assert(ssid_size < SSID_MAX);
    assert(pass_size < PASS_MAX);

    memcpy(_ssid, ssid, ssid_size);
    memcpy(_password, pass, pass_size);

    command_mode();

    char set_ssid[13 + ssid_size];
    const char temp_ssid[] = "AT+SETSSID=";
    const char end_of_line[2] = {'\r', '\n'};

    strncpy(set_ssid, temp_ssid, sizeof(temp_ssid) - 1);
    strncpy(&set_ssid[sizeof(temp_ssid) - 1], ssid, ssid_size);
    strncpy(&set_ssid[sizeof(temp_ssid) + ssid_size - 1], end_of_line, sizeof(end_of_line));

    Machine::delay(100000);
    send_data(set_ssid, sizeof(set_ssid), 1);

    flush_serial();

    char set_pass[17 + pass_size];
    const char temp_pass[] = "AT+SETPASSWORD=";

    strncpy(set_pass, temp_pass, sizeof(temp_pass) - 1);
    strncpy(&set_pass[sizeof(temp_pass) - 1], pass, pass_size);
    strncpy(&set_pass[sizeof(temp_pass) + pass_size - 1], end_of_line, sizeof(end_of_line));

    Machine::delay(100000);
    send_data(set_pass, sizeof(set_pass), 1);

    Machine::delay(100000);

    const char connect_wifi[] = "AT+CONNECTWIFI\r\n";

    _connected = send_data(connect_wifi, sizeof(connect_wifi) - 1, 1, 3000); //15 sec timeout
    flush_serial();
}

void ESP8266::config_endpoint(int port, const char *host, unsigned int host_size, const char *route, unsigned int route_size)
{
 
    flush_serial();
    command_mode();

    Machine::delay(100000);

    assert(host_size < HOST_MAX);
    assert(route_size < ROUTE_MAX);

    _port = port;
    memcpy(_host, host, host_size);
    memcpy(_route, route, route_size);

    char set_host[13 + host_size];
    const char temp_host[] = "AT+SETHOST=";
    const char end_of_line[2] = {'\r', '\n'};

    strncpy(set_host, temp_host, sizeof(temp_host) - 1);
    strncpy(&set_host[sizeof(temp_host) - 1], host, host_size);
    strncpy(&set_host[sizeof(temp_host) + host_size - 1], end_of_line, sizeof(end_of_line));

    Machine::delay(100000);
    send_data(set_host, sizeof(set_host), 5);

    //out << "AT+SETHOST=" << endl;

    char set_route[14 + route_size];
    const char temp_route[] = "AT+SETROUTE=";

    //out << "AT+SETROUTE=" << endl;

    strncpy(set_route, temp_route, sizeof(temp_route) - 1);
    strncpy(&set_route[sizeof(temp_route) - 1], route, route_size);
    strncpy(&set_route[sizeof(temp_route) + route_size - 1], end_of_line, sizeof(end_of_line));

    Machine::delay(100000);
    send_data(set_route, sizeof(set_route), 5);

    const char temp_port[] = "AT+SETPORT=";
    char port_str[33];

    itoa(port, port_str);

    int size_of_port = 0;

    while(port_str[size_of_port] != '\0')
        size_of_port++;

    char set_port[13 + size_of_port];

    strncpy(set_port, temp_port, sizeof(temp_port) - 1);
    strncpy(&set_port[sizeof(temp_port) - 1], port_str, size_of_port);
    strncpy(&set_port[sizeof(temp_port) + size_of_port - 1], end_of_line, sizeof(end_of_line));

    Machine::delay(100000);
    send_data(set_port, sizeof(set_port), 5);
    //out << "AT+SETROUTE=" << endl;
}

bool ESP8266::command_mode()
{
    _uart->put('+');
    _uart->put('+');
    _uart->put('+');
    _uart->put('\r');
    _uart->put('\n');

    return wait_response("OK\r\n", 4);
}

int ESP8266::post(const void * data, int data_size, char * res)
{
    command_mode();

    flush_serial();
    char command[] = "AT+SENDPOST=";

    char request[sizeof(command) - 1 + 2 + data_size]; //-1 for \0 and +2 for \r\n
    strncpy(request, command, sizeof(command) - 1);

    for(int i = 0; i < data_size; i++){
        request[i + sizeof(command) - 1] = reinterpret_cast<const char*>(data)[i];
    }

    request[sizeof(command) - 1 + data_size] = '\r';
    request[sizeof(command) + data_size] = '\n';

    send_no_wait(request, sizeof(request));

    while(!_uart->ready_to_get()){}

    int count = 0;

    while(_uart->ready_to_get()) {
        res[count++] = _uart->get();
        Machine::delay(50000);
    }

    flush_serial();

    command_mode();

    return count;
}

//This method returns the data with \r\n in the end
int ESP8266::get(char * res, char * sub_route, int sub_route_size)
{
    command_mode();
    flush_serial();
    int timer = 0, timeout = 200;
    char command[] = "AT+SENDGET";
    char final[] = "\r\n";
    
    if(sub_route_size > 0){
       int commandCompleteSize = sizeof(command) + sizeof('=') + sub_route_size;
       char commandComplete[commandCompleteSize];
       strncpy(commandComplete, command, sizeof(command));
       commandComplete[sizeof(command)] = '=';
       strncpy(commandComplete + sizeof(command) + 1, sub_route, sub_route_size);
       send_no_wait(commandComplete, commandCompleteSize);
    } else 
        send_no_wait(command, sizeof(command));

    send_no_wait(final, sizeof(final));

    int count = 0;

    while(!_uart->ready_to_get());

    while(_uart->ready_to_get()){
        timer = 0;
        res[count++] = _uart->get();
        while(!_uart->ready_to_get()){
            Machine::delay(5000);
            timer++;
            if (timer >= timeout){
                out << "TIMEOUT GET!" << endl;
                break;             
          }

        }      
    }

    flush_serial();
    out << "Tamanho GET:" << count << endl;
    command_mode();
    return count;
}

bool ESP8266::connected()
{
    command_mode();

    send_no_wait("AT+CONNECTIONSTATUS\r\n", 21);

    _connected = wait_response("1\r\n", 3);

    return _connected;
}

char * ESP8266::ip()
{
    flush_serial();

    command_mode();

    const char cmd[] = "AT+GETIP";
    send_no_wait(cmd, strlen(cmd));

    _uart->put('\r');
    _uart->put('\n');

    while(!_uart->ready_to_get()){
        Machine::delay(100);
    }

    int count = 0;
    while(_uart->ready_to_get()){
        Machine::delay(20000);

        _ip[count] = _uart->get();

        if(_ip[count] == '\n' && _ip[count-1] == '\r')
            break;

        count++;
    }

    _ip[count-1] = '\0';

    return _ip;
}

void ESP8266::flush_serial(unsigned int timeout)
{
    unsigned int timer = 0;
    while(!_uart->ready_to_get()){
        Machine::delay(5000);
        timer++;
        if (timer >= timeout){
            out << "TIMEOUT FLUSH!" << endl;
            break; 
        }
    }    
    timer = 0;
    while(_uart->ready_to_get()) {
        timer = 0;
        db<ESP8266>(TRC) << _uart->get();
        while(!_uart->ready_to_get()){
            Machine::delay(5000);
            timer++;
            if (timer >= timeout)
                out << "TIMEOUT FLUSH!" << endl;
                break; 
        }    
    }

}

int ESP8266::send_no_wait(const char *command, unsigned int size)
{
    for(unsigned int i = 0; i < size; i++)
        _uart->put(command[i]);

    return 1;
}

bool ESP8266::send_data(const char * data, unsigned int size, unsigned int attempts, unsigned int timeout)
{
    db<ESP8266>(TRC) << "Sending data, size is " << size << " bytes." << endl;

    bool response = false;

    while(!response && attempts > 0) {

        for(unsigned int i = 0; i < size; i++) {
            _uart->put(data[i]);
        }

        _last_send = TSC::time_stamp();

        response = wait_response("OK\r\n", 4, timeout);
        out << "TENTATIVA: " << attempts << endl;
        attempts--;
        Machine::delay(200000);
        command_mode();
    }

    return response;
}

bool ESP8266::wait_response(const char * expected, unsigned int response_size, unsigned int timeout)
{
    unsigned int timer = 0;

    while(!_uart->ready_to_get()){
        Machine::delay(5000);
        timer++;
        if (timer >= timeout){
            out << "TIMEOUT WAIT RESPONSE!" << endl;
            return false;           
        }
    }
    
    unsigned int position = 0;
    char response[response_size];
    out << "Lendo resposta: " << endl;
    while(_uart->ready_to_get() && response_size > 0) {
        response[position] = _uart->get();
        out << response[position];
        position++;
        response_size--;
        timer = 0;
        Machine::delay(100000);
    }
    out << endl;
    return (strncmp(expected, response, position) == 0)?true:false;
}

__END_SYS
