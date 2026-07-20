/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */
#include "TruthSegmentWriter.h"

#include "MuonTesterTree/GenericDecorBranch.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonReadoutGeometryR4/SpectrometerSector.h"

#include "Acts/Surfaces/PlaneSurface.hpp"
#include "Acts/Definitions/Units.hpp"
#include "Acts/Geometry/GeometryIdentifier.hpp"
#include "Acts/Surfaces/Surface.hpp"

using namespace Acts::UnitLiterals;
using namespace MuonVal;
using namespace MuonR4;
namespace MuonValR4{
    constexpr float inDeg(const float rad) {
        return rad / 1._degree;
   }

    StatusCode TruthSegmentWriter::initialize()  {
        ATH_CHECK(m_segmentKey.initialize());
        ATH_CHECK(m_truthLinkKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        ATH_CHECK(m_trackingGeometryTool.retrieve());

        m_truthTrks = std::make_unique<IParticleFourMomBranch>(m_tree, "Muons");
        m_truthTrks->addVariable<int>(-1, "truthOrigin");
        m_truthTrks->addVariable<int>(-1, "truthType");

        /// Link the truth segments to the truth partcle
        m_truthTrks->addVariable(
                std::make_unique<GenericPartDecorBranch<xAOD::TruthParticle,
                                                        std::vector<unsigned short>>>(m_tree.tree(), 
                std::format("{:}_truthSegLinks", m_truthTrks->name()), [&] (const xAOD::TruthParticle& p){
                    std::vector<unsigned short> idx{};
                    for (const xAOD::MuonSegment* truthSeg: getTruthSegments(p)){
                        idx.push_back(m_segmentBranches->push_back(*truthSeg));
                    }
                    return idx;
                }));
        /// Count the number of matched truth segments
        m_truthTrks->addVariable(
                std::make_unique<GenericPartDecorBranch<xAOD::TruthParticle, unsigned short>>(m_tree.tree(), 
                std::format("{:}_nTruthSegments", m_truthTrks->name()), [&] (const xAOD::TruthParticle& p) -> unsigned short {
                    return getTruthSegments(p).size();
                }));

        m_tree.addBranch(m_truthTrks);

        const std::string segName = "Segments";
        m_segmentBranches = std::make_unique<MuonPRDTest::SegmentVariables>(m_tree, m_segmentKey.key(),
                                                                            segName, msgLevel());
        
        m_segmentFrames = std::make_unique<MuonVal::CoordSystemsBranch>(m_tree, std::format("{:}_localToGlobal", segName));
        m_tree.addBranch(m_segmentFrames);
        m_segmentBranches->addVariable(std::make_unique<MuonVal::GenericAuxDecorationBranch<unsigned short>>(m_tree, 
                    std::format("{:}_truthLink", segName),[this](const SG::AuxElement* aux){
                    const auto* seg = static_cast<const xAOD::MuonSegment*>(aux);
                    const xAOD::TruthParticle* truthP = MuonR4::getTruthMatchedParticle(*seg);
                    m_truthTrks->push_back(truthP);
                    unsigned short linkIdx = m_truthTrks->find(truthP);                    
                    return linkIdx;
                }));

        m_segmentBranches->addVariable(std::make_unique<MuonVal::GenericAuxDecorationBranch<
                                                  std::vector<Acts::GeometryIdentifier::Value>>>(m_tree, 
                    std::format("{:}_hitGeoIds", segName),[this](const SG::AuxElement* aux){
                    const auto* seg = static_cast<const xAOD::MuonSegment*>(aux);
                    std::vector<Acts::GeometryIdentifier::Value> geoIds{};
                    for (const xAOD::MuonSimHit* hit : MuonR4::getMatchingSimHits(*seg)) {
                        geoIds.push_back(getSurface(hit->identify()).geometryId().value());
                    }
                    std::ranges::sort(geoIds);
                    return geoIds;
                }));
        
        m_segmentBranches->addVariable(std::make_unique<MuonVal::GenericAuxDecorationBranch<
                                                    std::vector<float>>>(m_tree, 
                    std::format("{:}_localSegPars", segName),[](const SG::AuxElement* aux){
                    using namespace MuonR4::SegmentFit;
                    const auto* seg = static_cast<const xAOD::MuonSegment*>(aux);
                    auto pars = MuonR4::SegmentFit::localSegmentPars(*seg);
                    std::vector<float> ret{pars.begin(), pars.end()};
                    ret[Acts::toUnderlying(ParamDefs::phi)] = inDeg(ret[Acts::toUnderlying(ParamDefs::phi)]);
                    ret[Acts::toUnderlying(ParamDefs::theta)] = inDeg(ret[Acts::toUnderlying(ParamDefs::theta)]);
                    return ret;
                }));
        
        m_tree.addBranch(m_segmentBranches);
        ATH_CHECK(m_tree.init(this));
        return StatusCode::SUCCESS;
    }
    StatusCode TruthSegmentWriter::execute(const EventContext& ctx) {
        m_eventId = ctx.eventID().event_number();
        const xAOD::MuonSegmentContainer* segments{nullptr};
        ATH_CHECK(SG::get(segments, m_segmentKey, ctx));
        const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();
        for (const xAOD::MuonSegment* segment: *segments) {
            m_segmentBranches->push_back(*segment);
            m_segmentFrames->push_back(getSurface(*segment).localToGlobalTransform(tgContext));
        }

        ATH_CHECK(m_tree.fill(ctx));
        return StatusCode::SUCCESS;
    }
    StatusCode TruthSegmentWriter::finalize()  {
        ATH_CHECK(m_tree.write());
        return StatusCode::SUCCESS;
    }
    const Acts::Surface& TruthSegmentWriter::getSurface(const Identifier& simHitId) const {
        const MuonGMR4::MuonReadoutElement* re = m_detMgr->getReadoutElement(simHitId);
        const IdentifierHash surfHash = re->detectorType() == ActsTrk::DetectorType::Mdt ?
                                        re->measurementHash(simHitId) : re->layerHash(simHitId);
        return re->surface(surfHash);
    }
    const Acts::Surface& TruthSegmentWriter::getSurface(const xAOD::MuonSegment& segment) const {
        return m_detMgr->getSectorEnvelope(segment.chamberIndex(), segment.sector(), 
                                           segment.etaIndex())->surface();
    }
}
