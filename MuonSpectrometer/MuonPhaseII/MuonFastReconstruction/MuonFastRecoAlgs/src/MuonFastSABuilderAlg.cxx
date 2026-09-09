/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonFastSABuilderAlg.h"

#include "MuonTrackEvent/MsTrackSeed.h"
#include "MuonFastRecoEvent/FastRecoUtils.h"
#include "Acts/Utilities/VectorHelpers.hpp"

namespace {
    /** Convert angle from radians to degrees */
    double inDeg(double angle) {
        return angle / Gaudi::Units::deg;
    }
    /** Print muon information */
    std::string print(const xAOD::Muon& muon) {
        std::ostringstream oss;
        using namespace MuonR4::FastReco;
        const double P {muon.p4().P()};
        const double momUncertainty {std::sqrt(getQOverPCov(muon)*std::pow(P,4))};
        oss<<"Eta: "<<muon.eta()<<", Phi: "<<inDeg(muon.phi())
           <<", Pt: "<<muon.pt()/Gaudi::Units::GeV<<" GeV, Charge: "<<muon.charge()
           <<", P: "<<P/ Gaudi::Units::GeV<<" += "<<momUncertainty/Gaudi::Units::GeV
           <<" Gev"<<std::endl<<"Associated "<< *getParentPattern(muon);
        return oss.str();
    }
}

namespace MuonR4 {
using namespace Acts::UnitLiterals;
using namespace Muon::MuonStationIndex;

StatusCode MuonFastSABuilderAlg::initialize() {
    ATH_CHECK(m_outMuons.initialize());
    ATH_CHECK(m_qOverPCovKey.initialize());
    ATH_CHECK(m_inSegments.initialize());
    ATH_CHECK(m_seedingTool.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode MuonFastSABuilderAlg::execute(const EventContext& ctx) const {
    ATH_MSG_VERBOSE(__func__<<"() Start building muon candidates");

    const xAOD::MuonSegmentContainer* inSegments{nullptr};
    ATH_CHECK(SG::get(inSegments, m_inSegments, ctx));

    MuonDataShip dataShip{};
    ATH_CHECK(dataShip.muonContainer.record(m_outMuons, ctx));
    dataShip.dec_qOverPCov.initialize(m_qOverPCovKey, ctx);

    /** The segments are already grouped by their associated global patterns */
    auto firstSeg = inSegments->stdcont().begin();
    while(firstSeg != inSegments->stdcont().end()) {
        const GlobalPattern* pattern {FastReco::getParentPattern(**firstSeg)};
        assert(pattern);

        const auto nextToLastSeg {std::find_if(firstSeg, inSegments->stdcont().end(),
            [pattern](const xAOD::MuonSegment* seg) {
                return FastReco::getParentPattern(*seg) != pattern;
            })};
        const std::span<const xAOD::MuonSegment* const> muonSegments {
            firstSeg, 
            static_cast<std::size_t>(nextToLastSeg-firstSeg)
        };
        xAOD::Muon* newMuon {buildMuonCandidate(ctx, *pattern, muonSegments, dataShip)};
        if (!newMuon) {
            ATH_MSG_DEBUG(__func__<<"() No muon candidate could be built from pattern " << *pattern);
            firstSeg = nextToLastSeg;
            continue;
        }
        /** Link the segments to the new muon */
        std::vector<ElementLink<xAOD::MuonSegmentContainer>> segLinks{};
        for (const xAOD::MuonSegment* seg : muonSegments) {
            segLinks.emplace_back(*inSegments, seg->index());
        }
        newMuon->setMuonSegmentLinks(segLinks);
        ATH_MSG_DEBUG(__func__<<"() Built new muon candidate: " << print(*newMuon));
        firstSeg = nextToLastSeg;
    }
    ATH_MSG_DEBUG("Written "<<dataShip.muonContainer->size()<<" FastMuonSA into StoreGate.");
    return StatusCode::SUCCESS;
}

xAOD::Muon* MuonFastSABuilderAlg::buildMuonCandidate(const EventContext& ctx,
                                                     const GlobalPattern& pattern,
                                                     std::span<const xAOD::MuonSegment* const> segments,
                                                     MuonDataShip& outMuonData) const {

    ATH_MSG_VERBOSE(__func__<<"() Starting muon building from "
        << segments.size()<<" segments for pattern " << std::endl << pattern);

    if (segments.size() < 2) {
        ATH_MSG_DEBUG(__func__<<"() Not enough muon segments to construct a candidate - abort.");
        return nullptr;
    }

    /** Count the barrel stations to classify the muon */
    const std::vector<StIndex> stations {pattern.getStations()};
    const unsigned nBarrelStations = std::ranges::count_if(stations, 
        [](const StIndex station) { return isBarrel(station); });
    
    /** Build a track seed. Setting the seed position is not required to estimate the momentum */
    using enum MsTrackSeed::Location;
    MsTrackSeed trackSeed {nBarrelStations * 2u >= stations.size() ? Barrel : Endcap, 
                           pattern.expSector()};
    for (const xAOD::MuonSegment* seg : segments) {
        trackSeed.addSegment(seg);
    }
    ATH_MSG_VERBOSE(__func__<<"() Constructed track seed: " << trackSeed);

    Acts::Result<Acts::BoundTrackParameters> seedPars {
        m_seedingTool->estimateStartParameters(ctx, trackSeed)};

    if (!seedPars.ok()) {
        ATH_MSG_DEBUG(__func__<<"() Failed to estimate seed parameters for track seed: " << trackSeed);
        return nullptr;
    }

    xAOD::Muon* newMuon = outMuonData.muonContainer->push_back(std::make_unique<xAOD::Muon>());
    newMuon->setAuthor(xAOD::Muon::Author::MuidSA);
    newMuon->setMuonType(xAOD::Muon::MuonType::MuonStandAlone);
    newMuon->setCharge(seedPars->charge() > 0. ? 1 : -1);

    newMuon->setP4(ActsTrk::energyToAthena(seedPars->transverseMomentum()), 
                   Acts::VectorHelpers::eta(*seedPars),seedPars->phi());
    
    /** Set the Q/P covariance */
    std::optional<Acts::BoundMatrix> cov {seedPars->covariance()};
    float& qOverPCov {outMuonData.dec_qOverPCov(*newMuon)};
    if (!cov || (*cov)(Acts::eBoundQOverP, Acts::eBoundQOverP) < Acts::s_epsilon) {
        ATH_MSG_WARNING(__func__<<"() No covariance matrix available for seed parameters of track seed: " << trackSeed);
        qOverPCov = 0.;
        return newMuon;
    }
    /** Covariance in athena units */
    static constexpr double conversionFactor {1 / Acts::square(ActsTrk::energyToAthena(1.))};
    qOverPCov = (*cov)(Acts::eBoundQOverP, Acts::eBoundQOverP) * conversionFactor;
    return newMuon;
}
}