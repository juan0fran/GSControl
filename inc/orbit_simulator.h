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
#include "link.h"
#include "payload_tracker_api.h"

struct SimulatorResults {
    propagation_output_t propagation;
    LinkResults link;
};

typedef std::vector<SimulatorResults> SimulatorResultsVec;

class OrbitSimulator : public Link{
    public:
        OrbitSimulator();
        ~OrbitSimulator();

        void SetMinimumElevation(double elev);
        void SetCommsFreq(double freq_dl, double freq_ul);
        void SetGroundLocation(double lat, double lon, double height);
        void SetSpaceTLEFile(const char *tle);
        void SetTimestep(unsigned long timestep);

        std::string GetResults();

        unsigned long getLastFoundPassStart();
        unsigned long getLastFoundPassEnd();
        unsigned long getLastPassDuration();
        unsigned long getPassAvailability();

        void run();
        int findNextPass(unsigned long start_timestamp);

        SimulatorResultsVec   Results;

    private:
        visibility_config_t     _orbit_conf;
        double                  _min_elev;
        unsigned long           _sim_timestep;

        unsigned long           _last_pass_start;
        unsigned long           _last_pass_end;

};

#endif
