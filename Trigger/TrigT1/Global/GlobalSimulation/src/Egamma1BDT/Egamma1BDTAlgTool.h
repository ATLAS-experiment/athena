/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EGAMMA1BDTALGTOOL_H
#define GLOBALSIM_EGAMMA1BDTALGTOOL_H

/**
 * AlgTool to read in LArStripNeighborhoods, and run the BDT Algorithm.
 */

#include "../GlobalSimComponents/IGlobalSimAlgTool.h"
#include "../IO/LArStripNeighborhood.h"
#include "../IO/eEmNbhoodTOB.h"
#include "../IO/eEmEg1BDTTOB.h"


#include "ap_int.h"
#include "ap_fixed.h"
#include "../Utilities/Digitizer.h"

#include "./Egamma1BDT/BDT.h"

#include "AthenaBaseComps/AthAlgTool.h"

#include <string>
#include <vector>

namespace GlobalSim {
 
  class Egamma1BDTAlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {

    using eEmEg1BDTTOBContainer = GlobalSim::IOBitwise::eEmEg1BDTTOBContainer;
    using eEmNbhoodTOBContainer =  GlobalSim::IOBitwise::eEmNbhoodTOBContainer;
    
  public:
    Egamma1BDTAlgTool(const std::string& type,
			    const std::string& name,
			    const IInterface* parent);
    
    virtual ~Egamma1BDTAlgTool() = default;
    
    StatusCode initialize() override;

    virtual StatusCode run(const EventContext& ctx) const override;
    
    virtual std::string toString() const override;

    static constexpr std::size_t BDT_ouput_width = 10;    
    
  private:
    

    Gaudi::Property<bool>
    m_enableDump{this,
	     "enableDump",
	     {false},
	     "flag to enable dumps"};
    
    SG::WriteHandleKey<std::vector<float>>
    m_BDTScoreKey {
      this,
      "BDTScoreKey",
      "eGamma1BDT"};    
        
    // input to the  BDT Algorithm
    SG::ReadHandleKey<eEmNbhoodTOBContainer>
    m_nbhdTOBContainerReadKey {
      this,
      "LArNeighborhoodTOBContainerReadKey",
      "stripNeighborhoodTOBContainer",
      "key to read inLArNeighborhoodTOBsReadKeys"};

    SG::WriteHandleKey<eEmEg1BDTTOBContainer>
    m_eEmEg1BDTTOBContainerKey {
      this,
      "eEmEg1BDTTOBContainerKey",
      "eEmEg1BDTTOBContainer"};
    
    int bitSetToInt(std::bitset<BDT_ouput_width> bitSet) const;
    std::vector<double> combine_phi(const IOBitwise::eEmNbhoodTOB*) const;

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
