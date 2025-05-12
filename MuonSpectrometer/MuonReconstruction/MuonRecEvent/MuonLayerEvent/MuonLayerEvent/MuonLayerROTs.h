/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUON_MUONLAYERROTS_H
#define MUON_MUONLAYERROTS_H

#include <vector>
#include <memory>

#include "MuonStationIndex/MuonStationIndex.h"
#include "MuonRIO_OnTrack/MdtDriftCircleOnTrack.h"
#include "MuonRIO_OnTrack/MuonClusterOnTrack.h"

namespace Muon {

    /** struct holding RIO_OnTracks for a given layer */
    class MuonLayerROTs {
    public:
        /** constructor */
        MuonLayerROTs() = default;
        /** Move constructor */
        MuonLayerROTs(MuonLayerROTs&& other) = default;
        /** Move assignment */
        MuonLayerROTs& operator=(MuonLayerROTs&&) = default;
        /** destructor */
        ~MuonLayerROTs() = default;

        /** add MDTs, will merge them with existing MDT's. Takes ownership of all pointers  */
        void addMdts(const std::vector<const MdtDriftCircleOnTrack*>& mdts);

        /** add MDTs, will remove any existing MDT's. Takes ownership of all pointers  */
        void replaceMdts(const std::vector<const MdtDriftCircleOnTrack*>& mdts);

        /** add MuonClusters of a given technology, will merge them with existing clusters. Takes ownership of all pointers  */
        void addClusters(const std::vector<const MuonClusterOnTrack*>& clusters, MuonStationIndex::TechnologyIndex tech);

        /** add MuonClusters of a given technology, will remove any existing clusters. Takes ownership of all pointers  */
        void replaceClusters(const std::vector<const MuonClusterOnTrack*>& clusters, MuonStationIndex::TechnologyIndex tech);

        /** access calibrated MDT's */
        const std::vector<const MdtDriftCircleOnTrack*>& getMdts() const;

        /** access calibrated MuonClusters for a given technolgy */
        const std::vector<const MuonClusterOnTrack*>& getClusters(MuonStationIndex::TechnologyIndex tech) const;

    private:
        /** payload */
        std::vector<const MdtDriftCircleOnTrack*> m_mdts{};
        static constexpr int s_techMax = MuonStationIndex::toInt(MuonStationIndex::TechnologyIndex::TechnologyIndexMax);
        std::array<std::vector<const MuonClusterOnTrack*>, s_techMax> m_clustersPerTechnology{};

        std::vector<std::unique_ptr<const Trk::RIO_OnTrack>> m_garbage{};
    };

    inline const std::vector<const MdtDriftCircleOnTrack*>& MuonLayerROTs::getMdts() const { return m_mdts; }

    inline const std::vector<const MuonClusterOnTrack*>& 
        MuonLayerROTs::getClusters(MuonStationIndex::TechnologyIndex tech) const {
        using namespace MuonStationIndex;
        return m_clustersPerTechnology[toInt(tech)];
    }
}  // namespace Muon

#endif
