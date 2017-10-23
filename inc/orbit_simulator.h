#ifndef __ORBIT_SIMULATOR_HPP__
#define __ORBIT_SIMULATOR_HPP__

#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <locale>
#include <thread>
#include <string>
#include <vector>

#include "payload_tracker_api.h"

struct PassInformation {
    propagation_output_t propagation;
    double motor_az;
    double motor_el;
    bool operator==(const PassInformation& pass) const { return (pass.propagation.timestamp == propagation.timestamp); }
    bool operator!=(const PassInformation& pass) const { return (pass.propagation.timestamp != propagation.timestamp); }
    bool operator<=(const PassInformation& pass) const { return (propagation.timestamp <= pass.propagation.timestamp); }
    bool operator< (const PassInformation& pass) const { return (propagation.timestamp < pass.propagation.timestamp); }
};

typedef std::vector<PassInformation> PassInformationVec;

class OrbitSimulator {
    public:
        OrbitSimulator();
        ~OrbitSimulator();

        void setMinimumElevation(double elev);
        void setCommsFreq(double freq_dl, double freq_ul);
        void setGroundLocation(double lat, double lon, double height);
        void setSpaceTLEFile(const char *tle);
        void setTimestep(unsigned long timestep);
        void setMaxPropagations(unsigned long prop);

        std::vector<PassInformationVec> passes;

        void setNoradIDFromTLE(char *tle);
        unsigned int getMyNoradID() {return (_norad_id);}

        void propagate();

    private:
        visibility_config_t     _orbit_conf;
        double                  _min_elev;
        unsigned long           _sim_timestep;
        unsigned long           _max_propagations;

        unsigned long           _last_pass_start;
        unsigned long           _last_pass_end;

        PassInformationVec      _pass;

        unsigned int            _norad_id;

        time_t getActualTime()
        {
            return (std::chrono::system_clock::to_time_t (std::chrono::system_clock::now()));
        }

        void computeMotorAngle(PassInformationVec pass_info);
        bool doesPassExist();
        int findNextPass(unsigned long start_timestamp);

};

#endif
