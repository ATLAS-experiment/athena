/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef Mdt_H
#define Mdt_H

#include "MuonGeoModel/DetectorElement.h"
#include "MuonGeoModel/MultiLayer.h"

#include <string>
#include <vector>

class GeoFullPhysVol;

namespace MuonGM {

    class Cutout;
    class Component;
    class MdtComponent;
    class MYSQL;

    class Mdt : public DetectorElement {

      public:
        double width{0.};
        double length{0.};
        double thickness{0.};
        double longWidth{0.}; // for trapezoidal layers
        int index{0};
        double tubelenStepSize{0.};
        double tubePitch{0.};

        std::unique_ptr<MultiLayer> layer{};
        Mdt(const MYSQL& mysql, Component *s1, const std::string& s2);
   
       
        GeoFullPhysVol *build(StoredMaterialManager& matManager,
                              const MYSQL& mysql);
        GeoFullPhysVol *build(StoredMaterialManager& matManager,
                              const MYSQL& mysql,
                              std::vector<Cutout *>&);
	void processCutouts(std::vector<Cutout *>& vcutdef);
        virtual void print() const override;

      private:
        MdtComponent *m_component{nullptr};

    };

} // namespace MuonGM

#endif
