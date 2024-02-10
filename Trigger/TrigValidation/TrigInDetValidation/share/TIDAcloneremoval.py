from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags

from AthenaCommon.Logging import logging 
log = logging.getLogger("TrigInDetValidation")

log.info( "preinclude: TIDAcloneremoval.py" ) 


flags.Trigger.InDetTracking.electron.doCloneRemoval = False

log.info( f"Setting clone removal: {flags.Trigger.InDetTracking.electron.doCloneRemoval}") )


