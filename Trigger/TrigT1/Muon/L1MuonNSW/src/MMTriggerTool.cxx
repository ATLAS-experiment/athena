/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MMTriggerTool.h"
#include "MMT_Hit.h"
#include "MMT_Road.h"
#include "NSWEncoding.h"

#include "GaudiKernel/SystemOfUnits.h"
#include "StoreGate/ReadHandle.h"
#include "MuonDigitContainer/MmDigit.h"
#include "MuonReadoutGeometryR4/MmReadoutElement.h"
#include "Acts/Utilities/MathHelpers.hpp"

#include <algorithm>
#include <cmath>
#include <memory>

namespace {
  /// LHC bunch spacing, used to assign a digit to a bunch crossing from its strip time
  constexpr double kBunchSpacing = 25. * Gaudi::Units::ns;
}

namespace L1Muon::L1MM {

  StatusCode MMTriggerTool::initialize() {
    ATH_MSG_DEBUG("Initializing " << name() << "...");
    ATH_CHECK(m_keyMmDigit.initialize());
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(detStore()->retrieve(m_detMgr));

    m_diamond = std::make_unique<MMT_Diamond>(m_diamXthreshold, m_uv, m_diamUVthreshold, m_diamRoadSize,
                                              m_diamOverlapEtaUp, m_diamOverlapEtaDown,
                                              m_diamOverlapStereoUp, m_diamOverlapStereoDown);
    return StatusCode::SUCCESS;
  }

