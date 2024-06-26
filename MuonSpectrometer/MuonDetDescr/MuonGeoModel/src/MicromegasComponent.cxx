/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonGeoModel/MicromegasComponent.h"

namespace MuonGM {

 
    std::ostream &operator<<(std::ostream &os, const MicromegasComponent &c) {
        os << "Component " << c.name << std::endl;
        return os;
    }

} // namespace MuonGM
