/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef sTGC_H
#define sTGC_H

#include "MuonGeoModel/DetectorElement.h"

#include <vector>

class GeoFullPhysVol;

namespace MuonGM {

    class Cutout;
    class Component;
    class sTGCComponent;

    class sTGC : public DetectorElement {

      public:
        double width{0.};
        double length{0.};
        double thickness{0.};
        double longWidth{0.};      // for trapezoidal layers
        double yCutout{0.};        // for Hexagonal layer
        double yCutoutCathode{0.}; // for Hexagonal layer
        int index{0};

        sTGC(Component *s);
        GeoFullPhysVol *build(StoredMaterialManager& matManager,
                              int minimalgeo);
        GeoFullPhysVol *build(StoredMaterialManager& matManager,
                              int minimalgeo, int cutoutson,
                              const std::vector<Cutout *>&);
        virtual void print() const override;

      private:
        sTGCComponent *m_component{nullptr};
    };

} // namespace MuonGM

#endif
