/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CHV_H
#define CHV_H

#include "MuonGeoModel/Technology.h"

namespace MuonGM {
    class MYSQL;

    class CHV : public Technology {
      public:
        inline CHV(MYSQL& mysql, const std::string& s);
        double largeness{0.};
        double height{0.};
        double excent{0.};
    };

    CHV::CHV(MYSQL& mysql, const std::string& s) : Technology(mysql, s) {}
} // namespace MuonGM

#endif
