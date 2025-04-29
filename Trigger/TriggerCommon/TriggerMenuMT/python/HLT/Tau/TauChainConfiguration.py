# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

########################################################################
#
# SliceDef file for tau chains
#
#########################################################################

from AthenaCommon.Logging import logging
logging.getLogger().info(f'Importing {__name__}')
log = logging.getLogger(__name__)

from TriggerMenuMT.HLT.Config.ChainConfigurationBase import ChainConfigurationBase

from .TauMenuSequences import (
    tauCaloMVAMenuSequenceGenCfg,
    tauFTFTauCoreSequenceGenCfg, tauFTFTauLRTSequenceGenCfg,
    tauFTFTauIsoSequenceGenCfg,
    tauPrecTrackIsoSequenceGenCfg, tauPrecTrackLRTSequenceGenCfg, 
    tauPrecisionSequenceGenCfg, tauPrecisionLRTSequenceGenCfg)

from .TauConfigurationTools import getChainSequenceConfigName, getChainPrecisionSeqName


############################################# 
###  Class to configure tau chains 
#############################################

class TauChainConfiguration(ChainConfigurationBase):
    def __init__(self, chainDict):
        ChainConfigurationBase.__init__(self, chainDict)
        
    def assembleChainImpl(self, flags):                            
        log.debug(f'Assembling chain for {self.chainName}')

        chain_steps = []

        # Overall Tau Trigger sequences steps: 
        step_dictionary = {
            # BRT calibration chains
            'ptonly'        : ['getCaloMVA', 'getFTFCoreEmpty', 'getFTFIsoEmpty', 'getPrecTrackEmpty', 'getPrecisionEmpty'],

            # 2-step tracking + ID chains 
            'tracktwoMVA'   : ['getCaloMVA', 'getFTFCore'     , 'getFTFIso'     , 'getPrecTrackIso'  , 'getPrecision'     ],
            'tracktwoLLP'   : ['getCaloMVA', 'getFTFCore'     , 'getFTFIso'     , 'getPrecTrackIso'  , 'getPrecision'     ],

            # LRT chains
            'trackLRT'      : ['getCaloMVA', 'getFTFLRT'      , 'getFTFIsoEmpty', 'getPrecTrackLRT'  , 'getPrecisionLRT'  ],
        }

        steps = step_dictionary[getChainSequenceConfigName(self.chainPart)]
        for step in steps:
            if 'Empty' in step:
                chain_step = getattr(self, step)(flags)
            else:
                is_probe_leg = self.chainPart['tnpInfo']=='probe'
                chain_step = getattr(self, step)(flags, is_probe_leg=is_probe_leg)

            chain_steps.append(chain_step)
    
        return self.buildChain(chain_steps)
    

    #--------------------------------------------------
    # Step 1: CaloMVA reconstruction
    #--------------------------------------------------
    def getCaloMVA(self, flags, is_probe_leg=False):
        stepName = 'CaloMVA_tau'
        return self.getStep(flags, stepName, [tauCaloMVAMenuSequenceGenCfg], is_probe_leg=is_probe_leg)

        
    #--------------------------------------------------
    # Step 2: 1st FTF stage (FTFCore/LRT)
    #--------------------------------------------------
    def getFTFCore(self, flags, is_probe_leg=False):
        stepName = 'FTFCore_tau'
        return self.getStep(flags, stepName, [tauFTFTauCoreSequenceGenCfg], is_probe_leg=is_probe_leg)

    def getFTFLRT(self, flags, is_probe_leg=False):
        stepName = 'FTFLRT_tau'
        return self.getStep(flags, stepName, [tauFTFTauLRTSequenceGenCfg], is_probe_leg=is_probe_leg)

    def getFTFCoreEmpty(self, flags):
        stepName = 'FTFCoreEmpty_tau'
        return self.getEmptyStep(stepName)


    #--------------------------------------------------
    # Step 3: 2nd FTF stage (FTFIso)
    #--------------------------------------------------
    def getFTFIso(self, flags, is_probe_leg=False):
        stepName = 'FTFIso_tau'
        return self.getStep(flags, stepName, [tauFTFTauIsoSequenceGenCfg], is_probe_leg=is_probe_leg)

    def getFTFIsoEmpty(self, flags):
        stepName = 'FTFIsoEmpty_tau'
        return self.getEmptyStep(stepName)


    #--------------------------------------------------
    # Step 4: Precision tracking
    #--------------------------------------------------
    def getPrecTrackIso(self, flags, is_probe_leg=False):
        stepName = 'PrecTrkIso_tau'
        return self.getStep(flags, stepName, [tauPrecTrackIsoSequenceGenCfg], is_probe_leg=is_probe_leg)

    def getPrecTrackLRT(self, flags, is_probe_leg=False):
        stepName = 'PrecTrkLRT_tau'
        return self.getStep(flags, stepName, [tauPrecTrackLRTSequenceGenCfg], is_probe_leg=is_probe_leg)

    def getPrecTrackEmpty(self, flags):
        stepName = 'PrecTrkEmpty_tau'
        return self.getEmptyStep(stepName)


    #--------------------------------------------------
    # Step 5: Precision reconstruction + ID
    #--------------------------------------------------
    def getPrecision(self, flags, is_probe_leg=False):
        sequenceName = getChainPrecisionSeqName(self.chainPart)
        stepName = f'Precision_{sequenceName}_tau'
        return self.getStep(
            flags, 
            stepName, 
            [tauPrecisionSequenceGenCfg],
            seq_name=sequenceName,
            is_probe_leg=is_probe_leg,
        )

    def getPrecisionLRT(self, flags, is_probe_leg=False):
        sequenceName = getChainPrecisionSeqName(self.chainPart)
        stepName = f'Precision_{sequenceName}_tau'
        return self.getStep(
            flags, 
            stepName, 
            [tauPrecisionLRTSequenceGenCfg],
            seq_name=sequenceName,
            is_probe_leg=is_probe_leg,
        )

    def getPrecisionEmpty(self, flags):
        stepName = 'PrecisionEmpty_tau'
        return self.getEmptyStep(stepName)
