#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

## This file runs runHLT_standalone with external menus
# 3 tests of the CF to cover different menu generation frameworks and data inputs:

#   - menuManual: chains are generated manually in generateCFChains, and run on input data file
#   - emuMenuTest: chains are generated in the menu framework as HLT_TestChain**, and run on emulated data 
#   - emuManual: chains are generated manually and run on emulated data


class testopt:
    menuType  = 'menuManual' # use either menu or manual chain building

from AthenaCommon.Logging import logging
log = logging.getLogger('test_menu_cf')
from AthenaCommon.Constants import DEBUG

log.info('Setup options:')
defaultOptions = [a for a in dir(testopt) if not a.startswith('__')]
for option in defaultOptions:
    if option in globals():
        setattr(testopt, option, globals()[option])
        log.info(' %20s = %s' , option, getattr(testopt, option))
    else:        
        log.info(' %20s = (Default) %s' , option, getattr(testopt, option))


# runHLT_standalone still requires global ConfigFlags usage:
from AthenaConfiguration.AllConfigFlags import ConfigFlags as flags
from TriggerJobOpts.TriggerConfigFlags import ROBPrefetching
flags.Trigger.generateMenuDiagnostics = True
flags.Trigger.triggerMenuSetup = "Dev_pp_run3_v1"
flags.Trigger.ROBPrefetchingOptions = [ROBPrefetching.StepRoI]
flags.Trigger.enableL1MuonPhase1 = False  # doesn't work in this minimal setup

createHLTMenuExternally=True # menu will be build up explicitly here 
doWriteRDOTrigger = False
doWriteBS = False
forceEnableAllChains=True
# enable singatures separately
flags.Trigger.enabledSignatures = ['Muon', 'Tau','MinBias','Bphysics','Egamma', 'Electron', 'Photon', 'MET', 'Jet','Bjet','Calib']


# load all configuration as the real HLT
include("TriggerJobOpts/runHLT_standalone.py")

from MuonIdHelpers.MuonIdHelpersConfigLegacy import MuonIdHelperSvc
svcMgr+=MuonIdHelperSvc()

# make menu manually here:
from TriggerMenuMT.HLT.Config.ControlFlow.HLTCFConfig import makeHLTTree
from TriggerMenuMT.HLT.Config.Utility.HLTMenuConfig import HLTMenuConfig
from TriggerMenuMT.CFtest.generateCFChains import generateCFChains
from TriggerMenuMT.CFtest.EmuStepProcessingConfig import generateHLTSeedingAndChainsManually, generateHLTSeedingAndChainsByMenu


from AthenaCommon.AlgSequence import AlgSequence
topSequence = AlgSequence()

if testopt.menuType == 'menuManual':
    generateCFChains(flags)
    from TriggerMenuMT.HLT.Config.Validation.CheckL1HLTConsistency import checkL1HLTConsistency
    checkL1HLTConsistency(flags)
elif testopt.menuType == 'emuMenuTest':
    # HLT_TestChain
    generateHLTSeedingAndChainsByMenu(flags, topSequence)
elif testopt.menuType == 'emuManual':
    generateHLTSeedingAndChainsManually(flags, topSequence)
else:
    log.error("Input parameter %s not accepted",testopt.menuType)


# set DEBUG flag on the control-flow builder (before building)

import TriggerMenuMT.HLT.Config.ControlFlow.HLTCFConfig
TriggerMenuMT.HLT.Config.ControlFlow.HLTCFConfig.log.setLevel(DEBUG)

# from here generate the ControlFlow and the Dataflow
# doing the same as menu.generateMT()
makeHLTTree(flags, hltMenuConfig=HLTMenuConfig )

       
from TriggerMenuMT.HLT.Config.JSON.HLTMenuJSON import generateJSON
generateJSON(flags)

from TriggerMenuMT.HLT.Config.JSON.HLTPrescaleJSON import generateJSON as generatePrescaleJSON
generatePrescaleJSON(flags)
   
from TriggerMenuMT.HLT.Config.JSON.HLTMonitoringJSON import generateDefaultMonitoringJSON
generateDefaultMonitoringJSON(flags)

# now some debug
print ("EmuStepProcessing: dump top Sequence after CF/DF Tree build")
from AthenaCommon.AlgSequence import dumpSequence
dumpSequence( topSequence )

from TriggerJobOpts.TriggerConfig import collectHypos, collectFilters
from AthenaCommon.CFElements import findSubSequence
hypos   = collectHypos(findSubSequence(topSequence, "HLTAllSteps"))
filters = collectFilters(findSubSequence(topSequence, "HLTAllSteps"))

nfilters = sum(len(v) for v in filters.values())
nhypos = sum(len(v) for v in hypos.values())
log.info( "Algorithms counting: Number of Filter algorithms: %d  -  Number of Hypo algorithms: %d", nfilters , nhypos) 


# switch on DEBUG on the trigger monitoring
topSequence.HLTTop.HLTEndSeq.TrigSignatureMoni.OutputLevel = DEBUG

from AthenaCommon.CFElements import getSequenceChildren, isSequence
for alg in getSequenceChildren( topSequence.HLTTop.HLTAllSteps ):
         if isSequence( alg ):
             continue
 
         if "TriggerSummary" in alg.getName():
             alg.OutputLevel = DEBUG

