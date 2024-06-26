/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef AlignPos_H
#define AlignPos_H

#include <iostream>
#include <string>
namespace MuonGM {

    class AlignPos {
      public:
        AlignPos() = default;

        int zindex{0};
        int phiindex{0};
        int jobindex{0};
        double tras{0.};
        double traz{0.};
        double trat{0.};
        double rots{0.};
        double rotz{0.};
        double rott{0.};
        bool isBarrel{true};
        bool isTrapezoid{false};    // yuck! GeoModel axes different for box, trap
        std::string tectype{}; // eg BOS1 (to match to station type)
        friend std::ostream &operator<<(std::ostream &os, const AlignPos &p);
    };

} // namespace MuonGM
#endif
