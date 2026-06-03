/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_COMMONMULTALGTOOL_H
#define GLOBALSIM_COMMONMULTALGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "../GlobalSimComponents/ITIPwriterAlgTool.h"
#include "../IO/CommonTOBContainer.h"

#include "ICommonSelector.h"

#include <string>

namespace GlobalSim {

  /**
   * @brief AlgTool to count GlobalSim::IOBitwise::CommonTOB objects.
   * Cutting on the et/eta/phi values
   */


  class CommonMultAlgTool: public extends<AthAlgTool, ITIPwriterAlgTool> {

  public:
    CommonMultAlgTool(const std::string& type,
		   const std::string& name,
		   const IInterface* parent);

    virtual ~CommonMultAlgTool() override = default;

    /** @brief Initialize function running before first event */
    virtual StatusCode initialize() override;

    virtual StatusCode updateTIP(std::bitset<s_nbits_TIP>&,
				 const EventContext&) const override;

    virtual std::string toString() const override;

  private:
  
    std::unique_ptr<ICommonSelector> m_c_selector{nullptr};

    SG::ReadHandleKey<IOBitwise::CommonTOBContainer> m_CommonTOBContainerKey {
      this,
      "CommonTOBsKey",
      "CommonTOBs",
      "Key for GlobalSim CommonTOB container"};
    
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
