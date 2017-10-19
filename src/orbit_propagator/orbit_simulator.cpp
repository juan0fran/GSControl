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

void OrbitSimulator::SetMinimumElevation(double elev)
{
    _min_elev = elev;
}

void OrbitSimulator::SetCommsFreq(double freq_dl, double freq_ul)
{
    _orbit_conf.in_dl_freq = freq_dl;
    _orbit_conf.in_ul_freq = freq_ul;
    Link::setFrequency(freq_dl);
}

void OrbitSimulator::SetGroundLocation(double lat, double lon, double height)
{
    _orbit_conf.station.llh.f.x = lat;
    _orbit_conf.station.llh.f.y = lon;
    _orbit_conf.station.llh.f.z = height;
}

void OrbitSimulator::SetSpaceTLEFile(const char *tle_file)
{
    strcpy(_orbit_conf.platform.tle, tle_file);
}

void OrbitSimulator::SetTimestep(unsigned long timestep)
{
    _sim_timestep = timestep;
}

std::string OrbitSimulator::GetResults()
{
    std::stringstream str;
    str.imbue(std::locale(""));
    for (SimulatorResultsVec::iterator it = Results.begin(); it != Results.end(); it++) {
        if (it->propagation.el >= _min_elev) {
            str << it->propagation.timestamp << ";";
            str << it->propagation.az << ";";
            str << it->propagation.el << ";";
            str << it->propagation.ul_doppler << ";";
            str << it->propagation.dl_doppler << std::flush << std::endl;
            //str << it->propagation.rel_dist <<  ";";
            //str << it->link.snr << ";";
            //str << it->link.ber << std::flush << std::endl;
            //std::cout << it->link.per << "; ";
            //std::cout << it->link.rfer <<  "; ";
            //std::cout << it->link.bfer << std::endl;
        }
    }
    return str.str();
}

unsigned long OrbitSimulator::getLastPassDuration()
{
    return (getLastFoundPassEnd() - getLastFoundPassStart());
}

unsigned long OrbitSimulator::getLastFoundPassStart()
{
    return _last_pass_start;
}

unsigned long OrbitSimulator::getLastFoundPassEnd()
{
    return _last_pass_end;
}

int OrbitSimulator::findNextPass(unsigned long start_timestamp)
{
    bool passFound = false;
    bool passfineFound = false;
    SimulatorResults single_result;
    memset(&single_result, 0, sizeof(single_result));
    Results.clear();
    unsigned long t = start_timestamp;
    while (t < (start_timestamp + 86400)) {
        _orbit_conf.station.timestamp   = t;
        _orbit_conf.platform.timestamp  = t;
        if (propagate_and_get_visibility(&_orbit_conf, &single_result.propagation) == -1 ) {
            std::cout << "Error in simulation" << std::endl;
            Results.clear();
            return -1;
        }
        if (single_result.propagation.el < _min_elev) {
            if (passFound == true) {
                _last_pass_end = t;
                if ((_last_pass_end - _last_pass_start) < 1) {
                    Results.clear();
                    passFound = false;
                    passfineFound = false;
                    continue;
                }
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
                //setDistance(single_result.propagation.rel_dist);
                //fillStructure(&single_result.link);
                Results.push_back(single_result);
                t = t + _sim_timestep;
            }
        }
    }
    return 0;
}

unsigned long OrbitSimulator::getPassAvailability()
{
    int count_good = 0, count_bad = 0;
    for (SimulatorResultsVec::iterator it = Results.begin(); it != Results.end(); it++) {
        if (it->link.ber > 1e-5) {
            count_bad++;
        }else {
            count_good++;
        }
    }
    return (count_good*100)/(count_good+count_bad);
}
