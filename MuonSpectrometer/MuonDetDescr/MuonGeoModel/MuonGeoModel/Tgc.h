/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef Tgc_H
#define Tgc_H

#include "MuonGeoModel/DetectorElement.h"
#include "MuonGeoModel/TgcComponent.h"

class GeoFullPhysVol;

namespace MuonGM {

    class Cutout;
    class MYSQL;

    class Tgc : public DetectorElement {

      public:
        double width{0.};
        double length{0.};
        double thickness{0.};
        double longWidth{0.}; // for trapezoidal layers
        double irad{0.};
        double orad{0.};
        double dphi{0.};
        int index{0};

        Tgc(const MYSQL& mysql, Component *s);
        GeoFullPhysVol *build(StoredMaterialManager& matManager,
                              const MYSQL& mysql,
                              int minimalgeo);
        GeoFullPhysVol *build(StoredMaterialManager& matManager,
                              const MYSQL& mysql,
                              int minimalgeo, int cutoutson,
                              const std::vector<Cutout *>&);
        virtual void print() const override;

      private:
        TgcComponent *m_component{nullptr};
    };

} // namespace MuonGM

#endif
