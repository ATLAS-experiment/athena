/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TGCHITCLUSTERING_H
#define TGCHITCLUSTERING_H

#include <vector>
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonPrepRawData/TgcPrepData.h"
#include "MuonReadoutGeometry/TgcReadoutElement.h"
#include "GeoPrimitives/GeoPrimitives.h"

#include "Identifier/Identifier.h"
#include "MuonIdHelpers/TgcIdHelper.h"

namespace Muon {

  class TgcClusterObj3D {
    public:
        
        using HitList =  std::vector< const TgcPrepData* >;

        TgcClusterObj3D( const HitList& etaC, const HitList& phiC )  : 
            etaCluster(etaC), phiCluster(phiC) {}
        HitList etaCluster{};
        HitList phiCluster{};

        /// Representation of the four edge points
        enum class Edge : uint8_t{
            LowEtaLowPhi = 0,
            LowEtaHighPhi,
            HighEtaLowPhi,
            HighEtaHighPhi
        };
        Amg::Vector3D& getEdge(const Edge e) {
            return m_edgePoints[static_cast<unsigned>(e)];
        }
        const Amg::Vector3D& getEdge(const Edge e) const {
            return m_edgePoints[static_cast<unsigned>(e)];
        }

      private:
        std::array<Amg::Vector3D, 4> m_edgePoints{make_array<Amg::Vector3D, 4>(Amg::Vector3D::Zero())};
  };

  struct TgcHitClusteringObj {

    using HitList =  TgcClusterObj3D::HitList;

    TgcHitClusteringObj( const TgcIdHelper* tgcIdHelp ) :
        m_tgcIdHelper(tgcIdHelp) {}
    
    bool cluster(const HitList& col );
    bool cluster(HitList& filteredHits,
                std::vector<HitList>& finalClusts);

    bool buildClusters3D();

    void dump() const;

    const HitList& bestEtaCluster() const;
    const HitList& bestPhiCluster() const;
 
   
    const TgcIdHelper*        m_tgcIdHelper{nullptr};
    std::vector<HitList>   clustersEta{};
    std::vector<HitList>   clustersPhi{};
    std::vector<TgcClusterObj3D> clusters3D{};

  };

}
#endif
