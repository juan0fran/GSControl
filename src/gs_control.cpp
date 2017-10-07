#include "gs_control.h"

GsControl::GsControl()
{
    _orb = new OrbitSimulator;
    _gs_angles = new Motor_Angles;
    _rot = new RotorControl((char *) "192.168.0.204", 8888);
    _server_conf.server.port = 55001;
    server_socket_init(&_server_conf, &_server_fd);

    _connected_user = false;
    _error_in_op = false;

    _rotors_enabled = false;

    _min_el = 5.0;
    _timestep = 5;
}

void GsControl::setMinElevation(float el)
{
    _min_el = el;
}

void GsControl::setFrequencies(float dl, float ul)
{
    _dl_freq = dl;
    _ul_freq = ul;
}

void GsControl::setTLE(char *path)
{
    strcpy(_tle_path, path);
}

void GsControl::setTimestep(int times)
{
    _timestep = times;
}

void GsControl::setLocation(float lat_gs, float lon_gs, float h_gs)
{
    _gs.lat = lat_gs;
    _gs.lon = lon_gs;
    _gs.h   = h_gs;
}

void GsControl::loadParms()
{
    initializeOrbitObject(_orb, _min_el, _dl_freq, _ul_freq, _tle_path, _timestep, _gs.lat, _gs.lon, _gs.h);
}

time_t GsControl::getActualTime()
{
    return (std::chrono::system_clock::to_time_t (std::chrono::system_clock::now()));
}

void GsControl::addNextPassFrom(TIMESTAMP_T t)
{
    if (_orb->findNextPass(t) == -1) {
        _error_in_op = true;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        /* sleep this to avoid processor overload */
    }else {
        _gs_angles->computeMotorAngle(_orb->Results);
    }
}

void GsControl::printPass()
{
    for (NewAnglesVec::iterator it = _gs_angles->_Angles.begin(); it != _gs_angles->_Angles.end(); it++) {
        std::cout << it->propagation.timestamp << "; ";
        std::cout << it->propagation.az << "; ";
        std::cout << it->propagation.el << "; ";
        std::cout << it->motor_az << "; ";
        std::cout << it->motor_el << "; ";
        std::cout << it->propagation.ul_doppler << "; ";
        std::cout << it->propagation.dl_doppler << std::endl;
    }
}

void GsControl::initializeOrbitObject(OrbitSimulator *orb,
        float minimum_elevation, float dl_frequency, float ul_frequency,
        char *tle_path, int timestep, float lat_gs, float lon_gs, float h_gs)
{
    /* start orbit object */
    orb->SetMinimumElevation(minimum_elevation);
    orb->SetCommsFreq(dl_frequency, ul_frequency);
    orb->SetGroundLocation(lat_gs, lon_gs, h_gs);
    orb->SetSpaceTLEFile(tle_path);
    orb->SetTimestep(timestep);
}

void GsControl::runSatelliteTracking()
{
    /* given the pass structure, point the antennas to the first poisition */
    if (!_error_in_op) {
        std::cout << "Moving to initial position -->";
        std::cout << " AZ: " << _gs_angles->_Angles.front().propagation.az;
        std::cout << " EL: " << _gs_angles->_Angles.front().propagation.el;
        std::cout << " Mot_AZ: " << _gs_angles->_Angles.front().motor_az;
        std::cout << " Mot_EL: " << _gs_angles->_Angles.front().motor_el << std::endl;
        if (_rotors_enabled) {
            _rot->setRotorPosition(_gs_angles->_Angles.front().motor_az, _gs_angles->_Angles.front().motor_el);
        }
        for (NewAnglesVec::iterator it = (_gs_angles->_Angles.begin()+1); it != _gs_angles->_Angles.end(); it++) {
            while(getActualTime() < it->propagation.timestamp) {
                //std::cout << (it->propagation.timestamp - getActualTime()) << " secs. to move the antennas..." << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                if (handleCommand() == RELOAD_GS) {
                    /* sudamendi */
                    return;
                }
            }
            std::cout << "Moving the antennas to AZ: " << it->propagation.az << " EL: " << it->propagation.el;
            std::cout << " Mot_AZ: " << it->motor_az << " Mot_EL: " << it->motor_el << std::endl;
            if (_rotors_enabled) {
                _rot->setRotorPosition(it->motor_az, it->motor_el);
            }
        }
    }
}

int GsControl::handleCommand()
{
    disable_enable_rotor_cmd    *cmd_enable_rot;
    set_manual_position_cmd     *cmd_manual;
    change_transceiver_cmd      *cmd_trx;
    change_polarization_cmd     *cmd_pol;
    change_op_parameters_cmd    *cmd_op;
    char cmd[128];
    char aux[128];
    su_errno_e err;
    /* check if there is a connected user */
    if (!_connected_user) {
        /* try to get a user */
        if (server_socket_new_client(&_server_fd, &_client) == SU_NO_ERROR) {
            _connected_user = true;
        }
    }
    /* poll it again */
    if (_connected_user) {
        _client.timeout_ms = 0; /* just poll if there is data */
        memset(_client.buffer, 0, sizeof(_client.buffer));
        if ((err = socket_read(&_client)) == SU_NO_ERROR) {
            switch(_client.buffer[0]) {
                case CMD_ID_DISABLE_ENABLE_ROTOR:
                    cmd_enable_rot = (disable_enable_rotor_cmd *) _client.buffer;
                    if (cmd_enable_rot->enable_flag)
                        std::cout << "Rotors Enabled" << std::endl;
                    else
                        std::cout << "Rotors Disabled" << std::endl;lara
                break;
                case CMD_ID_SET_MANUAL_POSITION:PG IN THA HOUSE

                    break;
                case CMD_ID_CHNG_TRX:SPARTAN ROCKET

                    break;
                case CMD_ID_CHNG_POL:#THISISWHYWEPLAY

                    break;
                case CMD_ID_CHANGE_OP_PARMS:#TRESKACIO

                    break;
                default:
                    std::cout << "Bad command input" << std::endl;
                    break;
            }
            #if 0
            std::cout << "New command received: " << _client.buffer << std::endl;
            std::cout << "Falsing a set new TLE GS command" << std::endl;
            if (sscanf((char *)_client.buffer, "%[^:] %*[:] %[^\n]", cmd, aux) == 2) {
                if (strcmp(cmd, "tle") == 0) {
                    std::cout << "New TLE input: " << aux << std::endl;
                    setTLE((char *)aux);
                }
                loadParms();
                return RELOAD_GS;
            }
            #endif
        }else if (err == SU_IO_ERROR) {
            _connected_user = false;
        }
    }
    return NON_OP;
}
