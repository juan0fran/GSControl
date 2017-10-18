#include "gs_control.h"

#define ROTORS_BYPASS

GsControl::GsControl()
{
    _orb = new OrbitSimulator;
    _rot = new RotorControl((char *) "192.168.0.204", 8888);
    _server_conf.server.port = 55001;
    server_socket_init(&_server_conf, &_server_fd);

    _connected_user = false;
    _error_in_op = false;

    _rotors_enabled = true;

    op_conf_s._min_el = 5.0;
    op_conf_s._timestep = 5;
    op_conf_s._max_propagations = 1;
    op_conf_s._pre_propagation_offset_seconds = 0;
}

void GsControl::get_config(std::string path)
{
    YAML::Node node;
    std::ifstream file(path);
    if (file.is_open()) {
        std::string file_str = std::string( (std::istreambuf_iterator<char>(file)),
                                            (std::istreambuf_iterator<char>()));
        file.close();
        /* now just load it! */
        node = YAML::Load(file_str.c_str());
        if (node[UL_FREQ_KEY]) {
            std::cout << "Old UL freq: " << op_conf_s._ul_freq << std::endl;
            op_conf_s._ul_freq = node[UL_FREQ_KEY].as<double>();
            std::cout << "New UL freq: " << op_conf_s._ul_freq << std::endl;
        }
        if (node[DL_FREQ_KEY]) {
            std::cout << "Old DL freq: " << op_conf_s._dl_freq << std::endl;
            op_conf_s._dl_freq = node[DL_FREQ_KEY].as<double>();
            std::cout << "New DL freq: " << op_conf_s._dl_freq << std::endl;
        }
        if (node[TLE_KEY]) {
            std::cout << "Old TLE: " << std::endl << op_conf_s._tle_string << std::endl;
            strcpy(op_conf_s._tle_string, node["tle"].as<std::string>().c_str());
            std::cout << "New TLE: " << std::endl << op_conf_s._tle_string << std::endl;
        }
        if (node[GS_KEY]) {
            std::cout << "Old GS: " << op_conf_s._gs.lat << " "
                                    << op_conf_s._gs.lon << " "
                                    << op_conf_s._gs.h << std::endl;

            /* auto-indent fix */
            if (node[GS_KEY].IsSequence() && node[GS_KEY].size() == 3) {
                op_conf_s._gs.lat = node[GS_KEY][0].as<double>();
                op_conf_s._gs.lon = node[GS_KEY][1].as<double>();
                op_conf_s._gs.h = node[GS_KEY][2].as<double>();

                std::cout << "New GS: " << op_conf_s._gs.lat << " "
                                        << op_conf_s._gs.lon << " "
                                        << op_conf_s._gs.h << std::endl;
            }
        }
        if (node[MIN_EL_KEY]) {
            std::cout << "Old min elevation: " << op_conf_s._min_el << std::endl;
            op_conf_s._min_el = node[MIN_EL_KEY].as<double>();
            std::cout << "New min elevation: " << op_conf_s._min_el << std::endl;
        }
        if (node[PRE_PROP_KEY]) {
            std::cout << "Old pre-prop offset elevation: " << op_conf_s._pre_propagation_offset_seconds << std::endl;
            op_conf_s._pre_propagation_offset_seconds = node[PRE_PROP_KEY].as<int>();
            std::cout << "New pre-prop offset elevation: " << op_conf_s._pre_propagation_offset_seconds << std::endl;
        }
        if (node[TIMESTEP_KEY]) {
            std::cout << "Old timestep: " << op_conf_s._timestep << std::endl;
            op_conf_s._timestep = node[TIMESTEP_KEY].as<int>();
            std::cout << "New timestep: " << op_conf_s._timestep << std::endl;
        }
        if (node[PROP_AMOUNT]) {
            std::cout << "Old max. propagations: " << op_conf_s._max_propagations << std::endl;
            op_conf_s._max_propagations = node[PROP_AMOUNT].as<int>();
            std::cout << "New max. propagations: " << op_conf_s._max_propagations << std::endl;
        }
    }
}

