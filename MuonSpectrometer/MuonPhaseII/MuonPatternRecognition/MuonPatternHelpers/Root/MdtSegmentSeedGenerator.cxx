/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPatternHelpers/MdtSegmentSeedGenerator.h>


#include <MuonRecToolInterfacesR4/ISpacePointCalibrator.h>
#include <MuonPatternEvent/SegmentSeed.h>
#include <xAODMuonPrepData/MdtDriftCircle.h>

#include <Acts/Utilities/Enumerate.hpp>
#include <Acts/Definitions/Units.hpp>
#include <Acts/Utilities/UnitVectors.hpp>

#include <format>

using namespace Acts;
using namespace Acts::UnitLiterals;
namespace MuonR4::SegmentFit{
    using HitVec = SpacePointPerLayerSplitter::HitVec;
    using HitLayerVec = SpacePointPerLayerSplitter::HitLayVec;

    SeederStateBase::SeederStateBase(const SegmentSeed* parentSeed,
                                    const ISpacePointCalibrator* calibrator,
                                    const bool calibratePull):
        SpacePointPerLayerSplitter{parentSeed->getHitsInMax()},
        m_parent{parentSeed}, 
        m_calibrator{calibrator}, 
        m_calibratePull{calibratePull} {}

    const SegmentSeed* SeederStateBase::parent() const { return m_parent; }

    double SeederStateBase::strawRadius(const SpacePoint& testMdt) const {
        assert(testMdt.type() == xAOD::UncalibMeasType::MdtDriftCircleType);
        return static_cast<const xAOD::MdtDriftCircle*>(testMdt.primaryMeasurement())
                ->readoutElement()->tubeRadius();
    }
    bool SeederStateBase::goodCandidate(const SpacePoint& testMdt) const {
        switch (testMdt.type()) {
            using enum xAOD::UncalibMeasType;
            case MdtDriftCircleType:
                return static_cast<const xAOD::MdtDriftCircle*>(testMdt.primaryMeasurement())->status() ==
                        Muon::MdtDriftCircleStatus::MdtStatusDriftTime;
            case RpcStripType:
            case TgcStripType:
                return true;
            default:
                return false;
        }
        return false;
    }
    double SeederStateBase::candidateChi2(const Acts::CalibrationContext& cctx,
                                            const Amg::Vector3D& seedPos,
                                            const Amg::Vector3D& seedDir,
                                            const double t0,
                                            const SpacePoint& candidate) const {
        if (m_calibratePull) {
            const EventContext& ctx{*cctx.get<const EventContext*>()};
            auto calibSp = m_calibrator->calibrate(ctx, &candidate, seedPos, seedDir, t0);
            assert(calibSp != nullptr);
            return SeedingAux::chi2Term(seedPos, seedDir, *calibSp);
        }
        return SeedingAux::chi2Term(seedPos, seedDir, candidate);
    }
    SeederStateBase::CalibCont_t 
        SeederStateBase::newContainer(const Acts::CalibrationContext& /*cctx*/) const {
        return CalibCont_t{};
    }
    void SeederStateBase::append(const Acts::CalibrationContext& cctx,
                                    const Amg::Vector3D& pos,
                                    const Amg::Vector3D& dir,
                                    const double t0,
                                    const SpacePoint& appendMe,
                                    CalibCont_t& appendTo) const {
        const EventContext& ctx{*cctx.get<const EventContext*>()};
        appendTo.push_back(m_calibrator->calibrate(ctx, &appendMe, pos, dir, t0));
    }
    bool SeederStateBase::stopSeeding(const std::size_t lowerLayer, 
                                        const std::size_t upperLayer) const {
        return lowerLayer >= firstLayerFrom2ndMl() || lowerLayer >= upperLayer;
    }
   
}
