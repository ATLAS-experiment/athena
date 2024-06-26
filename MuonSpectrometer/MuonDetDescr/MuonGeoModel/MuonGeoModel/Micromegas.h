/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef Micromegas_H
#define Micromegas_H

#include "MuonGeoModel/DetectorElement.h"

#include <vector>

class GeoFullPhysVol;

namespace MuonGM {

    class Cutout;
    class Component;
    class MicromegasComponent;

    class Micromegas : public DetectorElement {

      public:
        double width{0.};
        double length{0.};
        double thickness{0.};
        double longWidth{0.}; // for trapezoidal layers
        int index{0};

        Micromegas(Component *s);
        GeoFullPhysVol *build(StoredMaterialManager& matManager,
                              int minimalgeo);
        GeoFullPhysVol *build(StoredMaterialManager& matManager,
                              int minimalgeo, int cutoutson,
                              const std::vector<Cutout *>&);
        virtual void print() const override;

      private:
        MicromegasComponent *m_component{nullptr};
    };

} // namespace MuonGM

#endif
