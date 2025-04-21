# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#------------------------------------------------------------------------#
# Cosmic_run3_v1.py menu
#------------------------------------------------------------------------#

# All chains are represented as ChainProp objects in a ChainStore
from TriggerMenuMT.HLT.Config.Utility.ChainDefInMenu import ChainProp
from .SignatureDicts import ChainStore

from .Physics_pp_run3_v1 import (
    SingleJetGroup,
    SingleBjetGroup,
    SingleMuonGroup,
    MultiMuonGroup,
    EgammaMuonGroup,
    PrimaryLegGroup,
    PrimaryL1MuGroup,
    PrimaryPhIGroup,
    MinBiasGroup,
    JetStreamersGroup,
    METStreamersGroup,
    TauStreamersGroup,
    EgammaStreamersGroup,
    SupportGroup,
    SupportPhIGroup
)
from . import P1_run3_v1

def getCosmicSignatures():
    chains = ChainStore()

    chains['Muon'] = [
        ChainProp(name='HLT_mu4_cosmic_L1MU3V_EMPTY', l1SeedThresholds=['MU3V'], stream=['CosmicMuons'], groups=['RATE:Cosmic_Muon','BW:Muon'],monGroups=['idMon:shifter']),
        ChainProp(name='HLT_mu4_msonly_cosmic_L1MU3V_EMPTY', l1SeedThresholds=['MU3V'], stream=['CosmicMuons'], groups=['RATE:Cosmic_Muon','BW:Muon']),

        ChainProp(name='HLT_3mu6_msonly_L1MU3V_EMPTY', l1SeedThresholds=['MU3V'], stream=['Late'], groups=PrimaryL1MuGroup+MultiMuonGroup),
        ChainProp(name='HLT_3mu6_msonly_L12MU5VF_EMPTY', l1SeedThresholds=['MU5VF'], stream=['Late'], groups=PrimaryL1MuGroup+MultiMuonGroup),

        ChainProp(name='HLT_mu60_0eta105_msonly_L1MU14FCH', l1SeedThresholds=['MU14FCH'], stream=['Main'], groups=PrimaryL1MuGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu60_msonly_3layersEC_L1MU14FCH', l1SeedThresholds=['MU14FCH'], stream=['Main'], groups=PrimaryL1MuGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu80_msonly_3layersEC_L1MU14FCH', l1SeedThresholds=['MU14FCH'], stream=['Main'], groups=PrimaryL1MuGroup+SingleMuonGroup),

        # idperf ATR-24675
        ChainProp(name='HLT_mu4_idperf_L1MU3V', l1SeedThresholds=['MU3V'], stream=['Main'], groups=PrimaryL1MuGroup+SingleMuonGroup,monGroups=['idMon:shifter']),

        # ATR-24977 - LRT muon chains
        ChainProp(name='HLT_mu20_LRT_d0loose_L1MU14FCH', l1SeedThresholds=['MU14FCH'], stream=['Main'], groups=PrimaryL1MuGroup+SingleMuonGroup),
        ChainProp(name='HLT_mu6_LRT_idperf_L1MU5VF', l1SeedThresholds=['MU5VF'], stream=['Main'], groups=SupportGroup+SingleMuonGroup,monGroups=['idMon:shifter']),

        # ATR-25224 - LRT muon chain
        ChainProp(name='HLT_mu6_LRT_d0loose_L1MU5VF', l1SeedThresholds=['MU5VF'], stream=['Main'], groups=SupportGroup+SingleMuonGroup),
    ]

    chains['Egamma'] = [
        # ATR-21355 - cannot be moved to the calibSlice because they need to configure the photon/ sequence
        # phase-I
        ChainProp(name='HLT_g3_etcut_LArPEB_L1eEM5', stream=['LArCells'], groups=['RATE:SinglePhoton', 'BW:Egamma']),
        ChainProp(name='HLT_e5_etcut_L1eEM5',stream=['Main'], groups=['RATE:SingleElectron', 'BW:Egamma']),
    ]

    chains['Jet'] = [
        #phase-I
        ChainProp(name='HLT_j15_L1jJ60_EMPTY'   , l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=PrimaryPhIGroup+SingleJetGroup),
        ChainProp(name='HLT_j0_HT0_L1jJ60_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=PrimaryPhIGroup+SingleJetGroup),
    ]

    chains['Bjet'] = [
        ChainProp(name='HLT_j0_0eta290_boffperf_ftf_L1MU8F',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+SingleBjetGroup,monGroups=['idMon:shifter']),
        ChainProp(name='HLT_j0_0eta290_boffperf_ftf_L1RD0_EMPTY',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+SingleBjetGroup,monGroups=['idMon:shifter']),
        ChainProp(name='HLT_j0_0eta290_boffperf_ftf_L1jJ30_EMPTY',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=PrimaryLegGroup+SingleBjetGroup,monGroups=['idMon:shifter']),
    ]

    chains['Combined'] = [
        ChainProp(name='HLT_g15_loose_2mu10_msonly_L1MU3V_EMPTY', l1SeedThresholds=['EM8VH','MU3V'], stream=['Main'], groups=PrimaryLegGroup+EgammaMuonGroup),
        ChainProp(name='HLT_g15_loose_2mu10_msonly_L12MU5VF_EMPTY', l1SeedThresholds=['EM8VH','MU5VF'], stream=['Main'], groups=PrimaryLegGroup+EgammaMuonGroup),
    ]

    chains['MinBias'] = [
        ChainProp(name='HLT_mb_sptrk_costr_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['Rate:MinBias','BW:MinBias']),
        ChainProp(name='HLT_mb_sptrk_costr_L1RD0_EMPTY', l1SeedThresholds=['FSNOSEED'], stream=['MinBias'], groups=['Rate:Cosmic_MinBias','BW:MinBias']),
    ]

    chains['Monitor'] = [
        ChainProp(name='HLT_noalg_CostMonDS_L1All',        l1SeedThresholds=['FSNOSEED'], stream=['CostMonitoring'], groups=['Primary:CostAndRate', 'RATE:Monitoring', 'BW:Other']), # HLT_costmonitor
    ]

    chains['Streaming'] = [
        ChainProp(name='HLT_noalg_L1TRT_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['IDCosmic','express'],groups=['RATE:SeededStreamers','BW:Other'],monGroups=['detMon'] ),
        ChainProp(name='HLT_noalg_L1TRT_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['IDCosmic'],groups=['RATE:SeededStreamers','BW:Other']),

        ChainProp(name='HLT_noalg_L1RD0_EMPTY',  l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=MinBiasGroup),
        ChainProp(name='HLT_noalg_L1RD0_FILLED', l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=MinBiasGroup), 

        ChainProp(name='HLT_noalg_L1MU3V',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleMuonGroup),
        ChainProp(name='HLT_noalg_L1MU8VF',      l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=SingleMuonGroup),
        #phase-I
        ChainProp(name='HLT_noalg_L1eEM5',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=EgammaStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM9',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=EgammaStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM10L',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=EgammaStreamersGroup+SupportPhIGroup),  
        ChainProp(name='HLT_noalg_L1eEM12L',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=EgammaStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM15',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=EgammaStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM18',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=EgammaStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM24L',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=EgammaStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eEM26M',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=EgammaStreamersGroup+SupportPhIGroup),

        #Tau, phase-I
        ChainProp(name='HLT_noalg_L1eTAU12',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=TauStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eTAU60',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=TauStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1eTAU80',     l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=TauStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1cTAU20M',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=TauStreamersGroup+SupportPhIGroup),        
        ChainProp(name='HLT_noalg_L1cTAU30M',    l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=TauStreamersGroup+SupportPhIGroup),      

        #Jet, phase-I
        ChainProp(name='HLT_noalg_L1jJ40',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ50',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ55',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ60',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ80',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ90',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ125',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ140',        l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1jJ160',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=JetStreamersGroup+SupportPhIGroup),
        
        
        #XE, phase-I
        ChainProp(name='HLT_noalg_L1jXE110',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=METStreamersGroup+SupportPhIGroup),
        #ATR-27986
        ChainProp(name='HLT_noalg_L1jXE120',       l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=METStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1gXEJWOJ110',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=METStreamersGroup+SupportPhIGroup),
        ChainProp(name='HLT_noalg_L1gXEJWOJ120',   l1SeedThresholds=['FSNOSEED'], stream=['Main'], groups=METStreamersGroup+SupportPhIGroup),
    ]

    return chains

def setupMenu():

    chains = getCosmicSignatures()

    # Add all standard monitoring chains from addP1Signatures function
    P1_run3_v1.addCommonP1Signatures(chains)
    P1_run3_v1.addCosmicP1Signatures(chains)

    return chains
