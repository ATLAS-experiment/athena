/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef L1MUONMDTTOOLS_HITEXTRACTIONWINDOW_H
#define L1MUONMDTTOOLS_HITEXTRACTIONWINDOW_H

#include "ActsGeometryInterfaces/GeometryContext.h"
#include "Identifier/Identifier.h"
#include "MuonIdHelpers/MdtIdHelper.h"
#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace L1Muon::L1MDT {

  /// Hit extraction window of one tube layer of a multilayer: tubes [minTube, maxTube]
  struct LayerWindow {
    const MuonGMR4::MdtReadoutElement* re{nullptr};
    unsigned layer{0};  ///< tube layer in the multilayer [1, numLayers]
    unsigned minTube{0};
    unsigned maxTube{0};
  };

  /// Reference of the tube grid of a tube layer: the first existing tube (BMG chambers have
  /// cut-outs) gives the position of `refTube`, the z step per tube is the tube pitch, signed by the
  /// direction of increasing tube number along z (given by the next existing tube): negative on the
  /// z < 0 side. False if not found.
  inline bool tubeGrid(const MuonGMR4::MdtReadoutElement& re, const ActsTrk::GeometryContext& gctx,
                       unsigned layer, int& refTube, Amg::Vector3D& refPos, float& dzPerTube) {
    const int nTubes = re.numTubesInLay();
    int first{0};
    for (int tube = 1; tube <= nTubes; ++tube) {
      if (!re.isValid(re.measurementHash(layer, tube))) continue;
      if (!first) {
        first = tube;
        continue;
      }
      refTube = first;
      refPos = re.globalTubePos(gctx, re.measurementHash(layer, first));
      const float dz = re.globalTubePos(gctx, re.measurementHash(layer, tube)).z() - refPos.z();
      dzPerTube = std::copysign(static_cast<float>(re.tubePitch()), dz);
      return dz != 0.f && dzPerTube != 0.f;
    }
    return false;
  }

  /// Window of halfWidth tubes on each side of the tube closest to the line R = m * z + b, in every
  /// tube layer of the multilayer. The closest tube is extrapolated with the tube pitch also when the
  /// line passes outside of the multilayer, so that a window near the edge of the neighbouring
  /// chamber of the station continues into this one. Layers without any tube in the window are
  /// left out.
  inline std::vector<LayerWindow> hitExtractionWindow(const MuonGMR4::MdtReadoutElement& re,
                                                      const ActsTrk::GeometryContext& gctx,
                                                      float m, float b, int halfWidth) {
    std::vector<LayerWindow> res{};
    const int nTubes = re.numTubesInLay();
    for (unsigned lay = 1; lay <= re.numLayers(); ++lay) {
      int refTube{0};
      Amg::Vector3D refPos{};
      float dz{0.f};
      if (!tubeGrid(re, gctx, lay, refTube, refPos, dz)) continue;
      const float zPred = m != 0.f ? (refPos.perp() - b) / m : 0.f;
      const int center = static_cast<int>(std::lround((zPred - refPos.z()) / dz)) + refTube;
      const int lo = std::max(1, center - halfWidth);
      const int hi = std::min(nTubes, center + halfWidth);
      if (lo > hi) continue; // This can happen if the candidate line is below the first tube
      const unsigned minTube = lo, maxTube = hi;
      res.push_back({&re, lay, minTube, maxTube});
    }
    return res;
  }

  /// The two multilayers of a chamber (nullptr if absent)
  inline std::array<const MuonGMR4::MdtReadoutElement*, 2> chamberMultilayers(const MdtIdHelper& idh,
                                                                              const MuonGMR4::MuonDetectorManager& detMgr,
                                                                              const Identifier& chamberId) {
    std::array<const MuonGMR4::MdtReadoutElement*, 2> res{};
    for (int ml : {1, 2}) {
      bool valid{false};
      const Identifier tubeId = idh.channelID(chamberId, ml, 1, 1, valid);
      if (valid) res[ml - 1] = detMgr.getMdtReadoutElement(tubeId);
    }
    return res;
  }

  /// The chambers of the same station (name and phi) next to the chamber along eta, on the same side
  /// (stationEta +-1, including stationEta 0 if it exists)
  inline std::vector<Identifier> etaNeighbourChambers(const MdtIdHelper& idh,
                                                      const MuonGMR4::MuonDetectorManager& detMgr,
                                                      const Identifier& chamberId) {
    std::vector<Identifier> res{};
    const int stName = idh.stationName(chamberId);
    const int stEta = idh.stationEta(chamberId);
    const int stPhi = idh.stationPhi(chamberId);
    for (int eta : {stEta - 1, stEta + 1}) {
      /// Do not cross to the other side (no common tube numbering)
      if (eta != 0 && (eta > 0) != (stEta > 0) && stEta != 0) continue;
      bool valid{false};
      const Identifier id = idh.elementID(stName, eta, stPhi, valid);
      if (!valid) continue;
      if (!chamberMultilayers(idh, detMgr, id)[0]) continue;
      res.push_back(id);
    }
    return res;
  }

}  // namespace L1Muon::L1MDT

#endif
