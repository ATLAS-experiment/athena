# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# AnaAlgorithm import(s):
from AnaAlgorithm.AnaAlgSequence import AnaAlgSequence
from AnaAlgorithm.DualUseConfig import createAlgorithm, addPrivateTool
from AnalysisAlgorithmsConfig.ConfigAccumulator import DataType
from Campaigns.Utils import Campaign

import logging
log = logging.getLogger('makePileupAnalysisSequence')

def makePileupAnalysisSequence( dataType, campaign=None, files=None, useDefaultConfig=False, userLumicalcFiles=None, userPileupConfigs=None ):
    """Create a PRW analysis algorithm sequence

    Keyword arguments:
      dataType -- The data type enum
    """

    if isinstance(dataType, str):
        dataType = {
            "data": DataType.Data,
            "fullsim": DataType.FullSim,
            "fastsim": DataType.FastSim,
            "mc": DataType.FullSim,
            "afii": DataType.FastSim,
        }.get(dataType)

        if not dataType:
            raise ValueError ("invalid data type")

    # Create the analysis algorithm sequence object:
    seq = AnaAlgSequence( "PileupAnalysisSequence" )

    # TODO: support per-campaign config

    toolConfigFiles = []
    toolLumicalcFiles = []
    if files is not None and (campaign is None or campaign is Campaign.Unknown or userPileupConfigs is None):
        if campaign is None or campaign is Campaign.Unknown:
            from Campaigns.Utils import getMCCampaign
            campaign = getMCCampaign(files)
            if campaign:
                log.info(f'Autoconfiguring PRW with campaign: {campaign}')
            else:
                log.info('Campaign could not be determined.')

        if campaign:
            if userPileupConfigs is None:
                if dataType is DataType.Data:
                    log.info('Data needs no configuration files')
                else:
                    from PileupReweighting.AutoconfigurePRW import getConfigurationFiles
                    toolConfigFiles = getConfigurationFiles(campaign=campaign, files=files, useDefaultConfig=useDefaultConfig)
                    log.info('Setting PRW configuration based on input files')

                    if toolConfigFiles:
                        log.info(f'Using PRW configuration: {", ".join(toolConfigFiles)}')
            else:
                log.info('Using user provided PRW configuration')

    if userPileupConfigs is not None:
        toolConfigFiles = userPileupConfigs[:]

    if userLumicalcFiles is not None:
        toolLumicalcFiles = userLumicalcFiles[:]
        log.info(f'Using user-provided lumicalc files: {", ".join(toolLumicalcFiles)}')
    else:
        from PileupReweighting.AutoconfigurePRW import getLumicalcFiles
        toolLumicalcFiles = getLumicalcFiles(campaign)
        log.info(f'Using autoconfigured lumicalc files: {", ".join(toolLumicalcFiles)}')

    # Set up the only algorithm of the sequence:
    alg = createAlgorithm( 'CP::PileupReweightingAlg', 'PileupReweightingAlg' )
    addPrivateTool( alg, 'pileupReweightingTool', 'CP::PileupReweightingTool' )
    alg.pileupReweightingTool.ConfigFiles = toolConfigFiles
    if not toolConfigFiles and dataType is not DataType.Data:
        log.info("No PRW config files provided. Disabling reweighting")
        # Setting the weight decoration to the empty string disables the reweighting
        alg.pileupWeightDecoration = ""
    alg.pileupReweightingTool.LumiCalcFiles = toolLumicalcFiles

    seq.append( alg, inputPropName = {} )

    # Return the sequence:
    return seq
