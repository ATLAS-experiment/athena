# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

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
    tauCaloMVASequenceGenCfg,
    tauCaloHitsSequenceGenCfg,
    tauFTFCoreSequenceGenCfg,
    tauFTFIsoSequenceGenCfg,
    tauPrecTrackSequenceGenCfg, 
    tauPrecisionSequenceGenCfg,
)

from .TauConfigurationTools import (
    getChainSequenceConfigName,
    getChainCaloHitsSeqName, getHitZConfig,
    getChainPrecisionSeqName,
)



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
            'ptonly'                : ['getCaloMVA', 'getCaloHitsEmpty', 'getFTFCoreEmpty', 'getFTFIsoEmpty', 'getPrecTrackEmpty', 'getPrecisionEmpty'],

            # 2-step tracking + ID chains 
            'tracktwoMVA'           : ['getCaloMVA', 'getCaloHitsEmpty', 'getFTFCore'     , 'getFTFIso'     , 'getPrecTrackIso'  , 'getPrecision'     ],
            'tracktwoLLP'           : ['getCaloMVA', 'getCaloHitsEmpty', 'getFTFCore'     , 'getFTFIso'     , 'getPrecTrackIso'  , 'getPrecision'     ],

            # CaloHits + 2-step tracking + ID chains
            'CaloHits_tracktwoMVA'  : ['getCaloMVA', 'getCaloHits'     , 'getFTFCore'     , 'getFTFIso'     , 'getPrecTrackIso'  , 'getPrecision'     ],

            # LRT chains
            'trackLRT'              : ['getCaloMVA', 'getCaloHitsEmpty', 'getFTFLRT'      , 'getFTFIsoEmpty', 'getPrecTrackLRT'  , 'getPrecision'     ],
        }

        steps = step_dictionary[getChainSequenceConfigName(self.chainPart)]
        for step in steps:
            if 'Empty' in step:
                chain_step = getattr(self, step)(flags)
            else:
                is_probe_leg = self.chainPart['tnpInfo']=='probe'
                jet = self.chainPart['jet']
                chain_step = getattr(self, step)(flags, is_probe_leg=is_probe_leg, jet=jet)
            
            chain_steps.append(chain_step)
    
        return self.buildChain(chain_steps)
    

    #--------------------------------------------------
    # Step 1: CaloMVA reconstruction
    #--------------------------------------------------
    def getCaloMVA(self, flags, is_probe_leg=False, jet='lc'):
        stepName = f'Calo{jet.upper()}MVA_tau'
        return self.getStep(flags, stepName, [tauCaloMVASequenceGenCfg], is_probe_leg=is_probe_leg, jet=jet)

    
    #--------------------------------------------------
    # Step 2: Calo+Hits reconstruction + preselection
    #--------------------------------------------------
    def getCaloHits(self, flags, is_probe_leg=False, jet='lc'):
        sequenceName = getChainCaloHitsSeqName(self.chainPart)
        stepName = f'CaloHits_{sequenceName}_tau'
        return self.getStep(
            flags, 
            stepName, 
            [tauCaloHitsSequenceGenCfg],
            seq_name=sequenceName,
            precision_seq_name=getChainPrecisionSeqName(self.chainPart),
            hitz_config=getHitZConfig(flags, self.chainPart),
            is_probe_leg=is_probe_leg,
            jet=jet
        )

    def getCaloHitsEmpty(self, flags):
        stepName = 'CaloHitsEmpty_tau'
        return self.getEmptyStep(stepName)

        
    #--------------------------------------------------
    # Step 3: 1st FTF stage (FTFCore/LRT)
    #--------------------------------------------------
    def getFTFCore(self, flags, is_probe_leg=False, jet='lc'):
        stepName = f'FTFCore_{jet.upper()}'
        if calohits_seq_name := getChainCaloHitsSeqName(self.chainPart):
            stepName += f'_fromCaloHits_{calohits_seq_name}'
        stepName += '_tau'

        return self.getStep(
            flags, 
            stepName, 
            [tauFTFCoreSequenceGenCfg], 
            calohits_seq_name=calohits_seq_name,
            is_probe_leg=is_probe_leg,
            jet=jet
        )

    def getFTFLRT(self, flags, is_probe_leg=False, jet='lc'):
        stepName = 'FTFLRT_tau'
        return self.getStep(
            flags,
            stepName,
            [tauFTFCoreSequenceGenCfg],
            do_lrt=True,
            is_probe_leg=is_probe_leg,
            jet=jet
        )

    def getFTFCoreEmpty(self, flags):
        stepName = 'FTFCoreEmpty_tau'
        return self.getEmptyStep(stepName)


    #--------------------------------------------------
    # Step 4: 2nd FTF stage (FTFIso)
    #--------------------------------------------------
    def getFTFIso(self, flags, is_probe_leg=False, jet='lc'):
        stepName = f'FTFIso_{jet.upper()}'
        if calohits_seq_name := getChainCaloHitsSeqName(self.chainPart):
            stepName += f'_fromCaloHits_{calohits_seq_name}'
        stepName += '_tau'

        return self.getStep(
            flags,
            stepName,
            [tauFTFIsoSequenceGenCfg],
            calohits_seq_name=calohits_seq_name,
            is_probe_leg=is_probe_leg,
            jet=jet
        )

    def getFTFIsoEmpty(self, flags):
        stepName = 'FTFIsoEmpty_tau'
        return self.getEmptyStep(stepName)


    #--------------------------------------------------
    # Step 5: Precision tracking
    #--------------------------------------------------
    def getPrecTrackIso(self, flags, is_probe_leg=False, jet='lc'):
        stepName = f'PrecTrkIso_{jet.upper()}'
        if calohits_seq_name := getChainCaloHitsSeqName(self.chainPart):
            stepName += f'_fromCaloHits_{calohits_seq_name}'
        stepName += '_tau'

        return self.getStep(
            flags,
            stepName,
            [tauPrecTrackSequenceGenCfg],
            calohits_seq_name=calohits_seq_name,
            is_probe_leg=is_probe_leg,
            jet=jet
        )

    def getPrecTrackLRT(self, flags, is_probe_leg=False, jet='lc'):
        stepName = 'PrecTrkLRT_tau'
        return self.getStep(
            flags,
            stepName,
            [tauPrecTrackSequenceGenCfg],
            do_lrt=True,
            is_probe_leg=is_probe_leg,
            jet=jet
        )

    def getPrecTrackEmpty(self, flags):
        stepName = 'PrecTrkEmpty_tau'
        return self.getEmptyStep(stepName)


    #--------------------------------------------------
    # Step 6: Precision reconstruction + ID
    #--------------------------------------------------
    def getPrecision(self, flags, is_probe_leg=False, jet='lc'):
        sequenceName = getChainPrecisionSeqName(self.chainPart)
        stepName = f'Precision_{sequenceName}_{jet.upper()}'
        if calohits_seq_name := getChainCaloHitsSeqName(self.chainPart):
            stepName += f'_fromCaloHits_{calohits_seq_name}'
        stepName += '_tau'

        return self.getStep(
            flags, 
            stepName, 
            [tauPrecisionSequenceGenCfg],
            seq_name=sequenceName,
            calohits_seq_name=calohits_seq_name,
            do_lrt='LRT' in sequenceName,
            is_probe_leg=is_probe_leg,
            jet=jet
        )

    def getPrecisionEmpty(self, flags):
        stepName = 'PrecisionEmpty_tau'
        return self.getEmptyStep(stepName)
