#ifndef HELIOS_RAY_H
#define HELIOS_RAY_H

#include <Eigen/Core>

namespace Helios {
    struct Ray {
        Eigen::Vector3f origin;
        Eigen::Vector3f direction;
    };
}

#endif /* End of HELIOS_RAY_H */