/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef Position_H
#define Position_H

#include <iostream>
namespace MuonGM {

    class Position {
      public:
        Position() = default;

        int zindex{0};
        int phiindex{0};
        int phitype{0};
        int icut{0};
        int subtype{0};
        double phi{0.};
        double radius{0.};
        double z{0.};
        double shift{0.};
        double inclination{0.};
        double alpha{0.};
        double beta{0.};
        double gamma{0.};

        bool isAssigned{false};

        bool isMirrored{false};
        bool isBarrelLike{false};
        friend std::ostream &operator<<(std::ostream &os, const Position &p);
    };
} // namespace MuonGM

#endif
