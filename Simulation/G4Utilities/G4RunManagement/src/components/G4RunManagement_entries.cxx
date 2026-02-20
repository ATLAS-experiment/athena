#include <Gaudi/PluginServiceV2.h>
#include "../G4RunTool.h"
#include "../SyncRunActionTool.h"
#include "../SyncPrimaryGeneratorActionTool.h"
#include "../SyncEventActionTool.h"


DECLARE_COMPONENT(G4RunTool)
DECLARE_COMPONENT( G4UA::SyncRunActionTool )
DECLARE_COMPONENT( G4UA::SyncPrimaryGeneratorActionTool )
DECLARE_COMPONENT( G4UA::SyncEventActionTool )