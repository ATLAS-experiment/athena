from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags

from AthenaCommon.Logging import logging 
log = logging.getLogger("TrigInDetValidation")

log.info( "preinclude: effpreinclude.py" ) 

from AthenaCommon.SystemOfUnits import GeV

flags.Trigger.InDetTracking.bjet.pTmin   = 0.8*GeV
flags.Trigger.InDetTracking.tauIso.pTmin = 0.8*GeV

log.info( f"ID Trigger bjet   pTmin: {flags.Trigger.InDetTracking.bjet.pTmin}" )
log.info( f"ID Trigger tauIso pTmin: {flags.Trigger.InDetTracking.tauIso.pTmin}" )



