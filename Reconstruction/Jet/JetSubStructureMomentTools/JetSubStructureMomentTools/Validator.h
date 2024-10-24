/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef jetsubstructuremomenttools_validator_header
#define jetsubstructuremomenttools_validator_header

#include "xAODCaloEvent/CaloCluster.h"
#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"

#include "GaudiKernel/ITHistSvc.h"
#include "AsgTools/AsgTool.h"
#include "JetInterface/IJetExecuteTool.h"

class TH1;

class Validator :
  virtual public IJetExecuteTool,
  public asg::AsgTool {
    ASG_TOOL_CLASS(Validator, IJetExecuteTool)

    public:
      // Constructor and destructor
      Validator(const std::string& name);

      virtual StatusCode initialize() override;
      virtual int execute() const override;

    private:
      std::string m_InputContainer;
      std::vector<std::string> m_FloatMoments;
      ServiceHandle<ITHistSvc> m_histSvc;
};

#endif
