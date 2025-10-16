/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_EFEXROIALGTOOL_H
#define GLOBALSIM_EFEXROIALGTOOL_H

/**
 * AlgTool to obtain a selection of eFex RoIs read in from the
 * event store.
 */

#include "AthenaBaseComps/AthAlgTool.h"
#include "xAODTrigger/eFexEMRoIContainer.h"


#include <string>
#include <vector>

namespace GlobalSim {

  class eFexRoIAlgTool: public AthAlgTool {
    
  public:
    eFexRoIAlgTool(const std::string& type,
		   const std::string& name,
		   const IInterface* parent);
    
    virtual ~eFexRoIAlgTool() = default;

    StatusCode initialize() override;
 
    virtual StatusCode RoIs(std::vector<const xAOD::eFexEMRoI*>&,
			    const EventContext& ctx) const;
    
    virtual std::string toString() const;
    
    
  private:
   
    SG::ReadHandleKey<xAOD::eFexEMRoIContainer>
    m_eEmRoIKey {this, "eFexEMRoIKey", "L1_eEMRoI", "eFEXEM EDM"};

    Gaudi::Property<double> m_etMin {
      this,
      "etMin",
      0.,
      "selection Et min"};
    
  };
}
#endif
