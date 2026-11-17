// -*- C++ -*-
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef STRIPDIGITIZATION_STRIPRADDAMAGELUTTOOL_H
#define STRIPDIGITIZATION_STRIPRADDAMAGELUTTOOL_H

#include "IStripRadDamageLUTTool.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "Identifier/IdentifierHash.h"

#include "TH2F.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

class SCT_ID;

namespace ITk {

/**
 * @class StripRadDamageLUTTool
 * @brief Implements radiation damage in ITk Strip sensors using a look-up table (LUT) of 
 *        charge collection efficiency vs 2D position (TH2F, x = depth z, y = offset dy).
 *
 * The tool parses the LUT histogram names to identify available luminosity points and 
 * interpolates to the requested IntegratedLuminosity with a single TH2F per detector region. 
 */
class StripRadDamageLUTTool : public extends<AthAlgTool, IStripRadDamageLUTTool> {
 public:
  using base_class::base_class;
  virtual ~StripRadDamageLUTTool() = default;

  virtual StatusCode initialize() override;

  virtual float cce(const IdentifierHash& hash, float z, float dy) const override;
  virtual float zHalfRange(const IdentifierHash& hash) const override;
  virtual float maxOffset(const IdentifierHash& hash) const override;
  virtual float chargeSmear() const override { return m_chargeSmear; }

 private:
  // Histograms stored in tables[luminosity][region name]
  using LumiTableMap = std::map<float, std::map<std::string, std::unique_ptr<TH2F>>>;

  // Read every "CCE2D_Lumi<N>fb_<region>" TH2 in the file
  StatusCode loadFile(const std::string& fileName, LumiTableMap& tables) const;

  // Get endcap ring from the etaModule
  static int ringOfEtaModule(int etaModule);

  /// For each region, create a flat array from a LUT histogram for quicker bilinear interpolation of uniform binnings 
  struct FlatTable {
    double zMin{0.};          // low edge of the z axis [mm]
    double zBinWidth{1.};
    double dyMin{0.};         // low edge of the dy axis [mm]
    double dyBinWidth{1.};
    int nZ{0};
    int nDy{0};
    float zHalf{0.f};         // half-range of the z axis [mm]
    float maxOffset{0.f};     // largest |dy| covered [mm]
    std::vector<float> values;  // values[iZ * nDy + iDy]
  };

  // Copy a histogram into a FlatTable for quicker interpolation
  StatusCode flattenTable(const TH2F& hist, const std::string& regionName, FlatTable& table) const;

  // Resolve an element hash to its table
  const FlatTable* tableFor(const IdentifierHash& hash) const;

  StringProperty m_lutFile{this, "LUTFile",
      "ITkStripDigitization/CarletonLUTs_Feb2025/ITkStrip_LUTs_Temp253K_LumiInclusive.root",
      "ROOT file with all 2D CCE tables (all regions and luminosity points)"};

  FloatProperty m_lumi{this, "IntegratedLuminosity", 0.,
      "Integrated luminosity [fb-1] for interpolation between two nearest points"};

  FloatProperty m_chargeSmear{this, "LUTChargeSmear", 0.0,
      "Random Gaussian smearing applied to each LUT-derived strip charge (0 = off)"};

  const SCT_ID* m_sctID{nullptr};
  // Element hash -> index into m_tables
  std::vector<int> m_hashToTable;
  // One luminosity-interpolated table for each ITk strip region
  std::vector<FlatTable> m_tables;
};

} // namespace ITk

#endif // STRIPDIGITIZATION_STRIPRADDAMAGELUTTOOL_H
