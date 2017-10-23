#include "gs_control.h"

#define ROTORS_BYPASS

GsControl::GsControl(double gs_lat, double gs_lon, double gs_h, int timestep, int max_propagations)
{
    _rot = new RotorControl((char *) "192.168.0.204", 8888);

    _doppler_port_ul = 52003;
    _doppler_port_dl = 52002;

    _server_conf.server.port = 55001;
    //server_socket_init(&_server_conf, &_server_fd);

    _connected_user = false;
    _error_in_op = false;

    _rotors_enabled = true;

    _gs_lat = gs_lat;
    _gs_lon = gs_lon;
    _gs_h   = gs_h;
    _sim_timestep = timestep;
    _max_propagations = max_propagations;
}

void GsControl::addNewSatellite(double min_el, double dl_f, double ul_f, char *tle)
{
    /*  check if the satellite already exists in any propagation module enabled...
     *  get all objects from orbit sim vector
     */
    OrbitSimulator orb = initializeOrbitObject( min_el, dl_f, ul_f, _gs_lat, _gs_lon, _gs_h,
                                                tle, _sim_timestep, _max_propagations);
    if (propagationVector.size() == 0) {
        if (orb.getMyNoradID() != 0) {
            std::cout << "Norad object " << orb.getMyNoradID() << " created" << std::endl;
            propagationVector.push_back(orb);
        }
    }else {
        int sat_count = 0;
        for (std::vector<OrbitSimulator>::iterator it = propagationVector.begin(); it != propagationVector.end(); it++) {
            /* get all OrbitSimulator from propagationVector, compare its NORAD ID */
            if (orb.getMyNoradID() != it->getMyNoradID() && orb.getMyNoradID() != 0) {
                sat_count++;
            }
        }
        if (sat_count == propagationVector.size()) {
            std::cout << "Norad object " << orb.getMyNoradID() << " created" << std::endl;
            propagationVector.push_back(orb);
        }
    }
}

GsControl::NotificationMap GsControl::getMap()
{
    /* map contains NORAD ID and start-stop times */
    NotificationMap map;
    SatPassTupleVector vec;
    int norad_id;
    for (std::vector<OrbitSimulator>::iterator it = propagationVector.begin(); it != propagationVector.end(); it++) {
        /* it-> is a OrbitSimulator class instance */
        vec.clear();
        norad_id = it->getMyNoradID();
        for (std::vector<PassInformationVec>::iterator it2 = it->passes.begin(); it2 != it->passes.end(); it2++) {
            /* it2-> is a iterator of different PassInformationVec containing a vector of PassInformation */
            SatPassTuple tup = std::make_tuple(it2->front().propagation.timestamp, it2->back().propagation.timestamp);
            vec.push_back(tup);
        }
        map[norad_id] = vec;
    }
    return map;
}

void GsControl::propagate()
{
    for (std::vector<OrbitSimulator>::iterator it = propagationVector.begin(); it != propagationVector.end(); it++) {
        it->propagate();
    }
}

void GsControl::get_config(std::string path)
{
#if 0
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
    }else {
        std::cout << "File path is incorrect" << std::endl;
    }
#endif
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
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", &tm);
    return (std::string(date));
}

#if 0
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
#endif

void GsControl::sendDoppler(float freq_dl, float freq_ul)
{
    int sock = -1;
    struct sockaddr_in myaddr;
    struct hostent *he;
    uint8_t datagram[128];
    int     datagramLength;
    /* información sobre la dirección del servidor */
    if ((he=gethostbyname("localhost")) != NULL) {
        if ((sock=socket(AF_INET, SOCK_DGRAM, 0))>0) {
            memset(&myaddr, 0, sizeof(myaddr));
            myaddr.sin_family=AF_INET;
            myaddr.sin_addr = *((struct in_addr *)he->h_addr);
            myaddr.sin_port=htons(_doppler_port_ul);
            sprintf((char *) datagram, "F%u\n", (unsigned int) freq_ul);
            datagramLength=strlen((char *) datagram);
            sendto(sock, datagram, strlen((char *) datagram), 0, (struct sockaddr *)&myaddr, sizeof(myaddr));
            close(sock);
        }
    }
    if ((he=gethostbyname("localhost")) != NULL) {
        if ((sock=socket(AF_INET, SOCK_DGRAM, 0))>0) {
            memset(&myaddr, 0, sizeof(myaddr));
            myaddr.sin_family=AF_INET;
            myaddr.sin_addr = *((struct in_addr *)he->h_addr);
            myaddr.sin_port=htons(_doppler_port_dl);
            sprintf((char *) datagram, "F%u\n", (unsigned int)freq_dl);
            datagramLength=strlen((char *) datagram);
            sendto(sock, datagram, strlen((char *) datagram), 0, (struct sockaddr *)&myaddr, sizeof(myaddr));
            close(sock);
        }
    }
}

OrbitSimulator GsControl::initializeOrbitObject(    double min_el,
                                                    double dl_f, double ul_f,
                                                    double gs_lat, double gs_lon, double gs_h,
                                                    char *tle, int sim_timestep, int max_propagations)
{
    /* start orbit object */
    OrbitSimulator orb;
    orb.setMinimumElevation(min_el);
    orb.setCommsFreq(dl_f, ul_f);
    orb.setGroundLocation(gs_lat, gs_lon, gs_h);
    orb.setSpaceTLEFile(tle);
    orb.setTimestep(sim_timestep);
    orb.setMaxPropagations(max_propagations);
    return orb;
}

#if 0
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
                #else
                _rot->setRotorPosition(_passes.front().front().motor_az, _passes.front().front().motor_el);
                #endif
                sendDoppler(op_conf_s._dl_freq+_passes.front().front().propagation.dl_doppler,
                            op_conf_s._ul_freq+_passes.front().front().propagation.ul_doppler);
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
                        #else
                        _rot->setRotorPosition(it->motor_az, it->motor_el);
                        #endif
                        sendDoppler((op_conf_s._dl_freq+it->propagation.dl_doppler),
                                    (op_conf_s._ul_freq+it->propagation.ul_doppler));
                    }
                }
            }
        }
    }
}
#endif

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
                    //loadParms();
                    /* TODO: clearPasses before re-excuting OB */
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
