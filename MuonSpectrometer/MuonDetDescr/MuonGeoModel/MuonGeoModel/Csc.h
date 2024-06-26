/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef Csc_H
#define Csc_H

#include "MuonGeoModel/DetectorElement.h"
#include "MuonGeoModel/CscMultiLayer.h"

#include <vector>
#include <memory>

class GeoFullPhysVol;

namespace MuonGM {

    class Component;
    class CscComponent;
    class Cutout;
    class MYSQL;

    class Csc : public DetectorElement {

      public:
        double width{0.};
        double length{0.};
        double thickness{0.};
        double longWidth{0.}; // for trapezoidal layers
        double excent{0.};    // for csc layers
        double physicalLength{0.};
        double maxwLength{0.};
        double upWidth{0.};

        int index{0};

        std::unique_ptr<CscMultiLayer> layer{nullptr};

        Csc(const MYSQL& mysql, Component *s);
        ~Csc() = default;
        
        GeoFullPhysVol *build(StoredMaterialManager& matManager,
                              const MYSQL& mysql,
                              int minimalgeo);
        GeoFullPhysVol *build(StoredMaterialManager& matManager,
                              const MYSQL& mysql,
                              int minimalgeo, int cutoutson,
                              const std::vector<Cutout *>&);
        virtual void print() const override;

      private:
        CscComponent *m_component{nullptr};      
    };

} // namespace MuonGM

#endif
