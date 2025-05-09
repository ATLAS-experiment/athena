/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonLayerEvent/MuonLayerROTs.h"

namespace Muon {
    using namespace MuonStationIndex;

    void MuonLayerROTs::addMdts(const std::vector<const MdtDriftCircleOnTrack*>& mdts) {
        m_mdts.insert(m_mdts.end(), mdts.begin(), mdts.end());
        std::for_each(mdts.begin(), mdts.end(), 
                      [this](const MdtDriftCircleOnTrack* rot){
                           m_garbage.emplace_back(rot);
                       });
    }

    void MuonLayerROTs::addClusters(const std::vector<const MuonClusterOnTrack*>& clusters, 
                                    MuonStationIndex::TechnologyIndex tech) {
        
        std::vector<const MuonClusterOnTrack*>& insertMe{m_clustersPerTechnology[toInt(tech)]};
        insertMe.insert(insertMe.end(), clusters.begin(), clusters.end());
        std::for_each(clusters.begin(), clusters.end(), 
                      [this](const MuonClusterOnTrack* rot){
                           m_garbage.emplace_back(rot);
                       });
    }

    void MuonLayerROTs::replaceMdts(const std::vector<const MdtDriftCircleOnTrack*>& mdts) {
        m_mdts.clear();
        addMdts(mdts);
    }

    void MuonLayerROTs::replaceClusters(const std::vector<const MuonClusterOnTrack*>& clusters, 
                                        MuonStationIndex::TechnologyIndex tech) {
        m_clustersPerTechnology[toInt(tech)].clear();
        addClusters(clusters, tech);
    }

}  // namespace Muon
