#include "orbit_simulator.h"

OrbitSimulator::OrbitSimulator()
{
    /* we have TLE for spacecraft, but LLH for station */
    _orbit_conf.station.has_tle = false;
    _orbit_conf.platform.has_tle = true;
    _orbit_conf.platform.has_tle_file = false;
    _sim_timestep = 1;
    _min_elev = 0.0;
}

OrbitSimulator::~OrbitSimulator() {};

void OrbitSimulator::setMinimumElevation(double elev)
{
    _min_elev = elev;
}

void OrbitSimulator::setCommsFreq(double freq_dl, double freq_ul)
{
    _orbit_conf.in_dl_freq = freq_dl;
    _orbit_conf.in_ul_freq = freq_ul;
}

void OrbitSimulator::setGroundLocation(double lat, double lon, double height)
{
    _orbit_conf.station.llh.f.x = lat;
    _orbit_conf.station.llh.f.y = lon;
    _orbit_conf.station.llh.f.z = height;
}

void OrbitSimulator::setSpaceTLEFile(const char *tle_file)
{
    strcpy(_orbit_conf.platform.tle, tle_file);
    setNoradIDFromTLE((char *) tle_file);
}

void OrbitSimulator::setTimestep(unsigned long timestep)
{
    _sim_timestep = timestep;
}

void OrbitSimulator::setMaxPropagations(unsigned long prop)
{
    _max_propagations = prop;
}

void OrbitSimulator::setNoradIDFromTLE(char *tle)
{
    tle_set tleset;
    unsigned int satno;
    if (load_tle_from_stringd(tle, &tleset) == 0) {
        if (sscanf(tleset.lines._2, "2 %05d", &satno) == 1) {
            _norad_id = satno;
        }else {
            _norad_id = 0;
        }
    }else {
        _norad_id = 0;
    }
}

int OrbitSimulator::findNextPass(unsigned long start_timestamp)
{
    bool passFound = false;
    bool passfineFound = false;
    PassInformation single_result;
    PassInformationVec pass;
    memset(&single_result, 0, sizeof(single_result));

    unsigned long t = start_timestamp;
    while (t < (start_timestamp + 86400)) {
        _orbit_conf.station.timestamp   = t;
        _orbit_conf.platform.timestamp  = t;
        if (propagate_and_get_visibility(&_orbit_conf, &single_result.propagation) == -1 ) {
            std::cout << "Error in simulation" << std::endl;
            pass.clear();
            exit(1);
        }
        if (single_result.propagation.el < _min_elev) {
            if (passFound == true) {
                _last_pass_end = t;
                if ((_last_pass_end - _last_pass_start) < 1) {
                    pass.clear();
                    passFound = false;
                    passfineFound = false;
                    continue;
                }
                computeMotorAngle(pass);
                /* this stores in the class local variable info about the pass */
                return 0;
            }
            /* look in 60 seconds step in case of not pass found yet
               otherwise look it in passes of 1 second
            */
            if (passfineFound == true) {
                t = t + 1;
            }else {
                t = t + 60;
            }
        }else {
            if (passfineFound == false) {
                passfineFound = true;
                t = t - 60;
                continue;
            }else {
                if (passFound == false) {
                    _last_pass_start = t;
                    passFound = true;
                }
                pass.push_back(single_result);
                t = t + _sim_timestep;
            }
        }
    }
    return 0;
}

void OrbitSimulator::computeMotorAngle(PassInformationVec pass_info)
{
    PassInformation single_angle;
    _pass.clear();
    if( ((pass_info[0].propagation.az < pass_info[1].propagation.az) &&
         (pass_info[0].propagation.az > pass_info[pass_info.size()-1].propagation.az)) ||
        ((pass_info[0].propagation.az > pass_info[1].propagation.az) &&
         (pass_info[0].propagation.az < pass_info[pass_info.size()-1].propagation.az))) {

        for (PassInformationVec::iterator it = pass_info.begin(); it != pass_info.end(); it++) {
            if(it->propagation.az > 180.0f) {
                single_angle.motor_az = it->propagation.az - 180.0f;
            }else {
                single_angle.motor_az = it->propagation.az + 180.0f;
            }
            single_angle.propagation = it->propagation;
            single_angle.motor_el = 180.0f - it->propagation.el;
            _pass.push_back(single_angle);
        }
    }else{
        for (PassInformationVec::iterator it = pass_info.begin(); it != pass_info.end(); it++) {
            single_angle.propagation = it->propagation;
            single_angle.motor_az = it->propagation.az;
            single_angle.motor_el = it->propagation.el;
            _pass.push_back(single_angle);
        }
    }
}

bool OrbitSimulator::doesPassExist()
{
    /*  */
    for (std::vector<PassInformationVec>::iterator it = passes.begin(); it != passes.end(); it++) {
        if ((_pass.front() <= it->back())) {
            /* in case that the calculated pass is, in time, lower any calculated, just erase */
            return true;
        }
    }
    return false;
}

void OrbitSimulator::propagate()
{
    for (std::vector<PassInformationVec>::iterator it = passes.begin(); it != passes.end(); ) {
        if ((time_t) it->back().propagation.timestamp < (time_t) getActualTime()) {
            /* remove myself */
            it = passes.erase(it);
        }else {
            ++it;
        }
    }
    while ((size_t)passes.size() < (size_t)_max_propagations) {
        /* just start the simulation i.e. 30 minutes after */
        if (passes.size() == 0) {
            findNextPass(getActualTime());
            if (!doesPassExist()) {
                passes.push_back(_pass);
            }
        }else {
            findNextPass((passes.back().back().propagation.timestamp) + (30*60));
            if (!doesPassExist()) {
                passes.push_back(_pass);
            }
        }
    }
}
