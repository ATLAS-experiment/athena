# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG

# Setup flags
flags = initConfigFlags()
flags.Common.MsgSuppression = False
flags.Exec.MaxEvents = 2
flags.Input.Files = []
stream = "Stream1"
flags.lock()

# Main services
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg(flags)

from McEventSelector.McEventSelectorConfig import McEventSelectorCfg
acc.merge( McEventSelectorCfg(flags,
                              RunNumber         = 1,
                              EventsPerRun      = 0x100000010,
                              FirstEvent        = 0x100000000,
                              EventsPerLB       = 1,
                              FirstLB           = 1,
                              InitialTimeStamp  = 0,
                              TimeStampInterval = 5) )

# Pool writing
acc.addEventAlgo( CompFactory.AthenaPoolTestDataWriter(OutputLevel = DEBUG),
                  sequenceName = 'AthAlgSeq' )

from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
acc.merge( OutputStreamCfg(flags, stream, disableEventTag = True,
                           ItemList = ['EventInfo#*', 'EventStreamInfo#*',
                                       'MergedEventInfo#*', 'PileUpEventInfo#*',
                                       'IAthenaPoolTestCollection#*',
                                       'AthenaPoolTestMatrix#*', 'AthenaPoolTestMap#*',
                                       'dummy_A#*', 'dummy_E#*']) )

# Run
import sys
sc = acc.run(flags.Exec.MaxEvents)
sys.exit(sc.isFailure())
