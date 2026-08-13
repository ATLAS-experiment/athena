/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DQM_ALGORITHMS_DUMMY_ALGORITHM_H
#define DQM_ALGORITHMS_DUMMY_ALGORITHM_H

#include <dqm_algorithms/BasicHistoCheck.h>

namespace dqm_algorithms {

    struct Dummy_Algorithm : public BasicHistoCheck {
        Dummy_Algorithm(): BasicHistoCheck("Dummy_Algorithm") {};
    };

}

#endif // DQM_ALGORITHMS_DUMMY_ALGORITHM_H