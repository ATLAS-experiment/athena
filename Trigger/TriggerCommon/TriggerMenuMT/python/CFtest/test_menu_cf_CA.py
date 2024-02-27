#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

## This file runs runHLT with external menus
# 3 tests of the CF to cover different menu generation frameworks and data inputs:

#   - menuManual: chains are generated manually in generateCFChains, and run on input data file
#   - emuMenuTest: chains are generated in the menu framework as HLT_TestChain**, and run on emulated data 
#   - emuManual: chains are genrated manually and run on emulated data

from AthenaCommon.Logging import logging
log = logging.getLogger('test_menu_cf_CA')
from AthenaCommon.Constants import DEBUG


# load all configuration as the real HLT
def set_flags(flags):
        
    # Prevent usage of legacy job properties
    from AthenaCommon import JobProperties
    JobProperties.jobPropertiesDisallowed = True

    # generic trigger flags
    flags.Trigger.doHLT = True    # needs to be set early as other flags depend on it
    flags.Trigger.EDMVersion = 3  # Run-3 EDM
    flags.Common.isOnline = True    # online environment
    flags.Input.Files = []          # menu cannot depend on input files
    
    from TriggerJobOpts.TriggerConfigFlags import ROBPrefetching
    flags.Trigger.triggerMenuSetup = "Dev_pp_run3_v1"
    flags.Trigger.ROBPrefetchingOptions = [ROBPrefetching.StepRoI]
    flags.Trigger.enableL1MuonPhase1 = False  # doesn't work in this minimal setup
    flags.Trigger.enabledSignatures = ['Muon', 'Tau','MinBias','Bphysics','Egamma', 'Electron', 'Photon', 'MET', 'Jet','Bjet','Calib']
    log.info("Running on these signatures: %s",flags.Trigger.enabledSignatures)
    
    # these flags are proper for these tests
    flags.Trigger.doCFEmulationTest = True # this enables this emulation tests
    flags.Trigger.generateMenuDiagnostics = True
    # set DEBUG flag on the control-flow builder (before building)
    import TriggerMenuMT.HLT.Config.ControlFlow.HLTCFConfig
    TriggerMenuMT.HLT.Config.ControlFlow.HLTCFConfig.log.setLevel(DEBUG)


def configure(flags, args):
   
    from TriggerMenuMT.CFtest.generateCFChains import generateCFChains
    from TriggerMenuMT.CFtest.EmuStepProcessingConfig import generateEmuEvents, generateChainsManually, generateEmuMenu
    
    if args.menuType == 'menuManual': 
        # test generating chains from real sequences and real data
        generateCFChains(flags)     
    elif args.menuType == 'emuMenuTest': 
        # test using menu code (HLT_TestChain) with dummy sequences and events
        generateEmuEvents()
        generateEmuMenu(flags)
    elif args.menuType == 'emuManual':
        # test generating emulation chains with dummy segments and events
        generateEmuEvents()
        generateChainsManually(flags)
    else:
        log.error("Input parameter %s not accepted",args.menuType)


def makeMenu(flags):
    from TrigConfigSvc.TrigConfigSvcCfg import generateL1Menu
    generateL1Menu(flags)
    # from here generate the ControlFlow and the Dataflow
    # doing the same as menu.generateMT()
    from TriggerMenuMT.HLT.Config.GenerateMenuMT import makeHLTTree
    menuCA = makeHLTTree(flags)
    return menuCA

def main():
    # Make sure nobody uses deprecated global ConfigFlags
    import AthenaConfiguration.AllConfigFlags
    del AthenaConfiguration.AllConfigFlags.ConfigFlags

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    set_flags(flags)
    # Add options to command line parser
    parser = flags.getArgumentParser()

    parser.add_argument('--menuType', default='emuMenuTest', #'emuManual',
                       help='use either menu or manual chain building')

    # Fill flags from command line
    args = flags.fillFromArgs(parser=parser)
    log.info('Setup options:')
    log.info(' %20s = %s' , 'menuType', args.menuType)
  
    # Configure main services
    _allflags = flags.clone()   # copy including Concurrency flags
    _allflags.lock()
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(_allflags)
    del _allflags    
    flags.lock()

    configure(flags, args)
    from TriggerJobOpts.TriggerConfig import triggerRunCfg
    menu = triggerRunCfg(flags, menu=makeMenu)
    cfg.merge(menu)
    cfg.printConfig(withDetails=False, summariseProps=True, printDefaults=True)

    from AthenaConfiguration.AccumulatorCache import AccumulatorDecorator
    AccumulatorDecorator.printStats()

    from AthenaCommon.CFElements import isSequence
    for alg in menu.getSequence("HLTAllSteps").Members:
         if isSequence( alg ):
             continue
 
         if "TriggerSummary" in alg.getName():            
             alg.OutputLevel = DEBUG

    return cfg

# This entry point is only used when running in athena
if __name__ == "__main__":
    import sys
    sys.exit(main().run().isFailure())
