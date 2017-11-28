// EPOS Cortex ESP8266 Wi-Fi Board Mediator Declarations

#include <system/config.h>
#ifndef __esp8266_h
#define __esp8266_h

#include <machine/cortex/uart.h>
#include <machine/cortex/gpio.h>
#include <utility/string.h>

__BEGIN_SYS

class ESP8266
{
    static const unsigned int SSID_MAX = 64;
    static const unsigned int PASS_MAX = 64;
    static const unsigned int HOST_MAX = 64;
    static const unsigned int ROUTE_MAX = 64;

public:
    ESP8266(UART *uart, GPIO *power) : _rstkey(power), _uart(uart), _connected(false) { }

    ESP8266(UART *uart, GPIO *power, int port, const char *host, unsigned int host_size, const char *route, unsigned int route_size) : _rstkey(power), _uart(uart), _connected(false) {
        config_endpoint(port, host, host_size, route, route_size);
    }

    ~ESP8266() {}

    void on() { _rstkey->set(1); }
    void off() { _rstkey->set(0); }

    RTC::Microsecond now();

    int send(const char * data, unsigned int size) {
        return send_data(data, size);
    }

    void reset();

    void connect(const char *ssid, int ssid_size, const char *pass, int pass_size);

    void config_endpoint(int port, const char *host, unsigned int host_size, const char *route, unsigned int route_size);

    bool command_mode();

    int post(const void * data, int data_size, char * res);

    int get(char * res, char * sub_route = 0, int sub_route_size = 0);

    bool connected();

    char * ssid() { return _ssid; }
    char * pass() { return _password; }
    char * host() { return _host; }
    char * route() { return _route; }
    int port(){ return _port; }
    char * ip();

public:
    void flush_serial(unsigned timeout = 20);

    int send_no_wait(const char *command, unsigned int size);

    bool send_data(const char * data, unsigned int size, unsigned int attempts = 1, unsigned timeout = 20);

    bool wait_response(const char * expected, unsigned int response_size = 0, unsigned timeout = 20);

    GPIO * _rstkey;
    UART * _uart;
    TSC::Time_Stamp _last_send;
    TSC::Time_Stamp _init_timeout;

    char _ssid[SSID_MAX];
    char _password[PASS_MAX];
    char _host[HOST_MAX];
    char _route[ROUTE_MAX];
    char _ip[16];
    int _port;

    bool _connected;
};

__END_SYS

#endif
