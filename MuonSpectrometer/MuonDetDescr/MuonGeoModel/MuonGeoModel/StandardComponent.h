/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef StandardComponent_H
#define StandardComponent_H

#include "MuonGeoModel/Component.h"

#include <iosfwd>

namespace MuonGM {
    class MYSQL;

    class StandardComponent : public Component {
      public:
        StandardComponent() = default;
     
        double posx{0.};
        double posy{0.};
        double posz{0.};
        double deadx{0.};
        double deady{0.};
        double dead3{0.};
        double excent{0.};
        int iswap{0};
        int index{0};
        double GetThickness(const MYSQL& mysql) const;
        friend std::ostream &operator<<(std::ostream &os, const StandardComponent &c);
    };
} // namespace MuonGM

#endif
