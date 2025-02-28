/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// DESDM_EXOTHIP_SkimmingTool.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Author: Priyanka Kumari (pkumari@cern.ch)
// follows closely DerivationFrameworkExamples


#ifndef DERIVATIONFRAMEWORK_DESDM_EXOTHIP_SkimmingTool_H
#define DERIVATIONFRAMEWORK_DESDM_EXOTHIP_SkimmingTool_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"

#include "GaudiKernel/ToolHandle.h"
#include "Gaudi/Property.h"

#include "StoreGate/ReadHandle.h"

#include "DerivationFrameworkInterfaces/ISkimmingTool.h"

#include "TrigDecisionTool/TrigDecisionTool.h"

namespace DerivationFramework {
  
  class DESDM_EXOTHIP_SkimmingTool : public AthAlgTool, public ISkimmingTool {
  
  public:
  /* Constructor with parameter */
    DESDM_EXOTHIP_SkimmingTool( const std::string& CPP_exothip, const std::string& skimtool, const IInterface* interface );
    
    // Destructor
    virtual ~DESDM_EXOTHIP_SkimmingTool() = default;
    
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
        
    virtual bool eventPassesFilter() const override;
  
  private:
    std::string m_trnnoutContName;

    ToolHandle<Trig::TrigDecisionTool> m_trigDec;
    mutable std::atomic<unsigned int> m_ntot = 0;
    mutable std::atomic<unsigned int> m_npass = 0;

    double m_minHTratioWedge;
    
  };

}

#endif
