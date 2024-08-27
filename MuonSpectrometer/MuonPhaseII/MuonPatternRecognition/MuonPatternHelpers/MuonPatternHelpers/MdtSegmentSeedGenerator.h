/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONPATTERNHELPERS_MDTSEGMENTSEEDGENERATOR_H
#define MUONR4_MUONPATTERNHELPERS_MDTSEGMENTSEEDGENERATOR_H

#include <AthenaBaseComps/AthMessaging.h>
#include <MuonSpacePoint/SpacePointPerLayerSorter.h>
#include <MuonPatternEvent/SegmentSeed.h>
#include <GaudiKernel/SystemOfUnits.h>


#include <vector>
#include <array>


namespace MuonR4 {
    class ISpacePointCalibrator;
    /** @brief Helper class to generate valid seeds for the segment fit. The generator first returns a seed
     *         directly made from the patten recogntion. Afterwards it builds seeds by lying tangent lines
     *         to a pair of drift circles. The pairing starts from the innermost & outermost layers with tubes.
     *         A valid seed must have at least 4 associated hits which are within a chi2 of 5. If two seeds
     *         within the parameter resolution are generated, then the latter one is skipped. */
    class MdtSegmentSeedGenerator: public AthMessaging {
        public:
            
            /** @brief Configuration switches of the module  */
            struct Config{
                /** @brief Upper cut on the hit chi2 w.r.t. seed in order to be associated to the seed*/
                double chi2PerHit{5.};
                /** @brief Try at the first time the pattern seed as candidate */
                bool startWithPattern{true};
                /** @brief How many drift circles may be on a layer to be used for seeding */
                unsigned int busyLayerLimit{2};
                /** @brief How many drift circle hits needs the seed to contain in order to be valid */
                unsigned int nMdtHitCut{4};
                /** @brief Once a seed with even more than initially required hits is found,
                 *         reject all following seeds with less hits */
                bool tightenHitCut{true};
                /** @brief Check whether a new seed candidate shares the same left-right solution with already accepted ones
                 *         Reject the seed if it has the same amount of hits */
                bool overlapCorridor{true};
                /** @brief Two seeds having an intercept within this parameter
                 *         are considered to have the same intercept */
                double interceptReso{200. * Gaudi::Units::micrometer};
                /** @brief Two seeds having an angle within this parameter
                 *         are considered to have the same angle. If intercept & angle
                 *         are equivalent, then the second seed is rejected as duplicate */
                double tanThetaReso{250.* Gaudi::Units::mrad};
                /** @brief Scale factor of the drift radius uncertainty estimates */
                double uncertScale{2.};
                /** @brief Pointer to the space point calibrator */
                const ISpacePointCalibrator* calibrator{nullptr};
            };
        
        
        /** @brief Standard constructor taking the segmentSeed to start with and then few
         *         configuration tunes
         * @param name: Name of the Seed generator's logger
         * @param segmentSeed: Seed from which the seeds for the fit are built
         * @param configuration: Passed configuration settings of the generator */
        MdtSegmentSeedGenerator(const std::string& name,
                                const SegmentSeed* segmentSeed, 
                                const Config& configuration);

        ~MdtSegmentSeedGenerator();
        /** @brief returns the next seed in the row */
        std::optional<SegmentSeed> nextSeed(const EventContext& ctx);
        /** @brief Returns how many seeds have been generated */
        unsigned int numGenerated() const;
    
        private:
            /** @brief Tries to build the seed from the two hits. Fails if the solution is invalid
             *         or if the seed has already been built before
             *  @param topHit: Hit candidate from the upper layer
             *  @param bottomHit: Hit candidate from the lower layer
             *  @param sign: Object encoding whether the tangent is left / right & also
             *               whether to pick the positive or negative solution. */
            std::optional<SegmentSeed> buildSeed(const EventContext& ctx,
                                                 const HoughHitType & topHit, 
                                                 const HoughHitType & bottomHit, 
                                                 const std::array<int,3> & signs ); 
        
            /** @brief Prepares the generator to generate the seed from the next pair of drift circles */
            void moveToNextCandidate();
            Config m_cfg{};
            
            /** @brief Sign combinations to draw the 4 lines tangent to 2 drift circles
             *         The first two are indicating whether the tangent is left/right to the
             *         first/second circle. The last sign is picking the sign of the 
             *         solution arising from the final quadratic equation. */
            constexpr static std::array<std::array<int,3>,8> s_signCombos{
                std::array{-1,-1,-1}, std::array{-1,-1, 1}, 
                std::array{-1, 1,-1}, std::array{-1, 1, 1}, 
                std::array{ 1,-1,-1}, std::array{ 1,-1, 1}, 
                std::array{ 1, 1,-1}, std::array{ 1, 1, 1}
            };
            const SegmentSeed* m_segmentSeed{nullptr};
            SpacePointPerLayerSorter m_hitLayers{m_segmentSeed->getHitsInMax()};
            using HitVec = SpacePointPerLayerSorter::HitVec;
        
            /** @brief Indices indicating which element in the row is built next */
            
            /** @brief Considered layer to pick the top drift circle from*/
            std::size_t m_upperLayer{0};
            /** @brief Considered layer to pick the bottom drift circle from*/
            std::size_t m_lowerLayer{0}; 
            /** @brief Explicit hit to pick in the selected bottom layer */
            std::size_t m_lowerHitIndex{0};
            /** @brief Explicit hit to pick in the selected top layer */
            std::size_t m_upperHitIndex{0};
            /** @brief Explicit hit to pick in the selected top layer */
            std::size_t m_signComboIndex{0};

            /** @brief Cache of all solutions seen thus far */
            struct SeedSolution{
                /** @brief: Theta of the line */
                double tanTheta{0.};
                /** @brief Intersecpt of the line */
                double Y0{0.};
                /** @brief: Uncertainty on the slope*/
                double dTanTheta{0.};
                /** @brief: Uncertainty on the intercept */
                double dY0{0.};
                /** @brief Used hits in the seed */
                HitVec seedHits{};
                /** @brief Vector of radial signs of the valid hits */
                std::vector<int> solutionSigns{};
            };

            std::vector<SeedSolution> m_seenSolutions{};
            /** Counter on how many seeds have been generated */
            unsigned int m_nGenSeeds{0};
        
    };
}

#endif
