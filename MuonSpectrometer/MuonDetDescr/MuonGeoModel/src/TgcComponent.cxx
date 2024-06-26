/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonGeoModel/TgcComponent.h"

#include <string>

namespace MuonGM {


    std::ostream &operator<<(std::ostream &os, const TgcComponent &c) {
        os << "Component " << c.name << std::endl;
        return os;
    }

} // namespace MuonGM
