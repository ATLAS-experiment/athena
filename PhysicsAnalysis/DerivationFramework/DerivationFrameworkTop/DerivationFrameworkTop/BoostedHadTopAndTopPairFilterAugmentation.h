/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file DerivationFrameworkTop/BoostedHadTopAndTopPairFilterAugmentation.h
 * @author Georges Aad
 * @date Apr. 2015
 * @brief tool to add a variable to the EventInfo corresponding to the ttbar+HF filter flag
 */


#ifndef DerivationFramework_BoostedHadTopAndTopPairFilterAugmentation_H
#define DerivationFramework_BoostedHadTopAndTopPairFilterAugmentation_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "GaudiKernel/ToolHandle.h"
#include "xAODEventInfo/EventInfo.h"
#include "DerivationFrameworkTop/BoostedHadTopAndTopPairFilterTool.h"

namespace DerivationFramework {

  class BoostedHadTopAndTopPairFilterTool;

  class BoostedHadTopAndTopPairFilterAugmentation : public extends<AthAlgTool, IAugmentationTool> {


  public:
    BoostedHadTopAndTopPairFilterAugmentation(const std::string& t, const std::string& n, const IInterface* p);
    ~BoostedHadTopAndTopPairFilterAugmentation();
    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoName{this, "EventInfoName", "EventInfo", ""};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_ttbarSysPt_HighKey{this, "ttbarSysPt_HighKey", "TTbar350"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_HadTopPt_HighKey{this, "HadTopPt_HighKey", "HadTop350"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_ttbarSysPt_LowKey{this, "ttbarSysPt_LowKey", "TTbar150"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_HadTopPt_LowKey{this, "HadTopPt_LowKey", "HadTop200"};
    ToolHandle<DerivationFramework::BoostedHadTopAndTopPairFilterTool> m_filterTool_Low{this, "FilterTool_Low", ""};
    ToolHandle<DerivationFramework::BoostedHadTopAndTopPairFilterTool> m_filterTool_High{this, "FilterTool_High", ""};

  }; /// class

} /// namespace


#endif
