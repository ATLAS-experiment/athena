# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

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
    flags.Exec.EventPrintoutInterval = 100
    commonRunArgsToFlags(runArgs, flags)

    flags.Common.ProductionStep = ProductionStep.Derivation

    # Switch on PerfMon
    from PerfMonComps.PerfMonConfigHelpers import setPerfmonFlagsFromRunArgs
    setPerfmonFlagsFromRunArgs(flags, runArgs)

    # Input types
    allowedInputTypes = [ 'AOD', 'DAOD_PHYS', 'DAOD_PHYSLITE', 'EVNT' ]
    availableInputTypes = [ hasattr(runArgs, f'input{inputType}File') for inputType in allowedInputTypes ]
    if sum(availableInputTypes) != 1:
        raise ValueError('Input must be exactly one of the following types: '+','.join(map(str, availableInputTypes)))
    idx = availableInputTypes.index(True)
    flags.Input.Files = getattr(runArgs, f'input{allowedInputTypes[idx]}File')

    # Augmentations
    # For the time being one parent (primary stream) can have multiple children (augmentations)
    # However, an augmentation cannot have multiple parents, which will be supported in the future
    if hasattr(runArgs, 'augmentations'):
        for val in runArgs.augmentations:
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

    # Command line skimming - limited to SKIM format and PHYS/PHYSLITE input
    if hasattr(runArgs, 'skimmingExpression'):
        if runArgs.skimmingExpression:
            if not (len(formats)==1 and formats[0]=='SKIM'):
                raise ValueError('Command-line skimming only available with SKIM format')
            availableInputTypes = [ hasattr(runArgs, f'input{inputType}File') for inputType in [ 'DAOD_PHYS', 'DAOD_PHYSLITE' ] ]
            if sum(availableInputTypes) != 1:
                raise ValueError('Command-line skimming only available with input types '+','.join(map(str, availableInputTypes))) 
            flags.Derivation.skimmingExpression = runArgs.skimmingExpression
            if not hasattr(runArgs, 'skimmingContainers'):
                logDerivation.warning('All containers used for skimming must be listed with the skimmingContainers option - job likely to fail')
            else:
                flags.Derivation.dynamicConsumers = runArgs.skimmingContainers

    # Output files
    for runArg in dir(runArgs):
        if 'output' in runArg and 'File' in runArg and 'Type' not in runArg and 'NTUP_PHYSVAL' not in runArg:
            outputFileName = getattr(runArgs, runArg)
            flagString = f'Output.{runArg.removeprefix("output")}Name'
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

    # Fix MC channel number with EVNT files
    if allowedInputTypes[idx] == 'EVNT':
        # runNumber is MC channel number in reco
        if hasattr(runArgs, 'runNumber') and flags.Input.MCChannelNumber != runArgs.runNumber:
            logDerivation.warning('Got different MC channel number (%d) from runNumber than from metadata (%d)', runArgs.runNumber, flags.Input.MCChannelNumber)
            flags.Input.MCChannelNumber = runArgs.runNumber
        elif flags.Input.MCChannelNumber == 0 and flags.Input.RunNumbers and flags.Input.RunNumbers[0] != 0:
            logDerivation.info('Will recover MC channel number to: %d', flags.Input.RunNumbers[0])
            flags.Input.MCChannelNumber = flags.Input.RunNumbers[0]
        else:
            logDerivation.info('MC channel number: %d', flags.Input.MCChannelNumber)

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

    # Further workaround for issues with overlap removal.
    # As further explained in JetCommonConfig.AddEventCleanFlagsCfg,
    # we can schedule multiple overlap removal algorithms which overwrite
    # each other's decorations.  To get the decorations locked, we put
    # all the OR-related decoration algorithms in the EventCleanSeq
    # sequence followed by decoration locking algorithms in EventCleanLockSeq.
    # Each derivation format ensures that these sequences are in the
    # correct place.  But we can still run into trouble when multiple
    # formats are combined.  When we merge two CAs both of which
    # contain the same sequence S, S will end up with the algorithms
    # from both, but S will stay at its existing position in the
    # first (destination) CA.  However, for these sequences, we need
    # them to run at the sequence's position in the second (source) CA,
    # i.e., the later of the two positions.  We accomplish this by
    # munging the CAs before merging: remove the sequence from the
    # destination CA and merge its algorithms to the source CA.
    def premerge (cfg, newcfg, seqnam):
        # Check if both CAs contain the requested sequence.
        seq = cfg.getSequence(seqnam)
        if not seq: return
        newseq = newcfg.getSequence(seqnam)
        if not newseq: return

        # Make a temporary CA with the sequence to hold algorithms
        # removed from CFG.
        from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
        ca = ComponentAccumulator()
        ca.addSequence (CompFactory.AthSequencer (seqnam, Sequential=True))

        # Remove the sequence's algorithms from CFG and add to the temp CA.
        for a in seq.Members:
            ca.addEventAlgo (cfg.popEventAlgo (a.getName(), seqnam), seqnam)

        # Remove the sequence itself (which should now by empty) from CFG.
        cfg.getSequence('AthAlgSeq').Members.remove (seq)

        # Add the moved algorithms to NEWCFG.
        newcfg.merge (ca)
        return

    for formatName in formats:
        derivationConfig = getattr(DerivationConfigList, f'{formatName}Cfg')
        newcfg = derivationConfig(flags)
        premerge (cfg, newcfg, 'EventCleanSeq')
        premerge (cfg, newcfg, 'EventCleanLockSeq')
        cfg.merge(newcfg)

    # Pass-through mode (ignore skimming and accept all events)
    if hasattr(runArgs, 'passThrough'):
        logDerivation.info('Pass-through mode was requested. All events will be written to the output.')
        for algo in cfg.getEventAlgos():
            if isinstance(algo, CompFactory.DerivationFramework.DerivationKernel):
                algo.SkimmingTools = []

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

    # Post-include
    processPostInclude(runArgs, flags, cfg)

    # Post-exec
    processPostExec(runArgs, flags, cfg)

    # Configure components' logging levels
    from AthenaConfiguration.Utils import setupLoggingLevels
    setupLoggingLevels(flags, cfg)
    
    # Run the final configuration
    sc = cfg.run()
    sys.exit(not sc.isSuccess())
