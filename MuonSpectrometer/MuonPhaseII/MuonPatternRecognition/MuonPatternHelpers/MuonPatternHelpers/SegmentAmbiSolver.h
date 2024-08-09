/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONPATTERNHELPERS_SEGMENTAMBISOLVER_H
#define MUONR4_MUONPATTERNHELPERS_SEGMENTAMBISOLVER_H

#include <AthenaBaseComps/AthMessaging.h>
#include <MuonPatternEvent/Segment.h>
#include <GaudiKernel/SystemOfUnits.h>

#include <vector>
#include <unordered_map>
#include <array>

namespace MuonR4{
    class SegmentAmbiSolver : public AthMessaging {
        public:
            
            SegmentAmbiSolver(const std::string&name);


            using SegmentVec = std::vector<std::unique_ptr<Segment>>; 
            SegmentVec resolveAmbiguity(const ActsGeometryContext& gctx,
                                        SegmentVec&& toResolve) const;


            enum class Resolution{
                noOverlap,
                superSet,
                subSet
            };

            using MeasByLayerMap = std::unordered_map<Identifier, const SpacePoint*>;            
            MeasByLayerMap extractPrds(const Segment& segment) const;

            std::vector<int> driftSigns(const ActsGeometryContext& gctx,
                                        const Segment& segment,
                                        const std::vector<const SpacePoint*>& measurements) const;

    };
}


#endif