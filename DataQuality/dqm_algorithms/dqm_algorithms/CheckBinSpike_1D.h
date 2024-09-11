#ifndef DQM_ALGORITHMS_CHECKBINSPIKE_1D_H
#define DQM_ALGORITHMS_CHECKBINSPIKE_1D_H

#include <dqm_algorithms/CheckBinSpike.h>

namespace dqm_algorithms
{
    struct CheckBinSpike_1D : public CheckBinSpike
    {
        CheckBinSpike_1D(): CheckBinSpike("1D"){};
    };
}
#endif // DQM_ALGORITHMS_CHECKBINSPIKE_1D_H