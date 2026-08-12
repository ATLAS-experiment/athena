/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DQM_ALGORITHMS_HISTOGRAM_NOT_EMPTY_DISABLED_H
#define DQM_ALGORITHMS_HISTOGRAM_NOT_EMPTY_DISABLED_H

#include <dqm_algorithms/BasicHistoCheck.h>

namespace dqm_algorithms
{
    struct Histogram_Not_Empty_Disabled : public BasicHistoCheck
    {
        Histogram_Not_Empty_Disabled(): BasicHistoCheck("Histogram_Not_Empty_Disabled") {};
    };
}

#endif // DQM_ALGORITHMS_HISTOGRAM_NOT_EMPTY_DISABLED_H