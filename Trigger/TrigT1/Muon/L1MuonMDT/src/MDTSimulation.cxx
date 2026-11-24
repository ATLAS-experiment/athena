/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MDTSimulation.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"
#include "MuonIdHelpers/MdtIdHelper.h"
#include "xAODMuonViews/ChamberViewer.h"
#include "StoreGate/WriteHandle.h"
#include "L1MuonMDTTools/RpcLayerRadii.h"
#include "L1MuonMDTTools/HitExtractionWindow.h"
#include "MuonStationIndex/MuonStationIndex.h"
#include "Acts/Utilities/AngleHelpers.hpp"
#include "Acts/Utilities/Helpers.hpp"
#include <cmath>
#include <limits>
#include <memory>
#include <vector>


namespace L1Muon {

  StatusCode MDTSimulation::initialize() {
    ATH_MSG_DEBUG("Initializing " << name() << "...");
    ATH_CHECK(m_mdtDriftCircleKey.initialize());
    ATH_CHECK(m_barrelCandidateKey.initialize());
    ATH_CHECK(m_regionSelector.retrieve());
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_geoCtxKey.initialize());
    ATH_CHECK(m_csfSegmentFinder.retrieve());
    ATH_CHECK(m_legendreSegmentFinder.retrieve());
    ATH_CHECK(m_ptEstimationTool.retrieve());
    ATH_CHECK(m_candInfoKey.initialize(SG::AllowEmpty));
    ATH_CHECK(m_calibTool.retrieve(EnableTool{!m_calibTool.empty()}));
    if (m_calibTool.empty()) {
      ATH_MSG_INFO("No CalibrationTool: the hit extraction has no time window");
    } else {
      ATH_MSG_INFO("Hit extraction time window from " << m_calibTool.typeAndName());
    }
    ATH_CHECK(detStore()->retrieve(m_detMgr));