void GsControl::setTimePointingOffset(time_t offset)
{
    op_conf_s._pre_propagation_offset_seconds = offset;
}

void GsControl::setMaxPropagations(int max)
{
    op_conf_s._max_propagations = max;
}

void GsControl::setMinElevation(float el)
{
    op_conf_s._min_el = el;
}

void GsControl::setFrequencies(float dl, float ul)
{
    op_conf_s._dl_freq = dl;
    op_conf_s._ul_freq = ul;
}

void GsControl::setTLE(std::string path)
{
    strncpy(op_conf_s._tle_string, path.c_str(), path.length());
}

void GsControl::setTLE(char *path)
{
    strcpy(op_conf_s._tle_string, path);
}

void GsControl::setTimestep(int times)
{
    op_conf_s._timestep = times;
}

void GsControl::setLocation(float lat_gs, float lon_gs, float h_gs)
{
    op_conf_s._gs.lat = lat_gs;
    op_conf_s._gs.lon = lon_gs;
    op_conf_s._gs.h   = h_gs;
}

void GsControl::loadParms()
{
    initializeOrbitObject(_orb);
}

time_t GsControl::fakeGetActualTime(long long offset)
{
    return ((std::chrono::system_clock::to_time_t (std::chrono::system_clock::now())) + offset);
}

time_t GsControl::getActualTime()
{
    return (std::chrono::system_clock::to_time_t (std::chrono::system_clock::now()));
}

std::string GsControl::printReadble(time_t t)
{
    struct tm tm;
    char date[20];
    localtime_r(&t, &tm);
    strftime(date, sizeof(date), "%Y-%m-%d", &tm);
    return (std::string(date));
}

void GsControl::clearPasses()
{
    _passes.clear();
}

void GsControl::addNextPassFrom(TIMESTAMP_T t)
{
    /* i have to take a look if the pass exists before "adding" it */
    if (_orb->findNextPass(t) == -1) {
        _error_in_op = true;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        /* sleep this to avoid processor overload */
    }else {
        /* the pass exists? */
        computeMotorAngle(_orb->Results);
        if (!doesPassExist()) {
            _passes.push_back(_pass);
        }
        //printPass(_passes.size());
    }
}

void GsControl::addNextPassFromLast()
{
    if (_passes.size() > 0) {
        if ((size_t)_passes.size() < (size_t)op_conf_s._max_propagations) {
            /* just start the simulation i.e. 30 minutes after */
            addNextPassFrom((_passes.back().back().propagation.timestamp) + (30*60));
        }
    }else {
        addNextPassFrom(getActualTime());
    }
}

bool GsControl::isPassesFull()
{
    return ((size_t)op_conf_s._max_propagations == (size_t)_passes.size());
}

bool GsControl::doesPassExist()
{
    /*  */
    for (PassesVec::iterator it = _passes.begin(); it != _passes.end(); it++) {
        if ((_pass.front() <= it->back())) {
            /* in case that the calculated pass is, in time, lower any calculated, just erase */
            return true;
        }
    }
    return false;
}

void GsControl::checkPasses()
{
    /* checks if the pass is on the "past" */
    for (PassesVec::iterator it = _passes.begin(); it != _passes.end(); ) {
        if ((time_t) it->back().propagation.timestamp < (time_t) getActualTime()) {
            /* remove myself */
            it = _passes.erase(it);
        }else {
            ++it;
        }
    }
}

void GsControl::printPass(int pass_number)
{
    if ((size_t)pass_number > (size_t)_passes.size()) {
        std::cout << "Bad pass selected: " << pass_number << " from " << _passes.size() << std::endl;
    }
    for (PassInformationVec::iterator it = _passes[pass_number-1].begin(); it != _passes[pass_number-1].end(); it++) {
        std::cout << it->propagation.timestamp << "; ";
        std::cout << it->propagation.az << "; ";
        std::cout << it->propagation.el << "; ";
        std::cout << it->motor_az << "; ";
        std::cout << it->motor_el << "; ";
        std::cout << it->propagation.ul_doppler << "; ";
        std::cout << it->propagation.dl_doppler << std::endl;
    }
}

