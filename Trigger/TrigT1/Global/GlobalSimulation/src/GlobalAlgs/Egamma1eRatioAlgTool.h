/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EGAMMA1ERATIOALGTOOL_H
#define GLOBALSIM_EGAMMA1ERATIOALGTOOL_H

/**
 * AlgTool to read in LArStripNeighborhoods, and run the eRatio Algorithm.
 */

#include "../IGlobalSimAlgTool.h"
#include "../IO/LArStripNeighborhoodContainer.h"
#include "../IO/IeEmNbhoodTOBContainer.h"
#include "../IO/IeEmNbhoodTOB.h"
#include "../IO/IeEmEg1eRatioTOBContainer.h"
#include "../IO/IeEmEg1eRatioTOB.h"

#include "ap_int.h"
#include "ap_fixed.h"
#include "Digitizer.h"

#include "AthenaBaseComps/AthAlgTool.h"

#include <string>
#include <vector>

namespace GlobalSim {
  class Egamma1eRatioAlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {
    
  public:
    Egamma1eRatioAlgTool(const std::string& type,
			    const std::string& name,
			    const IInterface* parent);
    
    virtual ~Egamma1eRatioAlgTool() = default;
    
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

    // input to the  eRatio Algorithm
    SG::ReadHandleKey<IOBitwise::IeEmNbhoodTOBContainer>
    m_nbhdTOBContainerReadKey {
      this,
      "LArNeighborhoodTOBContainerReadKey",
      "stripNeighborhoodTOBContainer",
      "key to read inLArNeighborhoodTOBsReadKeys"};

    SG::WriteHandleKey<IOBitwise::IeEmEg1eRatioTOBContainer>
    m_eRatioResultKey {
      this,
      "eRatioResultKey",
      "eRatioResult"};
    
    SG::WriteHandleKey<std::vector<int>>
    m_eRatioKey {
      this,
      "eRatioKey",
      "eRatio"};

    SG::WriteHandleKey<std::vector<float>>
    m_eRatioSimpleKey {
      this,
      "eRatioSimpleKey",
      "eRatioSimple"};
    
    std::vector<double> combine_phi(const IOBitwise::IeEmNbhoodTOB*) const;
    ap_int<16> secondPeakSearch(const std::vector<ap_int<16>>& input, const ap_int<16> peak,
				const int startCell, const int endCell,
				const ap_int<16> noiseMargin) const;
    
    // a neighborhood has 3 vectors of strip energies (phi_low, phi_center.
    // phi_high). Provide the length thes vectors must have for the eRatio to be
    // evaluated
    static inline constexpr int s_required_phi_len = 17;
    
    // the three strip energy vectors are combined to form a single vector.
    // the length of this vector have the following length.
    static inline constexpr int s_combination_len = 51;
  };
}
#endif
