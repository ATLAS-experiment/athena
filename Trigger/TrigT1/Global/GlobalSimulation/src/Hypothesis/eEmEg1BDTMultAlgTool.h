/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EEMEG1BDTMULTALGTOOL_H
#define GLOBALSIM_EEMEG1BDTMULTALGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "../GlobalSimComponents/ITIPwriterAlgTool.h"
#include "../IO/eEmEg1BDTTOB.h"

#include "ICommonSelector.h"
#include "IeEmEg1BDTSelector.h"

#include <string>

namespace GlobalSim {

  /**
   * @brief AlgTool to count GlobalSim::IOBitwise::eEmEg1BDTTOB objects.
   * Cutting on the  eEmEg1BDT score
   */


  class eEmEg1BDTMultAlgTool: public extends<AthAlgTool, ITIPwriterAlgTool> {

  public:
    eEmEg1BDTMultAlgTool(const std::string& type,
		   const std::string& name,
		   const IInterface* parent);

    virtual ~eEmEg1BDTMultAlgTool() override = default;

    /** @brief Initialize function running before first event */
    virtual StatusCode initialize() override;

    virtual StatusCode updateTIP(std::bitset<s_nbits_TIP>&,
				 const EventContext&) const override;

    virtual std::string toString() const override;

  private:
  
    std::unique_ptr<ICommonSelector> m_c_selector{nullptr};
    std::unique_ptr<IeEmEg1BDTSelector> m_bdt_selector{nullptr};

    SG::ReadHandleKey<IOBitwise::eEmEg1BDTTOBContainer>
    m_eEmEg1BDTTOBContainerKey {
      this,
      "eEmEg1BDTTOBContainerKey",
      "eEmEg1BDTTOBContainer",
      "Key for GlobalSim eEmEg1BDTTOB container"};
    
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


   Gaudi::Property<std::string> m_Eg1BDT_str {
      this,
      "Eg1BDT",
      "0",
      "Eg1BDT lcut_value"};

    Gaudi::Property<std::string> m_Eg1BDT_op {
      this,
      "Eg1BDT_op",
      "unknown",
      "Eg1BDT cut_operator"};
    
    Gaudi::Property<int> m_TIP_position {
      this,
      "TIPposition",
      0,
      "start position to write into the TIP"};

    
    Gaudi::Property<int> m_n_multbits {
      this,
      "n_multbits",
      3,
      "number of bits to write into the TIP"};

    Gaudi::Property<std::string> m_menu_name {
      this,
      "menu_name",
      "unknown",
      "name from json menu file"
    };

    Gaudi::Property<bool> m_enableDump {
      this,
      "enable_dump",
      false,
      "flag to enable debug dumps"
    };


    ulong m_maxtob{0};

  };
}
#endif
