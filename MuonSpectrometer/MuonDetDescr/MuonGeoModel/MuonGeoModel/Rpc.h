/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef Rpc_H
#define Rpc_H
#include "MuonGeoModel/DetectorElement.h"
///
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "MuonGeoModel/RpcComponent.h"

class GeoVFullPhysVol;
namespace MuonGM {
  class MYSQL;
}

namespace MuonGM {

    class Cutout;

    class Rpc : public DetectorElement {

      public:
        double width{0.};
        double length{0.};
        double thickness{0.};
        double longWidth{0.}; // for trapezoidal layers
        double idiv{0.};
        double jdiv{0.};
        float y_translation{0.f};
        float z_translation{0.f};

        Rpc(const MYSQL& mysql, Component *s);
        GeoVFullPhysVol *build();
        GeoFullPhysVol *build(StoredMaterialManager& matManager,
                              const MYSQL& mysql,
                              int minimalgeo);
        GeoFullPhysVol *build(StoredMaterialManager& matManager,
                              const MYSQL& mysql,
                              int minimalgeo, int cutoutson,
                              const std::vector<Cutout *>&);
        virtual void print() const override;
        unsigned int nGasGaps() const;

      private:
        RpcComponent *m_component{nullptr};
        unsigned int m_nlayers{0};
    };
    inline unsigned int Rpc::nGasGaps() const { return m_nlayers; }
} // namespace MuonGM

#endif
