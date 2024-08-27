/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONSPACEPOINT_SPACEPOINTPERLAYERSORTER_H
#define MUONR4_MUONSPACEPOINT_SPACEPOINTPERLAYERSORTER_H

#include <MuonSpacePoint/SpacePointContainer.h>

namespace MuonR4{
    /** @brief The SpacePointPerLayerSorter groups the space points by their layer Identifier. It is defined as the
     *         Identifier of the first tube in layer for the Mdts or as the Identifier of the first strip in a gasGap 
     *         expressed in an eta view. First, all hits are sorted by increasing chamber z - i.e. going outwards the detector,
     *         and then grouped into two sets of vectors. One for the Mdts and the other for the remaining strip detectors. */
    class SpacePointPerLayerSorter {
        public:
            using HitVec = std::vector<const SpacePoint*>;
            using HitLayVec = std::vector<HitVec>;
            /** @brief Constructor taking a complete bucket  */
            SpacePointPerLayerSorter(const SpacePointBucket& bucket);
            /** @brief Constructor taking a subset of SpacePoints */
            SpacePointPerLayerSorter(HitVec vec);
            /** @brief Returns the sorted Mdt hits */
            const HitLayVec& mdtHits() const {
              return m_mdtLayers;
            }
            /** @brief Returns the sorted strip hits */
            const HitLayVec& stripHits() const {
              return m_stripLayers;
            }

        private:
            HitLayVec m_mdtLayers{};
            HitLayVec m_stripLayers{};
    
    };    

}


#endif