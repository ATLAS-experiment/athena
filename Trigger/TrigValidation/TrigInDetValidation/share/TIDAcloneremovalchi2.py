from AthenaCommon.Logging import logging 
log = logging.getLogger("TrigInDetValidation")

log.info( "preinclude: TIDAcloneremoval.py" ) 

from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
flags.Trigger.InDetTracking.electron.Xi2max=12.


log.info( f"Setting clone removal: {flags.Trigger.InDetTracking.electron.doCloneRemoval}" )
log.info( f"Setting Xi2max:        {flags.Trigger.InDetTracking.electron.Xi2max}" )


