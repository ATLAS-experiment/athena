/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONPATTERNHELPERS_SEGMENTAMBISOLVER_H
#define MUONR4_MUONPATTERNHELPERS_SEGMENTAMBISOLVER_H

#include <AthenaBaseComps/AthMessaging.h>
#include <MuonPatternEvent/Segment.h>

#include <vector>
#include <unordered_map>
#include <array>

namespace MuonR4::SegmentFit {
    class SegmentAmbiSolver : public AthMessaging {
        public:
            struct Config{
                /** @brief If two overlapping segments have both the chi2 below the threshold, the one 
                 *         with more degrees of freedom is chosen */
                double selectByNDoFChi2{5.};
                /** @brief Cut on the number of shared precision hits */
                unsigned int sharedPrecHits{3};
                /** @brief Allow for left-right ambiguities */
                bool remLeftRightAmbi{false};
            };
            
            SegmentAmbiSolver(const std::string&name,
                              Config&& config);


            using SegmentVec = std::vector<std::unique_ptr<Segment>>; 
            SegmentVec resolveAmbiguity(const ActsGeometryContext& gctx,
                                        SegmentVec&& toResolve) const;


            enum class Resolution{
                noOverlap,
                superSet,
                subSet
            };
        private:
            const Config m_cfg{};

            std::vector<int> driftSigns(const ActsGeometryContext& gctx,
                                        const Segment& segment,
                                        const Segment::MeasVec& measurements) const;
            
            using MeasurementSet = std::unordered_set<const xAOD::UncalibratedMeasurement*>;

            /** @brief Extract the Uncalibrated measurements used to build the segment */
            MeasurementSet extractPrds(const Segment& segment) const;
            
            /** @brief counts the number of measurements that're in both sets */
            unsigned int countShared(const MeasurementSet& measSet1, 
                                     const MeasurementSet& measSet2) const;

            /** @brief Returns the reduced chi2 of the segment */
            double redChi2(const Segment& segment) const;




    };
}


#endif