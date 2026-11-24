/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4USERACTIONS_G4UA_LENGTHINTEGRATOR_H
#define G4USERACTIONS_G4UA_LENGTHINTEGRATOR_H

// Framework includes
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"

// Infrastructure includes
#include "G4AtlasTools/UserActionToolBase.h"

// Local includes
#include "LengthIntegrator.h"

namespace G4UA
{

  /// @class LengthIntegratorTool
  /// @brief Tool which manages the LengthIntegrator action.
  ///
  /// Creates the LengthIntegrator for each worker thread.
  ///
  class LengthIntegratorTool : public UserActionToolBase<LengthIntegrator>
  {

    public:

      using UserActionToolBase<LengthIntegrator>::UserActionToolBase;

      /// Initialize the tool
      virtual StatusCode initialize() override;

    protected:

      /// Create aciton for this thread
      virtual std::unique_ptr<LengthIntegrator>
      makeAndFillAction(G4AtlasUserActions&) override final;

    private:

      /// Handle to the histogram service
      ServiceHandle<ITHistSvc> m_hSvc{this, "HistoSvc", "THistSvc"};

      //do we fill histograms as well as TTree?
      //(very large filesize...)
      Gaudi::Property<bool> m_doHistos{this, "CreateHistograms", true};

  }; // class LengthIntegratorTool

} // namespace G4UA

#endif
