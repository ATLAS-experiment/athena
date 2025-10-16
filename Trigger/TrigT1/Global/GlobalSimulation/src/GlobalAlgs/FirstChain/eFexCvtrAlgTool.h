/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EFEXCVTRALGTOOL_H
#define GLOBALSIM_EFEXCVTRALGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "../../IGlobalSimAlgTool.h"
#include "xAODTrigger/eFexEMRoIContainer.h"

#include "../../IO/IeEmTOBContainer.h"

namespace GlobalSim {

  /**
   * @brief AlgTool to move eFex TOBS into the GlobalSim TOB system.
   *
   */


  class eFexCvtrAlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {

  public:
    eFexCvtrAlgTool(const std::string& type,
		   const std::string& name,
		   const IInterface* parent);

    virtual ~eFexCvtrAlgTool() override = default;

    /** @brief Initialize function running before first event */
    virtual StatusCode initialize() override;
  
    /** @brief Main functional block running for each event */
    virtual StatusCode run(const EventContext& ctx) const override;

    virtual std::string toString() const override;
  
    virtual StatusCode updateTIP(std::bitset<s_nbits_TIP>&,
				 const EventContext&) const override;

  private:

       
    SG::ReadHandleKey<xAOD::eFexEMRoIContainer>
    m_eEmRoIKey {this, "eFexEMRoIKey", "L1_eEMRoI", "eFEXEM EDM"};

  
    SG::WriteHandleKey<GlobalSim::IOBitwise::IeEmTOBContainer>
    m_eEmTOBContainerKey {
      this,
      "eEmTOBs",
      "eEmTOBs",
      "Key for GlobalSim eEmTOB container"};

  };
}
#endif
