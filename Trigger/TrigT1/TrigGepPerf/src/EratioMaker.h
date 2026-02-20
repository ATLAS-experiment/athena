/*
 *   Copyright (C) 2002-2025 CERN
 *   for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_ERATIO_H
#define TRIGGEPPERF_ERATIO_H

#include <vector>
#include <utility>
#include <cmath>
#include "xAODTrigger/eFexEMRoIContainer.h"
#include "TrigGepPerf/GepCellMap.h"
#include "Math/Vector4D.h"

namespace Gep {

// Auxiliary structure for Eratio result, and init values to 0.
struct EratioObj {
    double seedEta{0.};
    double seedPhi{0.};
    double E1{0.};
    double E2{0.};
    double Eratio{0.};
};

class EratioMaker {
public:

    // Constructor
    EratioMaker(const GepCellMap& caloCellsMap);

    // Main method to get Eratio for each seed
    const EratioObj makeEratio(const ROOT::Math::PtEtaPhiEVector obj) const;

private:
    // Auxiliary methods - intermediate steps
    std::vector<std::vector<GepCaloCell>> makeWindow(const ROOT::Math::PtEtaPhiEVector &obj) const;
    std::pair<double, double> findLocalMaxima(const std::vector<std::vector<GepCaloCell>>& window) const;
    double computeEratio(double E1, double E2) const;

    // Attributes
    const GepCellMap& m_caloCellsMap;    
};

} // namespace Gep

#endif // TRIGGEPPERF_ERATIO_H
