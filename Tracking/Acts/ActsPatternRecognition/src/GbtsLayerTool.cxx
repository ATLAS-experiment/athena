/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "src/GbtsLayerTool.h"

#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"

#include <cmath>
#include <fstream>
#include <set>
#include <stdexcept>

namespace ActsTrk {

namespace {
/// Technology codes, as they enter the layer key. They order the layers, so
/// the pixel layers come before the strip ones.
constexpr int kPixel = 1;
constexpr int kStrip = 2;

/// The barrel's `barrel_ec` is 0, which would sort between the two endcaps.
/// Remap it so that a layer key orders barrel first, as the trigger tool did.
constexpr int kBarrelSideKey = -100;
}  // namespace

GbtsLayerTool::GbtsLayerTool(const std::string& type, const std::string& name,
                             const IInterface* parent)
    : base_class(type, name, parent) {}

StatusCode GbtsLayerTool::initialize() {
  ATH_MSG_DEBUG("Initializing " << name() << "...");

  ATH_CHECK(detStore()->retrieve(m_pixelId, "PixelID"));
  ATH_CHECK(detStore()->retrieve(m_stripId, "SCT_ID"));
  ATH_CHECK(detStore()->retrieve(m_pixelManager, "ITkPixel"));
  ATH_CHECK(detStore()->retrieve(m_stripManager, "ITkStrip"));

  ATH_CHECK(buildLayers());

  ATH_MSG_INFO("Built " << m_layerDescriptions.size() << " GBTS layers");

  return StatusCode::SUCCESS;
}

StatusCode GbtsLayerTool::buildLayers() {
  std::map<LayerKey, std::vector<ModuleEntry>> hashMap;

  // Pixel modules. The GBTS volume id follows the trigger convention:
  // 7 / 8 / 9 for the negative endcap, the barrel and the positive endcap,
  // refined by the layer or disk number.
  for (int hash = 0; hash < static_cast<int>(m_pixelId->wafer_hash_max());
       ++hash) {
    const Identifier offlineId = m_pixelId->wafer_id(hash);
    if (offlineId == 0) {
      continue;
    }

    const short barrelEc = m_pixelId->barrel_ec(offlineId);
    if (std::abs(barrelEc) > 2) {
      continue;  // no DBM needed
    }

    const short phiIndex = m_pixelId->phi_module(offlineId);
    const short etaIndex = m_pixelId->eta_module(offlineId);
    const int layerDisk = m_pixelId->layer_disk(offlineId);
    const int etaModule = m_pixelId->eta_module(offlineId);

    int volId = -1;
    if (barrelEc == 0) {
      volId = 8;
    } else if (barrelEc == -2) {
      volId = 7;
    } else if (barrelEc == 2) {
      volId = 9;
    }

    int newVol = 0;
    int newLay = 0;
    if (volId == 7 || volId == 9) {
      newVol = 10 * volId + layerDisk;
      newLay = etaModule;
    } else if (volId == 8) {
      newVol = 10 * volId + layerDisk;
      newLay = 0;
    }

    const LayerKey key{barrelEc == 0 ? kBarrelSideKey : barrelEc, kPixel,
                       static_cast<short>(newVol), static_cast<short>(newLay)};
    hashMap[key].push_back(ModuleEntry{phiIndex, etaIndex, hash});
  }

  // Strip modules. Volume 13 is the barrel, 12 and 14 the two endcaps.
  for (int hash = 0; hash < static_cast<int>(m_stripId->wafer_hash_max());
       ++hash) {
    const Identifier offlineId = m_stripId->wafer_id(hash);
    if (offlineId == 0) {
      continue;
    }

    const short barrelEc = m_stripId->barrel_ec(offlineId);
    const short phiIndex = m_stripId->phi_module(offlineId);
    const short etaIndex = m_stripId->eta_module(offlineId);

    int volId = 13;
    if (barrelEc != 0) {
      volId = 12;
    }
    if (barrelEc > 0) {
      volId = 14;
    }

    const int layerDisk = m_stripId->layer_disk(offlineId);

    const LayerKey key{barrelEc == 0 ? kBarrelSideKey : barrelEc, kStrip,
                       static_cast<short>(volId),
                       static_cast<short>(layerDisk)};
    hashMap[key].push_back(ModuleEntry{phiIndex, etaIndex, hash});
  }

  m_pixelLayers.assign(m_pixelId->wafer_hash_max(), kNoLayer);
  m_stripLayers.assign(m_stripId->wafer_hash_max(), kNoLayer);
  m_layerDescriptions.clear();
  m_layerDescriptions.reserve(hashMap.size());

  std::ofstream geometryStream;
  if (m_dumpGeometry) {
    geometryStream.open(m_geometryDumpDir);
  }

  // The layer id packs the volume and the layer into one integer, and the
  // connection table addresses a layer by it. It has room for the ITk
  // layouts in the release: the pixel endcaps reach disk 8, and their
  // volumes, 10 * 7 + disk, only run into the barrel's 80 at disk 10.
  // Nothing enforces that though, and two layers carrying the same id would
  // silently become one layer in GbtsGeometry.
  std::set<int> layerIds;

  short layerIndex = 0;
  for (const auto& [key, modules] : hashMap) {
    const auto& [sideKey, technology, volId, layId] = key;

    const short barrelEc = sideKey == kBarrelSideKey ? 0 : sideKey;
    const int combinedId = static_cast<int>(volId) * 1000 + layId;

    if (!layerIds.insert(combinedId).second) {
      ATH_MSG_ERROR("Two GBTS layers carry the id "
                    << combinedId << ": this detector does not fit the layer "
                    "id encoding");
      return StatusCode::FAILURE;
    }

    float refCoordSum = 0.f;
    int nModules = 0;
    double minZ = 100000.0;
    double maxZ = -100000.0;
    double minR = 100000.0;
    double maxR = -100000.0;

    for (const ModuleEntry& module : modules) {
      const InDetDD::SiDetectorElement* element = nullptr;
      if (technology == kPixel) {
        m_pixelLayers[module.hash] = layerIndex;
        element = m_pixelManager->getDetectorElement(module.hash);
      } else {
        m_stripLayers[module.hash] = layerIndex;
        element = m_stripManager->getDetectorElement(module.hash);
      }
      if (element == nullptr) [[unlikely]] {
        ATH_MSG_WARNING("SiDetectorElement pointer is null.");
        continue;
      }

      minZ = std::min(minZ, element->zMin());
      maxZ = std::max(maxZ, element->zMax());
      minR = std::min(minR, element->rMin());
      maxR = std::max(maxR, element->rMax());

      // The reference coordinate is the one that is constant across the
      // layer: the radius for a barrel layer, z for an endcap one.
      const Amg::Vector3D& centre = element->center();
      refCoordSum += barrelEc == 0
                         ? std::hypot(centre(0), centre(1))
                         : centre(2);
      ++nModules;
    }

    if (nModules == 0) [[unlikely]] {
      ATH_MSG_ERROR("GBTS layer " << combinedId << " has no modules.");
      return StatusCode::FAILURE;
    }

    Acts::Experimental::GbtsLayerDescription& layer =
        m_layerDescriptions.emplace_back();
    layer.id = combinedId;
    layer.type = barrelEc == 0 ? Acts::Experimental::GbtsLayerType::Barrel
                               : Acts::Experimental::GbtsLayerType::Endcap;
    layer.technology = technology == kPixel
                           ? Acts::Experimental::GbtsLayerTechnology::Pixel
                           : Acts::Experimental::GbtsLayerTechnology::Strip;
    layer.refCoord = refCoordSum / nModules;
    // The bounds span the coordinate the layer extends along.
    layer.minBound = barrelEc == 0 ? minZ : minR;
    layer.maxBound = barrelEc == 0 ? maxZ : maxR;

    if (m_dumpGeometry) {
      geometryStream << minR << " " << maxR << " " << minZ << " " << maxZ << " "
                     << combinedId << "\n";
    }

    ATH_MSG_DEBUG("Layer " << layerIndex << " (" << combinedId
                           << ") : reference coordinate = " << layer.refCoord
                           << " boundaries: " << layer.minBound << " "
                           << layer.maxBound << " type=" << barrelEc
                           << " technology="
                           << (technology == kPixel ? "pixel" : "strip"));

    ++layerIndex;
  }

  return StatusCode::SUCCESS;
}

}  // namespace ActsTrk
