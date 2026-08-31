/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_GFEXRHOCVTRALGTOOL_H
#define GLOBALSIM_GFEXRHOCVTRALGTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"

#include "../GlobalSimComponents/IGlobalSimAlgTool.h"
#include "xAODTrigger/gFexJetRoIContainer.h"
#include "../Utilities/IDataCollector.h"


#include "../IO/gFexRhoTOB.h"

namespace GlobalSim {

  /**
   * @brief AlgTool to extract the gFex Rho information into the GlobalSim TOB system.
   *
   */


  class gFexRhoCvtrAlgTool: public extends<AthAlgTool, IGlobalSimAlgTool> {

  public:
    gFexRhoCvtrAlgTool(const std::string& type,
		       const std::string& name,
		       const IInterface* parent);

    virtual ~gFexRhoCvtrAlgTool() override = default;

    /** @brief Initialize function running before first event */
    virtual StatusCode initialize() override;
  
    /** @brief Main functional block running for each event */
    virtual StatusCode run(const std::unique_ptr<IDataCollector>&,
			   const EventContext& ctx) const override;

    virtual std::string toString() const override;
  
  private:

       
    SG::ReadHandleKey<xAOD::gFexJetRoIContainer>
    m_gFexJetRoIKey {this, "gFexJetRoIKey", "L1_gFexRhoRoI", "gFexRho EDM"};

  
    SG::WriteHandleKey<GlobalSim::IOBitwise::gFexRhoTOBContainer>
    m_gFexRhoTOBContainerKey {
      this,
      "gFexRhoTOBContainerKey",
      "gFexRhoTOBs",
      "Key for GlobalSim gFexRhoTOB container"};

  };
}
#endif
