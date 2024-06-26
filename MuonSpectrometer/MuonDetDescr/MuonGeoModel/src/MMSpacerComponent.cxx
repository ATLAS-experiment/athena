/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonGeoModel/MMSpacerComponent.h"

#include <string>

namespace MuonGM {

    std::ostream &operator<<(std::ostream &os, const MMSpacerComponent &c) {
        os << "Component " << c.name << std::endl;
        return os;
    }

} // namespace MuonGM
