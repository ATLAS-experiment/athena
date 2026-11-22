/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4DEBUGGINGTOOLS_G4UA__STEPHISTOGRAMTOOL_H
#define G4DEBUGGINGTOOLS_G4UA__STEPHISTOGRAMTOOL_H

#include "G4AtlasTools/UserActionToolBase.h"
#include "StepHistogram.h"
#include "GaudiKernel/ITHistSvc.h"

namespace G4UA{ 
  
  
  /// @class StepHistogramTool
  /// @brief Tool which manages the StepHistogram action.
  ///
  /// Create the StepHistogram for each worker thread
  ///
  /// @author Miha Muskinja
  ///
  
  class StepHistogramTool: public UserActionToolBase<StepHistogram>
  {

  public:
    using UserActionToolBase<StepHistogram>::UserActionToolBase;
    /// return the event action

    // initialize
    virtual StatusCode initialize() override;
    // finalize
    virtual StatusCode finalize() override;

  protected:
    /// Create action for this thread
    virtual std::unique_ptr<StepHistogram>
    makeAndFillAction(G4AtlasUserActions&) override final;

  private:
    Gaudi::Property<bool> m_do2DHistograms{this, "do2DHistograms", false};
    Gaudi::Property<bool> m_doGeneralHistograms{this, "doGeneralHistograms", false};

    ServiceHandle<ITHistSvc> m_histSvc{this, "THistSvc", "THistSvc"};

    void BookHistograms(StepHistogram::HistoMapMap_t &hMap, const char* suffix, const char* subfolder = "");
    
  }; // class StepHistogramTool
  
  
} // namespace G4UA 
#endif