  StatusCode MMTriggerTool::runTrigger(const EventContext& ctx, std::vector<MMCandidate>& candidates,
                                       MMDebugInfo* debug) const {
    candidates.clear();
    if (debug) {
      debug->hits.clear();
      debug->nTrig.clear();
    }

    const MmDigitContainer* mmDigits{nullptr};
    ATH_CHECK(SG::get(mmDigits, m_keyMmDigit, ctx));
    const ActsTrk::GeometryContext* gctx{nullptr};
    ATH_CHECK(SG::get(gctx, m_geoCtxKey, ctx));

    const MmIdHelper& idHelper = m_idHelperSvc->mmIdHelper();

    unsigned int nCollections{0}, nHitsTotal{0};
    for (const MmDigitCollection* digitCollection : *mmDigits) {

      std::vector<MMT_Hit> ev_hits;
      double tanStereoAngle{0.};  // tan of the U/V stereo angle, from the geometry of the stereo layers
      unsigned int nOutOfRange{0}, nNegTime{0}, nInfSlope{0};
      Identifier firstHitId{};  // any accepted hit of the collection, to get the sector centre from the geometry

      for (const MmDigit* digit : *digitCollection) {
        const Identifier id = digit->identify();
        if (!m_idHelperSvc->isMM(id)) continue;

        // The Run 3 DCS / data-quality selection for real data is not ported: MC only for now.

        const int channel = idHelper.channel(id);

        // Strip-range check. Run 3: 1 <= channel <= totalStrips. Phase II: the active
        // strips of the layer, [firstStrip, firstStrip + numStrips). Mandatory here, since
        // the Phase-II strip position of an out-of-range strip is undefined.
        const MuonGMR4::MmReadoutElement* readout = m_detMgr->getMmReadoutElement(id);
        const IdentifierHash layHash = readout->layerHash(id);
        const int firstStrip = readout->firstStrip(layHash);
        const int numStrips = readout->numStrips(layHash);
        const MuonGMR4::StripDesign& design = readout->stripLayer(layHash).design();
        // X layers have no stereo angle (0): keep the value of the stereo layers of the collection
        tanStereoAngle = std::max(tanStereoAngle, std::tan(std::abs(design.stereoAngle())));
        if (channel < firstStrip || channel >= firstStrip + numStrips) {
          ++nOutOfRange;
          ATH_MSG_DEBUG("Strip out of active range: " << m_idHelperSvc->toString(id)
                        << " allowed [" << firstStrip << ", " << firstStrip + numStrips - 1 << "]");
          continue;
        }

        const float stripTime = digit->stripResponseTime();
        // Checking positive digitization time
        if (stripTime < 0.) {
          ++nNegTime;
          continue;
        }
        const int BC = static_cast<int>(std::ceil(stripTime / kBunchSpacing));
        ev_hits.emplace_back(id, stripTime, BC, m_detMgr, *gctx);

        if (ev_hits.back().infSlope()) {
          ATH_MSG_WARNING("Infinite slope, removing hit");
          ++nInfSlope;
          ev_hits.pop_back();
          continue;
        }
        if (!firstHitId.is_valid()) firstHitId = id;

        // Every accepted hit, also in collections without coincidence (as the Run 3 ntuple)
        if (debug) {
          const MMT_Hit& h = ev_hits.back();
          MMHitInfo& info = debug->hits.emplace_back();
          info.stationEta = h.getStationEta();
          info.stationPhi = h.getStationPhi();
          info.sectorPhi = h.getSectorPhi();
          info.sector = h.getSector();
          info.plane = h.getPlane();
          info.strip = h.getChannel();
          info.VMM = h.getVMM();
          info.ART = h.getART();
          info.BC = h.getBC();
          info.time = h.getTime();
          info.R = h.getR();
          info.Z = h.getZ();
          info.RZslope = h.getRZSlope();
          info.shift = h.getShift();
        }
      }

      if (ev_hits.empty() && nOutOfRange == 0) continue;
      ++nCollections;
      nHitsTotal += ev_hits.size();

      ATH_MSG_DEBUG("MM station " << m_idHelperSvc->toStringChamber((*digitCollection->begin())->identify())
                    << ": " << ev_hits.size() << " hits"
                    << " (dropped: out-of-range " << nOutOfRange << ", negative time " << nNegTime
                    << ", inf slope " << nInfSlope << ")");

      // Go ahead when hits are more than X+UV thresholds (as in Run 3)
      if (ev_hits.size() < (m_diamond->getXthreshold() + m_diamond->getUVthreshold())) {
        ATH_MSG_DEBUG("Available hits are " << ev_hits.size()
                      << ", less than X+UV threshold, skipping digit collection");
        continue;
      }

      const bool isLarge = std::ranges::all_of(ev_hits, [](const auto& hit) { return hit.getSector() == 'L'; });
      const SideType side = std::ranges::all_of(ev_hits, [](const auto& hit) { return hit.getStationEta() < 0; })
                            ? SideType::C : SideType::A;
      const int sectorPhi = ev_hits[0].getSectorPhi();
      const bool allSectorPhi = std::ranges::all_of(ev_hits,
                                                    [&](const auto& hit) { return hit.getSectorPhi() == sectorPhi; });
      if (!allSectorPhi) {
        ATH_MSG_ERROR("Available digits belongs to different sectors IDs, unable to assign an unique ID");
        return StatusCode::FAILURE;
      }
      const bool isEta1 = std::ranges::all_of(ev_hits,
                                              [](const auto& hit) { return std::abs(hit.getStationEta()) == 1; });

      // Setup roads
      std::vector<MMT_Road> ev_roads;
      m_diamond->createRoads(ev_roads, isLarge, isEta1);

      // Evaluate coincidences
      std::vector<slope_t> diamondSlopes;
      m_diamond->findDiamonds(ev_hits, ev_roads, diamondSlopes, sectorPhi, tanStereoAngle);
      ATH_MSG_DEBUG("  -> " << diamondSlopes.size() << " coincidence(s) in sector " << sectorPhi << side);
      if (diamondSlopes.empty()) continue;
      if (debug) debug->nTrig.push_back(diamondSlopes.size());

      // Sector centre phi from the Phase-II geometry: the readout element is a trapezoid centred on
      // the sector axis. It is used for the global phi instead of MMT_Diamond::phiShift(sectorPhi, ...),
      // which places sector n at n*pi/8 while the Athena sectors (1-16) are centred at (n-1)*pi/8.
      // The Run 3 output carries the local phi, so there the offset only affects MM_diamond_phiShf.
      const MuonGMR4::MmReadoutElement* stationEl = m_detMgr->getMmReadoutElement(firstHitId);
      const double sectorCentrePhi = stationEl->center(*gctx).phi();

      for (const slope_t& slope : diamondSlopes) {
        MMCandidate& cand = candidates.emplace_back();
        cand.sectorPhi = sectorPhi;
        cand.side = side;
        cand.BC = slope.BC;
        cand.eta = slope.eta;
        cand.phi = slope.phi;
        cand.phiShf = slope.phiShf;
        // Same local-phi sign convention as Run 3 phiShift: flipped on side A
        cand.phiGlobal = NSWEncoding::wrapTwoPi(sectorCentrePhi - Acts::copySign(slope.phi, slope.side));
        cand.dtheta = slope.dtheta;
        cand.lowRes = slope.lowRes;
        cand.iX = slope.iRoad;
        cand.iU = slope.iRoadu;
        cand.iV = slope.iRoadv;
        cand.nHits = slope.totalCount;
        // Rest of the Run 3 slope_t, for the validation ntuple
        cand.sector = isLarge ? 'L' : 'S';
        cand.xCount = slope.xCount;
        cand.uCount = slope.uCount;
        cand.age = slope.age;
        cand.mx = slope.mx;
        cand.my = slope.my;
        cand.mxl = slope.mxl;
        cand.uavg = slope.uavg;
        cand.vavg = slope.vavg;
        cand.theta = slope.theta;
        cand.secCentrePhi = sectorCentrePhi;
      }
    }

    ATH_MSG_DEBUG("MM trigger: " << nHitsTotal << " hits in " << nCollections
                  << " digit collections -> " << candidates.size() << " candidates");
    return StatusCode::SUCCESS;
  }

}  // namespace L1Muon::L1MM
