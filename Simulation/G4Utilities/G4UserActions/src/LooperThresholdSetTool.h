/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef G4USERACTIONS_G4UA_LooperThresholdSetTOOL_H
#define G4USERACTIONS_G4UA_LooperThresholdSetTOOL_H

// Infrastructure includes
#include "G4AtlasTools/UserActionToolBase.h"

// Local includes
#include "LooperThresholdSet.h"

namespace G4UA
{

/// @class LooperThresholdSetTool
/// @brief Tool which manages the Looper Threshold options.
///
/// Create the LooperThresholdSet for each worker thread
///
/// @author Miha Muskinja
///
class LooperThresholdSetTool : public UserActionToolBase<LooperThresholdSet>
{

public:

using UserActionToolBase<LooperThresholdSet>::UserActionToolBase;

virtual StatusCode initialize() override;
virtual StatusCode finalize() override;

protected:

/// Create action for this thread
virtual std::unique_ptr<LooperThresholdSet>
makeAndFillAction(G4AtlasUserActions&) override final;

private:

/// Configuration parameters
Gaudi::Property<double> m_warningEnergy{this, "WarningEnergy", 100.0 * CLHEP::MeV};
Gaudi::Property<double> m_importantEnergy{this, "ImportantEnergy", 250.0 * CLHEP::MeV};
Gaudi::Property<int> m_numberOfTrials{this, "NumberOfTrials", 10};

};   // class LooperThresholdSetTool

} // namespace G4UA

#endif
