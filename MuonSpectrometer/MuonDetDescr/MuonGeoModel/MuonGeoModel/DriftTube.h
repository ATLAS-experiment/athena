/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DriftTube_H
#define DriftTube_H

#include "MuonGeoModel/DetectorElement.h"

#include <string>
class StoredMaterialManager;
class GeoVPhysVol;

namespace MuonGM {
    class MYSQL;

    class DriftTube : public DetectorElement {
      public:
        std::string gasMaterial{};
        std::string tubeMaterial{};
        std::string plugMaterial{};
        std::string wireMaterial{};
        double length{0.};
        double outerRadius{0.};
        double gasRadius{0.};
        double plugLength{0.};

        GeoVPhysVol *build(StoredMaterialManager& matManager);
        virtual void print() const override;
        DriftTube(const MYSQL& mysql, const std::string& s);
    };
} // namespace MuonGM

#endif
