/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef Ded_H
#define Ded_H

#include "MuonGeoModel/DedComponent.h"
#include "MuonGeoModel/DetectorElement.h"

class GeoVPhysVol;

namespace MuonGM {

    class Cutout;
    class MYSQL;

    class Ded : public DetectorElement {

      public:
        double width{0.};
        double length{0.};
        double thickness{0.};
        double longWidth{0.}; // for trapezoidal layers

        Ded(const MYSQL& mysql, Component *s);
        GeoVPhysVol *build(StoredMaterialManager& matManager,
                           const MYSQL& mysql);
        GeoVPhysVol *build(StoredMaterialManager& matManager,
                           const MYSQL& mysql,
                           int cutoutson,
                           const std::vector<Cutout *>&);
        virtual void print() const override;

      private:
        DedComponent *m_component{nullptr};
    };
} // namespace MuonGM

#endif
