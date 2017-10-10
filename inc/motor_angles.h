#ifndef __MOTOR_ANGLES_HPP__
#define __MOTOR_ANGLES_HPP__

#include <vector>
#include "orbit_simulator.h"

struct PassInformation{
    propagation_output_t propagation;
    double motor_az;
    double motor_el;
    bool operator==(const PassInformation& pass) const { return (pass.propagation.timestamp == propagation.timestamp); }
    bool operator!=(const PassInformation& pass) const { return (pass.propagation.timestamp != propagation.timestamp); }
    bool operator<=(const PassInformation& pass) const { return (propagation.timestamp <= pass.propagation.timestamp); }
    bool operator< (const PassInformation& pass) const { return (propagation.timestamp < pass.propagation.timestamp); }
};

typedef std::vector<PassInformation> PassInformationVec;
typedef std::vector<PassInformationVec> PassesVec;

class Motor_Angles {
    public:
        Motor_Angles();
        void computeMotorAngle(SimulatorResultsVec Results);
        void clear();

    protected:
        PassInformationVec _pass;

    private:

};

#endif
