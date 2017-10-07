#include "rotor_control.h"

void RotorControl::init_socket(RotorControl *sc)
{
    sc->_socket_fd = 0;
    memset(&sc->_server, 0, sizeof(sc->_server));
    sc->_server.sin_family = AF_INET;
    if ((sc->_socket_fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        _exit(1);
    }
    memset(&_data, 0, sizeof(_data));
    connected_motor = false;
}

/* Public initializers */
RotorControl::RotorControl()
{
    init_socket(this);
}

RotorControl::RotorControl(char *ip, int port)
{
    init_socket(this);
    setIP_Port(ip, port);
}

void RotorControl::setIP_Port(char *ip, int port)
{
    setIP(ip);
    setPort(port);
}

void RotorControl::setIP(char *ip)
{
    if (inet_aton(ip, &_server.sin_addr) == 0) {
        _exit(1);
    }
}

void RotorControl::setPort(int port)
{
    _server.sin_port = htons(port);
}

/* Private functions */
int RotorControl::blockingRead(int timeout_ms)
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

int RotorControl::sendAndReceive(void *p, size_t len, int ms_timeout)
{
    int ret;
    int slen = sizeof(_server);
    ret = sendto(_socket_fd, p, len, 0, (struct sockaddr*) &_server, slen);
    if (ret == -1) {
        return 0;
    }
    if (blockingRead(ms_timeout) > 0) {
        ret = recvfrom(_socket_fd, p, len, 0, (struct sockaddr *) &_server, (socklen_t *) &slen);
        return ret;
    }else {
        return 0;
    }
}

int RotorControl::sendAndReceive(void *p, size_t len)
{
    int ret;
    int slen = sizeof(_server);
    ret = sendto(_socket_fd, p, len, 0, (struct sockaddr*) &_server, slen);
    if (ret == -1) {
        return 0;
    }
    if (blockingRead(500) > 0) {
        ret = recvfrom(_socket_fd, p, len, 0, (struct sockaddr *) &_server, (socklen_t *) &slen);
        return ret;
    }else {
        return 0;
    }
}


void RotorControl::requestRotorStatus(RotorsData *data)
{
    data->mode = ROT_GET;
    if (sendAndReceive(data, sizeof(RotorsData), 250) > 0) {
        connected_motor = true;
    }else {
        connected_motor = false;
    }
}

void RotorControl::setRotorPosition(float az, float el)
{
    RotorsData set_data;
    RotorsData get_data;
    requestRotorStatus(&get_data);
    if (connected_motor == true) {
        /* calculate how much degrees it will take */
        set_data.mode = ROT_SET;
        set_data.az = az;
        set_data.el = el;
        /* send to other thread */
        if (sendAndReceive(&set_data, sizeof(set_data), 250) > 0) {
            connected_motor = true;
        }else {
            connected_motor = false;
        }
    }
}
