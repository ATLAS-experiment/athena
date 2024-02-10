from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags

from AthenaCommon.Logging import logging 
log = logging.getLogger("TrigInDetValidation")

log.info( "preinclude: TIDAwithpid.py" ) 

flags.Trigger.InDetTracking.electron.electronPID = True

log.info( f"Setting electronPID in the TrackSumaryTool: {flags.Trigger.InDetTracking.electron.electronPID}")


