/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "StripRadDamageLUTTool.h"

#include "InDetIdentifier/SCT_ID.h"
#include "PathResolver/PathResolver.h"

#include "TFile.h"
#include "TKey.h"

#include <algorithm>
#include <cmath>
#include <regex>

namespace ITk {


// Load every "CCE2D_Lumi<N>fb_<region>" TH2 in the file
StatusCode StripRadDamageLUTTool::loadFile(const std::string& fileName, LumiTableMap& tables) const {
  const std::string path = PathResolverFindCalibFile(fileName);
  if (path.empty()) {
    ATH_MSG_FATAL("Could not resolve LUT file " << fileName);
    return StatusCode::FAILURE;
  }
  std::unique_ptr<TFile> file(TFile::Open(path.c_str(), "READ"));
  if (!file || file->IsZombie()) {
    ATH_MSG_FATAL("Could not open LUT file " << path);
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Loading ITk Strip 2D CCE tables from " << path);

  // Histogram naming convention is CCE2D_Lumi<N>fb_<region>
  static const std::regex pattern("^CCE2D_Lumi([0-9]+(?:\\.[0-9]+)?)fb_(.+)$");

  for (auto* obj : *file->GetListOfKeys()) {
    TKey* key = static_cast<TKey*>(obj);
    std::smatch nameMatch;   // [luminosity in fb-1, region name]
    const std::string histName = key->GetName();
    if (!std::regex_match(histName, nameMatch, pattern)) continue;
    std::unique_ptr<TObject> readObj(key->ReadObj());
    const TH2* hist = dynamic_cast<const TH2*>(readObj.get());
    if (!hist) continue;

    // Own a TH2F copy so the TFile can be closed and ROOT I/O never touches the histogram again
    auto hist_copy = std::make_unique<TH2F>();
    hist->Copy(*hist_copy);
    hist_copy->SetDirectory(nullptr);
    tables[std::stof(nameMatch[1].str())][nameMatch[2].str()] = std::move(hist_copy);
  }

  if (tables.empty()) {
    ATH_MSG_FATAL("No histograms matching CCE2D_Lumi<N>fb_<region> in " << path);
    return StatusCode::FAILURE;
  }
  std::string lumiList;
  for (const auto& [lumi, regions] : tables) lumiList += " " + std::to_string(static_cast<int>(lumi)) + "(" + std::to_string(regions.size()) + ")";
  ATH_MSG_DEBUG("  available luminosity points [fb-1] (regions):" << lumiList);
  return StatusCode::SUCCESS;
}

// ---------------------------------------------------------------------------
StatusCode StripRadDamageLUTTool::initialize() {
  // ITk software currently reuses SCT_ID 
  ATH_CHECK(detStore()->retrieve(m_sctID, "SCT_ID"));

  LumiTableMap all;
  ATH_CHECK(loadFile(m_lutFile, all));

  // Find the neighboring luminosity points
  std::vector<float> lumi_points;
  for (const auto& [lumi, regions] : all) lumi_points.push_back(lumi);   // std::map keys are already sorted
  const float constrained_lumi = std::clamp(m_lumi.value(), lumi_points.front(), lumi_points.back());
  auto upperPoint = std::lower_bound(lumi_points.begin(), lumi_points.end(), constrained_lumi);
  if (upperPoint == lumi_points.begin() && lumi_points.size() > 1) ++upperPoint;
  const float upperLumi = *upperPoint;
  const float lowerLumi = (lumi_points.size() > 1) ? *(upperPoint - 1) : upperLumi;
  // weight of the upper point for linear interpolation
  const float w = (upperLumi > lowerLumi) ? (constrained_lumi - lowerLumi) / (upperLumi - lowerLumi) : 0.f;
  ATH_MSG_INFO("Interpolating tables to " << constrained_lumi << " fb-1 between " << lowerLumi << " and " << upperLumi << " (w = " << w << ")");

  // Interpolate each region by blending h = (1-w)*h(lowerLumi) + w*h(upperLumi)
  auto& lo = all.at(lowerLumi);
  auto& up = all.at(upperLumi);
  std::map<std::string, std::unique_ptr<TH2F>> byRegion;
  for (auto& [region, h0] : lo) {
    auto it = up.find(region);
    if (it == up.end()) {
      ATH_MSG_FATAL("Region " << region << " present at " << lowerLumi << " fb-1 but missing at " << upperLumi << " fb-1");
      return StatusCode::FAILURE;
    }
    if (w > 0.f && !h0->Add(h0.get(), it->second.get(), 1.f - w, w)) {
      ATH_MSG_FATAL("Binning of " << region << " differs between " << lowerLumi << " and " << upperLumi << " fb-1");
      return StatusCode::FAILURE;
    }
    byRegion[region] = std::move(h0);
  }

  // Create hash table assigning every detector element (wafer) to its region's histogram
  std::map<std::string, int> regionToIndex;
  const unsigned int nHash = m_sctID->wafer_hash_max();
  m_hashToTable.assign(nHash, -1);

  for (unsigned int iHash = 0; iHash < nHash; ++iHash) {
    const Identifier id = m_sctID->wafer_id(IdentifierHash(iHash));
    const bool isBarrel = (m_sctID->barrel_ec(id) == 0);
    const int layerOrDisk = m_sctID->layer_disk(id);
    const int ring = isBarrel ? -1 : ringOfEtaModule(std::abs(m_sctID->eta_module(id)));
    if (!isBarrel && ring < 0) {
      ATH_MSG_FATAL("Unexpected endcap eta_module " << m_sctID->eta_module(id) << " for wafer hash " << iHash);
      return StatusCode::FAILURE;
    }
    const std::string regionName = isBarrel
        ? "Barrel_" + std::to_string(layerOrDisk)
        : "Endcap_D" + std::to_string(layerOrDisk) + "R" + std::to_string(ring);

    auto it = regionToIndex.find(regionName);
    if (it == regionToIndex.end()) {
      auto tab = byRegion.find(regionName);
      if (tab == byRegion.end()) {
        ATH_MSG_FATAL("No table for region " << regionName << " (wafer hash " << iHash << ")");
        return StatusCode::FAILURE;
      } else {
        const TH2F& hist = *tab->second;
        FlatTable table;
        ATH_CHECK(flattenTable(hist, regionName, table));
        m_tables.push_back(std::move(table));
        it = regionToIndex.emplace(regionName, static_cast<int>(m_tables.size()) - 1).first;
        ATH_MSG_DEBUG("  " << regionName << ": z in [" << hist.GetXaxis()->GetXmin() << ", " << hist.GetXaxis()->GetXmax()
                     << "] mm, dy in [" << hist.GetYaxis()->GetXmin() << ", " << hist.GetYaxis()->GetXmax() << "] mm");
      }
    }
    m_hashToTable[iHash] = it->second;
  }

  ATH_MSG_INFO("Assigned " << m_tables.size() << " distinct CCE tables to " << nHash << " wafers");
  return StatusCode::SUCCESS;
}

// Helper function to find ITk Strip endcap ring from eta_module
// Rows per ring: R0 4, R1 4, R2 2, R3 4, R4 2, R5 2, first row of each ring is eta 0, 4, 8, 10, 14, 16
int StripRadDamageLUTTool::ringOfEtaModule(int etaModule) {
  static constexpr int ringOf[18] = {0,0,0,0, 1,1,1,1, 2,2, 3,3,3,3, 4,4, 5,5};
  return (etaModule >= 0 && etaModule < 18) ? ringOf[etaModule] : -1;
}

// Copy one region's uniform-binned histogram into a flat array
StatusCode StripRadDamageLUTTool::flattenTable(const TH2F& hist, const std::string& regionName, FlatTable& table) const {
  const TAxis* zAxis = hist.GetXaxis();
  const TAxis* dyAxis = hist.GetYaxis();
  if (zAxis->GetXbins()->GetSize() != 0 || dyAxis->GetXbins()->GetSize() != 0) {
    ATH_MSG_FATAL("Table " << regionName << " has variable-width bins, which the fast lookup does not support");
    return StatusCode::FAILURE;
  }
  if (zAxis->GetNbins() < 2 || dyAxis->GetNbins() < 2) {
    ATH_MSG_FATAL("Table " << regionName << " needs at least two bins on each axis for interpolation");
    return StatusCode::FAILURE;
  }

  table.nZ = zAxis->GetNbins();
  table.nDy = dyAxis->GetNbins();
  table.zMin = zAxis->GetXmin();
  table.zBinWidth = zAxis->GetBinWidth(1);
  table.dyMin = dyAxis->GetXmin();
  table.dyBinWidth = dyAxis->GetBinWidth(1);
  table.zHalf = 0.5f * static_cast<float>(zAxis->GetXmax() - zAxis->GetXmin());
  table.maxOffset = static_cast<float>(std::max(std::abs(dyAxis->GetXmin()), std::abs(dyAxis->GetXmax())));

  // ROOT bins are 1-based while values are stored 0-based 
  table.values.resize(static_cast<std::size_t>(table.nZ) * table.nDy);
  for (int iZ = 0; iZ < table.nZ; ++iZ) {
    for (int iDy = 0; iDy < table.nDy; ++iDy) {
      table.values[static_cast<std::size_t>(iZ) * table.nDy + iDy] = hist.GetBinContent(iZ + 1, iDy + 1);
    }
  }
  return StatusCode::SUCCESS;
}

// Returns the LUT for given IdentifierHash
const StripRadDamageLUTTool::FlatTable* StripRadDamageLUTTool::tableFor(const IdentifierHash& hash) const {
  const unsigned int iHash = hash;
  if (iHash >= m_hashToTable.size() || m_hashToTable[iHash] < 0) return nullptr;
  return &m_tables[m_hashToTable[iHash]];
}

// Returns cce, interpolated bilinearly between bin centers
float StripRadDamageLUTTool::cce(const IdentifierHash& hash, float z, float dy) const {
  const FlatTable* table = tableFor(hash);
  if (!table) return 0.f;

  // Zero outside the table
  const double zMax = table->zMin + table->nZ * table->zBinWidth;
  const double dyMax = table->dyMin + table->nDy * table->dyBinWidth;
  if (z < table->zMin || z > zMax || dy < table->dyMin || dy > dyMax) return 0.f;

  // Position in units of bins from the first bin center, clamped to the bin-center range
  const double zPosition = std::clamp((z - table->zMin) / table->zBinWidth - 0.5, 0., table->nZ - 1.);
  const double dyPosition = std::clamp((dy - table->dyMin) / table->dyBinWidth - 0.5, 0., table->nDy - 1.);
  const int iZ = std::min(static_cast<int>(zPosition), table->nZ - 2);
  const int iDy = std::min(static_cast<int>(dyPosition), table->nDy - 2);
  const double zFraction = zPosition - iZ;
  const double dyFraction = dyPosition - iDy;

  // Bilinear interpolation between four surrounding bin centers
  const float* lowZRow = &table->values[static_cast<std::size_t>(iZ) * table->nDy + iDy];
  const float* highZRow = lowZRow + table->nDy;
  const double lowZValue = (1. - dyFraction) * lowZRow[0] + dyFraction * lowZRow[1];
  const double highZValue = (1. - dyFraction) * highZRow[0] + dyFraction * highZRow[1];
  return static_cast<float>((1. - zFraction) * lowZValue + zFraction * highZValue);
}

// Return max deposit depth (z-axis) of LUT, which uses readout surface at +zHalf, backplane at -zHalf.
float StripRadDamageLUTTool::zHalfRange(const IdentifierHash& hash) const {
  const FlatTable* table = tableFor(hash);
  return table ? table->zHalf : 0.f;
}

// Return max |dy| of LUT
float StripRadDamageLUTTool::maxOffset(const IdentifierHash& hash) const {
  const FlatTable* table = tableFor(hash);
  return table ? table->maxOffset : 0.f;
}

} // namespace ITk
