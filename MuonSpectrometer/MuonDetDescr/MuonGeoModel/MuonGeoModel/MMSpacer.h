/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MMSpacer_H
#define MMSpacer_H

#include "MuonGeoModel/DetectorElement.h"

#include <vector>

class GeoPhysVol;

namespace MuonGM {

    class Cutout;
    class Component;
    class MMSpacerComponent;
    class MYSQL;

    class MMSpacer : public DetectorElement {

      public:
        double width{0.};
        double length{0.};
        double thickness{0.};
        double longWidth{0.}; // for trapezoidal layers
        int index{0};

        MMSpacer(const MYSQL& mysql, Component *s);
        GeoPhysVol *build(StoredMaterialManager& matManager,
                          const MYSQL& mysql,
                          int minimalgeo);
        GeoPhysVol *build(StoredMaterialManager& matManager,
                          const MYSQL& mysql,
                          int minimalgeo, int cutoutson,
                          const std::vector<Cutout *>&);
        virtual void print() const override;

      private:
        MMSpacerComponent *m_component{nullptr};
    };

} // namespace MuonGM

#endif
