/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef XAODTRACKING_VERSIONS_TRACKSUMMARYAUXCONTAINER_V1_H
#define XAODTRACKING_VERSIONS_TRACKSUMMARYAUXCONTAINER_V1_H


#include "xAODCore/AuxContainerBase.h"
#include "xAODTracking/TrackingPrimitives.h"
#include <vector>
namespace xAOD {
 class TrackSummaryAuxContainer_v1 : public AuxContainerBase {
    public:
        TrackSummaryAuxContainer_v1();
        // we use vector instead of array even though the size is fixed
        // this saves on generating ROOT dictionaries for all array dimensions
        
        typedef std::vector<double> Storage;
        std::vector<Storage> params;
        std::vector<Storage> covParams;
        // The referenceSurface (pointer to const Surface) should be added here

        std::vector<unsigned int> nMeasurements;
        std::vector<unsigned int> nHoles;
        std::vector<float> chi2f;
        std::vector<unsigned int> ndf;
        std::vector<unsigned int> nOutliers;
        std::vector<unsigned int> nSharedHits;
        std::vector<unsigned int> tipIndex;
        std::vector<unsigned int> stemIndex;
        std::vector<unsigned int> surfaceIndex;
        std::vector<uint8_t> particleHypothesis;
    };
}

#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::TrackSummaryAuxContainer_v1, xAOD::AuxContainerBase);


#endif
