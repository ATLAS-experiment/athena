/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// Simple alg to wrap the pixel ToT PID tool

#ifndef PIXELTOTPIDDUALTOOL_PIXELTOTPIDDUALALG
#define PIXELTOTPIDDUALTOOL_PIXELTOTPIDDUALALG

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkAnalysisInterfaces/IPixelToTPIDDualTool.h"

namespace CP {

  class PixelToTPIDDualAlg : public AthAlgorithm {
  public:
    PixelToTPIDDualAlg(const std::string& name, ISvcLocator* svcloc);
    
    virtual StatusCode initialize();
    virtual StatusCode execute();

  private:
    std::string m_inputTracks;
    ToolHandle<CP::IPixelToTPIDDualTool> m_tool;
  };

}  // namespace CP

#endif
