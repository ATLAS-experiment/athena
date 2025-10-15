/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file DerivationFrameworkTop/TopHeavyFlavorFilterAugmentation.h
 * @author Georges Aad
 * @date Apr. 2015
 * @brief tool to add a variable to the EventInfo corresponding to the ttbar+HF filter flag
*/


#ifndef DerivationFramework_TopHeavyFlavorFilterAugmentation_H
#define DerivationFramework_TopHeavyFlavorFilterAugmentation_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "DerivationFrameworkTop/TTbarPlusHeavyFlavorFilterTool.h"

namespace DerivationFramework {

  class TTbarPlusHeavyFlavorFilterTool;
  
  class TopHeavyFlavorFilterAugmentation : public extends<AthAlgTool, IAugmentationTool> {


  public:
    TopHeavyFlavorFilterAugmentation(const std::string& t, const std::string& n, const IInterface* p);
    ~TopHeavyFlavorFilterAugmentation();
    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches() const;

  private:
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoName{this, "EventInfoName", "EventInfo"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_filterFlagKey{this, "FilterFlagKey", m_eventInfoName, "TopHeavyFlavorFilterFlag"};
    PublicToolHandle<DerivationFramework::TTbarPlusHeavyFlavorFilterTool> m_filterTool{this, "FilterTool", ""};

  }; /// class

} /// namespace


#endif
