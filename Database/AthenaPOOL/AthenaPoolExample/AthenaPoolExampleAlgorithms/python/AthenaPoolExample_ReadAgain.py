#!/env/python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

## @file AthenaPoolExample_ReadAgain.py
## @brief Example job options file to illustrate how to read event data from Pool.
#
# This Job option:
# ----------------
# 1. Reads the data from files have been written with AthneaPoolExample_Write.py
#    and AthneaPoolExample_ReadWrite.py
# 2. Same as 1., but using TAG collections. Instead of SkipEvents, a HelperTool is used to skip the
#    first 10 events 
#==============================================================

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import DEBUG

# Setup flags
flags = initConfigFlags()
flags.Input.Files = ["EmptyPoolFile.root", "SimplePoolReplica1.root",
                     "EmptyPoolFile.root", "SimplePoolFile2.root", "SimplePoolFile4.root"]
flags.Exec.MaxEvents = -1
flags.Common.MsgSuppression = False
flags.Exec.DebugMessageComponents = ["ReadData", "PoolSvc",
                                     "AthenaPoolAddressProviderSvc", "MetaDataSvc", "EventSelector"]
flags.lock()

# Main services
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
acc = MainServicesCfg( flags )

# Configure AthenaPool reading
from AthenaPoolExampleAlgorithms.AthenaPoolExampleConfig import AthenaPoolExampleReadCfg
acc.merge( AthenaPoolExampleReadCfg(flags, readCatalogs = ["file:Catalog1.xml"]) )

# Creata and attach the reading algorithm
acc.addEventAlgo( CompFactory.AthPoolEx.ReadData("ReadData", OutputLevel = DEBUG) )

evSelector = acc.getService("EventSelector")
evSelector.SkipEvents = 8 # // skip the first 8 events
evSelector.SkipEventSequence = [ 9, 10 ] # // skip two more events
#evSelector.SkipEventSequence = " 4-6,7 , 8 , 9 - 10 "; # // skip seven more events
#evSelector.ProcessEventSequence = "11, 12, 13,14,15, 16-20, 21 - 25 , 26- 100"; # // skip two more events
#svcMgr.EventSelector.CollectionType = "RootCollection"
#svcMgr.EventSelector.CounterTool = "EventSelectorCounterTool/Counter"

#Switch Off for TAG - start
poolAttribs = acc.getService("AthenaPoolCnvSvc").InputPoolAttributes
# Turn on the tree cache for the CollectionTree - tree cache only works for one tree.
# Set number of events for learning before turning on cache - default is 5
poolAttribs += [ "DatabaseName = '*'; TREE_CACHE_LEARN_EVENTS = '6'" ]
# And set tree cache size - default is 10 MB (10 000 000)
poolAttribs += [ "DatabaseName = '*'; ContainerName = 'CollectionTree'; TREE_CACHE = '100000'" ]

# Print out values - must have PoolSvc in info mode
poolAttribs += [ "DatabaseName = '*'; TREE_CACHE_LEARN_EVENTS = 'int'" ]
poolAttribs += [ "DatabaseName = '*'; TREE_CACHE_SIZE = 'int'" ]

# Print out for each event the number of bytes read and the number of
# read calls. With the tree cache, one should see jumps in the bytes
# read by the cache size, i.e. the bytes read should not change each
# event. However, the cache only works on one tree - the main event
# tree (CollectionTree) - and we read some things from other trees, so
# one does see a small increase event-by-event.
printOpts = acc.getService("AthenaPoolCnvSvc").PrintInputAttrPerEvt
printOpts += [ "DatabaseName = '*'; BYTES_READ = 'double'" ]
printOpts += [ "DatabaseName = '*'; READ_CALLS = 'int'" ]
#Switch Off for TAG - end

QueryTag = CompFactory.AthPoolEx.QueryTag("QueryTag", OutputLevel = DEBUG)
#svcMgr.EventSelector.HelperTools = [ QueryTag ]

# Run
import sys
sc = acc.run(flags.Exec.MaxEvents)
sys.exit(sc.isFailure())






