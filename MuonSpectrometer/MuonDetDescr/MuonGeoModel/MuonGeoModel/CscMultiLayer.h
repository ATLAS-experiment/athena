/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CscMultiLayer_H
#define CscMultiLayer_H

#include "MuonGeoModel/DetectorElement.h"
///
#include "GeoModelKernel/GeoShape.h"
#include "GeoModelKernel/GeoVPhysVol.h"


namespace MuonGM {

    class Cutout;
    class MYSQL;

    class CscMultiLayer : public DetectorElement {

      public: // data members
        int nrOfLayers{0};

        double width{0.};
        double longWidth{0.};
        double upWidth{0.};

        double excent{0.};
        double length{0.};
        double physicalLength{0.};
        double maxwLength{0.};

        double thickness{0.};
        double cscthickness{0.};
        std::array<double, 8> dim{};

      public: // methods
        CscMultiLayer(const MYSQL& mysql, const std::string& n);
        GeoVPhysVol *build(StoredMaterialManager& matManager,
                           const MYSQL& mysql);
        GeoVPhysVol *build(StoredMaterialManager& matManager,
                           const MYSQL& mysql,
                           int cutoutson,
                           const std::vector<Cutout *>& vcutdef);
        virtual void print() const override;
    };

} // namespace MuonGM

#endif
