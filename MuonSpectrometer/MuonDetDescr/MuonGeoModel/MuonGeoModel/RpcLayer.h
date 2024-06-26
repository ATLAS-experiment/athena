/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef RpcLayer_H
#define RpcLayer_H

#include "MuonGeoModel/DetectorElement.h"
///
#include "GeoModelKernel/GeoVPhysVol.h"


#include <string>
#include <vector>

namespace MuonGM {

    class Rpc;
    class Cutout;
    class MYSQL;

    class RpcLayer : public DetectorElement {

      public:
        double lwidth{0.};
        double llength{0.};
        double thickness{0.};
        double llongWidth{0.}; // for trapezoidal layers

        Rpc *m{nullptr};

        RpcLayer(const std::string& s, Rpc *t);
        GeoVPhysVol *build(StoredMaterialManager& matManager,
                           const MYSQL& mysql);
        GeoVPhysVol *build(StoredMaterialManager& matManager,
                           const MYSQL& mysql,
                           int cutoutson,
                           const std::vector<Cutout *>&);
        virtual void print() const override;
    };
} // namespace MuonGM

#endif
