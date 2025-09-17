/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONPATTERNHELPERS_MDTSEGMENTSEEDGENERATOR_H
#define MUONR4_MUONPATTERNHELPERS_MDTSEGMENTSEEDGENERATOR_H

#include <AthenaBaseComps/AthMessaging.h>
#include <ActsInterop/Logger.h>
#include <MuonSpacePoint/SpacePointPerLayerSplitter.h>
#include <MuonPatternEvent/SegmentSeed.h>

#include <vector>
#include <array>

namespace MuonR4{
    class ISpacePointCalibrator;
    class CalibratedSpacePoint;
}

namespace MuonR4::SegmentFit {
    /** @brief Helper class to generate valid seeds for the segment fit. The generator first returns a seed
     *         directly made from the patten recogntion. Afterwards it builds seeds by lying tangent lines
     *         to a pair of drift circles. The pairing starts from the innermost & outermost layers with tubes.
     *         A valid seed must have at least 4 associated hits which are within a chi2 of 5. If two seeds
     *         within the parameter resolution are generated, then the latter one is skipped. */
    class MdtSegmentSeedGenerator: public AthMessaging {
        public:
            using HitVec = SpacePointPerLayerSplitter::HitVec;
            /** @brief Configuration switches of the module  */
            struct Config{
                /** @brief Cut on the theta angle */
                std::array<double, 2> thetaRange{0, 180.*Gaudi::Units::deg};
                /** @brief Cut on the intercept range */
                std::array<double, 2> interceptRange{-20.*Gaudi::Units::m, 20.*Gaudi::Units::m};
                /** @brief Upper cut on the hit chi2 w.r.t. seed in order to be associated to the seed*/
                double hitPullCut{5.};
                /** @brief Try at the first time the pattern seed as candidate */
                bool startWithPattern{false};
                /** @brief How many drift circles may be on a layer to be used for seeding */
                unsigned int busyLayerLimit{2};
                /** @brief How many drift circle hits needs the seed to contain in order to be valid */
                unsigned int nMdtHitCut{3};
                /** @brief Hit cut based on the fraction of collected tube layers. 
                 *         The seed must pass the tighter of the two requirements.  */
                double nMdtLayHitCut{2./3.};
                /** @brief Once a seed with even more than initially required hits is found,
                 *         reject all following seeds with less hits */
                bool tightenHitCut{true};
                /** @brief Check whether a new seed candidate shares the same left-right solution with already accepted ones
                 *         Reject the seed if it has the same amount of hits */
                bool overlapCorridor{true};
                /** @brief Recalibrate the seed drift circles from the initial estimate  */
                bool recalibSeedCircles{false};
                /** @brief Pointer to the space point calibrator */
                const MuonR4::ISpacePointCalibrator* calibrator{nullptr};
                /** @brief Maximum number of iterations in the fast segment fit */
                unsigned int nMaxIter{100};
                /** @brief Precision cut off in the fast segment fit */
                double precCutOff{1.e-6};
            };
            /** @brief Helper struct from a generated Mdt seed */
            struct DriftCircleSeed{
                /** @brief Seed parameters */
                Parameters parameters{Parameters::Zero()};
                /** @brief List of calibrated measurements */
                std::vector<std::unique_ptr<CalibratedSpacePoint>> measurements{};
                /** @brief Iterations to obtain the seed */
                unsigned int nIter{0};
                /** @brief Seed chi2 */
                double chi2{0.};
                /** @brief Pointer to the parent bucket */
                const SpacePointBucket* parentBucket{nullptr};
                /** @brief number of Mdt hits on the seed */
                unsigned int nMdt{0};
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
        std::optional<DriftCircleSeed> nextSeed(const EventContext& ctx);
        /** @brief Returns how many seeds have been generated */
        unsigned int numGenerated() const;
        /** @brief Returns the current seed configuration */
        const Config& config() const;
        private:
            /** @brief Sign combinations to draw the 4 lines tangent to 2 drift circles
             *         The first two are indicating whether the tangent is left/right to the
             *         first/second circle. The last sign is picking the sign of the 
             *         solution arising from the final quadratic equation. */
            using SignComboType = std::array<int, 2>;
            constexpr static std::array<SignComboType,4> s_signCombos{
                std::array{ 1, 1}, std::array{ 1,-1}, 
                std::array{-1,-1}, std::array{-1, 1},  
            };
            /** @brief Cache of all solutions seen thus far */
            struct SeedSolution{
                /** @brief: Theta of the line */
                double theta{0.};
                /** @brief Intersecpt of the line */
                double y0{0.};
                /** @brief: Uncertainty on the slope*/
                double dTheta{0.};
                /** @brief: Uncertainty on the intercept */
                double dY0{0.};
                /** @brief Used hits in the seed */
                HitVec seedHits{};
                /** @brief Vector of radial signs of the valid hits */
                std::vector<int> solutionSigns{};
                /** @brief valid seed */
                bool isValid{true};
                /** @brief Outstream operator */
                friend std::ostream& operator<<(std::ostream& ostr, const SeedSolution& sol) {
                    return sol.print(ostr);
                }
                std::ostream& print(std::ostream& ostr) const;
            };
            /** @brief Estimate the line tangential to two space points for a given left/right pattern
             *  @param topHit: First drift circle space point
             *  @param bottomHit: Second drift cricle space point
             *  @param signs: Left/right ambiguity to the first & second space point */
            template <Acts::Experimental::CompositeSpacePoint SpacePoint_t>
                SeedSolution estimateTangentLine(const SpacePoint_t& topHit, const SpacePoint_t& bottomHit,
                                                 const SignComboType& signs) const;
            /** @brief Construct the 3D-Line parameters from the estimates theta & y0 from the tangent line
             *  @param theta: Tangent line theta in the y-z plane
             *  @param y0: Y intercept at z=0 */
            Line_t::ParamVector constructLinePars(const double theta, const double y0) const;
            /** @brief Tries to build the seed from the two hits. Fails if the solution is invalid
             *         or if the seed has already been built before
             *  @param topHit: Hit candidate from the upper layer
             *  @param bottomHit: Hit candidate from the lower layer
             *  @param sign: Object encoding whether the tangent is left / right  */
            std::optional<DriftCircleSeed> buildSeed(const EventContext& ctx,
                                                     const HoughHitType& topHit, 
                                                     const HoughHitType& bottomHit, 
                                                     const SignComboType& signs); 
            /** @brief Prepares the generator to generate the seed from the next pair of drift circles */
            void moveToNextCandidate();
            /** @brief Translate the SeedGenerator config to a Seed auxillary config */
            static SeedingAux::Config translate(const Config& cfg); 
            Config m_cfg{};

            /** @brief Line to instantiate the seed parameters */
            Line_t m_line{};
            const SegmentSeed* m_segmentSeed{nullptr};
            SpacePointPerLayerSplitter m_hitLayers{m_segmentSeed->getHitsInMax()};
            
            /** @brief Considered layer to pick the top drift circle from*/
            std::size_t m_upperLayer{0};
            /** @brief Considered layer to pick the bottom drift circle from*/
            std::size_t m_lowerLayer{0}; 
            /** @brief Explicit hit to pick in the selected bottom layer */
            std::size_t m_lowerHitIndex{0};
            /** @brief Explicit hit to pick in the selected top layer */
            std::size_t m_upperHitIndex{0};
            /** @brief Index of the left-right ambiguity between the circles */
            std::size_t m_signComboIndex{0};
            /** @brief Vector caching equivalent solutions to avoid double seeding */
            std::vector<SeedSolution> m_seenSolutions{};
            /** Counter on how many seeds have been generated */
            unsigned int m_nGenSeeds{0};
        
    };
}
#include <MuonPatternHelpers/MdtSegmentSeedGenerator.icc>

#endif
