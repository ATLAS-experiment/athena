/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_COMMONSORTSELECTALGTOOL_H
#define GLOBALSIM_COMMONSORTSELECTALGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "../GlobalSimComponents/IGlobalSimAlgTool.h"
#include "../IO/CommonTOB.h"
#include "../IO/CommonTOBContainer.h"

#include "ICommonSelector.h"

#include <string>

namespace  GlobalSim::IOBitwise {
  class CommonTOB;
}

namespace GlobalSim {

  /**
   * @brief AlgTool to count GlobalSim::IOBitwise::eEmTOB objects.
   *
   */


  class CommonSortSelectAlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {

  public:
    CommonSortSelectAlgTool(const std::string& type,
		   const std::string& name,
		   const IInterface* parent);

    virtual ~CommonSortSelectAlgTool() override = default;

    /** @brief Initialize function running before first event */
    virtual StatusCode initialize() override;
    virtual StatusCode run(const EventContext&) const override;
    virtual std::string toString() const override;

  private:
  
    std::unique_ptr<ICommonSelector> m_c_selector{nullptr};
  
    SG::ReadHandleKey<GlobalSim::IOBitwise::CommonTOBContainer>
    m_inTOBContainerKey {
      this,
      "inTOBs",
      "inTOBs",
      "Key for GlobalSim CommonTOB container"};

     
    SG::WriteHandleKey<GlobalSim::IOBitwise::CommonTOBContainer>
    m_outTOBContainerKey {
      this,
      "outTOBs",
      "outTOBs",
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

 
    Gaudi::Property<std::size_t> m_maxTOBs {
      this,
      "maxTOBs",
      10,
      "max number of TOBs in to be passed to client"};

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


    void dump() const;

  };
}
#endif
