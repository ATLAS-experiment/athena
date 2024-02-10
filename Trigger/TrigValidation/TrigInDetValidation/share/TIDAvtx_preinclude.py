from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags

from AthenaCommon.Logging import logging 
log = logging.getLogger("TrigInDetValidation")

log.info( "preinclude: TIDAvtx_preinclude.py" ) 

flags.Trigger.InDetTracking.fullScan.addSingleTrackVertices = True
flags.Trigger.InDetTracking.fullScan.minNSiHits_vtx = 8
flags.Trigger.InDetTracking.fullScan.TracksMaxZinterval = 3

log.info( f"ID Trigger addSingleVertices:  flags.Trigger.InDetTracking.fullScan.addSingleTrackVertices}" )
log.info( f"ID Trigger minNSiHits:         flags.Trigger.InDetTracking.fullScan.minNSiHits_vtx}" )
log.info( f"ID Trigger TracksMaxZinterval: flags.Trigger.InDetTracking.fullScan.TracksMaxZinterval}" )


