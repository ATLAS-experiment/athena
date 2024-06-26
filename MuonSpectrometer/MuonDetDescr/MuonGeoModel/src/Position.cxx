/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonGeoModel/Position.h"

namespace MuonGM {

    std::ostream &operator<<(std::ostream &os, const Position &p) {
        os << " Position eta/phi index " << p.zindex << "/" << p.phiindex << " phi/z/R/s_shift " << p.phi << " " << p.z << " " << p.radius << " " << p.shift << " alpha/beta/gamma "
           << p.alpha << " " << p.beta << " " << p.gamma;

        return os;
    }
} // namespace MuonGM
