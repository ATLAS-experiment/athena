/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDT_H
#define MDT_H

#include "MuonGeoModel/Technology.h"

#include <string>

namespace MuonGM {
    class MYSQL;

    class MDT : public Technology {
      public:
        int numOfLayers{0};
        double pitch{0.};
        double innerRadius{0.};
        double totalThickness{0.};
        double tubeDeadLength{0.};
        double tubeEndPlugLength{0.};
        double tubeWallThickness{0.};

        std::array<double,4> y{};
        std::array<double,4> x{};

        MDT(MYSQL& mysql, const std::string& s) : Technology(mysql, s) {}
    };
} // namespace MuonGM

#endif
