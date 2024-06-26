/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef sTGC_Technology_H
#define sTGC_Technology_H

#include "MuonGeoModel/Technology.h"

#include <vector>
namespace MuonGM {

    // Description class to build sTGC chambers

    class sTGC_Technology : public Technology {
      public:
        double thickness{0.};
        int nlayers{0};
        double gasThickness{0.};
        double pcbThickness{0.};
        double pcbThickness150{0.};
        double pcbThickness200{0.};
        double coverThickness{0.};
        double f4Thickness{0.};
        double f5Thickness{0.};
        double f6Thickness{0.};

        int geoLevel{0};

        // inner structure parameters (to be defined)

        // constructor
        inline sTGC_Technology(std::string s);
        inline double Thickness() const;
    };

    sTGC_Technology::sTGC_Technology(std::string s)
        : Technology(s){}

    double sTGC_Technology::Thickness() const {
        // thickness=nlayers*(gasThickness+pcbThickness) + pcbThickness;
        return thickness;
    }

} // namespace MuonGM

#endif
