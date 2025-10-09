/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EGAMMA1BASELINEALGTOOL_H
#define GLOBALSIM_EGAMMA1BASELINEALGTOOL_H

/**
 * AlgTool to read in LArStripNeighborhoods, and run the Baseline Algorithm.
 */

#include "../IGlobalSimAlgTool.h"
#include "../IO/LArStripNeighborhoodContainer.h"

#include "ap_int.h"
#include "ap_fixed.h"
#include "Digitizer.h"

#include "AthenaBaseComps/AthAlgTool.h"

#include <string>
#include <vector>

namespace GlobalSim {
  class Egamma1BaselineAlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {
    
  public:
    Egamma1BaselineAlgTool(const std::string& type,
			    const std::string& name,
			    const IInterface* parent);
    
    virtual ~Egamma1BaselineAlgTool() = default;
    
    StatusCode initialize() override;

    virtual StatusCode run(const EventContext& ctx) const override;
    
    virtual std::string toString() const override;

    virtual StatusCode updateTIP(std::bitset<s_nbits_TIP>&,
				 const EventContext&) const override;

  private:
    
    Gaudi::Property<bool>
    m_enableDump{this,
	     "enableDump",
	     {false},
	     "flag to enable dumps"};

    // input to the  Baseline Algorithm
    SG::ReadHandleKey<LArStripNeighborhoodContainer>
    m_nbhdContainerReadKey {
      this,
      "LArNeighborhoodContainerReadKey",
      "stripNeighborhoodContainer",
      "key to read inLArNeighborhoodReadKeys"};

    SG::WriteHandleKey<std::vector<float>>
    m_eRatioKey {
      this,
      "eRatioKey",
      "eRatio"};

    SG::WriteHandleKey<std::vector<float>>
    m_eRatioSimpleKey {
      this,
      "eRatioSimpleKey",
      "eRatioSimple"};
    
    std::vector<double> combine_phi(const LArStripNeighborhood*) const;
    ap_int<16> secondPeakSearch(const std::vector<ap_int<16>>& input, const ap_int<16> peak,
				const int startCell, const int endCell,
				const ap_int<16> noiseMargin) const;
    
    // a neighborhood has 3 vectors of strip energies (phi_low, phi_center.
    // phi_high). Provide the length thes vectors must have for the Baseline to be
    // evaluated
    static inline constexpr int s_required_phi_len = 17;
    
    // the three strip energy vectors are combined to form a single vector.
    // the length of this vector have the following length.
    static inline constexpr int s_combination_len = 51;
  };
}
#endif
