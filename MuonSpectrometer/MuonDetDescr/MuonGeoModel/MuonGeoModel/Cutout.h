/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef Cutout_H
#define Cutout_H

#include <iostream>

class GeoShape;

namespace MuonGM {

    class Cutout {
      public:
        Cutout() = default;
        void setThickness(double compThickness);
        const GeoShape *build();

        int ijob{0};
        int subtype{0};
        int icut{0};
        double dx{0.};
        double dy{0.};
        double widthXs{0.};
        double widthXl{0.};
        double lengthY{0.};
        double excent{0.};
        double dead1{0.};
        double thickness{0.};

        friend std::ostream &operator<<(std::ostream &os, const Cutout &p);

    };
} // namespace MuonGM

#endif
