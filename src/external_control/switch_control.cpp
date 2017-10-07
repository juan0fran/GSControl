#include "switch_control.h"
#include <cstdio>
#include <string>
#include <string.h>

void SwitchControl::init_socket(SwitchControl *sc)
{
    sc->_socket_fd = 0;
    memset(&sc->_server, 0, sizeof(sc->_server));
    sc->_server.sin_family = AF_INET;
    if ((sc->_socket_fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("Socket failed: ");
        _exit(1);
    }
    memset(&_set_data, 0, sizeof(_set_data));
    memset(&_get_data, 0, sizeof(_get_data));
    memset(&_lcd_data, 0, sizeof(_lcd_data));
}

/* Public initializers */
SwitchControl::SwitchControl()
{
    init_socket(this);
}

SwitchControl::SwitchControl(char *ip, int port)
{
    init_socket(this);
    setIP_Port(ip, port);
}

void SwitchControl::setIP_Port(char *ip, int port)
{
    setIP(ip);
    setPort(port);
}

void SwitchControl::setIP(char *ip)
{
    if (inet_aton(ip, &_server.sin_addr) == 0) {
        perror("Bad IP address: ");
        _exit(1);
    }
}

void SwitchControl::setPort(int port)
{
    _server.sin_port = htons(port);
}

/* Private functions */
int SwitchControl::blockingRead(int timeout_ms)
{
    // timeout structure passed into select
    struct timeval tv;
    // fd_set passed into select
    fd_set fds;
    int control_ret;

    if (timeout_ms >= 1000) {
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
    }else{
        tv.tv_sec = 0;
        tv.tv_usec = timeout_ms * 1000;
    }
    // Zero out the fd_set - make sure it's pristine
    FD_ZERO(&fds);
    // Set the FD that we want to read
    FD_SET(_socket_fd, &fds);
    // select takes the last file descriptor value + 1 in the fdset to check,
    // the fdset for reads, writes, and errors.  We are only passing in reads.
    // the last parameter is the timeout.  select will return if an FD is ready or
    // the timeout has occurred
    if ( (control_ret = select(_socket_fd+1, &fds, NULL, NULL, &tv) ) == -1) {
        return -1;
    }
    // return 0 if fd is not ready to be read.
    return (FD_ISSET(_socket_fd, &fds));
}

int SwitchControl::sendAndReceive(void *p, size_t len)
{
    int ret;
    int slen = sizeof(_server);
    ret = sendto(_socket_fd, p, len, 0, (struct sockaddr*) &_server, slen);
    if (ret == -1) {
        perror("Socket error on writing: ");
        return 0;
    }
    if (blockingRead(500) > 0) {
        ret = recvfrom(_socket_fd, p, len, 0, (struct sockaddr *) &_server, (socklen_t *) &slen);
        return ret;
    }else {
        printf("Timedout\n");
        return 0;
    }
}

void SwitchControl::setFan(GSSW_OUT state)
{
    _set_data.set.fan = state;
}

void SwitchControl::setOutput(uint8_t output, GSSW_OUT state)
{
    if (output <= 0 || output > 8) {
        return;
    }
    _set_data.set.channel[output-1] = state;
}

ERRNO_CODE SwitchControl::set()
{
    _set_data.mode = GSSW_MODE_SET;
    if (sendAndReceive(&_set_data, sizeof(_set_data)) > 0) {
        return NO_ERROR;
    }else {
        return ERROR;
    }
}

ERRNO_CODE SwitchControl::get()
{
    int ret;
    memset(&_get_data, 0, sizeof(_get_data));
    _get_data.mode = GSSW_MODE_GET;
    ret = sendAndReceive(&_get_data, sizeof(_get_data));
    if (ret == sizeof(_get_data)) {
        return NO_ERROR;
    }else {
        return ERROR;
    }
}

/* Public functions */
void SwitchControl::enableOutput(uint8_t output)
{
    setOutput(output, GSSW_OUT_ENABLED);
}

void SwitchControl::disableOutput(uint8_t output)
{
    setOutput(output, GSSW_OUT_DISABLED);
}

void SwitchControl::enableFan()
{
    setFan(GSSW_OUT_ENABLED);
}

void SwitchControl::disableFan()
{
    setFan(GSSW_OUT_DISABLED);
}

void SwitchControl::executeQuery()
{
    set();
}

GenericData SwitchControl::getStatus()
{
    GenericData data;
    get();
    memcpy(&data, &_get_data, sizeof(GenericData));
    return data;
}

void SwitchControl::setLCD(const char *line1, const char *line2)
{
    _lcd_data.mode = GSSW_MODE_LCD;
    strncpy(_lcd_data.set.line1, line1, 16);
    strncpy(_lcd_data.set.line2, line2, 16);
    sendAndReceive(&_lcd_data, sizeof(_lcd_data));

}

void SwitchControl::printContents()
{
    printf("Manual Flag: %d Temperature: %f Analog Readings: %f, %f, %f\r\n",
               _get_data.get.manual,
               (float)_get_data.get.temp/1000.0,
               (float)_get_data.get.analog[0]/1000.0,
               (float)_get_data.get.analog[1]/1000.0,
               (float)_get_data.get.analog[2]/1000.0);

   printf("Output switch states: %d %d %d %d %d %d %d %d Fan: %d\n",
               _get_data.get.channel[0],
               _get_data.get.channel[1],
               _get_data.get.channel[2],
               _get_data.get.channel[3],
               _get_data.get.channel[4],
               _get_data.get.channel[5],
               _get_data.get.channel[6],
               _get_data.get.channel[7], _get_data.get.fan);
}

void SwitchControl::printContents(GenericData *data)
{
    printf("Manual Flag: %d Temperature: %f Analog Readings: %f, %f, %f\r\n",
               data->get.manual,
               (float)data->get.temp/1000.0,
               (float)data->get.analog[0]/1000.0,
               (float)data->get.analog[1]/1000.0,
               (float)data->get.analog[2]/1000.0);

   printf("Output switch states: %d %d %d %d %d %d %d %d Fan: %d\n",
               data->get.channel[0],
               data->get.channel[1],
               data->get.channel[2],
               data->get.channel[3],
               data->get.channel[4],
               data->get.channel[5],
               data->get.channel[6],
               data->get.channel[7], data->get.fan);
}

int main (void)
{
    GenericData data;
    SwitchControl switch1((char *) "192.168.0.205", 8888);
    printf("Socket created\n");

    switch1.setLCD("FEO", "ARNAU");
    /*
    switch1.enableOutput(1);
    switch1.enableOutput(2);
    switch1.enableOutput(6);
    switch1.executeQuery();

    data = switch1.getStatus();
    switch1.printContents();

    sleep(1);

    switch1.disableOutput(1);
    switch1.disableOutput(2);
    switch1.disableOutput(6);
    switch1.executeQuery();

    data = switch1.getStatus();
    switch1.printContents();
    */
}
