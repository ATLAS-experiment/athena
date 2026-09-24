/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonTrackEvent/TrackingHelpers.h"

#include "MuonPatternEvent/MuonPatternContainer.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include "ActsEvent/Decoration.h"
#include "ActsCalibrators/xAODUncalibMeasCalibrator.h"
#include "FourMomUtils/xAODP4Helpers.h"

namespace{
  using PrdLink_t = ElementLink<xAOD::UncalibratedMeasurementContainer>;
  using PrdLinkVec_t = std::vector<PrdLink_t>;
  static const SG::ConstAccessor<PrdLinkVec_t> acc_prdLinks{"prdLinks"};
  static const SG::ConstAccessor<std::vector<char>> acc_prdState{"prdState"};
}

namespace MuonR4{

    std::string printID(const xAOD::MuonSegment& seg) {
        using namespace Muon::MuonStationIndex;
        return std::format("{:}{:}{:}{:}", chName(seg.chamberIndex()),
                                                  std::abs(seg.etaIndex()),
                                                  seg.etaIndex() > 0 ? 'A' : 'C',
                                                  seg.sector());
    }
    std::string printSegment(const xAOD::MuonSegment& seg) {
        std::ostringstream oss;
        using namespace SegmentFit;
        oss <<"Segment "<<printID(seg)
            <<", Dir theta/phi: "<<(seg.direction().theta() /Gaudi::Units::degree)
            <<" / "<<(seg.direction().phi() /Gaudi::Units::degree)
            <<", Pos theta/phi: "<<(seg.position().theta() / Gaudi::Units::degree)
            <<" / "<<(seg.position().phi() / Gaudi::Units::degree)
            <<", R: "<<Acts::fastHypot(seg.x(), seg.y()) <<", Z: "<<seg.z()
            <<", chi2: "<<(seg.chiSquared() / std::max(seg.numberDoF(), 1.f))
            <<", localPars: "<<toString(localSegmentPars(seg))
            <<", nPrec: "<<static_cast<int>(seg.nPrecisionHits())
            <<", nPhi: "<<static_cast<int>(seg.nPhiLayers())
            <<", nTrigEta: "<<static_cast<int>(seg.nTrigEtaLayers());
        return oss.str();
    }
    const Segment* detailedSegment(const xAOD::MuonSegment& seg) {
        using SegLink_t = ElementLink<SegmentContainer>;
        static const SG::ConstAccessor<SegLink_t> acc{"parentSegment"};
        if (acc.isAvailable(seg)){
            const SegLink_t& link{acc(seg)};
            if (link.isValid()){
                return *link;
            }
        }
        return nullptr;
    }
    
    std::size_t nMeasurements(const xAOD::MuonSegment& segment) {
      return acc_prdLinks.isAvailable(segment) ? acc_prdLinks(segment).size() : 0;
    }
    const xAOD::UncalibratedMeasurement* getMeasurement(const xAOD::MuonSegment& segment,
                                                        const std::size_t n) {
      if (!acc_prdLinks.isAvailable(segment)) {
          return nullptr;
      }
      assert(n < nMeasurements(segment));
      const PrdLink_t& link{acc_prdLinks(segment)[n]};
      return link.isValid() ? *link : nullptr;
    } 
    bool isOutlierMeasurement(const xAOD::MuonSegment& segment,
                              const std::size_t n) {
        if(!acc_prdState.isAvailable(segment)) {
            return false;
        }
        assert (n < acc_prdState(segment).size());
        return acc_prdState(segment)[n] != Acts::toUnderlying(CalibratedSpacePoint::State::Valid);
    }
    std::vector<const xAOD::UncalibratedMeasurement*> collectMeasurements(const xAOD::MuonSegment& segment,
                                                                          bool skipOutlier) {
        std::vector<const xAOD::UncalibratedMeasurement*> out{};
        const PrdLinkVec_t& links{acc_prdLinks(segment)};
        out.reserve(links.size());
        for (std::size_t l = 0 ; l < links.size(); ++l) {
            const PrdLink_t& link{links[l]};
            if (!skipOutlier || !isOutlierMeasurement(segment, l)) {
                out.push_back(*link);
            }
        }
        return out;
    }
    Acts::GeometryIdentifier volumeId(const Acts::Surface& surface) {
       return surface.geometryId().withSensitive(0).withBoundary(0);
    }

    const xAOD::UncalibratedMeasurement* firstMeasurement(const xAOD::MuonSegment& segment,
                                                          const bool skipOutlier) {
        const std::size_t n = nMeasurements(segment);
        for (std::size_t i = 0; i < n ; ++i) {
            if (!skipOutlier || !isOutlierMeasurement(segment, i)) {
                return getMeasurement(segment, i);
            }
        }
        return nullptr;
    }

    Amg::Vector3D atFirstSurface(const Acts::GeometryContext& gctx,
                                 const xAOD::MuonSegment& segment,
                                 const bool skipOutlier) {
        const xAOD::UncalibratedMeasurement* meas{firstMeasurement(segment, skipOutlier)};
        assert(meas != nullptr);
        const Acts::Surface& surface = xAOD::muonSurface(meas);

        const Acts::MultiIntersection isect = surface.intersect(gctx,
                                                                segment.position(),
                                                                segment.direction(),
                                                                Acts::BoundaryTolerance::Infinite());
        return isect.closest().position();
    }

    bool ParticleSorter::operator()(const xAOD::IParticle* a,
                                    const xAOD::IParticle* b) const {

        if (const float dPt = a->pt() - b->pt(); 
            std::abs(dPt) > std::numeric_limits<float>::epsilon()) {
            return dPt < 0.;
        }
        if (const float dEta = a->eta() - b->eta();
            std::abs(dEta) > std::numeric_limits<float>::epsilon()) {
            return dEta < 0.;
        }
        const float dPhi = xAOD::P4Helpers::deltaPhi(a, b);
        return dPhi < 0.;
    }
}