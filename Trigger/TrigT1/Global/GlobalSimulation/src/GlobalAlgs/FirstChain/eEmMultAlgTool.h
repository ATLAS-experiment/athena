/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EEMMULTALGTOOL_H
#define GLOBALSIM_EEMMULTALGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "../../IGlobalSimAlgTool.h"
#include "../../IO/IeEmTOB.h"
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
  
  private:
  
    std::unique_ptr<ICommonSelector> m_c_selector{nullptr};
    std::unique_ptr<IeEmSelector> m_e_selector{nullptr};
  
    /** @brief Key to the GlobalLArCellContainer */
    SG::ReadHandleKey<GlobalSim::IOBitwise::IeEmTOB> m_gblLArCellContainerKey {
      this,
      "eEmTOBs",
      "eEmTOBs",
      "Key for GlobalSim eEmTOB container"}; 
  };

}
#endif
