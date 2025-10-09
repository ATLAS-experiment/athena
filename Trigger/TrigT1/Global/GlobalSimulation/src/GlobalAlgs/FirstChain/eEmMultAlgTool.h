/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EEMMULTALGTOOL_H
#define GLOBALSIM_EEMMULTALGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "../../IGlobalSimAlgTool.h"
#include "../../IO/IeEmTOBContainer.h"

#include "ICommonSelector.h"
#include "IeEmSelector.h"

#include <string>

namespace  GlobalSim::IOBitwise {
  class ICommonTOB;
}

namespace GlobalSim {

  /**
   * @brief AlgTool to count GlobalSim::IOBitwise::eEmTOB objects.
   *
   */


  class eEmMultAlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {

  public:
    eEmMultAlgTool(const std::string& type,
		   const std::string& name,
		   const IInterface* parent);

    virtual ~eEmMultAlgTool() override = default;

    /** @brief Initialize function running before first event */
    virtual StatusCode initialize() override;
  
    /** @brief Main functional block running for each event */
    virtual StatusCode run(const EventContext& ctx) const override;

    virtual StatusCode updateTIP(std::bitset<s_nbits_TIP>&,
				 const EventContext&) const override;
  private:
  
    std::unique_ptr<ICommonSelector> m_c_selector{nullptr};
    std::unique_ptr<IeEmSelector> m_e_selector{nullptr};
  
    SG::ReadHandleKey<GlobalSim::IOBitwise::IeEmTOBContainer>
    m_eEmTOBContainerKey {
      this,
      "eEmTOBs",
      "eEmTOBs",
      "Key for GlobalSim eEmTOB container"};

    
    SG::WriteHandleKey<bool> m_resultKey {
      this,
      "eEmMultResult",
      "eEmMultResult",
      "Key for selection result"}; 
    
    
    Gaudi::Property<std::string> m_et_low_str {
      this,
      "et_low",
      "0",
      "et low for window selector"};
    
    Gaudi::Property<std::string> m_et_high_str {
      this,
      "et_high",
      "inf",
      "et high for window selector"};

    Gaudi::Property<std::string> m_eta_low_str {
      this,
      "eta_low",
      "0",
      "eta low for window selector"};
    
    Gaudi::Property<std::string> m_eta_high_str {
      this,
      "eta_high",
      "inf",
      "eta high for window selector"};

    Gaudi::Property<std::string> m_phi_low_str {
      this,
      "phi_low",
      "0",
      "phi low for window selector"};
    
    Gaudi::Property<std::string> m_phi_high_str {
      this,
      "phi_high",
      "inf",
      "phi high for window selector"};

   Gaudi::Property<std::string> m_rhad_low_str {
      this,
      "rhad_low",
      "0",
      "rhad low for window selector"};
    
    Gaudi::Property<std::string> m_rhad_high_str {
      this,
      "rhad_high",
      "inf",
      "rhad high for window selector"};
 
   Gaudi::Property<std::string> m_reta_low_str {
      this,
      "reta_low",
      "0",
      "reta low for window selector"};
    
    Gaudi::Property<std::string> m_reta_high_str {
      this,
      "reta_high",
      "inf",
      "reta high for window selector"};

   Gaudi::Property<std::string> m_wstot_low_str {
      this,
      "wstot_low",
      "0",
      "wstot low for window selector"};
    
    Gaudi::Property<std::string> m_wstot_high_str {
      this,
      "wstot_high",
      "inf",
      "wstot high for window selector"};

  };
}
#endif
