/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MM_Technology_H
#define MM_Technology_H

#include "MuonGeoModel/Technology.h"

#include <vector>
namespace MuonGM {

    // Description class to build MicroMegas chambers

    class MM_Technology : public Technology {
      public:
        double thickness{0.};
        int nlayers{0};
        double gasThickness{0.};
        double pcbThickness{0.};
        double roThickness{0.};
        double f1Thickness{0.};
        double f2Thickness{0.};
        double f3Thickness{0.};

        int geoLevel{0};

        // inner structure parameters (to be defined)

        // constructor
        inline MM_Technology(std::string s);
        inline double Thickness();
    };

    MM_Technology::MM_Technology(std::string s)
        : Technology(s) {}

    double MM_Technology::Thickness() {
        // thickness=nlayers*(gasThickness+pcbThickness) + 2.*pcbThickness;
        return thickness;
    }

} // namespace MuonGM

#endif
