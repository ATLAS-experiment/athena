#include "CelerOffload.h"

#include "accel/TrackingManagerIntegration.hh"
#include "G4Threading.hh"

namespace G4UA {
  CelerOffload::CelerOffload() : AthMessaging("CelerOffload") 
  {
    ATH_MSG_INFO( "Constructor " << "tid: " << G4Threading::G4GetThreadId() );
    celeritas::TrackingManagerIntegration::Instance();
    // Set options from constructor arg (from Tool?)
  }

  void CelerOffload::BeginOfRunAction(const G4Run* run)
  {
    ATH_MSG_INFO( "Begin of Run" );
    celeritas::TrackingManagerIntegration::Instance().BeginOfRunAction(run);
  }

  void CelerOffload::EndOfRunAction(const G4Run* run)
  {
    ATH_MSG_INFO( "End of Run" );
    celeritas::TrackingManagerIntegration::Instance().EndOfRunAction(run);
  }
}
