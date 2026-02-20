/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef xAODBTEFF_TOOLTESTER_H
#define xAODBTEFF_TOOLTESTER_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include <AsgTools/PropertyWrapper.h>

#include "FTagAnalysisInterfaces/IBTaggingEfficiencyTool.h"

  class BTagToolTester : public AthAlgorithm {
  public:
    BTagToolTester(const std::string & name, ISvcLocator * svcLoc );
    StatusCode initialize();
    StatusCode execute(); 

  private:
    Gaudi::Property<std::string>  m_sgKey {this, "SGKey", "AntiKt4LCTopoJets", "Jet collection name"}; // StoreGate key for the jets
    ToolHandle< IBTaggingEfficiencyTool > m_effTool {this, "BTaggingEfficiencyTool", "BTaggingEfficiencyTool/BTaggingEfficiencyTool", "Tagging efficiency tool"};
  };

#endif
