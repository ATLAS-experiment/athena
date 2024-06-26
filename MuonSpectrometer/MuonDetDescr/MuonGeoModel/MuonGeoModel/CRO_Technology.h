/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CRO_H
#define CRO_H

#include "MuonGeoModel/Technology.h"

namespace MuonGM {
    class MYSQL;

    class CRO : public Technology {
      public:
        inline CRO(MYSQL& mysql, const std::string& s);
        double largeness{0.};
        double height{0.};
        double excent{0.};
    };

    CRO::CRO(MYSQL& mysql, const std::string& s) : Technology(mysql, s) {}
} // namespace MuonGM

#endif