void GsControl::storeInDB()
{
    #if 0
    PGresult   *res;
    char call[2048];
    /* Check to see that the backend connection was successfully made */
    _pgconn = PQsetdbLogin("localhost", "5432", NULL, NULL, "juan0fran", "juan0fran", "root");
    /* Check to see that the backend connection was successfully made */
    if (PQstatus(_pgconn) != CONNECTION_OK) {
        std::cout << "Connection to database failed: " << PQerrorMessage(_pgconn) << std::endl;
        PQfinish(_pgconn);
        _exit(1);
    }

    /* Check if exists a pass between start and end timestamp --> get the pass id and remove it */

    for (!!!!!::iterator it = _passes.front().begin(); it != _passes.front().end(); it++) {
        memset(call, 0, sizeof(call));
        sprintf(call,   "INSERT INTO _example_table\r\n"
                        "VALUES (to_timestamp(%d), %f, %f, %f, %f, %f, %f, to_timestamp(%d))\r\n"
                        "ON CONFLICT (timestamp)\r\n"
                        "DO NOTHING",
                            (int) it->propagation.timestamp,
                            it->propagation.az,
                            it->propagation.el,
                            it->motor_el,
                            it->motor_el,
                            it->propagation.dl_doppler,
                            it->propagation.ul_doppler,
                            (int) _passes.front().front().propagation.timestamp);
        res = PQexec(_pgconn, call);
        if (PQresultStatus(res) != PGRES_COMMAND_OK) {
            std::cout << "INSERT failed: " << PQerrorMessage(_pgconn) << std::endl;
            PQclear(res);
            PQfinish(_pgconn);
            _exit(1);
        }
        PQclear(res);
    }
    PQfinish(_pgconn);
    std::cout << "Finished copying to DB" << std::endl;
    #endif
}

void GsControl::initializeOrbitObject(OrbitSimulator *orb)
{
    /* start orbit object */
    orb->SetMinimumElevation(op_conf_s._min_el);
    orb->SetCommsFreq(op_conf_s._dl_freq, op_conf_s._ul_freq);
    orb->SetGroundLocation(op_conf_s._gs.lat, op_conf_s._gs.lon, op_conf_s._gs.h);
    orb->SetSpaceTLEFile(op_conf_s._tle_string);
    orb->SetTimestep(op_conf_s._timestep);
}

void GsControl::runSatelliteTracking()
{
    /* given the pass structure, point the antennas to the first poisition */
    /* we could make a move 1 of every X timestamps */
    time_t timer;
    if (!_error_in_op) {
        if (_passes.size() != 0) {
            timer = getActualTime();
            if (timer < (time_t) _passes.front().front().propagation.timestamp) {
                std::cout   << "Still " << ((_passes.front().front().propagation.timestamp - timer)/60)
                            << " minutes to move the antennas" << std::endl;
                 /* if more than 120 secs. for the satellite pass... */
                if ((_passes.front().front().propagation.timestamp - timer) > SATELLITE_MIN_WAIT_TIME) {
                    /* and outside here we will have to sleep for a while ... */
                    return;
                }
            }
            if (_rotors_enabled) {
                #ifdef ROTORS_BYPASS
                std::cout << "Fake move rotors to: " << _passes.front().front().motor_az;
                std::cout << ", " << _passes.front().front().motor_el << std::endl;
                std::cout   << "Fake Doppler correction DL: "
                            << op_conf_s._dl_freq+_passes.front().front().propagation.dl_doppler
                            << " UL: " << op_conf_s._ul_freq+_passes.front().front().propagation.ul_doppler << std::endl;
                #else
                _rot->setRotorPosition(_passes.front().front().motor_az, _passes.front().front().motor_el);
                #endif
            }
            for (PassInformationVec::iterator it = (_passes.front().begin()+1); it != _passes.front().end(); it++) {
                /* we want to be there in advande, 1 second in advance... */
                while( (getActualTime()+op_conf_s._pre_propagation_offset_seconds) < (time_t) (it->propagation.timestamp)) {
                    /* std::cout << ((it->propagation.timestamp-1) - getActualTime())
                                 << " secs. to move the antennas..." << std::endl; */
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    if (handleCommand() == RELOAD_GS) {
                        /* sudamendi */
                        return;
                    }
                }
                /* if it is a past event, do not send a rotor move... */
                if (std::fabs((int)((getActualTime()+op_conf_s._pre_propagation_offset_seconds) - it->propagation.timestamp))
                        < op_conf_s._timestep) {
                    if (_rotors_enabled) {
                        #ifdef ROTORS_BYPASS
                        std::cout << "Fake move rotors to: " << it->motor_az;
                        std::cout << ", " << it->motor_el << std::endl;
                        std::cout   << "Fake Doppler correction DL: "
                                    << (op_conf_s._dl_freq+it->propagation.dl_doppler)/1e6
                                    << " MHz; UL: " << (op_conf_s._ul_freq+it->propagation.ul_doppler)/1e6
                                    << " MHz" << std::endl;
                        #else
                        _rot->setRotorPosition(it->motor_az, it->motor_el);
                        #endif
                    }
                }
            }
        }
    }
}

