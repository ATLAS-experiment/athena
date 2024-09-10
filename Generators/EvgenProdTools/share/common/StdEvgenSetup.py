#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
## Generic JO header for use in all evgen job options

## Pull in *really* generic stuff shared with StdAnalysisSetup.py
include("EvgenProdTools/StdJOSetup.py")

## Provide a special evgen logger
evgenLog = logging.getLogger("Evgen")

## Needed to set IS_SIMULATION metadata
import AthenaCommon.AtlasUnixGeneratorJob

## Provide a bit of compatibility with Generate_trf JOs via dummy
## evgenConfig and StreamEVGEN objects
if "evgenConfig" not in dir():
     if "EvgenConfig" not in dir():
          class EvgenConfig(object):
               pass
     evgenConfig = EvgenConfig()
#
if "StreamEVGEN" not in dir():
     from AthenaPoolCnvSvc.WriteAthenaPool import AthenaPoolOutputStream
     streamEVGEN = AthenaPoolOutputStream("StreamEVGEN")
     StreamEVGEN = streamEVGEN  # compatibility alias, breaking convention!
