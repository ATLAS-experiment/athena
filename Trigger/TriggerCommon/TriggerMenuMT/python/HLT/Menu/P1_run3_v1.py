# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# P1_run3_v1.py menu containing monitoring chains used only at P1
#------------------------------------------------------------------------#

# This defines the input format of the chain and it's properties with the defaults set
# always required are: name, stream and groups
#['name', 'L1chainParts'=[], 'stream', 'groups', 'merging'=[], 'topoStartFrom'=False],
#from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp

from ..Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

from .Physics_pp_run3_v1 import (
    SingleMuonGroup,
    SinglePhotonGroup,
    SingleJetGroup,
    JetStreamersGroup,
    MinBiasGroup,
    ZeroBiasGroup,
    SupportGroup,
    SupportLegGroup,
    SupportPhIGroup,
    Topo2Group,
    Topo3Group,
    LegacyTopoGroup,
)

from AthenaCommon.Logging import logging
log = logging.getLogger( __name__ )

def addCommonP1Signatures(chains):

    log.info('[setupMenu] Adding common P1 menu chains now')
    
    chainsP1 = ChainStore()

    chainsP1['Muon'] = [
        # ATR-20650
        ChainProp(name='HLT_mu0_muoncalib_L1MU3V_EMPTY', stream=['Muon_Calibration'], groups=['RATE:Muon_Calibration','BW:Muon']),
        ChainProp(name='HLT_mu0_muoncalib_L1MU14FCH', stream=['Muon_Calibration'], groups=['RATE:Muon_Calibration','BW:Muon']),
        ChainProp(name='HLT_mu0_muoncalib_L1MU3V', stream=['Muon_Calibration'], groups=['RATE:Muon_Calibration','BW:Muon']),
        ChainProp(name='HLT_mu0_muoncalib_L1MU5VF', stream=['Muon_Calibration'], groups=['RATE:Muon_Calibration','BW:Muon']),
    ]
    
    chainsP1['Egamma'] = [
        # ATR-21355 - cannot be moved to the calibSlice because they need to configure the photon/ sequence
        #ATR-27253, Phase-I
        ChainProp(name='HLT_g12_loose_LArPEBHLT_L1eEM12L', stream=['LArCells'], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g20_loose_LArPEBHLT_L1eEM18', stream=['LArCells'], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g40_loose_LArPEBHLT_L1eEM26M', stream=['LArCells'], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g60_loose_LArPEBHLT_L1eEM26M', stream=['LArCells'], groups=SinglePhotonGroup+SupportPhIGroup),
        ChainProp(name='HLT_g80_loose_LArPEBHLT_L1eEM26M', stream=['LArCells'], groups=SinglePhotonGroup+SupportPhIGroup),

    ]

    chainsP1['Jet'] = [

        # ATR-21355 - cannot be moved to the calibSlice because they need to configure the photon/ sequence
        ChainProp(name='HLT_j25_LArPEBHLT_L1J15', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j25_LArPEBHLT_L1jJ40', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j25f_LArPEBHLT_L1jJ40p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j40_LArPEBHLT_L1J20', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j40_LArPEBHLT_L1jJ50', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j75f_LArPEBHLT_L1jJ60p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j140f_LArPEBHLT_L1J75p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=SingleJetGroup+SupportLegGroup),
        ChainProp(name='HLT_j140f_LArPEBHLT_L1jJ125p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=SingleJetGroup+SupportPhIGroup),
        ChainProp(name='HLT_j165_LArPEBHLT_L1J100', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=SingleJetGroup+SupportLegGroup),
    ]

    chainsP1['Calib'] = [
        ChainProp(name='HLT_noalg_LArPEBCalib_L1RD0_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['LArPEB'], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1RD0_BGRP11', l1SeedThresholds=['FSNOSEED'], stream=['LArPEB'], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1RD2_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['LArPEB'], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_Lvl1CaloPEB_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['L1CaloCalib'], groups=['RATE:Calibration','BW:Detector']),

        ChainProp(name='HLT_noalg_LArPEBCalib_L1jJ500_LAR', l1SeedThresholds=['FSNOSEED'], stream=['LArPEBDigitalTrigger'], groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1LAR-ZEE', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['RATE:Calibration','BW:Detector']+SupportLegGroup+LegacyTopoGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1LAR-ZEE-eEM', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup+Topo2Group),

        ChainProp(name='HLT_noalg_LATOMEPEB_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['LArPEBDigitalTrigger'], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_LATOMEPEB_L1RD0_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['LArPEBDigitalTrigger'], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_LATOMEPEB_L1RD0_FIRSTEMPTY', l1SeedThresholds=['FSNOSEED'], stream=['LArPEBDigitalTrigger'], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_LATOMEPEB_L1RD0_BGRP7', l1SeedThresholds=['FSNOSEED'], stream=['LArPEBDigitalTrigger'], groups=['RATE:Calibration','BW:Detector']),

        ChainProp(name='HLT_noalg_LArPEBCalib_L1J15', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1jJ40p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1J20', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1jJ60p30ETA49', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1J75p31ETA49', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1J100', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['RATE:Calibration','BW:Detector']+SupportLegGroup),
        #ATR-27253
        ChainProp(name='HLT_noalg_LArPEBCalib_L1eEM18', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1eEM26M', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1eEM5', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_LArPEBCalib_L1eEM12L', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),

        # LAr noise burst chains
        ChainProp(name='HLT_larnoiseburst_L1XE60', l1SeedThresholds=['FSNOSEED'], stream=['LArNoiseBurst'], groups=['RATE:Calibration','BW:Detector']+SupportLegGroup),
        #ChainProp(name='HLT_larnoiseburst_L1jXE120', l1SeedThresholds=['FSNOSEED'], stream=['LArNoiseBurst'], groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),
        ChainProp(name='HLT_larnoiseburst_L1jJ125', l1SeedThresholds=['FSNOSEED'], stream=['LArNoiseBurst'], groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),
        ChainProp(name='HLT_larnoiseburst_L1J100', l1SeedThresholds=['FSNOSEED'], stream=['LArNoiseBurst'], groups=['RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_larnoiseburst_L1J40_XE50', l1SeedThresholds=['FSNOSEED'], stream=['LArNoiseBurst'], groups=['RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_larnoiseburst_L1J40_XE60', l1SeedThresholds=['FSNOSEED'], stream=['LArNoiseBurst'], groups=['RATE:Calibration','BW:Detector']+SupportLegGroup),
        ChainProp(name='HLT_larnoiseburst_L1All', l1SeedThresholds=['FSNOSEED'], stream=['LArNoiseBurst'], groups=['PS:NoHLTRepro','RATE:Calibration','BW:Detector']), # Temporary for testing, high CPU cost
        ChainProp(name='HLT_acceptedevts_larnoiseburst_L1All', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD','BW:DISCARD']),

        ## larpsall/em*FIRSTEMPTY Ph1
        ChainProp(name='HLT_larpsall_L1jJ30_FIRSTEMPTY', l1SeedThresholds=['jJ30'], stream=['LArNoiseBurst'], groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),
        ChainProp(name='HLT_larpsall_L1jJ60_FIRSTEMPTY', l1SeedThresholds=['jJ60'], stream=['LArNoiseBurst'], groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),

        ## larpsall/em*EMPTY
        ChainProp(name='HLT_larpsall_L1jJ30_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo'],groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportPhIGroup),
        ChainProp(name='HLT_larpsall_L1jJ60_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo'],groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportPhIGroup),
        ChainProp(name='HLT_larpsall_L1eTAU12_EMPTY', l1SeedThresholds=['eTAU12'], stream=['CosmicCalo'],groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportPhIGroup),
        ChainProp(name='HLT_larpsall_L1jJ60p30ETA49_EMPTY', l1SeedThresholds=['jJ60p30ETA49'], stream=['CosmicCalo'],groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportPhIGroup),

        # ATR-25019 Test the definition for the 'AFPCalib' stream
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_A_OR_C', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=['RATE:Calibration','BW:Detector']),
        # ATR-25019 AFPCalib streamers
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_NSA_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_NSC_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSA_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSC_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSA_TOF_T0_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSA_TOF_T1_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSA_TOF_T2_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSA_TOF_T3_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSC_TOF_T0_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSC_TOF_T1_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSC_TOF_T2_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_FSC_TOF_T3_BGRP12', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_A', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_C', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_A_AND_C_TOF_T0T1', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_A_OR_C_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_A_OR_C_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_A_OR_C_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1AFP_A_OR_C_FIRSTEMPTY', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_AFPPEB_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['AFPCalib'], groups=SupportGroup),

        #NSW mon
        ChainProp(name='HLT_noalg_L1NSW_MONITOR', l1SeedThresholds=['FSNOSEED'], stream=['NSWTriggerMonitor'], groups=SupportGroup),

        #ATR-26256 PixelNoise Stream
        ChainProp(name='HLT_noalg_LumiPEB_L1RD0_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['PixelNoise'], groups=SupportGroup),

        #ATR-25327 Test the definition for the 'PixelBeam' and 'VdM' streams
        ChainProp(name='HLT_noalg_LumiPEB_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['PixelBeam'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1RD0_BGRP11', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=SupportGroup),
        #ATR-25327 PixelBeam streamers
        ChainProp(name='HLT_noalg_LumiPEB_L1RD0_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['PixelBeam'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_LumiPEB_L1RD0_BGRP15', l1SeedThresholds=['FSNOSEED'], stream=['PixelBeam'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_LumiPEB_L1MBTS_1', l1SeedThresholds=['FSNOSEED'], stream=['PixelBeam'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_LumiPEB_L1MBTS_1_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['PixelBeam'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_LumiPEB_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=['PixelBeam'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_LumiPEB_L1MBTS_2_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['PixelBeam'], groups=SupportGroup),
        #ATR-25327 VdM streamers
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1MBTS_2_BGRP11', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1LUCID_A_BGRP11', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1LUCID_C_BGRP11', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1MBTS_1_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1MBTS_2_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1RD0_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1MBTS_1', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=SupportGroup),

        ChainProp(name='HLT_larpsallem_L1eEM9_EMPTY', l1SeedThresholds=['eEM9'], stream=['CosmicCalo'],groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1eEM9_EMPTY',  l1SeedThresholds=['eEM9'], stream=['LArCellsEmpty'],groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),
        ChainProp(name='HLT_larpsallem_L1eEM15_EMPTY', l1SeedThresholds=['eEM15'], stream=['CosmicCalo'],groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1eEM15_EMPTY',  l1SeedThresholds=['eEM15'], stream=['LArCellsEmpty'],groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),


    ]

    chainsP1['Cosmic'] = [
        ChainProp(name='HLT_noalg_SCTPEB_L1RD0_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['SCTNoise'], groups=['RATE:SCTCalibration','BW:Detector']), # HLT_sct_noise
        ChainProp(name='HLT_noalg_laser_TilePEB_L1CALREQ2', l1SeedThresholds=['FSNOSEED'], stream=['Tile'], groups=['RATE:TileCalibration','BW:Detector']), # HLT_tilecalib_laser 
        ChainProp(name='HLT_noalg_CIS_TilePEB_L1CALREQ1', l1SeedThresholds=['FSNOSEED'], stream=['Tile'], groups=['RATE:TileCalibration','BW:Detector']), # HLT_tilecalib_CIS 
        ChainProp(name='HLT_cosmic_id_L1MU3V_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['IDCosmic'], groups=SupportGroup+SingleMuonGroup),
        ChainProp(name='HLT_cosmic_id_L1MU8VF_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['IDCosmic'], groups=SupportGroup+SingleMuonGroup),
    ]

    chainsP1['Streaming'] = [

        ChainProp(name='HLT_noalg_L1RD0_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['Background'], groups=['BW:Other']),  
        
        ChainProp(name='HLT_noalg_mb_L1RD0_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup),
        ChainProp(name='HLT_noalg_mb_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup),  

        # ID monitoring
        ChainProp(name='HLT_noalg_idmon_L1RD0_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['IDMonitoring','express'],groups=['RATE:Monitoring','BW:Detector']),
        ChainProp(name='HLT_noalg_idmon_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['IDMonitoring'],groups=['RATE:Monitoring','BW:Detector']),
        ChainProp(name='HLT_noalg_idmon_L1RD0_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['IDMonitoring'],groups=['RATE:Monitoring','BW:Detector']),

        # L1 combined streamers
        ChainProp(name='HLT_noalg_L1Bkg',      l1SeedThresholds=['FSNOSEED'], stream=['Background'], groups=['RATE:SeededStreamers', 'BW:Other']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1Standby',      l1SeedThresholds=['FSNOSEED'], stream=['Standby'], groups=['RATE:SeededStreamers', 'BW:Other']+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1Calo',      l1SeedThresholds=['FSNOSEED'], stream=['L1Calo'], groups=['RATE:SeededStreamers', 'BW:Other']+SupportLegGroup),
        ChainProp(name='HLT_noalg_L1Calo_EMPTY',      l1SeedThresholds=['FSNOSEED'], stream=['L1Calo'], groups=['RATE:SeededStreamers', 'BW:Other']+SupportPhIGroup),

        # muon streamers
        ChainProp(name='HLT_noalg_L1MU3V_UNPAIRED_ISO', l1SeedThresholds=['FSNOSEED'], stream=['Background'], groups=SingleMuonGroup),
        ChainProp(name='HLT_noalg_cosmicmuons_L1MU3V_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicMuons','express'],groups=['RATE:Cosmic_Muon','BW:Muon']),
        ChainProp(name='HLT_noalg_cosmicmuons_L1MU8VF_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicMuons','express'],groups=['RATE:Cosmic_Muon','BW:Muon']),
        ChainProp(name='HLT_noalg_bkg_L1MU3V_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['Background'],groups=['BW:Other']),
        ChainProp(name='HLT_noalg_bkg_L1MU8VF_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['Background'],groups=['BW:Other']),

        # L1 calo streamers
        ChainProp(name='HLT_noalg_l1calo_L1J400', l1SeedThresholds=['FSNOSEED'], stream=['L1Calo'], groups=JetStreamersGroup+['BW:Other']+SupportLegGroup),
        
        # Cosmic calo stream
        ChainProp(name='HLT_noalg_L1RD1_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_L1jJ60p30ETA49_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo'],groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ30_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ60_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ30_FIRSTEMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ60_FIRSTEMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1RD0_FIRSTEMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo'], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_L1eTAU12_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1RD0_BGRP7', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo'], groups=['RATE:Calibration','BW:Detector']),
        #ATR-27257, Phase-I
        ChainProp(name='HLT_noalg_l1calo_L1jJ500', l1SeedThresholds=['FSNOSEED'], stream=['L1Calo'], groups=JetStreamersGroup+['BW:Other']+SupportPhIGroup),
        
        ChainProp(name='HLT_noalg_LArPEBNoise_L1jJ30_EMPTY',  l1SeedThresholds=['jJ30'], stream=['LArCellsEmpty'],groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1eTAU12_EMPTY', l1SeedThresholds=['eTAU12'], stream=['LArCellsEmpty'],groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1jJ60p30ETA49_EMPTY', l1SeedThresholds=['J30p31ETA49'], stream=['LArCellsEmpty'],groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),
        #                         
        ChainProp(name='HLT_noalg_LArPEBNoise_L1jJ30_FIRSTEMPTY',  l1SeedThresholds=['jJ30'],  stream=['LArCellsEmpty'],groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1jJ60_FIRSTEMPTY',  l1SeedThresholds=['jJ60'],  stream=['LArCellsEmpty'],groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),
        
        # TGC
        ChainProp(name='HLT_noalg_L1TGC_BURST',  l1SeedThresholds=['FSNOSEED'], stream=['TgcNoiseBurst'],groups=['RATE:Calibration','BW:Detector']),

#        ChainProp(name='HLT_noalg_L1RD1_BGRP10', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['RATE:Calibration','BW:Detector']),

        #ZeroBias
        ChainProp(name='HLT_noalg_L1ZB',        l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'],groups=['RATE:CPS_ZB']+ZeroBiasGroup+SupportLegGroup),# ATR-21367
        ChainProp(name='HLT_noalg_zb_L1RD1_EMPTY',        l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'],groups=ZeroBiasGroup+SupportGroup),# ATR-25032


        # MBTS
        ChainProp(name='HLT_noalg_L1MBTS_1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), 
        ChainProp(name='HLT_noalg_L1MBTS_2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), 
        ChainProp(name='HLT_noalg_L1MBTS_1_1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), 
        ChainProp(name='HLT_noalg_L1MBTS_A', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup),
        ChainProp(name='HLT_noalg_L1MBTS_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup),
        ChainProp(name='HLT_noalg_L1MBTS_1_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-21740
        ChainProp(name='HLT_noalg_L1RD2_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), # ATR-21367
        ChainProp(name='HLT_noalg_L1MBTS_2_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-21999
        ChainProp(name='HLT_noalg_L1MBTS_1_1_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-21999

        ChainProp(name='HLT_noalg_L1eEM9_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM15_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo','express'],groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportPhIGroup),

    ]

    chainsP1['MinBias'] =[
        # MBTS
        ChainProp(name='HLT_mb_mbts_all_L1MBTS_A', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], monGroups=['mbMon:online'], groups=MinBiasGroup),
        ChainProp(name='HLT_mb_mbts_all_L1MBTS_C', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], monGroups=['mbMon:online'], groups=MinBiasGroup),
        ChainProp(name='HLT_mb_mbts_all_L1MBTS_1_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], monGroups=['mbMon:online'], groups=MinBiasGroup), #ATR-21740
        ChainProp(name='HLT_mb_mbts_all_L1RD2_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], monGroups=['mbMon:online'], groups=MinBiasGroup), # ATR-21367
        ChainProp(name='HLT_mb_mbts_all_L1ZB',        l1SeedThresholds=['FSNOSEED'], stream=['ZeroBias'],monGroups=['mbMon:online'], groups=['RATE:CPS_ZB']+ZeroBiasGroup+SupportLegGroup),# ATR-21367
        ChainProp(name='HLT_mb_mbts_all_L1MBTS_2_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], monGroups=['mbMon:online'], groups=MinBiasGroup), #ATR-21999
        ChainProp(name='HLT_mb_mbts_all_L1MBTS_1_1_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], monGroups=['mbMon:online'], groups=MinBiasGroup), #ATR-21999

    ]

    chainsP1['Monitor'] = [
        ChainProp(name='HLT_timeburner_L1All', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['PS:NoHLTRepro','RATE:DISCARD','BW:DISCARD']),
        ChainProp(name='HLT_mistimemonj400_L1All', l1SeedThresholds=['FSNOSEED'], stream=['Mistimed'], groups=['PS:NoHLTRepro','RATE:Monitoring','BW:Other']),
    ]

    chainsP1['Beamspot'] = [
        ChainProp(name='HLT_beamspot_trkFS_trkfast_BeamSpotPEB_L1J15',  l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot',  'BW:BeamSpot']+SupportLegGroup),
    ]

    for sig in chainsP1:
        for chain in chainsP1[sig]:
            if 'Main' in chain.stream:
                log.error("chain %s in common P1 list with Main stream. Please move this to Physics menu file", chain.name)
                raise RuntimeError("Move %s chain to Physics menu file",chain.name)

    addP1Signatures(chains,chainsP1)

def addHighMuP1Signatures(chains):
    
    log.info('[setupMenu] Adding high-mu P1 menu chains now')
    
    chainsP1 = ChainStore()

    chainsP1['Egamma'] = [
        # ATR-21355 - cannot be moved to the calibSlice because they need to configure the photon/ sequence
        #ATR-27153, Phase-I
        ChainProp(name='HLT_g3_loose_LArPEBHLT_L1eEM5', stream=['LArCells'], groups=SinglePhotonGroup+SupportPhIGroup),
    ]

    chainsP1['Muon'] = [
        ChainProp(name='HLT_mu0_muoncalib_L1MU18VFCH', stream=['Muon_Calibration'], groups=['RATE:Muon_Calibration','BW:Muon']),
    ]

    chainsP1['Calib'] = [
        # Phase I jet inputs ATR-24411, seed needs to be checked
        #ChainProp(name='HLT_larpsall_L1jJ40', l1SeedThresholds=['jJ40'], stream=['CosmicCalo'],groups=['Support:PhaseI','RATE:Calibration','BW:Detector']),

        # IDCalib Chains
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1J100', stream=['IDCalib'], groups=SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1XE50', stream=['IDCalib'], groups=SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']),
        #IDcalib for lower lumi
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L1jJ60', stream=['IDCalib'], groups=SupportPhIGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L1XE35', stream=['IDCalib'], groups=SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 

        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L14J15', stream=['IDCalib'], groups=SupportLegGroup+['RATE:Calibration','BW:Detector'],  l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L14J15', stream=['IDCalib'], groups=SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        #
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1jJ160',  stream=['IDCalib'], groups=SupportPhIGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1jXE100', stream=['IDCalib'], groups=SupportPhIGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L14jJ40',  stream=['IDCalib'], groups=SupportPhIGroup+['RATE:Calibration','BW:Detector'],  l1SeedThresholds=['FSNOSEED']),
        #ATR-27253, Phase-I
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L14jJ40', stream=['IDCalib'], groups=SupportPhIGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L1jXE70', stream=['IDCalib'], groups=SupportPhIGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        
        # Lumi items for vdM programme
        ChainProp(name='HLT_noalg_LumiPEB_L1RD0_BGRP10', l1SeedThresholds=['FSNOSEED'], stream=['PixelBeam'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1MBTS_1_1_BGRP11', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=SupportGroup),

        ChainProp(name='HLT_larpsallem_L1eEM5_EMPTY', l1SeedThresholds=['eEM5'], stream=['CosmicCalo'],groups=['RATE:Cosmic_Calo','RATE:Calibration','BW:Jet']+SupportPhIGroup),
        #ATR-27253,Phase-I        
        ChainProp(name='HLT_larpsallem_L1eEM9_FIRSTEMPTY', l1SeedThresholds=['eEM9'], stream=['LArNoiseBurst'], groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1eEM5_EMPTY',  l1SeedThresholds=['eEM5'],  stream=['LArCellsEmpty'],groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_LArPEBNoise_L1eEM9_FIRSTEMPTY',  l1SeedThresholds=['eEM9'],  stream=['LArCellsEmpty'],groups=['RATE:Calibration','BW:Detector']+SupportPhIGroup),

        # End of event chains for MET
        ChainProp(name='HLT_acceptedevts_metcalo_L1All', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD', 'BW:DISCARD']),
        ChainProp(name='HLT_acceptedevts_mettrk_L1All', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD', 'BW:DISCARD']),
        
    ]

    chainsP1['Streaming'] = [
        ChainProp(name='HLT_noalg_L1eEM5_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo'],groups=['RATE:Cosmic_Calo','BW:MinBias','RATE:Calibration']+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1TRT_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['IDCosmic','express'],groups=['RATE:SeededStreamers','BW:Other']),
        ChainProp(name='HLT_noalg_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['IDCosmic'],groups=['RATE:SeededStreamers','BW:Other']),
    ]

    # Random Seeded EB chains which select at the HLT based on L1 TBP bits
    chainsP1['EnhancedBias'] = [
        ChainProp(name='HLT_eb_low_L1RD2_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_eb_medium_L1RD2_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

        ChainProp(name='HLT_noalg_L1PhysicsHigh_noPS', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["RATE:EnhancedBias", "BW:Detector"]+SupportLegGroup ),
        ChainProp(name='HLT_noalg_L1PhysicsVeryHigh_noPS', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["RATE:EnhancedBias", "BW:Detector"]+SupportLegGroup ),

        ChainProp(name='HLT_noalg_L1RD3_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
        ChainProp(name='HLT_noalg_L1RD3_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),

        ChainProp(name='HLT_noalg_L1EMPTY_noPS', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["RATE:EnhancedBias", "BW:Detector"]+SupportPhIGroup ),
        ChainProp(name='HLT_noalg_L1FIRSTEMPTY_noPS', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["RATE:EnhancedBias", "BW:Detector"]+SupportPhIGroup ),
        ChainProp(name='HLT_noalg_L1UNPAIRED_ISO_noPS', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["RATE:EnhancedBias", "BW:Detector"]+SupportPhIGroup ),
        ChainProp(name='HLT_noalg_L1UNPAIRED_NONISO_noPS', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups= ["RATE:EnhancedBias", "BW:Detector"]+SupportPhIGroup),

        ChainProp(name='HLT_noalg_eb_L1All', l1SeedThresholds=['FSNOSEED'], stream=['EnhancedBias'], groups=["RATE:EnhancedBias", "BW:Detector"]+SupportGroup ),
    ]

    chainsP1['Beamspot'] = [
        #ATR-27253
        ChainProp(name='HLT_beamspot_trkFS_trkfast_BeamSpotPEB_L14jJ50',  l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot',  'BW:BeamSpot']+SupportPhIGroup),

        # ATR-23061, ATR-26394
        # Beamspot chains using FS tracking -- no PEB, fill BeamSpot histograms then reject all events
        ChainProp(name='HLT_j0_pf_ftf_preselj20_beamspotVtx_L1J15' , l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD',  'BW:DISCARD', 'RATE:CPS_J15']+SupportLegGroup),
        ChainProp(name='HLT_j0_pf_ftf_preselcHT450_beamspotVtx_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD',  'BW:DISCARD']+SupportLegGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_pf_ftf_presel6c25_beamspotVtx_L14J15', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD',  'BW:DISCARD', 'RATE:CPS_4J15']+SupportLegGroup),
        # ChainProp(name='HLT_j0_pf_ftf_presel2c20b85_beamspotVtx_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD',  'BW:BeamSpot', 'RATE:CPS_J45p0ETA21_3J15p0ETA25']+SupportLegGroup),
        #
        ChainProp(name='HLT_j0_pf_ftf_preselj20_beamspotVtx_L1jJ40' , l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD',  'BW:DISCARD', 'RATE:CPS_jJ40']+SupportPhIGroup),
        ChainProp(name='HLT_j0_pf_ftf_preselcHT450_beamspotVtx_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD',  'BW:DISCARD', 'RATE:CPS_HT190-jJ40s5pETA21']+SupportPhIGroup+Topo3Group),
        ChainProp(name='HLT_j0_pf_ftf_presel6c25_beamspotVtx_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD',  'BW:DISCARD', 'RATE:CPS_4jJ40']+SupportPhIGroup),
        ChainProp(name='HLT_j0_pf_ftf_presel2c20b85_beamspotVtx_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD', 'BW:DISCARD', 'RATE:CPS_jJ85p0ETA21_3jJ40p0ETA25']+SupportPhIGroup),
    ]

    chainsP1['Jet'] = [
        # ATR-23061, ATR-26394
        # BeamspotPEB chains -- only run preselection without tracking, write PEB data
        ChainProp(name='HLT_j0_pf_ftf_preselj20_BeamSpotPEB_L1J15' , l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot', 'BW:BeamSpot', 'RATE:CPS_J15']+SupportLegGroup),
        ChainProp(name='HLT_j0_pf_ftf_preselcHT450_BeamSpotPEB_L1HT190-J15s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot',  'BW:BeamSpot']+SupportLegGroup+LegacyTopoGroup),
        ChainProp(name='HLT_j0_pf_ftf_presel6c25_BeamSpotPEB_L14J15', l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot',  'BW:BeamSpot', 'RATE:CPS_4J15']+SupportLegGroup),
        # ChainProp(name='HLT_j0_pf_ftf_presel2c20b85_BeamSpotPEB_L1J45p0ETA21_3J15p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot',  'BW:BeamSpot', 'RATE:CPS_J45p0ETA21_3J15p0ETA25']+SupportLegGroup),
        #
        ChainProp(name='HLT_j0_pf_ftf_preselj20_BeamSpotPEB_L1jJ40' , l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot',  'BW:BeamSpot', 'RATE:CPS_jJ40']+SupportPhIGroup),
        ChainProp(name='HLT_j0_pf_ftf_preselcHT450_BeamSpotPEB_L1HT190-jJ40s5pETA21', l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot',  'BW:BeamSpot', 'RATE:CPS_HT190-jJ40s5pETA21']+SupportPhIGroup+Topo3Group),
        ChainProp(name='HLT_j0_pf_ftf_presel6c25_BeamSpotPEB_L14jJ40', l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot',  'BW:BeamSpot', 'RATE:CPS_4jJ40']+SupportPhIGroup),
        ChainProp(name='HLT_j0_pf_ftf_presel2c20b85_BeamSpotPEB_L1jJ85p0ETA21_3jJ40p0ETA25', l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot',  'BW:BeamSpot', 'RATE:CPS_jJ85p0ETA21_3jJ40p0ETA25']+SupportPhIGroup),

        # Placed here because not in HI menu
        ChainProp(name='HLT_j165_LArPEBHLT_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=SingleJetGroup+SupportPhIGroup),
    ]

    addP1Signatures(chains,chainsP1)

def addLowMuP1Signatures(chains):
    log.info('[setupMenu] Adding low-mu P1 menu chains now')
    
    chainsP1 = ChainStore()

    chainsP1['Egamma'] = [
        # cannot be moved to the calibSlice because they need to configure the photon/ sequence
         #ATR-27153, Phase-I
        ChainProp(name='HLT_g3_loose_LArPEBHLT_L1eEM5', stream=['LArCells'], groups=SinglePhotonGroup+SupportPhIGroup),
    ]


    chainsP1['Jet'] = [
        # ATR-23061, ATR-26394
        # BeamspotPEB chains -- only run preselection without tracking, write PEB data
        ChainProp(name='HLT_j0_pf_ftf_preselj20_BeamSpotPEB_L1J15' , l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot', 'BW:BeamSpot', 'RATE:CPS_J15']+SupportLegGroup),
        ChainProp(name='HLT_j0_pf_ftf_preselj20_BeamSpotPEB_L1jJ40' , l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot',  'BW:BeamSpot', 'RATE:CPS_jJ40']+SupportPhIGroup),
    ]

    chainsP1['Calib'] = [
        # IDCalib Chains
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1J100', stream=['IDCalib'], groups=SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1jJ160', stream=['IDCalib'], groups=SupportPhIGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1XE50', stream=['IDCalib'], groups=SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']),
        #IDcalib for lower lumi
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L1jJ60', stream=['IDCalib'], groups=SupportPhIGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L1XE35', stream=['IDCalib'], groups=SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 

        #ZDCCalib Chains
        ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_OR',    l1SeedThresholds=['FSNOSEED'], stream=['ZDCCalib'], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_OR_UNPAIRED_NONISO',    l1SeedThresholds=['FSNOSEED'], stream=['ZDCCalib'], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_OR_EMPTY',    l1SeedThresholds=['FSNOSEED'], stream=['ZDCCalib'], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_A_AND_C', l1SeedThresholds=['FSNOSEED'], stream=["ZDCCalib"], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_A_AND_C_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=["ZDCCalib"], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_A_AND_C_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=["ZDCCalib"], groups=['RATE:Calibration','BW:Detector']),

        # ZDC RPD LED streamers (ATR-26826)
        ChainProp(name='HLT_noalg_ZDCPEB_L1CALREQ0', l1SeedThresholds=['FSNOSEED'], stream=['ZDCLEDCalib'], groups=['RATE:Calibration', 'BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1CALREQ1', l1SeedThresholds=['FSNOSEED'], stream=['ZDCLEDCalib'], groups=['RATE:Calibration', 'BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1CALREQ2', l1SeedThresholds=['FSNOSEED'], stream=['ZDCLEDCalib'], groups=['RATE:Calibration', 'BW:Detector']),

        # End of event chains for MET
        ChainProp(name='HLT_acceptedevts_metcalo_L1All', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD', 'BW:DISCARD']),
        ChainProp(name='HLT_acceptedevts_mettrk_L1All', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD', 'BW:DISCARD']),
        
    ]

    # Intentionally commented -- may be used specifically for low-mu MBTS validation
    # Probably only relevant in lowMu
    # chainsP1['Streaming'] = [
    #    ChainProp(name='HLT_noalg_L1MBTSA0', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA3', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA4', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA5', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA6', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA7', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA8', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA9', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA10', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA11', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA12', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA13', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA14', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSA15', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
       
    #    ChainProp(name='HLT_noalg_L1MBTSC0', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC1', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC2', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC3', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC4', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC5', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC6', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC7', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC8', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC9', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC10', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC11', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC12', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC13', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC14', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    #    ChainProp(name='HLT_noalg_L1MBTSC15', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=MinBiasGroup), #ATR-23216
    # ]

    chainsP1['Beamspot'] = [
        # Beamspot chains using FS tracking -- no PEB, fill BeamSpot histograms then reject all events
        ChainProp(name='HLT_j0_pf_ftf_preselj20_beamspotVtx_L1J15' , l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD',  'BW:DISCARD', 'RATE:CPS_J15']+SupportLegGroup),
        ChainProp(name='HLT_j0_pf_ftf_preselj20_beamspotVtx_L1jJ40' , l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD',  'BW:DISCARD', 'RATE:CPS_jJ40']+SupportPhIGroup),
    ]
    chainsP1['Jet'] = [
        # BeamspotPEB chains -- only run preselection without tracking, write PEB data
        ChainProp(name='HLT_j0_pf_ftf_preselj20_BeamSpotPEB_L1J15' , l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot', 'BW:BeamSpot', 'RATE:CPS_J15']+SupportLegGroup),
        ChainProp(name='HLT_j0_pf_ftf_preselj20_BeamSpotPEB_L1jJ40' , l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot',  'BW:BeamSpot', 'RATE:CPS_jJ40']+SupportPhIGroup),
    ]

    addP1Signatures(chains,chainsP1)

def addHeavyIonP1Signatures(chains):
    log.info('[setupMenu] Adding heavy-ion P1 menu chains now')    

    chainsP1 = ChainStore()

    #---- beamspot trigger with VTE to avoid busy tracking in central events
    chainsP1['Beamspot'] = [
        ChainProp(name='HLT_beamspot_trkFS_trkfast_BeamSpotPEB_L1J12_VTE200',  l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot',  'BW:BeamSpot']+SupportLegGroup),
        ChainProp(name='HLT_beamspot_trkFS_trkfast_BeamSpotPEB_L1J12_VTE100',  l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot',  'BW:BeamSpot']+SupportLegGroup),
        ChainProp(name='HLT_j0_ftf_beamspotVtx_L1J12_VTE100',  l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot',  'BW:BeamSpot']+SupportLegGroup),

        # Beamspot chains using FS tracking -- no PEB, fill BeamSpot histograms then reject all events
        ChainProp(name='HLT_j0_pf_jes_ftf_preselj20_beamspotVtx_L1VZDC_A_VZDC_C_TE5_VTE200' , l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD',  'BW:DISCARD', 'RATE:CPS_VZDC_A_VZDC_C_TE5_VTE200']+SupportLegGroup),
        ChainProp(name='HLT_j0_pf_jes_ftf_preselj20_beamspotVtx_L1ZDC_XOR_TE5_VTE200' , l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD',  'BW:DISCARD', 'RATE:CPS_ZDC_XOR_TE5_VTE200']+SupportLegGroup),
        ChainProp(name='HLT_j0_pf_jes_ftf_preselj20_beamspotVtx_L11ZDC_NZDC_TE5_VTE200' , l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD',  'BW:DISCARD', 'RATE:CPS_1ZDC_NZDC_TE5_VTE200']+SupportLegGroup),
        ChainProp(name='HLT_j0_pf_jes_ftf_preselj20_beamspotVtx_L15ZDC_A_5ZDC_C_TE5_VTE200' , l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD',  'BW:DISCARD', 'RATE:CPS_5ZDC_A_5ZDC_C_TE5_VTE200']+SupportLegGroup),
    ]
    chainsP1['Jet'] = [
        # BeamspotPEB chains -- only run preselection without tracking, write PEB data
        ChainProp(name='HLT_j0_pf_jes_ftf_preselj20_BeamSpotPEB_L1VZDC_A_VZDC_C_TE5_VTE200' , l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot', 'BW:BeamSpot', 'RATE:CPS_VZDC_A_VZDC_C_TE5_VTE200']+SupportLegGroup),
        ChainProp(name='HLT_j0_pf_jes_ftf_preselj20_BeamSpotPEB_L1ZDC_XOR_TE5_VTE200' , l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot', 'BW:BeamSpot', 'RATE:CPS_ZDC_XOR_TE5_VTE200']+SupportLegGroup),
        ChainProp(name='HLT_j0_pf_jes_ftf_preselj20_BeamSpotPEB_L11ZDC_NZDC_TE5_VTE200' , l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot', 'BW:BeamSpot', 'RATE:CPS_1ZDC_NZDC_TE5_VTE200']+SupportLegGroup),
        ChainProp(name='HLT_j0_pf_jes_ftf_preselj20_BeamSpotPEB_L15ZDC_A_5ZDC_C_TE5_VTE200' , l1SeedThresholds=['FSNOSEED'], stream=['BeamSpot'], groups=['RATE:BeamSpot', 'BW:BeamSpot', 'RATE:CPS_5ZDC_A_5ZDC_C_TE5_VTE200']+SupportLegGroup),
    ]

    chainsP1['Calib'] = [
        #---- ID calib trigger with VTE to avoid busy tracking in central events
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1J100_VTE200', stream=['IDCalib'], groups=SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1XE50_VTE200', stream=['IDCalib'], groups=SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']),
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L1J30_VTE200',  stream=['IDCalib'], groups=SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L1XE35_VTE200', stream=['IDCalib'], groups=SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L1J12_VTE200', stream=['IDCalib'], groups=SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']),

        # # 'ZDCCalib' stream
        # ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_OR_LHCF', l1SeedThresholds=['FSNOSEED'], stream=["ZDCCalib"], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_OR', l1SeedThresholds=['FSNOSEED'], stream=["ZDCCalib"], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_A', l1SeedThresholds=['FSNOSEED'], stream=["ZDCCalib"], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_C', l1SeedThresholds=['FSNOSEED'], stream=["ZDCCalib"], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_A_C', l1SeedThresholds=['FSNOSEED'], stream=["ZDCCalib"], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_OR_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=["ZDCCalib"], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_A_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=["ZDCCalib"], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_C_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=["ZDCCalib"], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_A_C_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=["ZDCCalib"], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_OR_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=["ZDCCalib"], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_A_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=["ZDCCalib"], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_C_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=["ZDCCalib"], groups=['RATE:Calibration','BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1ZDC_A_C_UNPAIRED_NONISO', l1SeedThresholds=['FSNOSEED'], stream=["ZDCCalib"], groups=['RATE:Calibration','BW:Detector']),

        # ZDC RPD LED streamers (ATR-26826)
        ChainProp(name='HLT_noalg_ZDCPEB_L1CALREQ0', l1SeedThresholds=['FSNOSEED'], stream=['ZDCLEDCalib'], groups=['RATE:Calibration', 'BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1CALREQ1', l1SeedThresholds=['FSNOSEED'], stream=['ZDCLEDCalib'], groups=['RATE:Calibration', 'BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1CALREQ2', l1SeedThresholds=['FSNOSEED'], stream=['ZDCLEDCalib'], groups=['RATE:Calibration', 'BW:Detector']),
        
        # Lumi
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1LUCID_A', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=SupportGroup),
        ChainProp(name='HLT_noalg_vdm_LumiPEB_L1LUCID_C', l1SeedThresholds=['FSNOSEED'], stream=['VdM'], groups=SupportGroup),
    ]

    addP1Signatures(chains,chainsP1)


def addCosmicP1Signatures(chains):
    log.info('[setupMenu] Adding cosmic P1 menu chains now')

    chainsP1 = ChainStore()

    chainsP1['Egamma'] = [
        # ATR-21355 - cannot be moved to the calibSlice because they need to configure the photon/ sequence
        #ATR-27153, Phase-I
        ChainProp(name='HLT_g3_loose_LArPEBHLT_L1eEM5', stream=['LArCells'], groups=SinglePhotonGroup+SupportPhIGroup),
    ]

    chainsP1['Calib'] = [
        # IDCalib Chains
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1J100', stream=['IDCalib'], groups=SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk9_IDCalibPEB_L1XE50', stream=['IDCalib'], groups=SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']),
        #IDcalib for lower lumi
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L1jJ60', stream=['IDCalib'], groups=SupportPhIGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 
        ChainProp(name='HLT_idcalib_trk4_IDCalibPEB_L1XE35', stream=['IDCalib'], groups=SupportLegGroup+['RATE:Calibration','BW:Detector'], l1SeedThresholds=['FSNOSEED']), 



        # ZDC RPD LED streamers (ATR-26826)
        ChainProp(name='HLT_noalg_ZDCPEB_L1CALREQ0', l1SeedThresholds=['FSNOSEED'], stream=['ZDCLEDCalib'], groups=['RATE:Calibration', 'BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1CALREQ1', l1SeedThresholds=['FSNOSEED'], stream=['ZDCLEDCalib'], groups=['RATE:Calibration', 'BW:Detector']),
        ChainProp(name='HLT_noalg_ZDCPEB_L1CALREQ2', l1SeedThresholds=['FSNOSEED'], stream=['ZDCLEDCalib'], groups=['RATE:Calibration', 'BW:Detector']),

        # End of event chains for MET
        ChainProp(name='HLT_acceptedevts_metcalo_L1All', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD', 'BW:DISCARD']),
        ChainProp(name='HLT_acceptedevts_mettrk_L1All', l1SeedThresholds=['FSNOSEED'], stream=['DISCARD'], groups=['RATE:DISCARD', 'BW:DISCARD']),
        
    ]

    chainsP1['Streaming'] = [
        ChainProp(name='HLT_noalg_L1eEM5_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['CosmicCalo'],groups=['RATE:Cosmic_Calo','BW:MinBias','RATE:Calibration']+SupportPhIGroup),
    ]

    chainsP1['Jet'] = [
        ChainProp(name='HLT_j165_LArPEBHLT_L1jJ160', l1SeedThresholds=['FSNOSEED'], stream=['LArCells'], groups=SingleJetGroup+SupportPhIGroup),
    ]

    addP1Signatures(chains,chainsP1)


def addP1Signatures(chains, chainsP1):

    for sig,chainsInSig in chainsP1.items():
        for chain in chainsInSig:
            if 'Main' in chain.stream:
                log.error("chain %s in P1 menu [%s] with Main stream. Please move this to PhysicsP1 menu file", chain.name, sig)
                raise RuntimeError("Move %s chain to PhysicsP1 menu file",chain.name)
            for group in chain.groups:
                if 'Primary' in group:
                    log.error("chain %s in P1 menu [%s] with Primary tag. Please move this to Physics menu file", chain.name, sig)
                    raise RuntimeError("Move %s chain to Physics menu file",chain.name)

    for sig,chainsInSig in chainsP1.items():
        chains[sig] += chainsInSig
