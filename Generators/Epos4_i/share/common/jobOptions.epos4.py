# ----------------------------------------------------------------------
# Example Epos4 JO file, adapted for Epos4 from the version for Epos by Sebastian Piec
# Author: Andrii Verbytskyi
#

# ----------------------------------------------------------------------
from AthenaCommon.AppMgr import ServiceMgr

from AthenaCommon.AlgSequence import AlgSequence
job = AlgSequence()

# ----------------------------------------------------------------------
# Epos4 initialization 
# ----------------------------------------------------------------------
from Epos4_i.Epos4_iConf import Epos4

Epos4 = Epos4()
Epos4.BeamMomentum     = -3500.0
Epos4.TargetMomentum   = 3500.0
job += Epos4

# Set output level threshold (2=DEBUG, 3=INFO, 4=WARNING, 5=ERROR, 6=FATAL )
MessageSvc = Service( "MessageSvc" )
MessageSvc.OutputLevel = 3

# Number of events to be processed (default is 10)
#theApp.EvtMax = 200
evgenConfig.minevents = 100

# ----------------------------------------------------------------------
# Printing service
# ----------------------------------------------------------------------
from TruthExamples.TruthExamplesConf import DumpMC
#job += DumpMC()

# ----------------------------------------------------------------------
# Ntuple service output
# ----------------------------------------------------------------------
import AthenaPoolCnvSvc.WriteAthenaPool
from AthenaPoolCnvSvc.WriteAthenaPool import AthenaPoolOutputStream

stream1 = AthenaPoolOutputStream( "StreamEVGEN" )
stream1.WritingTool = "AthenaOutputStreamTool"
stream1.OutputFile = "Epos4_events.pool.root"
stream1.TakeItemsFromInput = True
stream1.ItemList += [ 'EventInfo#*', 'McEventCollection#*' ]
# ----------------------------------------------------------------------

