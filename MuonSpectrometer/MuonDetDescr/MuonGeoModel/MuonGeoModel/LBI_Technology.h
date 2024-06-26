/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LBI_H
#define LBI_H

#include "MuonGeoModel/Technology.h"

namespace MuonGM {
    class MYSQL;

    class LBI : public Technology {
      public:
        inline LBI(MYSQL& mysql, const std::string& s);
        double height{0.};
        float lowerThickness{0.f};
        float yShift{0.f};
    };

    LBI::LBI(MYSQL& mysql, const std::string& s) : Technology(mysql, s) {}
} // namespace MuonGM

#endif