    return StatusCode::SUCCESS;
  }

  StatusCode MDTSimulation::execute(const EventContext& ctx) const {

    const xAOD::L1RPCCandDataContainer* barrelCandidates{};
    ATH_CHECK(SG::get(barrelCandidates, m_barrelCandidateKey, ctx));


    const ActsTrk::GeometryContext* geoCtx{nullptr};
    ATH_CHECK(SG::get(geoCtx, m_geoCtxKey, ctx));

    const ActsTrk::GeometryContext& gctx = *geoCtx;
    std::array<float, 4> z_positions{}, r_positions{};

    /// Optional record of the intermediate results of every candidate
    std::unique_ptr<L1MDT::CandidateInfoContainer> candInfos{};
    if (!m_candInfoKey.empty()) candInfos = std::make_unique<L1MDT::CandidateInfoContainer>();

    for(const xAOD::L1RPCCandData_v1* cand : *barrelCandidates) {

      float m=0, b=0;
      float eta = (static_cast<float>(cand->l1Eta()) / static_cast<float>(xAOD::IL1CandData_v1::etaBitRange())) * (2.0f * xAOD::IL1CandData_v1::etaRange()) - xAOD::IL1CandData_v1::etaRange();
      float phi = (static_cast<float>(cand->l1Phi()) / static_cast<float>(xAOD::IL1CandData_v1::phiBitRange())) * xAOD::IL1CandData_v1::phiRange() - M_PI;

      if (!fitRPC(*cand, eta, phi, m, b, z_positions, r_positions)) {
        ATH_MSG_DEBUG("RPC fit failed for this candidate");
      continue;  // do NOT abort event
      }

      std::vector<const xAOD::MdtDriftCircle*> mdtHits;
      std::vector<Identifier> regSelChambers;
      std::array<std::vector<Identifier>, L1MDT::nStationSlots> stationChambers{};
      ATH_CHECK(collectMDTHits(ctx, gctx, eta, phi, mdtHits, m, b, regSelChambers, stationChambers));

      L1MDT::CandidateInfo* info{nullptr};
      if (candInfos) {
        info = &candInfos->emplace_back();
        info->seedIndex = cand->index();
        info->eta = eta;
        info->phi = phi;
        info->m = m;
        info->b = b;
        info->rpcZ = z_positions;
        info->rpcR = r_positions;
        info->regSelChambers = regSelChambers;
        info->stationChambers = stationChambers;
        info->heHalfWidth = m_heHalfWidth;
      }

      // Split hits by station
      std::vector<const xAOD::MdtDriftCircle*> biHits;
      std::vector<const xAOD::MdtDriftCircle*> bmHits;
      std::vector<const xAOD::MdtDriftCircle*> boHits;


      for (const xAOD::MdtDriftCircle* dc : mdtHits) {
        if (!dc) continue;

        const Identifier id = dc->identify();
        using enum Muon::MuonStationIndex::StIndex;
        const auto stationIndex = m_idHelperSvc->stationIndex(id);

        if (stationIndex == BI) {
          biHits.push_back(dc);
        } else if (stationIndex == BM) {
          bmHits.push_back(dc);
        } else if (stationIndex == BO) {
          boHits.push_back(dc);
        }
      }

      if (info) {
        using enum Muon::MuonStationIndex::LayerIndex;
        info->hits[Acts::toUnderlying(Inner)] = biHits;
        info->hits[Acts::toUnderlying(Middle)] = bmHits;
        info->hits[Acts::toUnderlying(Outer)] = boHits;
      }

      // Require at least two stations with enough hits
      int nStationsWithEnoughHits = 0;
      if (biHits.size() >= 3) ++nStationsWithEnoughHits;
      if (bmHits.size() >= 3) ++nStationsWithEnoughHits;
      if (boHits.size() >= 3) ++nStationsWithEnoughHits;

      if (nStationsWithEnoughHits < 2) {
        ATH_MSG_DEBUG("Bad candidate: fewer than 2 stations with enough MDT hits");
        continue;
      } else {
        ATH_MSG_DEBUG("Good candidate");
      }

      // Run the segment finder independently for each station
      std::vector<L1MDT::Segment> biSegmentsCSF;
      std::vector<L1MDT::Segment> bmSegmentsCSF;
      std::vector<L1MDT::Segment> boSegmentsCSF;

      std::vector<L1MDT::Segment> biSegmentsLEG;
      std::vector<L1MDT::Segment> bmSegmentsLEG;
      std::vector<L1MDT::Segment> boSegmentsLEG;


      if (biHits.size() >= 2) {
        ATH_CHECK(m_csfSegmentFinder->findSegments(biHits, gctx, m, b, biSegmentsCSF));
        ATH_CHECK(m_legendreSegmentFinder->findSegments(biHits, gctx, m, b, biSegmentsLEG));
      }
      if (bmHits.size() >= 2) {
        ATH_CHECK(m_csfSegmentFinder->findSegments(bmHits, gctx, m, b, bmSegmentsCSF));
        ATH_CHECK(m_legendreSegmentFinder->findSegments(bmHits, gctx, m, b, bmSegmentsLEG));
      }
      if (boHits.size() >= 2) {
        ATH_CHECK(m_csfSegmentFinder->findSegments(boHits, gctx, m, b, boSegmentsCSF));
        ATH_CHECK(m_legendreSegmentFinder->findSegments(boHits, gctx, m, b, boSegmentsLEG));
      }


      ATH_MSG_DEBUG("CSF segment summary: "
      << " BI=" << biSegmentsCSF.size()
      << " BM=" << bmSegmentsCSF.size()
      << " BO=" << boSegmentsCSF.size());


      ATH_MSG_DEBUG("LEG segment summary: "
      << " BI=" << biSegmentsLEG.size()
      << " BM=" << bmSegmentsLEG.size()
      << " BO=" << boSegmentsLEG.size());

      const L1MDT::Segment* biSeg = biSegmentsCSF.empty() ? nullptr : &biSegmentsCSF.front();
      const L1MDT::Segment* bmSeg = bmSegmentsCSF.empty() ? nullptr : &bmSegmentsCSF.front();
      const L1MDT::Segment* boSeg = boSegmentsCSF.empty() ? nullptr : &boSegmentsCSF.front();

      const auto ptResult = m_ptEstimationTool->estimatePt(biSeg, bmSeg, boSeg);

      if (ptResult) {
        ATH_MSG_DEBUG("pT estimate | "
                       << "nStations=" << ptResult->nStations
                       << " pt=" << ptResult->pt
                       << " deltaBeta=" << ptResult->deltaBeta
                       << " sagitta=" << ptResult->sagitta
                       << " leverArm=" << ptResult->leverArm);
      } else {
        ATH_MSG_DEBUG("pT estimate not available for this candidate");
      }

      if (info) {
        using enum Muon::MuonStationIndex::LayerIndex;
        info->processed = true;
        info->csfSegments[Acts::toUnderlying(Inner)] = std::move(biSegmentsCSF);
        info->csfSegments[Acts::toUnderlying(Middle)] = std::move(bmSegmentsCSF);
        info->csfSegments[Acts::toUnderlying(Outer)] = std::move(boSegmentsCSF);
        info->legendreSegments[Acts::toUnderlying(Inner)] = std::move(biSegmentsLEG);
        info->legendreSegments[Acts::toUnderlying(Middle)] = std::move(bmSegmentsLEG);
        info->legendreSegments[Acts::toUnderlying(Outer)] = std::move(boSegmentsLEG);
        info->ptEstimate = ptResult;
      }
    }

    if (candInfos) {
      SG::WriteHandle candInfoHandle{m_candInfoKey, ctx};
      ATH_CHECK(candInfoHandle.record(std::move(candInfos)));
    }

    return StatusCode::SUCCESS;
  }


  bool MDTSimulation::fitRPC(const xAOD::L1RPCCandData& cand,
                             float eta, float phi,
                             float& m, float& b,
                             std::array<float, 4>& z_positions,
                             std::array<float, 4>& r_positions) const {
    const float tanTheta = std::tan(Acts::AngleHelpers::thetaFromEta(eta));
    /// The z positions are stored as |z| with a 0 for the layers without hits
    const float zLsb = 2.f * xAOD::L1RPCCandData_v1::zPosRange() / xAOD::L1RPCCandData_v1::zPosBitRange();

    z_positions.fill(0.f);
    r_positions.fill(0.f);
    const int sector = m_sectorMapping.getSector(phi);
    std::vector<std::pair<float, float>> points{};  // (R, z)
    for (unsigned i = 0; i < 4; ++i) {
      const float zAbs = std::abs(static_cast<float>(cand.l1ZPos().at(i)) / static_cast<float>(xAOD::L1RPCCandData_v1::zPosBitRange()) * (2.0f * xAOD::L1RPCCandData_v1::zPosRange()) - xAOD::L1RPCCandData_v1::zPosRange());
      if (zAbs < zLsb) continue;
      const float z = std::copysign(zAbs, eta);
      /// The z positions are given at a fixed radius per layer (see L1Muon::RPCSimulation)
      const float r = L1MDT::rpcLayerRadius(sector, i);
      z_positions[i] = z;
      r_positions[i] = r;
      points.emplace_back(r, z);
    }
    if (points.empty()) return false;

    /// Fit z = a * R + c (R has the large lever arm), then convert to R = m * z + b
    float a{0.f}, c{0.f};
    if (points.size() == 1) {
      /// Single layer: line from the IP through the hit
      a = points.front().second / points.front().first;
    } else {
      float sR{0.f}, sZ{0.f}, sRR{0.f}, sRZ{0.f};
      for (const auto& [r, z] : points) {
        sR += r; sZ += z; sRR += r * r; sRZ += r * z;
      }
      const float n = points.size();
      const float den = n * sRR - sR * sR;
      if (den == 0.f) return false;
      a = (n * sRZ - sR * sZ) / den;
      c = (sZ - a * sR) / n;
    }
    /// Protect against a line exactly perpendicular to the beam
    constexpr float minA = 1.e-6f;
    if (std::abs(a) < minA) a = std::copysign(minA, a);
    m = 1.f / a;
    b = -c / a;
    ATH_MSG_DEBUG("RPC fit with " << points.size() << " layers: m = " << m << " b = " << b
                  << " (IP line m = " << tanTheta << ")");
    return true;
  }




  StatusCode MDTSimulation::collectMDTHits(const EventContext& ctx,  const ActsTrk::GeometryContext& gctx, float eta, float phi, std::vector<const xAOD::MdtDriftCircle*>& hits, float m, float b,
                                           std::vector<Identifier>& regSelChambers,
                                           std::array<std::vector<Identifier>, L1MDT::nStationSlots>& stationChambers) const {


    hits.clear();
    regSelChambers.clear();
    for (auto& chambers : stationChambers) chambers.clear();
    std::array<Identifier, L1MDT::nStationSlots> crossed{};

    const float dEta = 0.01f, dPhi = 0.01f;


    RoiDescriptor roi(eta, eta-dEta, eta+dEta,
                      phi, phi-dPhi, phi+dPhi);
    TrigRoiDescriptor trigROI(roi);


    auto regSel = m_regionSelector->lookup(ctx);
    if (!regSel) return StatusCode::FAILURE;

    /// The region selector returns MDT module hashes
    std::vector<IdentifierHash> hashList{};
    regSel->HashIDList(trigROI, hashList);
    ATH_MSG_DEBUG("found n chamber hashes " << hashList.size());
    regSelChambers.reserve(hashList.size());

    const xAOD::MdtDriftCircleContainer* driftCircles{};
    ATH_CHECK(SG::get(driftCircles, m_mdtDriftCircleKey, ctx));

    const MdtIdHelper& idh = m_idHelperSvc->mdtIdHelper();
    const IdContext modContext = idh.module_context();
    /// z of the candidate line R = m * z + b at the radius R
    auto zOnLine = [m, b](float R) { return m != 0.f ? (R - b) / m : 0.f; };
    const int sector = m_sectorMapping.getSector(phi);

    /// One chamber per station: the barrel chamber of the sector crossed by the candidate line, i.e.
    /// whose tube range contains the line in the first tube layer; otherwise the one with the line
    /// the fewest tubes outside of it, then the one with the center the closest to the line along z
    std::array<std::pair<int, float>, L1MDT::nStationSlots> best{};
    best.fill({std::numeric_limits<int>::max(), std::numeric_limits<float>::max()});
    for (const IdentifierHash& h : hashList) {
      Identifier mId;
      if (idh.get_id(h, mId, &modContext) != 0) continue;
      regSelChambers.push_back(mId);
      if (m_idHelperSvc->sector(mId) != sector) continue;
      using enum Muon::MuonStationIndex::StIndex;
      const auto stationIndex = m_idHelperSvc->stationIndex(mId);
      if (stationIndex != BI && stationIndex != BM && stationIndex != BO) continue;
      const unsigned slot = Muon::MuonStationIndex::toInt(Muon::MuonStationIndex::toLayerIndex(stationIndex));

      const auto mls = L1MDT::chamberMultilayers(idh, *m_detMgr, mId);
      if (!mls[0]) continue;
      const Amg::Vector3D c1 = mls[0]->center(gctx);
      const Amg::Vector3D c2 = mls[1] ? mls[1]->center(gctx) : c1;
      const float dzCenter = std::abs(zOnLine(0.5 * (c1.perp() + c2.perp())) - 0.5 * (c1.z() + c2.z()));
      const MuonGMR4::MdtReadoutElement* re = mls[0];
      int refTube{0};
      Amg::Vector3D refPos{};
      float pitch{0.f};
      if (!L1MDT::tubeGrid(*re, gctx, 1, refTube, refPos, pitch)) continue;
      const int center = static_cast<int>(std::lround((zOnLine(refPos.perp()) - refPos.z()) / pitch)) + refTube;
      const int nTubes = re->numTubesInLay();
      const int outside = std::max({0, 1 - center, center - nTubes});
      if (std::make_pair(outside, dzCenter) < best[slot]) {
        best[slot] = {outside, dzCenter};
        crossed[slot] = mId;
      }
    }

    /// Hit extraction window: m_heHalfWidth tubes around the line in every tube layer of the crossed
    /// chamber, continued into its neighbours along eta near the chamber edges (as the old L0MDT
    /// standalone hit extraction, which selects on the distance to the line in any chamber of the station)
    xAOD::ChamberViewer viewer{*driftCircles, m_idHelperSvc.get(), xAOD::ChamberView::Mode::Chamber};
    for (unsigned slot = 0; slot < L1MDT::nStationSlots; ++slot) {
      if (!crossed[slot].is_valid()) continue;
      std::vector<Identifier> chambers{crossed[slot]};
      for (const Identifier& n : L1MDT::etaNeighbourChambers(idh, *m_detMgr, crossed[slot])) chambers.push_back(n);
      for (const Identifier& chamberId : chambers) {
        /// Tube range [min, max] per (multilayer - 1, layer - 1)
        std::array<std::vector<std::pair<int, int>>, 2> window{};
        bool anyTube{false};
        const auto mls = L1MDT::chamberMultilayers(idh, *m_detMgr, chamberId);
        for (unsigned iml = 0; iml < 2; ++iml) {
          if (!mls[iml]) continue;
          window[iml].assign(mls[iml]->numLayers(), {1, 0});
          for (const L1MDT::LayerWindow& w : L1MDT::hitExtractionWindow(*mls[iml], gctx, m, b, m_heHalfWidth)) {
            window[iml][w.layer - 1] = {w.minTube, w.maxTube};
            anyTube = true;
          }
        }
        /// The neighbours are kept only if the window reaches them
        if (!anyTube) continue;
        stationChambers[slot].push_back(chamberId);
        ATH_MSG_DEBUG("hit extraction in chamber " << m_idHelperSvc->toString(chamberId));
        if (!viewer.loadView(chamberId)) continue;
        for (const xAOD::MdtDriftCircle* dc : viewer) {
          const Identifier id = dc->identify();
          const unsigned iml = idh.multilayer(id) - 1;
          const unsigned ilay = idh.tubeLayer(id) - 1;
          if (iml >= 2 || ilay >= window[iml].size()) continue;
          const auto [minTube, maxTube] = window[iml][ilay];
          const int tube = idh.tube(id);
          if (tube < minTube || tube > maxTube) continue;
          /// Drift time = tdc - t0 - TOF, inside the time window of the calibration tool
          if (!m_calibTool.empty()) {
            const MdtCalibOutput calibOut = m_calibTool->calibrate(ctx, MdtCalibInput{*dc, gctx});
            if (calibOut.status() != Muon::MdtStatusDriftTime) continue;
          }
          hits.push_back(dc);
        }
      }
      /// The crossed chamber always describes the station, even without any hit
      if (stationChambers[slot].empty() || stationChambers[slot].front() != crossed[slot]) {
        stationChambers[slot].insert(stationChambers[slot].begin(), crossed[slot]);
      }
    }
    return StatusCode::SUCCESS;
  }



} // namespace L1Muon
