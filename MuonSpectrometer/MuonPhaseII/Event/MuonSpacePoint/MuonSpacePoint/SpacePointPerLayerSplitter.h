/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONSPACEPOINT_SPACEPOINTPERLAYERSPLITTER_H
#define MUONR4_MUONSPACEPOINT_SPACEPOINTPERLAYERSPLITTER_H

#include <MuonSpacePoint/SpacePointContainer.h>

namespace MuonR4{
    /** @brief The SpacePointPerLayerSplitter takes a set of spacepoints already sorted 
     *         by layer Identifier (see MuonSpacePoint/SpacePointPerLayerSorter.h) and 
     *         splits them into two sets of hits, one for MDT hits and the other one for
     *         strip detector hits. Each of these set contains the hits grouped by Layer
     *         Identifier.         
     */
    class SpacePointPerLayerSplitter {
        public:
            using HitVec = std::vector<const SpacePoint*>;
            using HitLayVec = std::vector<HitVec>;
            /** @brief Constructor taking a complete bucket  */
            SpacePointPerLayerSplitter(const SpacePointBucket& bucket);
            /** @brief Constructor taking a subset of SpacePoints */
            SpacePointPerLayerSplitter(const HitVec& vec);
            /** @brief Returns the sorted Mdt hits */
            const HitLayVec& mdtHits() const {
              return m_mdtLayers;
            }
            /** @brief Returns the number of all Mdt hits in the seed */
            unsigned int nMdtHits() const {
              return m_nMdtHits;
            }
            /** @brief Returns the sorted strip hits */
            const HitLayVec& stripHits() const {
              return m_stripLayers;
            }
            /** @brief Returns the number of all strip hits in the seed */
            unsigned int nStripHits() const {
              return m_nStripHits;
            }
            /** @brief Returns the layer index with hits from the second multilayer  */
            unsigned int firstLayerFrom2ndMl() const {
              return m_tubeLaySwitch;
            }
        private:
            /** @brief Sorted Mdt hits per tube layer */
            HitLayVec m_mdtLayers{};
            /** @brief Sorted Strip hits per  gasGap strip  */
            HitLayVec m_stripLayers{};
            /** @brief Number of all Mdt tube hits  */
            unsigned int m_nMdtHits{0};
            /** @brief Number of all strip hits */
            unsigned int m_nStripHits{0};
            /** @brief Index of the first tube-layer from the second multilayer */
            unsigned int m_tubeLaySwitch{0};
    
    };    

}


#endif