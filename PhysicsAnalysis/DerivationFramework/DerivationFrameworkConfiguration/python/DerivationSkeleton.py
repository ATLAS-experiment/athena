# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import sys

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import ProductionStep
from DerivationFrameworkConfiguration import DerivationConfigList
from PyJobTransforms.CommonRunArgsToFlags import commonRunArgsToFlags
from PyJobTransforms.TransformUtils import processPreExec, processPreInclude, processPostExec, processPostInclude

# force no legacy job properties
from AthenaCommon import JobProperties
JobProperties.jobPropertiesDisallowed = True


def fromRunArgs(runArgs):
    from AthenaCommon.Logging import logging
    logDerivation = logging.getLogger('Derivation')
    logDerivation.info('****************** STARTING DERIVATION *****************')

    logDerivation.info('**** Transformation run arguments')
    logDerivation.info(str(runArgs))

    logDerivation.info('**** Setting-up configuration flags')
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    commonRunArgsToFlags(runArgs, flags)

    flags.Common.ProductionStep = ProductionStep.Derivation

    # Switch on PerfMon
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(flags, runArgs)

    # Input types
    allowedInputTypes = [ 'AOD', 'DAOD_PHYS', 'EVNT' ]
    availableInputTypes = [ hasattr(runArgs, f'input{inputType}File') for inputType in allowedInputTypes ]
    if sum(availableInputTypes) != 1:
        raise ValueError('Input must be exactly one of the following types: inputAODFile, inputEVNTFile, inputDAOD_PHYSFile')
    idx = availableInputTypes.index(True)
    flags.Input.Files = getattr(runArgs, f'input{allowedInputTypes[idx]}File')

    # Augmentations
    # For the time being one parent (primary stream) can have multiple children (augmentations)
    # However, an augmentation cannot have multiple parents, which will be supported in the future
    if hasattr(runArgs, 'augmentations'):
        for val in getattr(runArgs, 'augmentations'):
            if ':' not in val or len(val.split(':')) != 2:
                logDerivation.error('Derivation job started, but with wrong augmentation syntax - aborting')
                raise ValueError('Invalid augmentation argument: {0}'.format(val))
            else:
                child, parent = val.split(':')
                flags.addFlag(f'Output.DAOD_{child}ParentStream',f'DAOD_{parent}')
                childStreamFlag = f'Output.DAOD_{parent}ChildStream'
                if not flags.hasFlag(childStreamFlag):
                    flags.addFlag(childStreamFlag, [f'DAOD_{child}'])
                else:
                    flags._set(childStreamFlag, flags._get(childStreamFlag) + [f'DAOD_{child}'])
                logDerivation.info('Setting up event augmentation as {0} => {1}'.format(child, parent))

    # Output formats
    formats = []
    if hasattr(runArgs, 'formats'):
        formats = runArgs.formats
        logDerivation.info('Will attempt to make the following derived formats: {0}'.format(formats))
    else:
        logDerivation.error('Derivation job started, but with no output formats specified - aborting')
        raise ValueError('No derived formats specified')

    # Output files
    for runArg in dir(runArgs):
        if 'output' in runArg and 'File' in runArg and 'Type' not in runArg and 'NTUP_PHYSVAL' not in runArg:
            outputFileName = getattr(runArgs, runArg)
            flagString = f'Output.{runArg.strip("output")}Name'
            flags.addFlag(flagString, outputFileName)
            flags.addFlag(f'Output.doWrite{runArg.removeprefix("output").removesuffix("File")}', True)
            flags.Output.doWriteDAOD = True

    # Fix campaign metadata
    from Campaigns.Utils import Campaign, campaign_runs
    if flags.Input.isMC and flags.Input.MCCampaign is Campaign.Unknown:
        if flags.Input.RunNumbers:
            mc_campaign = campaign_runs.get(flags.Input.RunNumbers[0], Campaign.Unknown)

            if mc_campaign is not Campaign.Unknown:
                flags.Input.MCCampaign = mc_campaign
                logDerivation.info('Will recover MC campaign to: %s', mc_campaign.value)

    # Pre-include
    processPreInclude(runArgs, flags)

    # Pre-exec
    processPreExec(runArgs, flags)

    # To respect --athenaopts 
    flags.fillFromArgs()

    # Lock flags
    flags.lock()

    # The D(2)AOD building configuration
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    # Pool file reading
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    # Ensure proper metadata propagation for EVNT->DAOD_TRUTHx
    if (allowedInputTypes[idx]=='EVNT'):
       from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
       cfg.merge(MetaDataSvcCfg(flags, ['IOVDbMetaDataTool']))

    for formatName in formats:
        derivationConfig = getattr(DerivationConfigList, f'{formatName}Cfg')
        cfg.merge(derivationConfig(flags))

    # Pass-through mode (ignore skimming and accept all events)
    if hasattr(runArgs, 'passThrough'):
        logDerivation.info('Pass-through mode was requested. All events will be written to the output.')
        for algo in cfg.getEventAlgos():
            if isinstance(algo, CompFactory.DerivationFramework.DerivationKernel):
                algo.SkimmingTools = []

    # PerfMonSD
    if flags.PerfMon.doFullMonMT or flags.PerfMon.doFastMonMT:
       from PerfMonComps.PerfMonCompsConfig import PerfMonMTSvcCfg
       cfg.merge(PerfMonMTSvcCfg(flags))

    # Write AMI tag into in-file metadata
    from PyUtils.AMITagHelperConfig import AMITagCfg
    cfg.merge(AMITagCfg(flags, runArgs, fixBroken=True))

    # Fix generator metadata
    if flags.Input.isMC:
        from GeneratorConfig.Versioning import GeneratorVersioningFixCfg
        cfg.merge(GeneratorVersioningFixCfg(flags))

    # Write TagInfo
    if not flags.Input.isMC and flags.Input.DataYear > 0:
        from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
        cfg.merge(TagInfoMgrCfg(flags, tagValuePairs={
            "data_year": str(flags.Input.DataYear)
        }))
    else:
        from EventInfoMgt.TagInfoMgrConfig import TagInfoMgrCfg
        cfg.merge(TagInfoMgrCfg(flags))

    # Set EventPrintoutInterval to 100 events
    #  (in run interactive mode there's no loop manager service, do nothing in this case)
    if flags.Exec.Interactive != 'run' :
        cfg.getService(cfg.getAppProps()['EventLoop']).EventPrintoutInterval = 100

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)
    
    # Run the final configuration
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
