/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EGAMMA1BDTALGTOOL_H
#define GLOBALSIM_EGAMMA1BDTALGTOOL_H

/**
 * AlgTool to read in LArStripNeighborhoods, and run the BDT Algorithm.
 */

#include "../IGlobalSimAlgTool.h"
#include "../IO/LArStripNeighborhoodContainer.h"

#include "../NumericTypes/ap_int.h"
#include "../NumericTypes/ap_fixed.h"

#include "Egamma1BDT/BDT.h"

#include "AthenaBaseComps/AthAlgTool.h"

#include <string>
#include <vector>

namespace GlobalSim {
  class Egamma1BDTAlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {
    
  public:
    Egamma1BDTAlgTool(const std::string& type,
			    const std::string& name,
			    const IInterface* parent);
    
    virtual ~Egamma1BDTAlgTool() = default;
    
    StatusCode initialize() override;

    virtual StatusCode run(const EventContext& ctx) const override;
    
    virtual std::string toString() const override;
    
  private:
    

    Gaudi::Property<bool>
    m_enableDump{this,
	     "enableDump",
	     {false},
	     "flag to enable dumps"};

    // input to the  BDT Algorithm
    SG::ReadHandleKey<LArStripNeighborhoodContainer>
    m_nbhdContainerReadKey {
      this,
      "LArNeighborhoodContainerReadKey",
      "stripNeighborhoodContainer",
      "key to read inLArNeighborhoodReadKeys"};

    std::vector<double> combine_phi(const LArStripNeighborhood*) const;
    std::vector<ap_int<10>> digitize(const std::vector<double>&) const;

    // a neighborhood has 3 vectors of strip energies (phi_low, phi_center.
    // phi_high). Provide the length thes vectors must have for the BDT to be
    // evaluated
    static inline constexpr int s_required_phi_len = 17;
    
    // the three strip energy vectors are combined to form a single vector.
    // the length of this vector have the following length.
    static inline constexpr int s_combination_len = 18;
  };
}
#endif
