/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONSPACEPOINT_SPACEPOINTPERLAYERSORTER_H
#define MUONR4_MUONSPACEPOINT_SPACEPOINTPERLAYERSORTER_H

#include <MuonSpacePoint/SpacePointContainer.h>

namespace MuonR4{
    /** @brief The SpacePointPerLayerSorter sort two given space points by their layer Identifier. It is defined as the
     *         Identifier of the first tube in layer for the Mdts or as the Identifier of the first strip in a gasGap 
     *         expressed in an eta view. First, all hits are sorted by layer Identifier - i.e. going outwards the detector.
     *         Then, hits in the same layer are sorted by y position (precision axis) in the sector frame and, if they have 
     *         also the same y, they are sorted by x position (phi direction).*/
    class SpacePointPerLayerSorter {
        public:

            SpacePointPerLayerSorter() = default;
            
            bool operator()(const std::shared_ptr<SpacePoint>& sp1, const std::shared_ptr<SpacePoint>& sp2) const;
            bool operator()(const std::unique_ptr<SpacePoint>& sp1, const std::unique_ptr<SpacePoint>& sp2) const;
            bool operator()(const SpacePoint* sp1, const SpacePoint* sp2) const;
            bool operator()(const SpacePoint& sp1, const SpacePoint& sp2) const;    
            
            /** @brief method returning the logic layer number */
            unsigned int sectorLayerNum(const SpacePoint& sp) const;
    };    

}


#endif