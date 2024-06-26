/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CMI_H
#define CMI_H

#include "MuonGeoModel/Technology.h"

namespace MuonGM {
    class MYSQL;

    class CMI : public Technology {
      public:
        inline CMI(MYSQL& mysql, const std::string& s);
        double largeness{0.};
        double height{0.};
        double excent{0.};
    };

    CMI::CMI(MYSQL& mysql, const std::string& s) : Technology(mysql, s) {}
} // namespace MuonGM

#endif
