/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DED_H
#define DED_H

#include "MuonGeoModel/Technology.h"

#include <string>

namespace MuonGM {

    class MYSQL;

    class DED : public Technology {
      public:
        double AlThickness{0.};
        double HoneyCombThickness{0.};

        DED(MYSQL& mysql, std::string s);

        bool hasAlHoneyComb() const { return false; };
        bool hasPaperHoneyComb() const { return true; };
    };
} // namespace MuonGM

#endif
