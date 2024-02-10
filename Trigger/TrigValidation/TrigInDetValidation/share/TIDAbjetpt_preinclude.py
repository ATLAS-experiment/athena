from AthenaCommon.Logging import logging 
log = logging.getLogger("TrigInDetValidation")

log.info( "preinclude: TIDAbjetpt_preinclude.py" ) 

from AthenaCommon.SystemOfUnits import GeV

from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
flags.Trigger.InDetTracking.bjet.Xi2max=12.
flags.Trigger.InDetTracking.bjet.pTmin=0.8*GeV

log.info(f"ID Trigger pTmin:  {flags.Trigger.InDetTracking.bjet.pTmin}" )
log.info(f"ID Trigger Xi2max: {flags.Trigger.InDetTracking.bjet.Xi2max}")

