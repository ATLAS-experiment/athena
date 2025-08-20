#include "TrigConfxAOD/xAODConfigTool.h"

#ifndef XAOD_STANDALONE
#include "../xAODConfigSvc.h"
#include "../xAODMenuWriter.h"
#include "../xAODMenuReader.h"
#include "../KeyWriterTool.h"
#endif

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT( TrigConf::xAODConfigTool )

#ifndef XAOD_STANDALONE
DECLARE_COMPONENT( TrigConf::xAODConfigSvc )
DECLARE_COMPONENT( TrigConf::xAODMenuWriter )
DECLARE_COMPONENT( TrigConf::xAODMenuReader )
DECLARE_COMPONENT( TrigConf::KeyWriterTool )
#endif
