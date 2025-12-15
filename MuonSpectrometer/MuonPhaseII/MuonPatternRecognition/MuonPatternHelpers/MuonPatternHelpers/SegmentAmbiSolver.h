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
    /** @brief The SegmentAmbiSolver removes ambiguities between segment candidates 
     *         from the fit. They typically arise from the sharing of hits between two
     *         candidates. The resolution happens in the following steps
     *            1) The passed segment candidates are sorted by the reduced chi2 in increasing
     *               order Candidates which already have a sufficiently good chi2 are internally
     *               sorted by the degress of freedom in decreasing order
     *            2) The first segment in the list has the most degrees of freedom with an acceptable
     *                chi2. Is is added to the list of good candidates.
     *            3) The other segments are compared with the good candidates. If the segment shares
     *               at minimum 3 hits with a good candidate and has a worse chi2 and fewer hits
     *               it is marked for removal. If the good candidate has worse chi2 the segment 
     *               takes over the place of the good candidate */
    class SegmentAmbiSolver : public AthMessaging {
        public:
            /** @brief Configuration object to stree the ambiguties */
            struct Config{
                /** @brief If two overlapping segments have both the chi2 below the threshold, the one 
                 *         with more degrees of freedom is chosen */
                double selectByNDoFChi2{5.};
                /** @brief Cut on the number of shared precision hits */
                unsigned int sharedPrecHits{3};
                /** @brief Two candidates with the sam number of precision hits but 
                 *         with different left / right ambiguities are kept both */
                bool remLeftRightAmbi{false};
            };
            /** @brief Constructor 
             *  @param name: Name of the algorithm to synchronize the msg output
             *  @param config: Ambiguity resolver configuration object */
            SegmentAmbiSolver(const std::string&name,
                              Config&& config);

            /** @brief Abrivation of the temporary segment container */
            using SegmentVec = std::vector<std::unique_ptr<Segment>>; 
            /** @brief Launches the ambiguity resoltuion and returns a new temporary
             *         contained containing the good candidates only
             * @param gctx: Geometry context to construct the local segment parameters
             * @param toResolve: Collection of segments which need to be ambiguity resolved */
            SegmentVec resolveAmbiguity(const ActsTrk::GeometryContext& gctx,
                                        SegmentVec&& toResolve) const;
        private:
            /** @brief Configuration object */
            const Config m_cfg{};
            /** @brief Auxiliary enum to indicate the ambiguity status of the segment candidates */
            enum class Resolution: std::uint8_t{
                noOverlap, /// No ambiguity with other candidates detected
                superSet,  /// Ambiguity detected and the candidate is of better quality
                subSet     /// Ambiguity detected but the candidate is of poorer quality
            };
            /** @brief Calculates the left-right ambiguities of the segment w.r.t each measurement
             *  @param gctx: Geometry context to construct the local segment parameters
             *  @param segment: The segment from which the line parameters are extracted
             *  @param measurements: List of segment measurments w.r. the left-right signs need
             *                       to be evaluated */
            std::vector<int> driftSigns(const ActsTrk::GeometryContext& gctx,
                                        const Segment& segment,
                                        const Segment::MeasVec& measurements) const;
            /** @brief List of measurements out of which the segment is made of */
            using MeasurementSet = std::unordered_set<const xAOD::UncalibratedMeasurement*>;
            /** @brief Extract the Uncalibrated measurements used to build the segment
             *  @param segment: The segment from which the raw measurements should be fetched */
            MeasurementSet extractPrds(const Segment& segment) const;
            
            /** @brief Counts the number of measurements that're in both sets
             *  @param measSet1: First set of measurements to compare
             *  @param measSet2: Second set of measurements to compare */
            unsigned int countShared(const MeasurementSet& measSet1, 
                                     const MeasurementSet& measSet2) const;

            /** @brief Returns the reduced chi2 of the segment
             *  @param segment: Segment of interest */
            double redChi2(const Segment& segment) const;




    };
}


#endif