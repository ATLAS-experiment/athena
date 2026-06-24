#ifndef G4USERACTIONS_CELEROFFLOAD_H
#define G4USERACTIONS_CELEROFFLOAD_H

#include "AthenaBaseComps/AthMessaging.h"

#include "G4UserRunAction.hh"

namespace G4UA {

  class CelerOffload : public AthMessaging, public G4UserRunAction
  {
    public:
      CelerOffload();
      virtual void BeginOfRunAction(const G4Run*) override;
      virtual void EndOfRunAction(const G4Run*) override;
  };
}
#endif