int GsControl::handleCommand(int timeout)
{
    disable_enable_rotor_cmd    *cmd_enable_rot;
    set_manual_position_cmd     *cmd_manual;
    change_transceiver_cmd      *cmd_trx;
    change_polarization_cmd     *cmd_pol;
    change_op_parameters_cmd    *cmd_op;
    su_errno_e err;
    /* check if there is a connected user */
    if (!_connected_user) {
        /* try to get a user */
        _client.timeout_ms = timeout*1000; /* just poll if there is data */
        if (server_socket_new_client(&_server_fd, &_client) == SU_NO_ERROR) {
            _connected_user = true;
        }
    }
    /* poll it again */
    if (_connected_user) {
        _client.timeout_ms = timeout*1000; /* just poll if there is data */
        memset(_client.buffer, 0, sizeof(_client.buffer));
        err = socket_read(&_client);
        if (err == SU_NO_ERROR && _client.len > 0) {
            std::cout << "Command received: " << (int) _client.buffer[0] << std::endl;
            switch(_client.buffer[0]) {
                case CMD_ID_DISABLE_ENABLE_ROTOR:
                    cmd_enable_rot = (disable_enable_rotor_cmd *) _client.buffer;
                    if (cmd_enable_rot->enable_flag) {
                        std::cout << "Rotors Enabled" << std::endl;
                        _rotors_enabled = true;
                    }
                    else {
                        std::cout << "Rotors Disabled" << std::endl;
                        _rotors_enabled = false;
                    }
                    break;
                case CMD_ID_SET_MANUAL_POSITION:
                    cmd_manual = (set_manual_position_cmd *) _client.buffer;
                    _rotors_enabled = false;
                    #ifdef ROTORS_BYPASS
                    std::cout << "Fake move rotors to: " << cmd_manual->az << ", " << cmd_manual->el << std::endl;
                    #else
                    _rot->setRotorPosition(cmd_manual->az, cmd_manual->el);
                    #endif
                    break;
                case CMD_ID_CHNG_TRX:

                    break;
                case CMD_ID_CHNG_POL:

                    break;
                case CMD_ID_CHANGE_OP_PARMS:
                    cmd_op = (change_op_parameters_cmd *) _client.buffer;
                    get_config(cmd_op->filepath);
                    loadParms();
                    _passes.clear();
                    return RELOAD_GS;
                    break;
                default:
                    std::cout << "Bad command input" << std::endl;
                    break;
            }
        }else if (err == SU_IO_ERROR || _client.len == 0) {
            close(_client.fd);
            _connected_user = false;
        }
    }
    return NON_OP;
}

int GsControl::handleCommand()
{
    /* 0 means no timeout --> just see if there is something there */
    return (handleCommand(0));
}
