/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONPATTERNHELPERS_MDTSEGMENTSEEDGENERATOR_H
#define MUONR4_MUONPATTERNHELPERS_MDTSEGMENTSEEDGENERATOR_H

#include <MuonSpacePoint/SpacePointPerLayerSplitter.h>
#include <MuonSpacePoint/CalibratedSpacePoint.h>
#include <MuonPatternEvent/Segment.h>

#include "Acts/Seeding/CompositeSpacePointLineSeeder.hpp"
namespace MuonR4{
    class ISpacePointCalibrator;
    class SegmentSeed;
}

namespace MuonR4::SegmentFit {
 
    /** @brief Helper struct to delegate the EDM interactions 
     *         with the space point container during the seeding 
     *         from Acts to the Athena EDM */
    struct SeederStateBase : public SpacePointPerLayerSplitter {
        protected:
            /** @brief Selector function to sort out seed lines that are incompatible
             *         with external geometrical constaints e.g. beampot */
            using SeedSelector_t  = std::function<bool(const Amg::Vector3D& pos,
                                                       const Amg::Vector3D& dir)>;
            /** @brief Protected constructor to prevent instantiation
              *        from anything else than the seeder state
              * @param parentSeed: Pointer to the HoughSeed from which all the 
              *                    Mdt segment seeds are constructed
              * @param calibrator: Pointer to the calibrator to be used to construct
              *                    the calibrated space points and (optionally)
              *                    to refine the pull calculation
              * @param calibratedPull: First calibrate the space point before calculating
              *                       the pull w.r.t. seed line 
              * @param seedSelector: Utility function to reject bad drift circle 
              *                      seeds based on geometrical constaints  */
            explicit SeederStateBase(const SegmentSeed* parentSeed,
                                     const ISpacePointCalibrator* calibrator,
                                     const bool calibratedPull,
                                     SeedSelector_t&& seedSelector = nullptr);

            /** @brief Constructor to initialize the seeder state with a predefined set of hits 
             * @param parentSeed: Pointer to the HoughSeed from which all the 
             *                    Mdt segment seeds are constructed
             * @param calibrator: Pointer to the calibrator to be used to construct
             *                    the calibrated space points and (optionally)
             *                    to refine the pull calculation
             * @param calibratePull: First calibrate the space point before calculating
             *                       the pull w.r.t. seed line 
             * @param hits: Predefined set of hits to initialize the seeder state with
             * @param seedSelector: Utility function to reject bad drift circle 
             *                      seeds based on geometrical constaints  */
            SeederStateBase(const SegmentSeed* parentSeed,
                            const ISpacePointCalibrator* calibrator,
                            bool calibratePull,
                            const SpacePointPerLayerSplitter::HitVec& hits,
                            SeedSelector_t&& seedSelector = nullptr);

        public:
            /** @brief Abrivation of the collection of calibrated space points */
            using CalibCont_t = Segment::MeasVec;
            /** @brief Returns the parent seed from which the state is constructed */
            const SegmentSeed* parent() const;
            /** @brief Returns whether the hit is a good candidate for seeding
             *  @param testMdt: Reference to the space point considered for seeding */
            bool goodCandidate(const SpacePoint& testMdt) const;
            /** @brief Returns the pull of the candidate w.r.t. the line & the time offset
                 *  @param cctx: Calibration context to access the conditions data
                 *  @param seedPos: Reference position of the seed line
                 *  @param seedDir: Reference direction of the seed line
                 *  @param t0: Offset in the time of arrival (Acts units)
                 *  @param candidate: Space point which chi2 is to be evaluated */
            double candidateChi2(const Acts::CalibrationContext& cctx,
                                    const Amg::Vector3D& seedPos,
                                    const Amg::Vector3D& seedDir,
                                    const double t0,
                                    const SpacePoint& candidate) const;
            /** @brief Returns the outer tube radius of the space point
             *  @param testMdt: Reference to the Mdt space point of interest */
            double strawRadius(const SpacePoint& testMdt) const;            
            /** @brief Creates a new candidate seed container
             *  @param cctx: Calibration context (defined by the interface) */
            CalibCont_t newContainer(const Acts::CalibrationContext& cctx) const;
            /** @brief Appends the space point measurement to the candidate seed container
             *         Optionally, the hit may be calibrated
             *  @param cctx: Calibration context to access the conditions data
             *  @param pos: Reference position of the seed line
             *  @param dir: Reference direction of the seed line
             *  @param t0: Offset in the time of arrival (Acts units)
             *  @param appendMe: Space point candidate to append to the container
             *  @param appendTo: Output container to which the space point is appended */       
            void append(const Acts::CalibrationContext& cctx,
                        const Amg::Vector3D& pos,
                        const Amg::Vector3D& dir,
                        const double t0,
                        const SpacePoint& appendMe,
                        CalibCont_t& appendTo) const;
            /** @brief Requests whether the seed line generation shall be stopped based on the 
                       pair of 
              * @param lowerLayer: Index of the lower hit layer from which the seed circles are picked
              * @param upperLayer: Index of the upper hit layer from which the seed circles are picked */
            bool stopSeeding(const std::size_t lowerLayer, 
                             const std::size_t upperLayer) const;

            /** @brief Selector function to remove the two line tangent
             *         seed using external constaints (e.g. beamspot)
             *  @param tangentPos: Position of the seed line 
             *  @param tangentDir: Direction of the seed line */
            bool goodForSeeding(const Amg::Vector3D& tangentPos,
                                const Amg::Vector3D& tangentDir) const;
        private:    
            const SegmentSeed* m_parent{};
            const ISpacePointCalibrator* m_calibrator{nullptr};
            bool m_calibratePull{false};
            const SeedSelector_t m_selector{nullptr};
    };
    static_assert(Acts::Experimental::detail::CompSpacePointSeederDelegate<SeederStateBase, 
                                                                            SeederStateBase::HitVec, 
                                                                            SeederStateBase::CalibCont_t>);

    static_assert(Acts::Experimental::detail::CompositeSpacePointSeedSelector<SeederStateBase>);
    /** @brief Helper class to generate valid seeds for the segment fit. The generator first returns a seed
     *         directly made from the patten recogntion. Afterwards it builds seeds by lying tangent lines
     *         to a pair of drift circles. The pairing starts from the innermost & outermost layers with tubes.
     *         A valid seed must have at least 4 associated hits which are within a chi2 of 5. If two seeds
     *         within the parameter resolution are generated, then the latter one is skipped. */
    
    class MdtSegmentSeedGenerator: public Acts::Experimental::CompositeSpacePointLineSeeder {
        public:
            using LineSeeder_t = Acts::Experimental::CompositeSpacePointLineSeeder;
            using HitVec_t = SpacePointPerLayerSplitter::HitVec;

            using CalibCont_t = SeederStateBase::CalibCont_t;
            /** @brief Copy over the constructor from the base class */
            using LineSeeder_t::LineSeeder_t;
            /** @brief Define the state holder object */
            using State_t = SeedingState<HitVec_t, CalibCont_t, SeederStateBase>;
            /** @brief Abrivation of the calibrated Seed type returned by the seeder */
            using Seed_t = SegmentSeed<CalibCont_t>;
    };
}
#endif
