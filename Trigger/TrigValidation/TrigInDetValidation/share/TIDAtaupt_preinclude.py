from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags

from AthenaCommon.Logging import logging 
log = logging.getLogger("TrigInDetValidation")

log.info( "preinclude: TIDAtaupt_preinclude.py" ) 

from AthenaCommon.SystemOfUnits import GeV
flags.Trigger.InDetTracking.tauIso.pTmin = 0.8*GeV

log.info( f"ID Trigger pTmin: {flags.Trigger.InDetTracking.tauIso.pTmin}" )


