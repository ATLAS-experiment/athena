# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# ------------------------------------------------------------
# Definition of trigger EDM for Run 3
#
# Concept of categories is kept similar to TriggerEDMRun2.py, categories are:
# AllowedCategories = ['Bjet', 'Bphys', 'Egamma', 'ID', 'Jet', 'L1', 'MET', 'MinBias', 'Muon', 'Steer', 'Tau', 'Calo' (new in Run 3), 'UTT' (new in Run 3)]
#
# New additions in Run 3:
#  * Dynamic varialbes/Container slimming: All dyn vars are removed unless explicitly specified to be kept!
#    Please refer to ATR-20275 for discussion about policy/handling of dynamic variables
# 
#  * Per target dynamic EDM content (ATR-25505) which is defined through a list that 
#    removes the variables from the AODFULL EDM for the target EDM AODSLIM
# ------------------------------------------------------------

from AthenaCommon.Logging import logging
__log = logging.getLogger('TriggerEDMRun3Config')

# ------------------------------------------------------------
# Additional properties for EDM collections
# ------------------------------------------------------------
from TrigEDMConfig.TriggerEDMDefs import Alias, InViews, allowTruncation

# ------------------------------------------------------------
# Lists of variables to be kept in the collections 
# ------------------------------------------------------------
# ============
# === JETS ===
JetVarsToKeep = ['ActiveArea', 'ActiveArea4vec_eta', 'ActiveArea4vec_m', 'ActiveArea4vec_phi', 'ActiveArea4vec_pt', 'AlgorithmType',
                 'DetectorEta', 'DetectorPhi', 'EMFrac', 'EnergyPerSampling', 'EnergyPerSamplingCaloBased', 'GhostTrack_ftf', 'HECFrac', 'InputType',
                 'JetConstitScaleMomentum_eta', 'JetConstitScaleMomentum_m', 'JetConstitScaleMomentum_phi', 'JetConstitScaleMomentum_pt',
                 'JetPileupScaleMomentum_eta', 'JetPileupScaleMomentum_m', 'JetPileupScaleMomentum_phi', 'JetPileupScaleMomentum_pt',
                 'JetEtaJESScaleMomentum_eta', 'JetEtaJESScaleMomentum_m', 'JetEtaJESScaleMomentum_phi', 'JetEtaJESScaleMomentum_pt',
                 'JetGSCScaleMomentum_eta', 'JetGSCScaleMomentum_m', 'JetGSCScaleMomentum_phi', 'JetGSCScaleMomentum_pt',
                 'Jvt', 'NumTrkPt1000', 'SizeParameter', 'SumPtChargedPFOPt500', 'SumPtTrkPt500', 
                 'Timing','TrackWidthPt1000',
             ]
JetVars = '.'.join(JetVarsToKeep)


JetCopyVarsToKeep = ['pt', 'eta', 'phi', 'm',
                     'JetPileupScaleMomentum_eta', 'JetPileupScaleMomentum_m', 'JetPileupScaleMomentum_phi', 'JetPileupScaleMomentum_pt',
                     'JetEtaJESScaleMomentum_eta', 'JetEtaJESScaleMomentum_m', 'JetEtaJESScaleMomentum_phi', 'JetEtaJESScaleMomentum_pt',
                     'JetGSCScaleMomentum_eta', 'JetGSCScaleMomentum_m', 'JetGSCScaleMomentum_phi', 'JetGSCScaleMomentum_pt',
                     'Jvt', 'JvtRpt','Timing', 'TracksForMinimalJetTag',
                     # additional variables also stored in TLA stream for jet cleaning studies
                     # stored for calibrated HLT container in the same way as in the TLA streams
                     "N90Constituents", "LArQuality", "FracSamplingMax", "NegativeE", "HECQuality", "AverageLArQF", "BchCorrCell",
                     # NN JVT -- TrkAug version
                     'NNJvt','NNJvtPass', 'RPtTrkPt500', 'DTrackWidthPt1000', 'DNumTrkPt1000', 'DRPtTrkPt500', 'SumPtTrkOrderedTrackWidthPt1000', 'SumPtTrkOrderedNumTrkPt1000',
                    ]

FastFtagPFlowVarsToKeep = [f'dips20211116_p{x}' for x in 'cub']
FastFtagPFlowVarsToKeep += [f'fastDIPS20211215_p{x}' for x in 'cub']
FastFtagPFlowVarsToKeep += [f'GN120230331_p{x}' for x in 'cub']
FastFtagPFlowVarsToKeep += [f'tlaGN220240122_p{x}' for x in 'cub']

JetCopyVarsToKeep += FastFtagPFlowVarsToKeep
JetCopyVars = '.'.join(JetCopyVarsToKeep)

JetFastFTagVarsToKeep = JetCopyVarsToKeep
JetFastFTagVarsToKeep += [f'fastDips_p{x}' for x in 'cub']
JetFastFTagVarsToKeep += [f'fastGN120230327_p{x}' for x in 'cub']
JetFastFTagVarsToKeep += [f'fastGN120230331_p{x}' for x in 'cub']
JetFastFTagVarsToKeep += [f'fastGN220240122_p{x}' for x in 'cub']
JetFastFTagVarsToKeep += [f'fastGNTau20240216_p{x}' for x in ["tau" , "u"]]
JetFastFTagVarsToKeep += ['dipz20231122_z']
JetFastFTagVarsToKeep += ['dipz20231122_negLogSigma2']
JetFastFTagVars = '.'.join(JetFastFTagVarsToKeep)

LargeRJetVarsToKeep = [f'GN2Xv01_p{x}' for x in ['hbb', 'top', 'qcd']]
LargeRJetVarsToKeep += [f'GN2XTrig_p{x}' for x in ['hbb', 'top', 'qcd']]
LargeRJetVarsToKeep += JetVarsToKeep 
LargeRJetVars = '.'.join(LargeRJetVarsToKeep)

VSIVarsToKeep = ['vsi_mass', 'vsi_pT', 'vsi_charge', 'vsi_isFake',
                 'vsi_twoCirc_dr', 'vsi_twoCirc_dphi', 'vsi_twoCirc_int_r', 'vsi_vrtFast_r', 'vsi_vrtFast_eta', 
                 'vsi_vrtFast_phi', 'vsi_vrtFast_trkd0', 'vsi_vrtFast_trkz0',
                 'vsi_vrtFit_r', 'vsi_vrtFit_chi2', 'vsi_vPos', 'vsi_vPosMomAngT', 'vsi_dphi1', 'vsi_dphi2',
                 'vsi_isPassMMV', 'vsi_trkd0cut', 'vsi_twoCircErrcut', 'vsi_twoCircRcut', 'vsi_fastErrcut', 
                 'vsi_fastRcut', 'vsi_fitErrcut', 'vsi_chi2cut']
VSIVars = '.'.join(VSIVarsToKeep)


# ===========
# === TLA ===
TLAJetVarsToKeep = [
                   'pt', 'eta', 'phi', 'm', 'ActiveArea', 'ActiveArea4vec_eta', 'ActiveArea4vec_m', 'ActiveArea4vec_phi', 'ActiveArea4vec_pt',
                   'DetectorEta', 'DetectorPhi', 'EMFrac', 'HECFrac', 'EnergyPerSampling', 'EnergyPerSamplingCaloBased',
                   'JetConstitScaleMomentum_eta', 'JetConstitScaleMomentum_m', 'JetConstitScaleMomentum_phi', 'JetConstitScaleMomentum_pt',
                   'JetPileupScaleMomentum_eta', 'JetPileupScaleMomentum_m', 'JetPileupScaleMomentum_phi', 'JetPileupScaleMomentum_pt',
                   'JetEtaJESScaleMomentum_eta', 'JetEtaJESScaleMomentum_m', 'JetEtaJESScaleMomentum_phi', 'JetEtaJESScaleMomentum_pt',
                   'JetGSCScaleMomentum_eta', 'JetGSCScaleMomentum_m', 'JetGSCScaleMomentum_phi', 'JetGSCScaleMomentum_pt',
                   'Jvt', 'JVFCorr', 'JvtRpt',
                   'NNJvt','NNJvtPass',
                   'SumPtChargedPFOPt500', 'NumTrkPt1000', 'SumPtTrkPt500', 'TrackWidthPt1000', 'N90Constituents',
                   'LArQuality','FracSamplingMax',  'NegativeE', 'Timing', 'HECQuality','AverageLArQF', 'BchCorrCell',
                   ]
TLAJetVarsToKeep+= FastFtagPFlowVarsToKeep
TLAJetVars='.'.join(TLAJetVarsToKeep)

# ==============
# === EGAMMA ===
ElToKeep = ['ptcone20', 'ptvarcone20', 'ptcone30', 'ptvarcone30', 'trk_d0','cl_eta2','cl_phi2', 'deltaEta1PearDistortion']
ElVars = '.'.join(ElToKeep)

PhToKeep = ['topoetcone20', 'topoetcone40', 'etcone20']

PhVars = '.'.join(PhToKeep)

# =============
# === BJETS ===
def getBTagViewName(jetType, jetDetailStr="subresjesgscIS_ftf"):
    return f"BTagViews_HLT_{jetType}_{jetDetailStr}Views"

BTagViewsEMTopo = getBTagViewName("AntiKt4EMTopoJets")
BTagViewsEMPFlow = getBTagViewName("AntiKt4EMPFlowJets")

HIJetVarsToKeep = JetVarsToKeep + ['HLT_HIClusters_DR8Assoc']
HIJetVars = '.'.join(HIJetVarsToKeep)

BTagOutput = ['jetLink','BTagTrackToJetAssociator','Muons',]
BTagOutput_IPxD = []
BTagOutput_IP2D = [f.format(x=2) for f in BTagOutput_IPxD]
BTagOutput_IP3D = [f.format(x=3) for f in BTagOutput_IPxD]
BTagOutput_SV1 = []
BTagOutput_JetFitter = []
BTagOutput_rnnip = ['rnnip_isDefaults','rnnip_pu','rnnip_pc','rnnip_pb','rnnip_ptau',]

# we don't plan to keep all these, they are just for comparisons while tuning
three_output_taggers = [
    'DL1',
    'DL1r',
    'dipsLoose20210517',
    'dips20210517',
    'DL1d20210519r22',          # uses dipsLoose
    'DL1d20210528r22',          # uses IP3D track selection dips
    'dipsLoose20210729',        # DIPS offline retraining in r22
    'DL1dv00',                  # first 'official' offline r22
    'dips20211116',             # DIPS input to the current online DL1d
    'DL1d20211216',             # current online r22
    'GN120220813',
    'GN220240122'
]

b_vs_bb_taggers = [
    'dl1dbb20230314'            # anti-bb-jet tagger r22
]

BTagOutput_highLevelTaggers = [
    'MV2c10_discriminant',
    *[f'{t}_p{x}' for x in 'cub' for t in three_output_taggers],
    *[f'{t}_p{x}' for x in ['b','bb'] for t in b_vs_bb_taggers],
]

BTagOutput += BTagOutput_IP2D + BTagOutput_IP3D + BTagOutput_SV1 + BTagOutput_JetFitter + BTagOutput_rnnip + BTagOutput_highLevelTaggers
BTagVars = '.'.join(BTagOutput)

BTagJetOutput = ['btaggingLink', 'Jvt', 'JVFCorr', 'SumPtTrkPt500']
BTagJetVars  ='.'.join(BTagJetOutput)

hitDVToKeepBase = ['seed_eta','seed_phi','seed_type','n_track_qual','ly0_sp_frac','ly1_sp_frac','ly2_sp_frac','ly3_sp_frac',
                   'ly4_sp_frac','ly5_sp_frac','ly6_sp_frac','ly7_sp_frac','bdt_score']
hitDVToKeep = []
for var in hitDVToKeepBase:
    hitDVToKeep.append('hitDV_'+var)
hitDVVars = '.'.join(hitDVToKeep)

# ==========
# === ID ===
dEdxTrkToKeepBase = ['id','pt','eta','phi','dedx','dedx_n_usedhits','a0beam','n_hits_innermost','n_hits_inner','n_hits_pix','n_hits_sct']
dEdxTrkToKeep = []
for var in dEdxTrkToKeepBase:
    dEdxTrkToKeep.append('dEdxTrk_'+var)
dEdxTrkVars = '.'.join(dEdxTrkToKeep)

dEdxHitToKeepBase = ['trkid','dedx','tot','trkchi2','trkndof','iblovfl','loc','layer']
dEdxHitToKeep = []
for var in dEdxHitToKeepBase:
    dEdxHitToKeep.append('dEdxHit_'+var)
dEdxHitVars = '.'.join(dEdxHitToKeep)

HPtdEdxTrkToKeepBase = ['pt','eta','phi','a0beam','dedx','n_hits_innermost','n_hits_inner','n_hits_pix','n_hits_sct',
                        'n_hdedx_hits_1p45','n_hdedx_hits_1p50','n_hdedx_hits_1p55','n_hdedx_hits_1p60','n_hdedx_hits_1p65',
                        'n_hdedx_hits_1p70','n_hdedx_hits_1p75','n_hdedx_hits_1p80']
HPtdEdxTrkToKeep = []
for var in HPtdEdxTrkToKeepBase:
    HPtdEdxTrkToKeep.append('HPtdEdxTrk_'+var)
HPtdEdxTrkVars = '.'.join(HPtdEdxTrkToKeep)

DisTrkToKeepNoIso = ['pt','eta','phi','d0','z0','chi2','ndof','n_hits_innermost','n_hits_inner','n_hits_pix','n_hits_sct',
                     'pt_wrtVtx','eta_wrtVtx','phi_wrtVtx','d0_wrtVtx','z0_wrtVtx',
                     'chi2sum_br_ibl','chi2sum_br_pix1','chi2sum_br_pix2','chi2sum_br_pix3','chi2sum_br_sct1','chi2sum_br_sct2',
                     'chi2sum_br_sct3','chi2sum_br_sct4',
                     'ndofsum_br_ibl','ndofsum_br_pix1','ndofsum_br_pix2','ndofsum_br_pix3','ndofsum_br_sct1','ndofsum_br_sct2',
                     'ndofsum_br_sct3','ndofsum_br_sct4']
DisTrkToKeepIso = ['category','is_fail','iso1_dr01','iso1_dr02','iso2_dr01','iso2_dr02','iso3_dr01','iso3_dr02']
DisTrkVars = []
for var in DisTrkToKeepNoIso:
    DisTrkVars.append('disTrkCand_'+var)
    DisTrkVars.append('disTrkCand_refit_'+var)
for var in DisTrkToKeepIso:
    DisTrkVars.append('disTrkCand_'+var)
DisTrkCandVars = '.'.join(DisTrkVars)

DisTrkBDTSelToKeepBase = ['category','pt','eta','phi','refit_pt','is_fail','d0_wrtVtx','z0_wrtVtx','chi2','ndof',
                          'n_hits_pix','n_hits_sct','n_hits_innermost','iso3_dr01','iso3_dr02','refit_d0_wrtVtx',
                          'refit_z0_wrtVtx','refit_chi2','refit_ndof','chi2ndof_pix','bdtscore']
DisTrkBDTSelToKeep = []
for var in DisTrkBDTSelToKeepBase:
    DisTrkBDTSelToKeep.append('disTrk_'+var)
DisTrkBDTSelVars = '.'.join(DisTrkBDTSelToKeep)

# ==============
# === L1Topo ===
L1TopoErrorFlagVars = '.'.join(['hasGenericRoiError', 'hasGenericDaqError', 'hasCrcTobError', 'hasCrcFibreError',
                                'hasCrcDaqError', 'hasRoibDaqDifference', 'hasRoibCtpDifference', 'hasDaqCtpDifference'])
# ===========
# === Tau ===
def getTauIDVars(name: str) -> list[str]:
    # Default list of variables required for all triggers, according to the naming convention
    return [f'{name}_{sfx}' for sfx in ['Score', 'ScoreSigTrans', 'VeryLoose', 'Loose', 'Medium', 'Tight']]

TauJet_vars = []
TauJet_vars += getTauIDVars('GNTau')
TauJet_vars_str = '.'.join(TauJet_vars)

TauTrack_vars = ['pt', 'eta', 'phi', 'flagSet', 'trackLinks', 'd0TJVA', 'd0SigTJVA', 'z0sinthetaTJVA', 'z0sinthetaSigTJVA']
TauTrack_vars_str = '.'.join(TauTrack_vars)

# ===========
# === LLP ===
MuRoiToKeep = ['ClusterEta','ClusterPhi','nRoIs']
MuRoiVars = '.'.join(MuRoiToKeep)


# ------------------------------------------------------------
# List of tuples with variables [0] and associated collections 
# to be removed from collections for AODSLIM
# Format: ('decoration', 'Container1Aux', 'Container2Aux',...)
# e.g. ('GhostTrack_ftf', 'HLT_AntiKt4EMTopoJets_nojcalib_ftfAux', 'HLT_AntiKt4EMTopoJets_nojcalibAux'),
#     ('ActiveArea', 'HLT_AntiKt4EMTopoJets_nojcalib_ftfAux'),
# ------------------------------------------------------------
varToRemoveFromAODSLIM = [
    ('GhostTrack_ftf', 'HLT_AntiKt4EMTopoJets_nojcalib_ftfAux', 'HLT_AntiKt4EMPFlowJets_nojcalib_ftfAux', 'HLT_AntiKt10EMPFlowCSSKSoftDropBeta100Zcut10Jets_jes_ftfAux'),
    ('TracksForMinimalJetTag', 'HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftfAux'),
    ]
# Detailed NNJvt input variables
for var in ['RPtTrkPt500', 'DTrackWidthPt1000', 'DNumTrkPt1000', 'DRPtTrkPt500', 'SumPtTrkOrderedTrackWidthPt1000', 'SumPtTrkOrderedNumTrkPt1000']:
    varToRemoveFromAODSLIM.append((var, 'HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftfAux', 'HLT_AntiKt4EMTopoJets_subresjesgscIS_ftfAux'))



# ------------------------------------------------------------
# Trigger EDM list for Run 3 with all containers that should
# be stored in the specified format as well as the category.
#
# Syntax:
#   (Type#Key, 'Format1 Format2...', 'Category', [Properties])
#
# The optional Properties list contains instances of InViews,
# Alias, etc. classes.
# ------------------------------------------------------------

TriggerHLTListRun3 = [

    # framework/steering
    ('xAOD::TrigDecision#xTrigDecision' ,                    'ESD AODFULL AODSLIM', 'Steer'),
    ('xAOD::TrigDecisionAuxInfo#xTrigDecisionAux.',          'ESD AODFULL AODSLIM', 'Steer'),
    ('xAOD::TrigConfKeys#TrigConfKeys' ,                     'ESD AODFULL AODSLIM', 'Steer'),
    ('xAOD::BunchConfKey#BunchConfKey' ,                     'ESD AODFULL AODSLIM', 'Steer'),
    ('xAOD::TrigConfKeys#TrigConfKeysOnline' ,               'BS ESD AODFULL AODSLIM', 'Steer'),
    ('xAOD::TrigCompositeContainer#HLT_RuntimeMetadata',     'CostMonDS BS ESD', 'Steer'),
    ('xAOD::TrigCompositeAuxContainer#HLT_RuntimeMetadataAux.hostname', 'CostMonDS BS ESD', 'Steer'),

    ('TrigRoiDescriptorCollection#HLT_FSRoI',                    'BS ESD AODFULL AODSLIM',  'Steer'),
    ('TrigRoiDescriptorCollection#HLT_MURoIs',                   'BS ESD AODFULL AODSLIM',  'Steer'),
    ('TrigRoiDescriptorCollection#HLT_eEMRoIs',                  'BS ESD AODFULL AODSLIM',  'Steer'),
    ('TrigRoiDescriptorCollection#HLT_eTAURoIs',                 'BS ESD AODFULL AODSLIM',  'Steer'),
    ('TrigRoiDescriptorCollection#HLT_jTAURoIs',                 'BS ESD AODFULL AODSLIM',  'Steer'),
    ('TrigRoiDescriptorCollection#HLT_cTAURoIs',                 'BS ESD AODFULL AODSLIM',  'Steer'),
    ('TrigRoiDescriptorCollection#HLT_jEMRoIs',                  'BS ESD AODFULL AODSLIM',  'Steer'),
    ('TrigRoiDescriptorCollection#HLT_jJRoIs',                   'BS ESD AODFULL AODSLIM',  'Steer'),
    ('TrigRoiDescriptorCollection#HLT_jLJRoIs',                  'BS ESD AODFULL AODSLIM',  'Steer'),
    ('TrigRoiDescriptorCollection#HLT_gJRoIs',                   'BS ESD AODFULL AODSLIM',  'Steer'),
    ('TrigRoiDescriptorCollection#HLT_gLJRoIs',                  'BS ESD AODFULL AODSLIM',  'Steer'),
    ('TrigRoiDescriptorCollection#HLT_EMRoIs',                   'BS ESD AODFULL AODSLIM',  'Steer'),
    ('TrigRoiDescriptorCollection#HLT_METRoI',                   'BS ESD AODFULL AODSLIM',  'Steer'),
    ('TrigRoiDescriptorCollection#HLT_JETRoI',                   'BS ESD AODFULL AODSLIM',  'Steer'),
    ('TrigRoiDescriptorCollection#HLT_TAURoI',                   'BS ESD AODFULL AODSLIM',  'Steer'),

    # xAOD::TrigCompositeContainer#HLTNav_Summary is now transient-only
    ('xAOD::TrigCompositeContainer#HLTNav_Summary_OnlineSlimmed', 'BS ESD', 'Steer'), 
    ('xAOD::TrigCompositeAuxContainer#HLTNav_Summary_OnlineSlimmedAux.', 'BS ESD', 'Steer'), 

    ('xAOD::TrigCompositeContainer#HLT_TrigCostContainer',   'CostMonDS ESD', 'Steer'),
    ('xAOD::TrigCompositeAuxContainer#HLT_TrigCostContainerAux.alg.store.view.thread.thash.slot.roi.start.stop', 'CostMonDS ESD', 'Steer'),
    ('xAOD::TrigCompositeContainer#HLT_TrigCostROSContainer',   'CostMonDS ESD', 'Steer'),
    ('xAOD::TrigCompositeAuxContainer#HLT_TrigCostROSContainerAux.alg_idx.lvl1ID.robs_id.robs_size.robs_history.robs_status.start.stop', 'CostMonDS ESD', 'Steer'),
    
    # PEB RoIs for full-scan chains
    ('TrigRoiDescriptorCollection#HLT_Roi_LArPEBHLT',            'BS ESD AODFULL',  'Calo'),

    # PEB RoIs for IDCalib
    ('TrigRoiDescriptorCollection#HLT_Roi_IDCalibPEB',           'BS ESD AODFULL',  'ID'),

    # Run-2 L1 (temporary)
    ('xAOD::EmTauRoIContainer#LVL1EmTauRoIs' ,               'ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::EmTauRoIAuxContainer#LVL1EmTauRoIsAux.' ,        'ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::JetRoIContainer#LVL1JetRoIs' ,                   'ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::JetRoIAuxContainer#LVL1JetRoIsAux.' ,            'ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::JetEtRoI#LVL1JetEtRoI' ,                         'ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::JetEtRoIAuxInfo#LVL1JetEtRoIAux.' ,              'ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::EnergySumRoI#LVL1EnergySumRoI' ,                 'ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::EnergySumRoIAuxInfo#LVL1EnergySumRoIAux.',       'ESD AODFULL AODSLIM', 'L1'),

    #Run-3 L1

    ("CaloCellContainer#SCell",                                'ESD', 'L1'),
    ('xAOD::TriggerTowerContainer#xAODTriggerTowers' ,         'ESD', 'L1'),
    ('xAOD::TriggerTowerAuxContainer#xAODTriggerTowersAux.' ,  'ESD', 'L1'),

    ('xAOD::MuonRoIContainer#LVL1MuonRoIs',                          'BS ESD AODFULL', 'L1'),
    ('xAOD::MuonRoIAuxContainer#LVL1MuonRoIsAux.thresholdPatterns',  'BS ESD AODFULL', 'L1'),
    ('xAOD::MuonRoIContainer#LVL1MuonRoIsBCm2',                      'BS ESD AODFULL', 'L1'),
    ('xAOD::MuonRoIAuxContainer#LVL1MuonRoIsBCm2Aux.',               'BS ESD AODFULL', 'L1'),
    ('xAOD::MuonRoIContainer#LVL1MuonRoIsBCm1',                      'BS ESD AODFULL', 'L1'),
    ('xAOD::MuonRoIAuxContainer#LVL1MuonRoIsBCm1Aux.',               'BS ESD AODFULL', 'L1'),
    ('xAOD::MuonRoIContainer#LVL1MuonRoIsBCp1',                      'BS ESD AODFULL', 'L1'),
    ('xAOD::MuonRoIAuxContainer#LVL1MuonRoIsBCp1Aux.',               'BS ESD AODFULL', 'L1'),
    ('xAOD::MuonRoIContainer#LVL1MuonRoIsBCp2',                      'BS ESD AODFULL', 'L1'),
    ('xAOD::MuonRoIAuxContainer#LVL1MuonRoIsBCp2Aux.',               'BS ESD AODFULL', 'L1'),

    ('xAOD::eFexEMRoIContainer#L1_eEMRoI',                                          'BS ESD AODFULL AODSLIM PhysicsTLA EgammaPEBTLA', 'L1'),
    ('xAOD::eFexEMRoIAuxContainer#L1_eEMRoIAux.thresholdPatterns',                  'BS ESD AODFULL AODSLIM PhysicsTLA EgammaPEBTLA', 'L1'),
    ('xAOD::eFexTauRoIContainer#L1_eTauRoI',                                        'BS ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::eFexTauRoIAuxContainer#L1_eTauRoIAux.thresholdPatterns',                'BS ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::eFexTauRoIContainer#L1_eTauxRoI',                                       'ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::eFexTauRoIAuxContainer#L1_eTauxRoIAux.',                                'ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::eFexTauRoIContainer#L1_cTauRoI',                                        'BS ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::eFexTauRoIAuxContainer#L1_cTauRoIAux.thresholdPatterns.jTauLink',       'BS ESD AODFULL AODSLIM', 'L1'),

    ('xAOD::jFexTauRoIContainer#L1_jFexTauRoI',                                     'BS ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::jFexTauRoIAuxContainer#L1_jFexTauRoIAux.thresholdPatterns',             'BS ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::jFexFwdElRoIContainer#L1_jFexFwdElRoI',                                 'BS ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::jFexFwdElRoIAuxContainer#L1_jFexFwdElRoIAux.thresholdPatterns',         'BS ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::jFexSRJetRoIContainer#L1_jFexSRJetRoI',                                 'BS ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::jFexSRJetRoIAuxContainer#L1_jFexSRJetRoIAux.thresholdPatterns',         'BS ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::jFexLRJetRoIContainer#L1_jFexLRJetRoI',                                 'BS ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::jFexLRJetRoIAuxContainer#L1_jFexLRJetRoIAux.thresholdPatterns',         'BS ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::jFexMETRoIContainer#L1_jFexMETRoI',                                     'BS ESD AODFULL', 'L1'),
    ('xAOD::jFexMETRoIAuxContainer#L1_jFexMETRoIAux.thresholdPatterns',             'BS ESD AODFULL', 'L1'),
    ('xAOD::jFexSumETRoIContainer#L1_jFexSumETRoI',                                 'BS ESD AODFULL', 'L1'),
    ('xAOD::jFexSumETRoIAuxContainer#L1_jFexSumETRoIAux.thresholdPatterns',         'BS ESD AODFULL', 'L1'),

    ('xAOD::gFexJetRoIContainer#L1_gFexSRJetRoI',                                       'BS ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::gFexJetRoIAuxContainer#L1_gFexSRJetRoIAux.thresholdPatterns',               'BS ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::gFexJetRoIContainer#L1_gFexLRJetRoI',                                       'BS ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::gFexJetRoIAuxContainer#L1_gFexLRJetRoIAux.thresholdPatterns',               'BS ESD AODFULL AODSLIM', 'L1'),
    ('xAOD::gFexJetRoIContainer#L1_gFexRhoRoI',                                         'BS ESD AODFULL', 'L1'),
    ('xAOD::gFexJetRoIAuxContainer#L1_gFexRhoRoIAux.thresholdPatterns',                 'BS ESD AODFULL', 'L1'),
    ('xAOD::gFexGlobalRoIContainer#L1_gScalarEJwoj',                                    'BS ESD AODFULL', 'L1'),
    ('xAOD::gFexGlobalRoIAuxContainer#L1_gScalarEJwojAux.thresholdPatterns',            'BS ESD AODFULL', 'L1'),
    ('xAOD::gFexGlobalRoIContainer#L1_gMETComponentsJwoj',                              'BS ESD AODFULL', 'L1'),
    ('xAOD::gFexGlobalRoIAuxContainer#L1_gMETComponentsJwojAux.thresholdPatterns',      'BS ESD AODFULL', 'L1'),
    ('xAOD::gFexGlobalRoIContainer#L1_gMHTComponentsJwoj',                              'BS ESD AODFULL', 'L1'),
    ('xAOD::gFexGlobalRoIAuxContainer#L1_gMHTComponentsJwojAux.thresholdPatterns',      'BS ESD AODFULL', 'L1'),
    ('xAOD::gFexGlobalRoIContainer#L1_gMSTComponentsJwoj',                              'BS ESD AODFULL', 'L1'),
    ('xAOD::gFexGlobalRoIAuxContainer#L1_gMSTComponentsJwojAux.thresholdPatterns',      'BS ESD AODFULL', 'L1'),
    ('xAOD::gFexGlobalRoIContainer#L1_gMETComponentsNoiseCut',                          'BS ESD AODFULL', 'L1'),
    ('xAOD::gFexGlobalRoIAuxContainer#L1_gMETComponentsNoiseCutAux.thresholdPatterns',  'BS ESD AODFULL', 'L1'),
    ('xAOD::gFexGlobalRoIContainer#L1_gMETComponentsRms',                               'BS ESD AODFULL', 'L1'),
    ('xAOD::gFexGlobalRoIAuxContainer#L1_gMETComponentsRmsAux.thresholdPatterns',       'BS ESD AODFULL', 'L1'),
    ('xAOD::gFexGlobalRoIContainer#L1_gScalarENoiseCut',                                'BS ESD AODFULL', 'L1'),
    ('xAOD::gFexGlobalRoIAuxContainer#L1_gScalarENoiseCutAux.thresholdPatterns',        'BS ESD AODFULL', 'L1'),
    ('xAOD::gFexGlobalRoIContainer#L1_gScalarERms',                                     'BS ESD AODFULL', 'L1'),
    ('xAOD::gFexGlobalRoIAuxContainer#L1_gScalarERmsAux.thresholdPatterns',             'BS ESD AODFULL', 'L1'),

    ('xAOD::EnergySumRoI#jXENOISECUTPerf' ,                 'ESD AODFULL', 'L1'),
    ('xAOD::EnergySumRoIAuxInfo#jXENOISECUTPerfAux.',       'ESD AODFULL', 'L1'),

    ('xAOD::EnergySumRoI#jXERHOPerf' ,                      'ESD AODFULL', 'L1'),
    ('xAOD::EnergySumRoIAuxInfo#jXERHOPerfAux.' ,           'ESD AODFULL', 'L1'),

    ('xAOD::EnergySumRoI#gXENOISECUTPerf' ,                  'ESD AODFULL', 'L1'),
    ('xAOD::EnergySumRoIAuxInfo#gXENOISECUTPerfAux.' ,       'ESD AODFULL', 'L1'),

    ('xAOD::EnergySumRoI#gXERHOPerf' ,                 'ESD AODFULL', 'L1'),
    ('xAOD::EnergySumRoIAuxInfo#gXERHOPerfAux.',       'ESD AODFULL', 'L1'),

    ('xAOD::EnergySumRoI#gXEJWOJPerf' ,                     'ESD AODFULL', 'L1'),
    ('xAOD::EnergySumRoIAuxInfo#gXEJWOJPerfAux.',           'ESD AODFULL', 'L1'),

    ("xAOD::TrigEMClusterContainer#eElesPerf" ,             'ESD AODFULL', 'L1'),
    ("xAOD::TrigEMClusterAuxContainer#eElesPerfAux." ,      'ESD AODFULL', 'L1'),

    ('xAOD::EmTauRoIContainer#eTausPerf',                   'ESD AODFULL', 'L1'),
    ('xAOD::EmTauRoIAuxContainer#eTausPerfAux.',            'ESD AODFULL', 'L1'),

    ('xAOD::TrigCompositeContainer#L1TopoErrorFlags_Legacy', 'BS ESD AODFULL', 'L1'),
    ('xAOD::TrigCompositeAuxContainer#L1TopoErrorFlags_LegacyAux.'+L1TopoErrorFlagVars, 'BS ESD AODFULL', 'L1'),

    #L1 mistime monitoring
    ('xAOD::TrigCompositeContainer#HLT_TrigCompositeMistimeJ400', 'BS ESD AODFULL', 'L1'),
    ('xAOD::TrigCompositeAuxContainer#HLT_TrigCompositeMistimeJ400Aux.pass.other_type.beforeafterflag.l1a_type', 'BS ESD AODFULL', 'L1'),

    # LArPS
    ('xAOD::TrigEMClusterContainer#HLT_LArPS_AllCaloEMClusters',           'BS ESD', 'Egamma', [InViews('LArPS_AllEM_Views')]),
    ('xAOD::TrigEMClusterAuxContainer#HLT_LArPS_AllCaloEMClustersAux.',    'BS ESD', 'Egamma'),
    ('xAOD::TrigEMClusterContainer#HLT_LArPS_AllCaloClusters',             'BS ESD', 'Egamma', [InViews('LArPS_All_Views')]),
    ('xAOD::TrigEMClusterAuxContainer#HLT_LArPS_AllCaloClustersAux.',      'BS ESD', 'Egamma'),

    # Egamma
    ('xAOD::TrigEMClusterContainer#HLT_FastCaloEMClusters',           'BS ESD', 'Egamma', [InViews('EMCaloViews')]),
    ('xAOD::TrigEMClusterAuxContainer#HLT_FastCaloEMClustersAux.',    'BS ESD', 'Egamma'),
    ('xAOD::TrigRingerRingsContainer#HLT_FastCaloRinger',             'BS ESD', 'Egamma', [InViews('EMCaloViews')]), #Ringer
    ('xAOD::TrigRingerRingsAuxContainer#HLT_FastCaloRingerAux.',      'BS ESD', 'Egamma'), #Ringer

    # Egamma FWD
    ('xAOD::TrigEMClusterContainer#HLT_FastCaloEMClusters_FWD',           'BS ESD AODFULL', 'Egamma', [InViews('EMCaloViews_FWD')]),
    ('xAOD::TrigEMClusterAuxContainer#HLT_FastCaloEMClusters_FWDAux.',    'BS ESD AODFULL', 'Egamma'),

    ('xAOD::TrigPhotonContainer#HLT_FastPhotons',                     'BS ESD AODFULL', 'Egamma', [InViews('EMPhotonViews')]),
    ('xAOD::TrigPhotonAuxContainer#HLT_FastPhotonsAux.',              'BS ESD AODFULL', 'Egamma'),
    ('xAOD::TrigElectronContainer#HLT_FastElectrons',                 'BS ESD AODFULL', 'Egamma', [InViews('EMElectronViews')]),
    ('xAOD::TrigElectronAuxContainer#HLT_FastElectronsAux.',          'BS ESD AODFULL', 'Egamma'),

    ('xAOD::TrigElectronContainer#HLT_FastElectrons_LRT',                 'BS ESD AODFULL', 'Egamma', [InViews('EMElectron_LRTViews')]),
    ('xAOD::TrigElectronAuxContainer#HLT_FastElectrons_LRTAux.',          'BS ESD AODFULL', 'Egamma'),

    ('xAOD::TrackParticleContainer#HLT_IDTrack_Electron_FTF',        'BS ESD AODFULL', 'Egamma', [InViews('EMFastTrackingViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_Electron_FTFAux.', 'BS ESD AODFULL', 'Egamma'),

    ('xAOD::TrackParticleContainer#HLT_IDTrack_ElecLRT_FTF',        'BS ESD AODFULL', 'Egamma', [InViews('EMFastTracking_LRTViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_ElecLRT_FTFAux.', 'BS ESD AODFULL', 'Egamma'),

    ('xAOD::TrackParticleContainer#HLT_IDTrack_Electron_IDTrig',        'BS ESD AODFULL', 'Egamma', [InViews('precisionTrackingViews'),InViews('precisionHITrackingViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_Electron_IDTrigAux.',        'BS ESD AODFULL', 'Egamma'),

    ('xAOD::TrackParticleContainer#HLT_IDTrack_ElecLRT_IDTrig',        'BS ESD AODFULL', 'Egamma', [InViews('precisionTracking_LRTViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_ElecLRT_IDTrigAux.', 'BS ESD AODFULL', 'Egamma'),

    ('xAOD::TrackParticleContainer#HLT_IDTrack_Electron_LRTGSF',               'BS ESD AODFULL', 'Egamma', [InViews('precisionTracking_GSFRefitted_LRTGSFViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_Electron_LRTGSFAux.',           'BS ESD AODFULL', 'Egamma'),

    ('xAOD::TrackParticleContainer#HLT_IDTrack_Electron_GSF',               'BS ESD AODFULL', 'Egamma', [InViews('precisionTracking_GSFRefitted_GSFViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_Electron_GSFAux.parameterPX.parameterPY.parameterPZ.parameterPosition',        'BS ESD AODFULL', 'Egamma'),

    # these two corresponds to the output of the precisionCalo step
    ('xAOD::CaloClusterContainer#HLT_CaloEMClusters_Electron',               'BS ESD AODFULL AODSLIM', 'Egamma', [InViews('precisionCaloElectronViews')]),
    ('xAOD::CaloClusterTrigAuxContainer#HLT_CaloEMClusters_ElectronAux.',    'BS ESD AODFULL AODSLIM', 'Egamma'),

    ('xAOD::CaloClusterContainer#HLT_CaloEMClusters_Photon',               'BS PhysicsTLA EgammaPEBTLA ESD AODFULL AODSLIM', 'Egamma', [InViews('precisionCaloPhotonViews')]),
    ('xAOD::CaloClusterTrigAuxContainer#HLT_CaloEMClusters_PhotonAux.',    'BS PhysicsTLA EgammaPEBTLA ESD AODFULL AODSLIM', 'Egamma'),

    # these two corresponds to the output of the precisionHICalo step
    ('xAOD::CaloClusterContainer#HLT_HICaloEMClusters',               'BS ESD AODFULL AODSLIM', 'Egamma', [InViews('precisionHICaloElectronViews'),InViews('precisionHICaloPhotonViews')]),
    ('xAOD::CaloClusterTrigAuxContainer#HLT_HICaloEMClustersAux.',    'BS ESD AODFULL AODSLIM', 'Egamma'),

    # these two corresponds to the output of the precisionCalo_LRT step
    ('xAOD::CaloClusterContainer#HLT_CaloEMClusters_LRT',               'BS ESD AODFULL', 'Egamma', [InViews('precisionCaloElectron_LRTViews')]),
    ('xAOD::CaloClusterTrigAuxContainer#HLT_CaloEMClusters_LRTAux.',    'BS ESD AODFULL', 'Egamma'),

    # these two corresponds to the output of the precisionCalo forward step
    ('xAOD::CaloClusterContainer#HLT_CaloEMClusters_FWD',               'BS ESD', 'Egamma', [InViews('precisionCaloElectron_FWDViews')]),
    ('xAOD::CaloClusterTrigAuxContainer#HLT_CaloEMClusters_FWDAux.',    'BS ESD', 'Egamma'),


    # This variant needed by TrigUpgradeTest/egammaRinger.py
    ('xAOD::CaloClusterContainer#HLT_TopoCaloClusters',             'BS ESD AODFULL', 'Egamma'),
    ('xAOD::CaloClusterTrigAuxContainer#HLT_TopoCaloClustersAux.',  'BS ESD AODFULL', 'Egamma'),
    #
    ('xAOD::CaloClusterContainer#HLT_TopoCaloClustersLCFS',                               'BS ESD', 'Jet', [Alias('CaloClusterContainerShallowCopy')]),
    ('xAOD::ShallowAuxContainer#HLT_TopoCaloClustersLCFSAux.calE.calEta.calPhi',         'BS ESD', 'Jet'),

    # Not sure we need these two...
    ('xAOD::CaloClusterContainer#HLT_TopoCaloClustersRoI',          'BS ESD AODFULL', 'Egamma', [InViews('precisionCaloElectronViews'), InViews('precisionCaloPhotonViews')]),
    ('xAOD::CaloClusterTrigAuxContainer#HLT_TopoCaloClustersRoIAux.nCells', 'BS ESD AODFULL', 'Egamma'),

    ('xAOD::CaloClusterContainer#HLT_TopoCaloClustersRoI_LRT',          'BS ESD AODFULL', 'Egamma', [InViews('precisionCaloElectron_LRTViews')]),
    ('xAOD::CaloClusterTrigAuxContainer#HLT_TopoCaloClustersRoI_LRTAux.nCells', 'BS ESD AODFULL', 'Egamma'),

    # UE subtracted versions for heavy ion paths
    ('xAOD::CaloClusterContainer#HLT_TopoCaloClustersHIRoI',                    'BS ESD AODFULL', 'Egamma', [InViews('precisionHICaloElectronViews'), InViews('precisionHICaloPhotonViews')]),
    ('xAOD::CaloClusterTrigAuxContainer#HLT_TopoCaloClustersHIRoIAux.nCells',   'BS ESD AODFULL', 'Egamma'),

    # These are for precision photon and precision Electron Keeping same names as in Run2
    ('xAOD::ElectronContainer#HLT_egamma_Electrons',                'BS ESD AODFULL AODSLIM', 'Egamma', [InViews('precisionElectronViews'), InViews('precisionHIElectronViews')]),
    ('xAOD::ElectronAuxContainer#HLT_egamma_ElectronsAux.'+ElVars,     'BS ESD AODFULL AODSLIM', 'Egamma'),
    ('xAOD::ElectronContainer#HLT_egamma_Electrons_GSF',                'BS ESD AODFULL AODSLIM', 'Egamma', [InViews('precisionElectron_GSFViews')]),
    ('xAOD::ElectronAuxContainer#HLT_egamma_Electrons_GSFAux.'+ElVars,     'BS ESD AODFULL AODSLIM', 'Egamma'),
    ('xAOD::ElectronContainer#HLT_egamma_Electrons_LRT',                'BS ESD AODFULL AODSLIM', 'Egamma', [InViews('precisionElectron_LRTViews')]),
    ('xAOD::ElectronAuxContainer#HLT_egamma_Electrons_LRTAux.'+ElVars,     'BS ESD AODFULL AODSLIM', 'Egamma'),
    ('xAOD::ElectronContainer#HLT_egamma_Electrons_LRTGSF',                'BS ESD AODFULL AODSLIM', 'Egamma', [InViews('precisionElectron_LRTGSFViews')]),
    ('xAOD::ElectronAuxContainer#HLT_egamma_Electrons_LRTGSFAux.'+ElVars,     'BS ESD AODFULL AODSLIM', 'Egamma'),

    ('xAOD::PhotonContainer#HLT_egamma_Photons',                    'BS ESD AODFULL AODSLIM', 'Egamma', [InViews('precisionPhotonViews'), InViews('precisionHIPhotonViews')]),
    ('xAOD::PhotonAuxContainer#HLT_egamma_PhotonsAux.'    ,         'BS ESD AODFULL AODSLIM', 'Egamma'),

    ('xAOD::PhotonContainer#HLT_egamma_Iso_Photons',                    'BS ESD AODFULL AODSLIM', 'Egamma'),
    ('xAOD::PhotonAuxContainer#HLT_egamma_Iso_PhotonsAux.'+PhVars,         'BS ESD AODFULL AODSLIM', 'Egamma'),

    ('TrigRoiDescriptorCollection#HLT_Roi_FastElectron',            'BS ESD AODFULL', 'Egamma'),
    ('TrigRoiDescriptorCollection#HLT_Roi_FastElectron_probe',      'BS ESD AODFULL', 'Egamma'),
    ('TrigRoiDescriptorCollection#HLT_Roi_FastElectron_LRT',        'BS ESD AODFULL', 'Egamma'),
    ('TrigRoiDescriptorCollection#HLT_Roi_FastElectron_LRT_probe',  'BS ESD AODFULL', 'Egamma'),
    ('TrigRoiDescriptorCollection#HLT_Roi_FastPhoton',              'BS ESD AODFULL', 'Egamma'),
    ('TrigRoiDescriptorCollection#HLT_Roi_FastPhoton_probe',        'BS ESD AODFULL', 'Egamma'),

    # hipTRT
    ('xAOD::TrigRNNOutputContainer#HLT_TrigTRTHTCounts',            'BS ESD AODFULL', 'Egamma', [InViews('TRTHitGeneratorViews')]),
    ('xAOD::TrigRNNOutputAuxContainer#HLT_TrigTRTHTCountsAux.',            'BS ESD AODFULL', 'Egamma'),

    # CaloCluster object written by EMClusterTool
    ('xAOD::CaloClusterContainer#HLT_TrigEMClusters_Electrons',        'BS ESD AODFULL', 'Egamma', [InViews('precisionElectronViews')]),
    ('xAOD::CaloClusterTrigAuxContainer#HLT_TrigEMClusters_ElectronsAux.',     'BS ESD AODFULL', 'Egamma'),
    ('xAOD::CaloClusterContainer#HLT_TrigEMClusters_Electrons_LRT',        'BS ESD AODFULL', 'Egamma', [InViews('precisionElectron_LRTViews')]),
    ('xAOD::CaloClusterTrigAuxContainer#HLT_TrigEMClusters_Electrons_LRTAux.',     'BS ESD AODFULL', 'Egamma'),
    ('xAOD::CaloClusterContainer#HLT_TrigEMClusters_Electrons_GSF',        'BS ESD AODFULL', 'Egamma', [InViews('precisionElectron_GSFViews')]),
    ('xAOD::CaloClusterTrigAuxContainer#HLT_TrigEMClusters_Electrons_GSFAux.',     'BS ESD AODFULL', 'Egamma'),
    ('xAOD::CaloClusterContainer#HLT_TrigEMClusters_Electrons_LRTGSF',        'BS ESD AODFULL', 'Egamma', [InViews('precisionElectron_LRTGSFViews')]),
    ('xAOD::CaloClusterTrigAuxContainer#HLT_TrigEMClusters_Electrons_LRTGSFAux.',     'BS ESD AODFULL', 'Egamma'),
    ('xAOD::CaloClusterContainer#HLT_TrigEMClusters_Photons',        'BS ESD AODFULL', 'Egamma', [InViews('precisionPhotonViews')]),
    ('xAOD::CaloClusterTrigAuxContainer#HLT_TrigEMClusters_PhotonsAux.',     'BS ESD AODFULL', 'Egamma'),

    # Muon

    # Id track particles
    ('xAOD::TrackParticleContainer#HLT_IDTrack_Muon_FTF',                 'BS ESD AODFULL', 'Muon', [InViews('L2MuCombRecoViews'), InViews('EFMuCBReco_FSViews'), InViews('EFLateMuRecoViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_Muon_FTFAux.',          'BS ESD AODFULL', 'Muon'),

    ('xAOD::TrackParticleContainer#HLT_IDTrack_Muon_IDTrig',                 'BS ESD AODFULL', 'Muon', [InViews('EFMuCBReco_RoIViews'), InViews('EFMuCBReco_FSViews'), InViews('EFLateMuRecoViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_Muon_IDTrigAux.',          'BS ESD AODFULL', 'Muon'),

    ('xAOD::TrackParticleContainer#HLT_IDTrack_MuonLRT_IDTrig',                 'BS ESD AODFULL', 'Muon', [InViews('EFMuCBLRTRecoViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_MuonLRT_IDTrigAux.',          'BS ESD AODFULL', 'Muon'),

    ('xAOD::TrackParticleContainer#HLT_IDTrack_MuonIso_FTF',                 'BS ESD AODFULL', 'Muon', [InViews('EFMuIsoRecoViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_MuonIso_FTFAux.',          'BS ESD AODFULL', 'Muon'),

    ('xAOD::TrackParticleContainer#HLT_IDTrack_MuonIso_IDTrig',              'BS ESD AODFULL', 'Muon', [InViews('EFMuIsoRecoViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_MuonIso_IDTrigAux.',       'BS ESD AODFULL', 'Muon'),


    ('xAOD::TrackParticleContainer#HLT_IDTrack_MuonLRT_FTF',                 'BS ESD AODFULL', 'Muon', [InViews('l2muCombLRTViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_MuonLRT_FTFAux.',          'BS ESD AODFULL', 'Muon'),

    #Muon DS calibration
    ('xAOD::TrigCompositeContainer#MuonCalibrationStream',                    'MuonDS', 'Muon'),
    ('xAOD::TrigCompositeAuxContainer#MuonCalibrationStreamAux.MuonCalibrationStream',             'MuonDS', 'Muon'),

    

# These extra muon collections have all be removed now, as they all have identical
# reconstruction as the standard Muon collections so unique collections names should
# not be needed. Once we are sure that there are no affected clients these commented
# collections can be removed

#    ('xAOD::TrackParticleContainer#HLT_IDTrack_MuonFS_FTF',                 'BS ESD AODFULL', 'Muon', [InViews('EFMuCBReco_FSViews')]),
#    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_MuonFS_FTFAux.',          'BS ESD AODFULL', 'Muon'),

#    ('xAOD::TrackParticleContainer#HLT_IDTrack_MuonFS_IDTrig',              'BS ESD AODFULL', 'Muon', [InViews('EFMuCBReco_FSViews')]),
#    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_MuonFS_IDTrigAux.',       'BS ESD AODFULL', 'Muon'),

#    ('xAOD::TrackParticleContainer#HLT_IDTrack_MuonLate_FTF',                 'BS ESD AODFULL', 'Muon', [InViews('MUEFLATEViewRoIs')]),
#    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_MuonLate_FTFAux.',          'BS ESD AODFULL', 'Muon'),

#    ('xAOD::TrackParticleContainer#HLT_IDTrack_MuonLate_IDTrig',              'BS ESD AODFULL', 'Muon', [InViews('MUEFLATEViewRoIs')]),
#    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_MuonLate_IDTrigAux.',       'BS ESD AODFULL', 'Muon'),


    # Bphysics Dimuon chains
    ('xAOD::TrigBphysContainer#HLT_DimuEF',                                 'BS ESD AODFULL AODSLIM AODBLSSLIM', 'Bphys'),
    ('xAOD::TrigBphysAuxContainer#HLT_DimuEFAux.',                          'BS ESD AODFULL AODSLIM AODBLSSLIM', 'Bphys'),
    # Bphysics Bmumux chains
    ('xAOD::TrigBphysContainer#HLT_Bmumux',                                 'BS ESD AODFULL AODSLIM AODBLSSLIM', 'Bphys'),
    ('xAOD::TrigBphysAuxContainer#HLT_BmumuxAux.',                          'BS ESD AODFULL AODSLIM AODBLSSLIM', 'Bphys'),
    ('xAOD::TrackParticleContainer#HLT_IDTrack_Bmumux_FTF',                 'BS ESD AODFULL', 'Bphys', [InViews('BmumuxViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_Bmumux_FTFAux.',          'BS ESD AODFULL', 'Bphys'),
    ('xAOD::TrackParticleContainer#HLT_IDTrack_Bmumux_IDTrig',              'BS ESD AODFULL AODSLIM', 'Bphys', [InViews('BmumuxViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_Bmumux_IDTrigAux.',       'BS ESD AODFULL AODSLIM', 'Bphys'),
    ('TrigRoiDescriptorCollection#HLT_Roi_Bmumux',                          'BS ESD AODFULL', 'Bphys'),
    # Bphysics Bmux chains
    ('xAOD::TrigBphysContainer#HLT_Bmux',                                   'BS ESD AODFULL AODSLIM AODBLSSLIM', 'Bphys'),
    ('xAOD::TrigBphysAuxContainer#HLT_BmuxAux.',                            'BS ESD AODFULL AODSLIM AODBLSSLIM', 'Bphys'),
    # Bphysics Tag-and-Probe J/psi from muon + track
    ('xAOD::TrigBphysContainer#HLT_Bmutrk',                                 'BS ESD AODFULL AODSLIM AODBLSSLIM', 'Bphys'),
    ('xAOD::TrigBphysAuxContainer#HLT_BmutrkAux.',                          'BS ESD AODFULL AODSLIM AODBLSSLIM', 'Bphys'),
    # Low mass Drell-Yan chains
    ('xAOD::TrigBphysContainer#HLT_DrellYan',                               'BS ESD AODFULL AODSLIM AODBLSSLIM', 'Bphys'),
    ('xAOD::TrigBphysAuxContainer#HLT_DrellYanAux.',                        'BS ESD AODFULL AODSLIM AODBLSSLIM', 'Bphys'),
    # Bphysics Di-electron chains
    ('xAOD::TrigBphysContainer#HLT_DiElecPrecision',                        '', 'Bphys'),
    ('xAOD::TrigBphysAuxContainer#HLT_DiElecPrecisionAux.',                 '', 'Bphys'),
    ('xAOD::TrigBphysContainer#HLT_NoMuonDiElecPrecision',                  '', 'Bphys'),
    ('xAOD::TrigBphysAuxContainer#HLT_NoMuonDiElecPrecisionAux.',           '', 'Bphys'),
    ('xAOD::TrigBphysContainer#HLT_DiElecPrecisionGSF',                     'BS ESD AODFULL AODSLIM AODBLSSLIM', 'Bphys'),
    ('xAOD::TrigBphysAuxContainer#HLT_DiElecPrecisionGSFAux.',              'BS ESD AODFULL AODSLIM AODBLSSLIM', 'Bphys'),
    ('xAOD::TrigBphysContainer#HLT_NoMuonDiElecPrecisionGSF',               'BS ESD AODFULL AODSLIM AODBLSSLIM', 'Bphys'),
    ('xAOD::TrigBphysAuxContainer#HLT_NoMuonDiElecPrecisionGSFAux.',        'BS ESD AODFULL AODSLIM AODBLSSLIM', 'Bphys'),

    # xAOD muons (msonly (x2: roi+FS), combined (x2: FS+RoI)
    ('xAOD::MuonContainer#HLT_Muons_RoI',                                       'BS ESD AODFULL AODSLIM', 'Muon', [InViews('EFMuMSReco_RoIViews')]),
    ('xAOD::MuonAuxContainer#HLT_Muons_RoIAux.',                                'BS ESD AODFULL AODSLIM', 'Muon'),

    ('xAOD::MuonContainer#HLT_Muons_FS',                                        'BS ESD AODFULL AODSLIM', 'Muon', [InViews('EFMuMSReco_FSViews')]),
    ('xAOD::MuonAuxContainer#HLT_Muons_FSAux.',                                 'BS ESD AODFULL AODSLIM', 'Muon'),

    ('xAOD::MuonContainer#HLT_MuonsCB_RoI',                                     'BS ESD AODFULL AODSLIM', 'Muon', [InViews('EFMuCBReco_RoIViews'),InViews('CosmicEFCBViews')]),
    ('xAOD::MuonAuxContainer#HLT_MuonsCB_RoIAux.',                              'BS ESD AODFULL AODSLIM', 'Muon'),

    ('xAOD::MuonContainer#HLT_MuonsCB_LRT',                                     'BS ESD AODFULL AODSLIM', 'Muon', [InViews('EFMuCBLRTRecoViews')]),
    ('xAOD::MuonAuxContainer#HLT_MuonsCB_LRTAux.',                              'BS ESD AODFULL AODSLIM', 'Muon'),

    ('xAOD::MuonContainer#HLT_MuonsCB_FS',                                      'BS ESD AODFULL AODSLIM', 'Muon', [InViews('EFMuCBReco_FSViews')]),
    ('xAOD::MuonAuxContainer#HLT_MuonsCB_FSAux.',                               'BS ESD AODFULL AODSLIM', 'Muon'),

    ('xAOD::MuonContainer#HLT_MuonsCBInsideOut_Late',                           'BS ESD AODFULL AODSLIM', 'Muon', [InViews('EFLateMuRecoViews')]),
    ('xAOD::MuonAuxContainer#HLT_MuonsCBInsideOut_LateAux.',                    'BS ESD AODFULL AODSLIM', 'Muon'),

    ('TrigRoiDescriptorCollection#MuonCandidates_FS_ROIs',                      'BS ESD AODFULL', 'Muon'),

    # xAOD isolated muon
    ('xAOD::MuonContainer#HLT_MuonsIso',                                         'BS ESD AODFULL AODSLIM', 'Muon', [InViews('EFMuIsoRecoViews')]),
    ('xAOD::MuonAuxContainer#HLT_MuonsIsoAux.ptcone02.ptcone03',                 'BS ESD AODFULL AODSLIM', 'Muon'),

    # Muon track particle containers (combined (x2: FS+RoI), extrapolated (x2: FS+RoI), MSonly (x1: FS))
    ('xAOD::TrackParticleContainer#HLT_CBCombinedMuon_RoITrackParticles',                     'BS ESD AODFULL AODSLIM', 'Muon', [InViews('EFMuCBReco_RoIViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_CBCombinedMuon_RoITrackParticlesAux.',              'BS ESD AODFULL AODSLIM', 'Muon'),

    ('xAOD::TrackParticleContainer#HLT_CBCombinedMuon_LRTTrackParticles',                     'BS ESD AODFULL AODSLIM', 'Muon', [InViews('EFMuCBLRTRecoViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_CBCombinedMuon_LRTTrackParticlesAux.',              'BS ESD AODFULL AODSLIM', 'Muon'),

    ('xAOD::TrackParticleContainer#HLT_CBCombinedMuon_FSTrackParticles',                      'BS ESD AODFULL AODSLIM', 'Muon', [InViews('EFMuCBReco_FSViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_CBCombinedMuon_FSTrackParticlesAux.',               'BS ESD AODFULL AODSLIM', 'Muon'),

    ('xAOD::TrackParticleContainer#HLT_MSExtrapolatedMuons_RoITrackParticles',                'BS ESD AODFULL', 'Muon', [InViews('EFMuMSReco_RoIViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_MSExtrapolatedMuons_RoITrackParticlesAux.',         'BS ESD AODFULL', 'Muon'),

    ('xAOD::TrackParticleContainer#HLT_MSExtrapolatedMuons_FSTrackParticles',                 'BS ESD AODFULL', 'Muon', [InViews('EFMuMSReco_FSViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_MSExtrapolatedMuons_FSTrackParticlesAux.',          'BS ESD AODFULL', 'Muon'),

    ('xAOD::TrackParticleContainer#HLT_MSOnlyExtrapolatedMuons_FSTrackParticles',             'BS ESD AODFULL', 'Muon', [InViews('EFMuMSReco_FSViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_MSOnlyExtrapolatedMuons_FSTrackParticlesAux.',      'BS ESD AODFULL', 'Muon'),

    # Muon container from NSW L1 Simulation
    ('Muon::NSW_TrigRawDataContainer#L1_NSWTrigContainer', 'ESD AODFULL', 'Muon'),

    #xAOD L2 muons (SA, CB, isolation)
    ('xAOD::L2StandAloneMuonContainer#HLT_MuonL2SAInfo',        'BS ESD AODFULL', 'Muon', [InViews('L2MuFastRecoViews')]),
    ('xAOD::L2StandAloneMuonAuxContainer#HLT_MuonL2SAInfoAux.', 'BS ESD AODFULL', 'Muon'),

    ('xAOD::L2StandAloneMuonContainer#HLT_MuonL2SAInfoIOmode',        'BS ESD AODFULL', 'Muon', [InViews('L2MuCombRecoViews')]),
    ('xAOD::L2StandAloneMuonAuxContainer#HLT_MuonL2SAInfoIOmodeAux.', 'BS ESD AODFULL', 'Muon'),

    ('xAOD::L2StandAloneMuonContainer#HLT_MuonL2SAInfol2mtmode',        'BS ESD AODFULL', 'Muon', [InViews('L2MuFastRecoViews')]),
    ('xAOD::L2StandAloneMuonAuxContainer#HLT_MuonL2SAInfol2mtmodeAux.', 'BS ESD AODFULL', 'Muon'),

    ('xAOD::L2CombinedMuonContainer#HLT_MuonL2CBInfo',          'BS ESD AODFULL', 'Muon', [InViews('L2MuCombRecoViews'), InViews('CosmicViews')]),
    ('xAOD::L2CombinedMuonAuxContainer#HLT_MuonL2CBInfoAux.',   'BS ESD AODFULL', 'Muon'),

    ('xAOD::L2CombinedMuonContainer#HLT_MuonL2CBInfoLRT',          'BS ESD AODFULL', 'Muon', [InViews('l2muCombLRTViews')]),
    ('xAOD::L2CombinedMuonAuxContainer#HLT_MuonL2CBInfoLRTAux.',   'BS ESD AODFULL', 'Muon'),

    ('xAOD::L2CombinedMuonContainer#HLT_MuonL2CBInfoIOmode',          'BS ESD AODFULL', 'Muon', [InViews('L2MuCombRecoViews')]),
    ('xAOD::L2CombinedMuonAuxContainer#HLT_MuonL2CBInfoIOmodeAux.',   'BS ESD AODFULL', 'Muon'),

    ('xAOD::L2CombinedMuonContainer#HLT_MuonL2CBInfol2mtmode',          'BS ESD AODFULL', 'Muon', [InViews('L2MuCombRecoViews')]),
    ('xAOD::L2CombinedMuonAuxContainer#HLT_MuonL2CBInfol2mtmodeAux.',   'BS ESD AODFULL', 'Muon'),

    ('TrigRoiDescriptorCollection#HLT_Roi_L2SAMuon',                   'BS ESD AODFULL', 'Muon'),
    ('TrigRoiDescriptorCollection#HLT_Roi_L2SAMuon_probe',             'BS ESD AODFULL', 'Muon'),
    ('TrigRoiDescriptorCollection#HLT_Roi_L2SAMuon_LRT',               'BS ESD AODFULL', 'Muon'),
    ('TrigRoiDescriptorCollection#HLT_Roi_L2SAMuon_LRT_probe',         'BS ESD AODFULL', 'Muon'),
    ('TrigRoiDescriptorCollection#HLT_Roi_L2SAMuonForEF',              'BS ESD AODFULL', 'Muon'),
    ('TrigRoiDescriptorCollection#HLT_Roi_L2SAMuonForEF_probe',        'BS ESD AODFULL', 'Muon'),
    ('TrigRoiDescriptorCollection#HLT_Roi_MuonIso',                    'BS ESD AODFULL', 'Muon'),
    ('TrigRoiDescriptorCollection#HLT_Roi_MuonIso_probe',              'BS ESD AODFULL', 'Muon'),

    # Tau

    ('xAOD::TrackParticleContainer#HLT_IDTrack_TauCore_FTF',                 'BS ESD AODFULL', 'Tau', [InViews('tauFastTrackCoreViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_TauCore_FTFAux.',          'BS ESD AODFULL', 'Tau'),

    ('xAOD::TrackParticleContainer#HLT_IDTrack_TauLRT_FTF',                 'BS ESD AODFULL', 'Tau', [InViews('tauFastTrackLRTViews')]),

    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_TauLRT_FTFAux.',          'BS ESD AODFULL', 'Tau'),

    ('xAOD::TrackParticleContainer#HLT_IDTrack_TauIso_FTF',                 'BS ESD AODFULL', 'Tau', [InViews('tauFastTrackIsoViews')]),

    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_TauIso_FTFAux.',          'BS ESD AODFULL', 'Tau'),

    ('xAOD::TrackParticleContainer#HLT_IDTrack_Tau_IDTrig',                 'BS ESD AODFULL', 'Tau', [InViews('precIsoTrackViews'), InViews('tauPrecTrackIsoViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_Tau_IDTrigAux.eProbabilityNN',          'BS ESD AODFULL', 'Tau'),

    ('xAOD::TrackParticleContainer#HLT_IDTrack_TauLRT_IDTrig',                 'BS ESD AODFULL', 'Tau', [InViews('precLRTTauViews'), InViews('precLRTTrackViews'), InViews('tauPrecTrackLRTViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_TauLRT_IDTrigAux.eProbabilityNN',          'BS ESD AODFULL', 'Tau'),

    ('xAOD::VertexContainer#HLT_IDVertex_Tau',                  'BS ESD AODFULL', 'Tau', [InViews('precIsoTrackViews'), InViews('tauPrecTrackIsoViews'), InViews('precLRTTrackViews'), InViews('tauPrecTrackLRTViews')]),
    ('xAOD::VertexAuxContainer#HLT_IDVertex_TauAux.',           'BS ESD AODFULL', 'Tau'),

    ('TrigRoiDescriptorCollection#HLT_Roi_Tau',              'BS ESD AODFULL',  'Tau'),
    ('TrigRoiDescriptorCollection#HLT_Roi_Tau_probe',        'BS ESD AODFULL',  'Tau'),
    ('TrigRoiDescriptorCollection#HLT_Roi_TauCore',          'BS ESD AODFULL',  'Tau'),
    ('TrigRoiDescriptorCollection#HLT_Roi_TauCore_probe',    'BS ESD AODFULL',  'Tau'),
    ('TrigRoiDescriptorCollection#HLT_Roi_TauLRT',           'BS ESD AODFULL',  'Tau'),
    ('TrigRoiDescriptorCollection#HLT_Roi_TauLRT_probe',     'BS ESD AODFULL',  'Tau'),
    ('TrigRoiDescriptorCollection#HLT_Roi_TauIso',           'BS ESD AODFULL',  'Tau'),
    ('TrigRoiDescriptorCollection#HLT_Roi_TauIso_probe',     'BS ESD AODFULL',  'Tau'),
    ('TrigRoiDescriptorCollection#HLT_Roi_TauIsoBDT',        'BS ESD AODFULL',  'Tau'),
    ('TrigRoiDescriptorCollection#HLT_Roi_TauIsoBDT_probe',  'BS ESD AODFULL',  'Tau'),

    ('xAOD::JetContainer#HLT_jet_seed',                         '', 'Tau', [InViews('tauCaloMVAViews')]),
    ('xAOD::JetAuxContainer#HLT_jet_seedAux.',                  '', 'Tau'),

    # Jet
    ('xAOD::JetContainer#HLT_AntiKt4EMTopoJets_subjesIS',                        'BS ESD AODFULL AODSLIM', 'Jet', [Alias('JetContainerShallowCopy')]),
    ('xAOD::ShallowAuxContainer#HLT_AntiKt4EMTopoJets_subjesISAux.'+JetCopyVars, 'BS ESD AODFULL AODSLIM', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMTopoJets_subjesIS_fastftag',                        'BS ESD AODFULL', 'Jet', [Alias('JetContainerShallowCopy')]),
    ('xAOD::ShallowAuxContainer#HLT_AntiKt4EMTopoJets_subjesIS_fastftagAux.'+JetFastFTagVars, 'BS ESD AODFULL', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMTopoJets_nojcalib_ftf',                  'BS ESD AODFULL AODSLIM', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt4EMTopoJets_nojcalib_ftfAux.'+JetVars,   'BS ESD AODFULL AODSLIM', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMTopoJets_nojcalib_roiftf',                  'BS ESD', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt4EMTopoJets_nojcalib_roiftfAux.'+JetVars,   'BS ESD', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMTopoJets_subjesIS_ftf',                        '', 'Jet', [Alias('JetContainerShallowCopy')]),
    ('xAOD::ShallowAuxContainer#HLT_AntiKt4EMTopoJets_subjesIS_ftfAux.'+JetCopyVars, '', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMTopoJets_subjesIS_roiftf',                        'BS ESD', 'Jet', [Alias('JetContainerShallowCopy')]),
    ('xAOD::ShallowAuxContainer#HLT_AntiKt4EMTopoJets_subjesIS_roiftfAux.'+JetCopyVars, 'BS ESD', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMTopoJets_subjesgscIS_ftf',                        'BS ESD AODFULL', 'Jet', [Alias('JetContainerShallowCopy')]),
    ('xAOD::ShallowAuxContainer#HLT_AntiKt4EMTopoJets_subjesgscIS_ftfAux.'+JetCopyVars, 'BS ESD AODFULL', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMTopoJets_subjesgsc_ftf',                        'BS ESD AODFULL', 'Jet', [Alias('JetContainerShallowCopy')]),
    ('xAOD::ShallowAuxContainer#HLT_AntiKt4EMTopoJets_subjesgsc_ftfAux.'+JetCopyVars, 'BS ESD AODFULL', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMTopoJets_subresjesgscIS_ftf',                        'BS ESD AODFULL AODSLIM', 'Jet', [Alias('JetContainerShallowCopy')]),
    ('xAOD::ShallowAuxContainer#HLT_AntiKt4EMTopoJets_subresjesgscIS_ftfAux.'+JetCopyVars, 'BS ESD AODFULL AODSLIM', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMTopoJets_subresjesgsc_ftf',                        '', 'Jet', [Alias('JetContainerShallowCopy')]),
    ('xAOD::ShallowAuxContainer#HLT_AntiKt4EMTopoJets_subresjesgsc_ftfAux.'+JetCopyVars, '', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMTopoJets_subjes',                        '', 'Jet', [Alias('JetContainerShallowCopy')]),
    ('xAOD::ShallowAuxContainer#HLT_AntiKt4EMTopoJets_subjesAux.'+JetCopyVars, '', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMTopoJets_nojcalib',                      'BS ESD AODFULL AODSLIM', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt4EMTopoJets_nojcalibAux.'+JetVars,       'BS ESD AODFULL AODSLIM', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMTopoSKJets_nojcalib',                    '', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt4EMTopoSKJets_nojcalibAux.'+JetVars,     '', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMTopoCSSKJets_nojcalib',                  '', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt4EMTopoCSSKJets_nojcalibAux.'+JetVars,   '', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt10LCTopoJets_subjes',                        'BS ESD AODFULL', 'Jet', [Alias('JetContainerShallowCopy')]),
    ('xAOD::ShallowAuxContainer#HLT_AntiKt10LCTopoJets_subjesAux.'+JetCopyVars, 'BS ESD AODFULL', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt10LCTopoJets_nojcalib',                     'BS ESD AODFULL', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt10LCTopoJets_nojcalibAux.'+JetVars,      'BS ESD AODFULL', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt10EMTopoJets_nojcalib',                      'BS ESD AODFULL', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt10EMTopoJets_nojcalibAux.'+JetVars,       'BS ESD AODFULL', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt10EMTopoRCJets_subjesIS',                      'BS ESD AODFULL', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt10EMTopoRCJets_subjesISAux.'+JetVars,       'BS ESD AODFULL', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt10EMTopoRCJets_subjesIS_ftf',                      'BS ESD AODFULL', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt10EMTopoRCJets_subjesIS_ftfAux.'+JetVars,       'BS ESD AODFULL', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt10LCTopoTrimmedPtFrac4SmallR20Jets_jes',                'BS ESD AODFULL AODSLIM', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt10LCTopoTrimmedPtFrac4SmallR20Jets_jesAux.'+JetVars, 'BS ESD AODFULL AODSLIM', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt10LCTopoTrimmedPtFrac4SmallR20Jets_nojcalib',                'BS ESD', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt10LCTopoTrimmedPtFrac4SmallR20Jets_nojcalibAux.'+JetVars, 'BS ESD', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt10LCTopoSoftDropBeta100Zcut10Jets_nojcalib',                '', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt10LCTopoSoftDropBeta100Zcut10Jets_nojcalibAux.'+JetVars, '', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt10EMPFlowJets_nojcalib_ftf',                       '', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt10EMPFlowJets_nojcalib_ftfAux.'+JetVars,        '', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt10EMPFlowSoftDropBeta100Zcut10Jets_nojcalib_ftf',                '', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt10EMPFlowSoftDropBeta100Zcut10Jets_nojcalib_ftfAux.'+JetVars, '', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt10EMPFlowCSSKJets_nojcalib_ftf',                       'BS ESD', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt10EMPFlowCSSKJets_nojcalib_ftfAux.'+JetVars,        'BS ESD', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt10EMPFlowCSSKSoftDropBeta100Zcut10Jets_nojcalib_ftf',                'BS ESD AODFULL', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt10EMPFlowCSSKSoftDropBeta100Zcut10Jets_nojcalib_ftfAux.'+JetVars, 'BS ESD AODFULL', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt10EMPFlowCSSKSoftDropBeta100Zcut10Jets_jes_ftf',                'BS ESD AODFULL AODSLIM', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt10EMPFlowCSSKSoftDropBeta100Zcut10Jets_jes_ftfAux.'+LargeRJetVars, 'BS ESD AODFULL AODSLIM', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMPFlowJets_subjesIS_ftf',                        '', 'Jet', [Alias('JetContainerShallowCopy')]),
    ('xAOD::ShallowAuxContainer#HLT_AntiKt4EMPFlowJets_subjesIS_ftfAux.'+JetCopyVars, '', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMPFlowJets_subjesgscIS_ftf',                        'BS ESD AODFULL', 'Jet', [Alias('JetContainerShallowCopy')]),
    ('xAOD::ShallowAuxContainer#HLT_AntiKt4EMPFlowJets_subjesgscIS_ftfAux.'+JetCopyVars, 'BS ESD AODFULL', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMPFlowJets_subjesgsc_ftf',                        '', 'Jet', [Alias('JetContainerShallowCopy')]),
    ('xAOD::ShallowAuxContainer#HLT_AntiKt4EMPFlowJets_subjesgsc_ftfAux.'+JetCopyVars, '', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf',                        'BS ESD AODFULL AODSLIM', 'Jet', [Alias('JetContainerShallowCopy')]),
    ('xAOD::ShallowAuxContainer#HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftfAux.'+JetFastFTagVars, 'BS ESD AODFULL AODSLIM', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMPFlowJets_subresjesgsc_ftf',                        '', 'Jet', [Alias('JetContainerShallowCopy')]),
    ('xAOD::ShallowAuxContainer#HLT_AntiKt4EMPFlowJets_subresjesgsc_ftfAux.'+JetCopyVars, '', 'Jet'),
    
    ('xAOD::JetContainer#HLT_AntiKt4EMPFlowJets_jes_ftf',                        'BS ESD AODFULL', 'Jet', [Alias('JetContainerShallowCopy')]),
    ('xAOD::ShallowAuxContainer#HLT_AntiKt4EMPFlowJets_jes_ftfAux.'+JetCopyVars, 'BS ESD AODFULL', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMPFlowJets_nojcalib_ftf',                'BS ESD AODFULL AODSLIM', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt4EMPFlowJets_nojcalib_ftfAux.'+JetVars, 'BS ESD AODFULL AODSLIM', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMPFlowCSSKJets_nojcalib_ftf',                '', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt4EMPFlowCSSKJets_nojcalib_ftfAux.'+JetVars, '', 'Jet'),

    ## event info
    ('xAOD::TrigCompositeContainer#HLT_TCEventInfo_jet',                                 'BS ESD AODFULL', 'Jet' ),
    ('xAOD::TrigCompositeAuxContainer#HLT_TCEventInfo_jetAux.JetDensityEMPFlow.AvgMu.NumPV',         'BS ESD AODFULL', 'Jet'    ),

     # VR track jets
    ('xAOD::JetContainer#HLT_AntiKtVR30Rmax4Rmin02PV0TrackJets',                'BS ESD AODFULL AODSLIM', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKtVR30Rmax4Rmin02PV0TrackJetsAux.'+JetVars, 'BS ESD AODFULL AODSLIM', 'Jet'),

    # Heavy ion
    ('xAOD::JetContainer#HLT_AntiKt4HIJets',                      'BS ESD AODFULL', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt4HIJetsAux.'+HIJetVars,       'BS ESD AODFULL', 'Jet'),
    
    ('xAOD::JetContainer#HLT_AntiKt2HIJets',                      'BS ESD AODFULL', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt2HIJetsAux.'+HIJetVars,       'BS ESD AODFULL', 'Jet'),

    # TLA jets + PEB jets
    ('TrigRoiDescriptorCollection#HLT_Roi_DarkJetPEBTLA',             'BS ESD DarkJetPEBTLA',  'Jet'),
    ('TrigRoiDescriptorCollection#HLT_Roi_FTagPEBTLA',             'BS ESD FTagPEBTLA',  'Jet'),
    ('TrigRoiDescriptorCollection#HLT_Roi_EgammaPEBTLA',             'BS ESD EgammaPEBTLA',  'Jet'),

    ('xAOD::TrigCompositeContainer#HLT_TCEventInfo_TLA',                                 'BS PhysicsTLA DarkJetPEBTLA FTagPEBTLA ESD', 'Jet' ),
    ('xAOD::TrigCompositeAuxContainer#HLT_TCEventInfo_TLAAux.JetDensityEMPFlow.JetDensityEMTopo.AvgMu.NumPV',         'BS PhysicsTLA DarkJetPEBTLA FTagPEBTLA ESD', 'Jet'    ),

    ('xAOD::JetContainer#HLT_AntiKt4EMTopoJets_subjesIS_TLA',                      'BS PhysicsTLA DarkJetPEBTLA FTagPEBTLA ESD', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt4EMTopoJets_subjesIS_TLAAux.'+TLAJetVars,       'BS PhysicsTLA DarkJetPEBTLA FTagPEBTLA ESD', 'Jet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_TLA',                      'BS PhysicsTLA DarkJetPEBTLA FTagPEBTLA ESD', 'Jet'),
    ('xAOD::JetAuxContainer#HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_TLAAux.'+TLAJetVars,       'BS PhysicsTLA DarkJetPEBTLA FTagPEBTLA ESD', 'Jet'),


    # TLA Photons
    ('xAOD::PhotonContainer#HLT_egamma_Photons_TLA',                                           'BS PhysicsTLA DarkJetPEBTLA EgammaPEBTLA ESD', 'Egamma'),
    ('xAOD::PhotonAuxContainer#HLT_egamma_Photons_TLAAux.'+PhVars,                             'BS PhysicsTLA DarkJetPEBTLA EgammaPEBTLA ESD', 'Egamma'),

    # TLA Muons
    ('xAOD::MuonContainer#HLT_MuonsCB_RoI_TLA',                                       'BS PhysicsTLA DarkJetPEBTLA ESD', 'Muon'),
    ('xAOD::MuonAuxContainer#HLT_MuonsCB_RoI_TLAAux.',                                'BS PhysicsTLA DarkJetPEBTLA ESD', 'Muon'),
    
    # FS vertices
    ('xAOD::VertexContainer#HLT_IDVertex_FS',                  'BS PhysicsTLA ESD AODFULL AODSLIM', 'Jet'),
    ('xAOD::VertexAuxContainer#HLT_IDVertex_FSAux.',           'BS PhysicsTLA ESD AODFULL AODSLIM', 'Jet'),

    ('xAOD::VertexContainer#HLT_IDVertex_FSJet',                  'BS ESD AODFULL', 'Jet'),
    ('xAOD::VertexAuxContainer#HLT_IDVertex_FSJetAux.',           'BS ESD AODFULL', 'Jet'),



    #FSLRT
    ('xAOD::TrackParticleContainer#HLT_IDTrack_FSLRT_FTF',                  'BS ESD AODFULL', 'UTT'),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_FSLRT_FTFAux.',          'BS ESD AODFULL', 'UTT'),

    ('xAOD::VertexContainer#HLT_IDVertex_FSLRT',                  'BS ESD AODFULL', 'UTT'),
    ('xAOD::VertexAuxContainer#HLT_IDVertex_FSLRTAux.',          'BS ESD AODFULL', 'UTT'),

    ('xAOD::TrigCompositeContainer#HLT_FSLRT_TrackCount',                                             'BS ESD AODFULL AODSLIM', 'UTT'),
    ('xAOD::TrigCompositeAuxContainer#HLT_FSLRT_TrackCountAux.ntrks.pTcuts.z0cuts.counts',            'BS ESD AODFULL AODSLIM', 'UTT'),

    #FSLRT PT
    ('xAOD::TrackParticleContainer#HLT_IDTrack_FSLRT_IDTrig',                  'BS ESD AODFULL', 'UTT'),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_FSLRT_IDTrigAux.',          'BS ESD AODFULL', 'UTT'),

    #DisVtx
    # - LRT
    ('xAOD::TrackParticleContainer#HLT_IDTrack_DVLRT_FTF',          'BS ESD', 'UTT', [InViews('DVRoIViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_DVLRT_FTFAux.',   'BS ESD', 'UTT'),
    ('TrigRoiDescriptorCollection#HLT_Roi_DV',                      'BS ESD', 'UTT'),
    # - Vertex
    ('xAOD::VertexContainer#HLT_TrigDV_VSIVertex',                 'BS ESD AODFULL', 'UTT', [InViews('DVRoIViews')]),
    ('xAOD::VertexAuxContainer#HLT_TrigDV_VSIVertexAux.'+VSIVars,  'BS ESD AODFULL', 'UTT'),
    ('xAOD::VertexContainer#HLT_TrigDV_VSITrkPair',                'BS ESD AODFULL', 'UTT', [InViews('DVRoIViews')]),
    ('xAOD::VertexAuxContainer#HLT_TrigDV_VSITrkPairAux.'+VSIVars, 'BS ESD AODFULL', 'UTT'),
    # - Hypo
    ('xAOD::TrigCompositeContainer#HLT_TrigDV_VtxCount',           'BS ESD AODFULL', 'UTT'),
    ('xAOD::TrigCompositeAuxContainer#HLT_TrigDV_VtxCountAux.vsiHypo_nVtx.vsiHypo_pTcut.vsiHypo_rCut.vsiHypo_nTrkCut.vsiHypo_counts',  'BS ESD AODFULL', 'UTT'),

    #Fullscan TrigVSI
    ('xAOD::VertexContainer#HLT_TrigVSIVertex',                 'BS ESD AODFULL', 'UTT'),
    ('xAOD::VertexAuxContainer#HLT_TrigVSIVertexAux.'+VSIVars,  'BS ESD AODFULL', 'UTT'),
    ('xAOD::VertexContainer#HLT_TrigVSITrkPair',                'BS ESD AODFULL', 'UTT'),
    ('xAOD::VertexAuxContainer#HLT_TrigVSITrkPairAux.'+VSIVars, 'BS ESD AODFULL', 'UTT'),
    ('xAOD::TrigCompositeContainer#HLT_TrigVSI_VtxCount',       'BS ESD AODFULL', 'UTT'),
    ('xAOD::TrigCompositeAuxContainer#HLT_TrigVSI_VtxCountAux.vsiHypo_nVtx.vsiHypo_pTcut.vsiHypo_rCut.vsiHypo_nTrkCut.vsiHypo_counts',  'BS ESD AODFULL', 'UTT'),

    # HI event shape
    ('xAOD::HIEventShapeContainer#HLT_HIEventShapeEG',          'BS ESD AODFULL',   'Egamma'),
    ('xAOD::HIEventShapeAuxContainer#HLT_HIEventShapeEGAux.',   'BS ESD AODFULL',   'Egamma'),

    # HI event shape for jet
    ('xAOD::HIEventShapeContainer#HLTHIEventShapeWeighted',          'BS ESD AODFULL',   'Jet'),
    ('xAOD::HIEventShapeAuxContainer#HLTHIEventShapeWeightedAux.',   'BS ESD AODFULL',   'Jet'),
    ('xAOD::HIEventShapeContainer#HLTHIEventShapeWeighted_iter0',          'BS ESD AODFULL',   'Jet'),
    ('xAOD::HIEventShapeAuxContainer#HLTHIEventShapeWeighted_iter0Aux.',   'BS ESD AODFULL',   'Jet'),
    ('xAOD::HIEventShapeContainer#HLTHIEventShape_iter1',          'BS ESD AODFULL',   'Jet'),
    ('xAOD::HIEventShapeAuxContainer#HLTHIEventShape_iter1Aux.',   'BS ESD AODFULL',   'Jet'),


    # custom BeamSpot tracks - we don't want to write these out in general so this
    # is commented, if we want to write them out at some point, then these lines
    # should be uncommented and they should get written out
    #    ('xAOD::TrackParticleContainer#HLT_IDTrack_BeamSpot_FTF',         'BS ESD AODFULL', 'ID', [InViews('beamspotViewRoIs')] ),
    #    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_BeamSpot_FTFAux.',  'BS ESD AODFULL', 'ID', [InViews('beamspotViewRoIs')] ),

    # MET
    ('xAOD::TrigMissingETContainer#HLT_MET_cell',                               'BS ESD AODFULL AODSLIM', 'MET'),
    ('xAOD::TrigMissingETAuxContainer#HLT_MET_cellAux.',                        'BS ESD AODFULL AODSLIM', 'MET'),

    ('xAOD::TrigMissingETContainer#HLT_MET_mht',                           'BS ESD AODFULL AODSLIM', 'MET'),
    ('xAOD::TrigMissingETAuxContainer#HLT_MET_mhtAux.',                    'BS ESD AODFULL AODSLIM', 'MET'),

    ('xAOD::TrigMissingETContainer#HLT_MET_tcpufit',                       'BS ESD AODFULL AODSLIM', 'MET'),
    ('xAOD::TrigMissingETAuxContainer#HLT_MET_tcpufitAux.',                'BS ESD AODFULL AODSLIM', 'MET'),

    ('xAOD::TrigMissingETContainer#HLT_MET_tc',                            'BS ESD AODFULL AODSLIM', 'MET'),
    ('xAOD::TrigMissingETAuxContainer#HLT_MET_tcAux.',                     'BS ESD AODFULL AODSLIM', 'MET'),

    ('xAOD::TrigMissingETContainer#HLT_MET_tc_em',                            'BS ESD AODFULL AODSLIM', 'MET'),
    ('xAOD::TrigMissingETAuxContainer#HLT_MET_tc_emAux.',                     'BS ESD AODFULL AODSLIM', 'MET'),

    ('xAOD::TrigMissingETContainer#HLT_MET_trkmht',                        'BS ESD AODFULL AODSLIM', 'MET'),
    ('xAOD::TrigMissingETAuxContainer#HLT_MET_trkmhtAux.',                 'BS ESD AODFULL AODSLIM', 'MET'),

    ('xAOD::TrigMissingETContainer#HLT_MET_pfsum',                         'BS ESD AODFULL AODSLIM', 'MET'),
    ('xAOD::TrigMissingETAuxContainer#HLT_MET_pfsumAux.',                  'BS ESD AODFULL AODSLIM', 'MET'),

    ('xAOD::TrigMissingETContainer#HLT_MET_pfsum_cssk',                    'BS ESD AODFULL AODSLIM', 'MET'),
    ('xAOD::TrigMissingETAuxContainer#HLT_MET_pfsum_csskAux.',             'BS ESD AODFULL AODSLIM', 'MET'),

    ('xAOD::TrigMissingETContainer#HLT_MET_pfsum_vssk',                    'BS ESD AODFULL AODSLIM', 'MET'),
    ('xAOD::TrigMissingETAuxContainer#HLT_MET_pfsum_vsskAux.',             'BS ESD AODFULL AODSLIM', 'MET'),

    ('xAOD::TrigMissingETContainer#HLT_MET_pfopufit',                      'BS ESD AODFULL AODSLIM', 'MET'),
    ('xAOD::TrigMissingETAuxContainer#HLT_MET_pfopufitAux.',               'BS ESD AODFULL AODSLIM', 'MET'),

    ('xAOD::TrigMissingETContainer#HLT_MET_cvfpufit',                      'BS ESD AODFULL AODSLIM', 'MET'),
    ('xAOD::TrigMissingETAuxContainer#HLT_MET_cvfpufitAux.',               'BS ESD AODFULL AODSLIM', 'MET'),

    ('xAOD::TrigMissingETContainer#HLT_MET_mhtpufit_pf_subjesgscIS',       'BS ESD AODFULL', 'MET'),
    ('xAOD::TrigMissingETAuxContainer#HLT_MET_mhtpufit_pf_subjesgscISAux.', 'BS ESD AODFULL', 'MET'),

    ('xAOD::TrigMissingETContainer#HLT_MET_mhtpufit_em_subjesgscIS',       'BS ESD AODFULL', 'MET'),
    ('xAOD::TrigMissingETAuxContainer#HLT_MET_mhtpufit_em_subjesgscISAux.', 'BS ESD AODFULL', 'MET'),

    ('xAOD::TrigMissingETContainer#HLT_MET_mhtpufit_pf',       'BS ESD AODFULL AODSLIM', 'MET'),
    ('xAOD::TrigMissingETAuxContainer#HLT_MET_mhtpufit_pfAux.', 'BS ESD AODFULL AODSLIM', 'MET'),

    ('xAOD::TrigMissingETContainer#HLT_MET_mhtpufit_em',       'BS ESD AODFULL AODSLIM', 'MET'),
    ('xAOD::TrigMissingETAuxContainer#HLT_MET_mhtpufit_emAux.', 'BS ESD AODFULL AODSLIM', 'MET'),

    ('xAOD::TrigMissingETContainer#HLT_MET_nn',                'BS ESD AODFULL AODSLIM', 'MET'),
    ('xAOD::TrigMissingETAuxContainer#HLT_MET_nnAux.',          'BS ESD AODFULL AODSLIM', 'MET'),

    # ATR-25509 MET Containers needed to test nSigma=3
    ('xAOD::TrigMissingETContainer#HLT_MET_pfopufit_sig30',                'BS ESD AODFULL', 'MET'),
    ('xAOD::TrigMissingETAuxContainer#HLT_MET_pfopufit_sig30Aux.',         'BS ESD AODFULL', 'MET'),

    ('xAOD::TrigMissingETContainer#HLT_MET_tcpufit_sig30',                 'BS ESD AODFULL', 'MET'),
    ('xAOD::TrigMissingETAuxContainer#HLT_MET_tcpufit_sig30Aux.',          'BS ESD AODFULL', 'MET'),

    ('xAOD::CaloClusterContainer#HLT_TopoCaloClustersFS',                  'BS ESD', 'Calo'),
    ('xAOD::CaloClusterTrigAuxContainer#HLT_TopoCaloClustersFSAux.nCells', 'BS ESD', 'Calo'),

    # tau
    ('xAOD::TauJetContainer#HLT_TrigTauRecMerged_CaloMVAOnly',                      'BS ESD AODFULL',         'Tau', [InViews('tauCaloMVAViews')]),
    ('xAOD::TauJetAuxContainer#HLT_TrigTauRecMerged_CaloMVAOnlyAux.',               'BS ESD AODFULL',         'Tau'),

    ('xAOD::TauJetContainer#HLT_TrigTauRecMerged_MVA',                              'BS ESD AODFULL AODSLIM', 'Tau', [InViews('precMVATauViews'), InViews('tauPrecisionReco_MVAViews')]),
    ('xAOD::TauJetAuxContainer#HLT_TrigTauRecMerged_MVAAux.'+TauJet_vars_str,       'BS ESD AODFULL AODSLIM', 'Tau'),

    ('xAOD::TauJetContainer#HLT_TrigTauRecMerged_LLP',                              'BS ESD AODFULL AODSLIM', 'Tau', [InViews('precLLPTauViews'), InViews('tauPrecisionReco_LLPViews')]),
    ('xAOD::TauJetAuxContainer#HLT_TrigTauRecMerged_LLPAux.',                       'BS ESD AODFULL AODSLIM', 'Tau'),

    ('xAOD::TauJetContainer#HLT_TrigTauRecMerged_LRT',                              'BS ESD AODFULL AODSLIM', 'Tau', [InViews('precLRTTauViews'), InViews('tauPrecisionReco_LRTViews')]),
    ('xAOD::TauJetAuxContainer#HLT_TrigTauRecMerged_LRTAux.',                       'BS ESD AODFULL AODSLIM', 'Tau'),

    # tau tracks
    ('xAOD::TauTrackContainer#HLT_tautrack_MVA',                                'BS ESD AODFULL AODSLIM', 'Tau', [InViews('precMVATauViews'), InViews('tauPrecisionReco_MVAViews')]),
    ('xAOD::TauTrackAuxContainer#HLT_tautrack_MVAAux.'+TauTrack_vars_str,       'BS ESD AODFULL AODSLIM', 'Tau'),
    ('xAOD::TauTrackContainer#HLT_tautrack_LLP',                                'BS ESD AODFULL AODSLIM', 'Tau', [InViews('precLLPTauViews'), InViews('tauPrecisionReco_LLPViews')]),
    ('xAOD::TauTrackAuxContainer#HLT_tautrack_LLPAux.'+TauTrack_vars_str,       'BS ESD AODFULL AODSLIM', 'Tau'),
    ('xAOD::TauTrackContainer#HLT_tautrack_LRT',                                'BS ESD AODFULL AODSLIM', 'Tau', [InViews('precLRTTauViews'), InViews('tauPrecisionReco_LRTViews')]),
    ('xAOD::TauTrackAuxContainer#HLT_tautrack_LRTAux.'+TauTrack_vars_str,       'BS ESD AODFULL AODSLIM', 'Tau'),

    # bjet RoI Descriptor used for EventView creation
    ('TrigRoiDescriptorCollection#HLT_Roi_Bjet',                   'BS ESD AODFULL', 'Bjet'),



    # jet superRoI Descriptor and associated track and vertex class used for EventView creation
    ('TrigRoiDescriptorCollection#HLT_Roi_FS',                         'BS ESD AODFULL', 'Jet'),
    ('TrigRoiDescriptorCollection#HLT_Roi_JetSuper',                   'BS ESD AODFULL', 'Jet'),

    ('xAOD::TrackParticleContainer#HLT_IDTrack_JetSuper_FTF',          'BS ESD AODFULL', 'Jet', [InViews('JetSuperRoIViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_JetSuper_FTFAux.',   'BS ESD AODFULL', 'Jet'),

    ('xAOD::VertexContainer#HLT_IDVertex_JetSuper',                  'BS ESD AODFULL AODSLIM', 'Jet', [InViews('JetSuperRoIViews')]),
    ('xAOD::VertexAuxContainer#HLT_IDVertex_JetSuperAux.',           'BS ESD AODFULL AODSLIM', 'Jet'),




    # bjet Second Stage Fast tracks
    ('xAOD::TrackParticleContainer#HLT_IDTrack_Bjet_FTF',        'BS ESD AODFULL', 'Bjet', [InViews(BTagViewsEMTopo), InViews(BTagViewsEMPFlow)]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_Bjet_FTFAux.', 'BS ESD AODFULL', 'Bjet'),



    # bjet Second Stage Precision tracks
    ('xAOD::TrackParticleContainer#HLT_IDTrack_Bjet_IDTrig',        'BS ESD AODFULL', 'Bjet', [InViews(BTagViewsEMTopo), InViews(BTagViewsEMPFlow)]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_Bjet_IDTrigAux.btagIp_d0.btagIp_d0Uncertainty.btagIp_trackDisplacement.btagIp_trackMomentum.btagIp_z0SinTheta.btagIp_z0SinThetaUncertainty', 'BS ESD AODFULL', 'Bjet'),

    # FIXME: add vertex tracks

    # bjet jets

    ('xAOD::JetContainer#HLT_AntiKt4EMTopoJets_subresjesgscIS_ftf_bJets',                        'BS ESD AODFULL', 'Bjet', [InViews(BTagViewsEMTopo)]),
    ('xAOD::JetAuxContainer#HLT_AntiKt4EMTopoJets_subresjesgscIS_ftf_bJetsAux.'+BTagJetVars, 'BS ESD AODFULL', 'Bjet'),

    ('xAOD::JetContainer#HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_bJets',                        'BS ESD AODFULL AODSLIM', 'Bjet', [InViews(BTagViewsEMPFlow)]),
    ('xAOD::JetAuxContainer#HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_bJetsAux.'+BTagJetVars, 'BS ESD AODFULL AODSLIM', 'Bjet'),

    # secvertex for b-jets
    ('xAOD::VertexContainer#HLT_AntiKt4EMTopoJets_subresjesgscIS_ftf_BTaggingSecVtx',                          '', 'Bjet'),
    ('xAOD::VertexAuxContainer#HLT_AntiKt4EMTopoJets_subresjesgscIS_ftf_BTaggingSecVtxAux.',                   '', 'Bjet'),

    ('xAOD::VertexContainer#HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_BTaggingSecVtx',                          '', 'Bjet'),
    ('xAOD::VertexAuxContainer#HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_BTaggingSecVtxAux.',                   '', 'Bjet'),

    # btagvertex for b-jets
    ('xAOD::BTagVertexContainer#HLT_AntiKt4EMTopoJets_subresjesgscIS_ftf_BTaggingJFVtx',                          '', 'Bjet'),
    ('xAOD::BTagVertexAuxContainer#HLT_AntiKt4EMTopoJets_subresjesgscIS_ftf_BTaggingJFVtxAux.',                   '', 'Bjet'),

    ('xAOD::BTagVertexContainer#HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_BTaggingJFVtx',                          '', 'Bjet'),
    ('xAOD::BTagVertexAuxContainer#HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_BTaggingJFVtxAux.',                   '', 'Bjet'),

    # bjet b-tagging
    ('xAOD::BTaggingContainer#HLT_AntiKt4EMTopoJets_subresjesgscIS_ftf_BTagging',                          'BS ESD AODFULL', 'Bjet', [InViews(BTagViewsEMTopo)]),
    ('xAOD::BTaggingAuxContainer#HLT_AntiKt4EMTopoJets_subresjesgscIS_ftf_BTaggingAux.'+BTagVars,          'BS ESD AODFULL', 'Bjet'),

    ('xAOD::BTaggingContainer#HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_BTagging',                          'BS ESD AODFULL AODSLIM', 'Bjet', [InViews(BTagViewsEMPFlow)]),
    ('xAOD::BTaggingAuxContainer#HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_BTaggingAux.'+BTagVars,          'BS ESD AODFULL AODSLIM', 'Bjet'),

    # TLA bjet b-tagging
    ('xAOD::BTaggingContainer#HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_TLA_BTagging',                      'BS PhysicsTLA ESD', 'Bjet'),
    ('xAOD::BTaggingAuxContainer#HLT_AntiKt4EMPFlowJets_subresjesgscIS_ftf_TLA_BTaggingAux.'+BTagVars,       'BS PhysicsTLA ESD', 'Bjet'),

    # MinBias

    ('xAOD::TrackParticleContainer#HLT_IDTrack_MinBias_IDTrig',                 'BS ESD AODFULL AODSLIM', 'MinBias', [InViews('MBTrackRecoViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_MinBias_IDTrigAux.',          'BS ESD AODFULL AODSLIM', 'MinBias'),

    ('xAOD::TrackParticleContainer#HLT_IDTrack_MinBiasPixel_IDTrig',                 'BS ESD AODFULL', 'MinBias', [InViews('MBPixelTrackRecoViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_MinBiasPixel_IDTrigAux.',          'BS ESD AODFULL', 'MinBias'),

    ('xAOD::TrigT2MbtsBitsContainer#HLT_MbtsBitsContainer',                     'BS ESD AODFULL', 'MinBias'),
    ('xAOD::TrigT2MbtsBitsAuxContainer#HLT_MbtsBitsContainerAux.',              'BS ESD AODFULL', 'MinBias'),

    ('xAOD::TrigCompositeContainer#HLT_SpacePointCounts',            'BS ESD AODFULL AODSLIM', 'MinBias', [InViews('SPCountingRecoViews')]),
    ('xAOD::TrigCompositeAuxContainer#HLT_SpacePointCountsAux.pixCL.pixCL_1.pixCL_2.pixCLmin3.pixCLBarrel.pixCLEndcapA.pixCLEndcapC.pixCLnoToT.sctSP.sctSPBarrel.sctSPEndcapA.sctSPEndcapC.pixQ2mod',     'BS ESD AODFULL AODSLIM', 'MinBias'),

    ('xAOD::TrigCompositeContainer#HLT_TrackCount',                                                'BS ESD AODFULL AODSLIM', 'MinBias'),
    ('xAOD::TrigCompositeAuxContainer#HLT_TrackCountAux.ntrks.pTcuts.z0cuts.vertexZcuts.counts',  'BS ESD AODFULL AODSLIM', 'MinBias'),

    ('xAOD::TrigCompositeContainer#HLT_PixelTrackCount',                                                'BS ESD AODFULL', 'MinBias'),
    ('xAOD::TrigCompositeAuxContainer#HLT_PixelTrackCountAux.ntrks.pTcuts.z0cuts.vertexZcuts.counts',  'BS ESD AODFULL', 'MinBias'),

    ('xAOD::TrigCompositeContainer#HLT_vtx_z',                              'BS ESD AODFULL AODSLIM', 'MinBias' ,[InViews('ZVertFinderRecoViews')]),
    ('xAOD::TrigCompositeAuxContainer#HLT_vtx_zAux.zfinder_vtx_z.zfinder_vtx_weight.zfinder_tool', 'BS ESD AODFULL AODSLIM', 'MinBias'),

    ('xAOD::AFPSiHitsClusterContainer#HLT_AFPSiHitsClusterContainer',         'BS ESD AODFULL AODSLIM', 'MinBias'),
    ('xAOD::AFPSiHitsClusterAuxContainer#HLT_AFPSiHitsClusterContainerAux.',  'BS ESD AODFULL AODSLIM', 'MinBias'),
    ('xAOD::AFPTrackContainer#HLT_AFPTrackContainer',                         'BS ESD AODFULL AODSLIM', 'MinBias'),
    ('xAOD::AFPTrackAuxContainer#HLT_AFPTrackContainerAux.',                  'BS ESD AODFULL AODSLIM', 'MinBias'),
    ('xAOD::AFPToFTrackContainer#HLT_AFPToFTrackContainer',                   'BS ESD AODFULL AODSLIM', 'MinBias'),
    ('xAOD::AFPToFTrackAuxContainer#HLT_AFPToFTrackContainerAux.',            'BS ESD AODFULL AODSLIM', 'MinBias'),
    ('xAOD::AFPProtonContainer#HLT_AFPProtonContainer',                       'BS ESD AODFULL AODSLIM', 'MinBias'),
    ('xAOD::AFPProtonAuxContainer#HLT_AFPProtonContainerAux.',                'BS ESD AODFULL AODSLIM', 'MinBias'),
    ('xAOD::AFPVertexContainer#HLT_AFPVertexContainer',                       'BS ESD AODFULL AODSLIM', 'MinBias'),
    ('xAOD::AFPVertexAuxContainer#HLT_AFPVertexContainerAux.',                'BS ESD AODFULL AODSLIM', 'MinBias'),

    # Cosmic
    ('xAOD::TrackParticleContainer#HLT_IDTrack_Cosmic_FTF',                   'BS ESD AODFULL', 'Cosmic'),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_Cosmic_FTFAux.',            'BS ESD AODFULL', 'Cosmic'),

    #FullScan version from offline patern recognition (these will potentially be removed,, as the ones from the precision tracking might be sufficient)
    ('xAOD::TrackParticleContainer#HLT_IDTrack_Cosmic_EFID',                 'BS ESD AODFULL', 'Cosmic'),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_Cosmic_EFIDAux.',          'BS ESD AODFULL', 'Cosmic'),
    #And their respective precision tracks
    ('xAOD::TrackParticleContainer#HLT_IDTrack_Cosmic_IDTrig',               'BS ESD AODFULL', 'Cosmic', [InViews('CosmicViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_Cosmic_IDTrigAux.',        'BS ESD AODFULL', 'Cosmic'),

    # UTT
    # hit-based displaced vertex
    ('xAOD::TrigCompositeContainer#HLT_HitDV',                             'BS ESD AODFULL AODSLIM', 'UTT'),
    ('xAOD::TrigCompositeAuxContainer#HLT_HitDVAux.'+hitDVVars,            'BS ESD AODFULL AODSLIM', 'UTT'),

    # dE/dx
    ('xAOD::TrigCompositeContainer#HLT_dEdxTrk',                           'BS ESD AODFULL', 'UTT'),
    ('xAOD::TrigCompositeAuxContainer#HLT_dEdxTrkAux.'+dEdxTrkVars,        'BS ESD AODFULL', 'UTT'),
    ('xAOD::TrigCompositeContainer#HLT_dEdxHit',                           'BS ESD', 'UTT'),
    ('xAOD::TrigCompositeAuxContainer#HLT_dEdxHitAux.'+dEdxHitVars,        'BS ESD', 'UTT'),
    ('xAOD::TrigCompositeContainer#HLT_HPtdEdxTrk',                        'BS ESD AODFULL AODSLIM', 'UTT'),
    ('xAOD::TrigCompositeAuxContainer#HLT_HPtdEdxTrkAux.'+HPtdEdxTrkVars,  'BS ESD AODFULL AODSLIM', 'UTT'),

    # disappearing track
    ('xAOD::TrigCompositeContainer#HLT_DisTrkCand',                           'BS ESD AODFULL', 'UTT'),
    ('xAOD::TrigCompositeAuxContainer#HLT_DisTrkCandAux.'+DisTrkCandVars,     'BS ESD AODFULL', 'UTT'),
    ('xAOD::TrigCompositeContainer#HLT_DisTrkBDTSel',                         'BS ESD AODFULL AODSLIM', 'UTT'),
    ('xAOD::TrigCompositeAuxContainer#HLT_DisTrkBDTSelAux.'+DisTrkBDTSelVars, 'BS ESD AODFULL AODSLIM', 'UTT'),

    #
    ('xAOD::TrigCompositeContainer#HLTNav_R2ToR3Summary',   'ESD AODFULL AODSLIM AODBLSSLIM', 'Steer'),
    ('xAOD::TrigCompositeAuxContainer#HLTNav_R2ToR3SummaryAux.',   'ESD AODFULL AODSLIM AODBLSSLIM', 'Steer'),

    #displaced jet
    ('TrigRoiDescriptorCollection#HLT_Roi_DJ',                   'BS ESD AODFULL', 'Jet'),
    ('xAOD::TrackParticleContainer#HLT_IDTrack_DJLRT_FTF',          'BS ESD AODFULL', 'Jet', [InViews('DJRoIViews')]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_DJLRT_FTFAux.',   'BS ESD AODFULL', 'Jet'),

    # muon roi clusters
    ('xAOD::TrigCompositeContainer#HLT_MuRoICluster_Composites',                      'BS ESD AODFULL AODSLIM', 'Muon'),
    ('xAOD::TrigCompositeAuxContainer#HLT_MuRoICluster_CompositesAux.'+MuRoiVars,     'BS ESD AODFULL AODSLIM', 'Muon'),

    # Anomaly Detection Trigger
    ('xAOD::TrigCompositeContainer#HLT_AnomDet_ComboHypo', 'BS ESD AODFULL AODSLIM', 'AnomDet'),
    ('xAOD::TrigCompositeAuxContainer#HLT_AnomDet_ComboHypoAux.adScore', 'BS ESD AODFULL AODSLIM', 'AnomDet'),

    # tau calo clusters (moved as last because of the truncation errors ATR-29142)
    ('xAOD::CaloClusterContainer#HLT_TopoCaloClustersLC',                             'BS ESD AODFULL', 'Tau', [InViews('tauCaloMVAViews'), allowTruncation]),   
    ('xAOD::CaloClusterTrigAuxContainer#HLT_TopoCaloClustersLCAux.nCells.CENTER_MAG', 'BS ESD AODFULL', 'Tau', [allowTruncation]),
 
    # FS tracks (moved as last because of the truncation errors ATR-29142)
    ('xAOD::TrackParticleContainer#HLT_IDTrack_FS_FTF',                 'BS PhysicsTLA ESD AODFULL', 'Jet', [allowTruncation]),
    ('xAOD::TrackParticleAuxContainer#HLT_IDTrack_FS_FTFAux.passPFTrackPresel.muonCaloTag.muonScore.ptCone20.etConeCore.trackIso.RErr.EOverP.caloIso.trkPtFraction.tagFakeTrack.tagMuonTrack.tagIsoTrack',          'BS PhysicsTLA ESD AODFULL', 'Jet', [allowTruncation]),
    
]



#-------------------------------------------------------------------------------
# EDM details list to store the transient-persistent version
#-------------------------------------------------------------------------------
EDMDetailsRun3 = {}

# xAOD versions are auto-detected at serialisation, but T/P classes need to specify P version here
EDMDetailsRun3[ "TrigRoiDescriptorCollection" ]     = {'persistent':"TrigRoiDescriptorCollection_p3"}
EDMDetailsRun3[ "Muon::NSW_TrigRawDataContainer" ]  = {'persistent':"Muon::NSW_TrigRawDataContainer_p1"}

EDMDetailsRun3[ "xAOD::TrigDecisionAuxInfo" ]         = {'parent':"xAOD::TrigDecision"}
EDMDetailsRun3[ "xAOD::EnergySumRoIAuxInfo" ]         = {'parent':"xAOD::EnergySumRoI"}
EDMDetailsRun3[ "xAOD::JetEtRoIAuxInfo" ]             = {'parent':"xAOD::JetEtRoI"}
EDMDetailsRun3[ "xAOD::CaloClusterTrigAuxContainer" ] = {'parent':"xAOD::CaloClusterContainer"}


#-------------------------------------------------------------------------------
# Helper functions
#-------------------------------------------------------------------------------
def persistent( transient ):
    """
    Persistent EDM class, for xAOD it is the actual class version

    Uses list defined above. If absent assumes v1
    """
    if transient in EDMDetailsRun3:
        if 'persistent' in EDMDetailsRun3[transient]:
            return EDMDetailsRun3[transient]['persistent']
    return transient


def tpMap():
    """
    List
    """
    l = {}
    for tr in EDMDetailsRun3.keys():
        if "xAOD" in tr:
            continue
        l[tr] = persistent(tr)
    return l

# Fn added as simply appending new HLT Nav entries to start of list
# during runtime appears to cause Deserialisation issues (ATR-29945).
# Appending new entries towards end is safer.
def getSafeEDMInsertPosition(edm_list):
  """
  Need to avoid inserting new runtime EDM entries at end of list
  as 'allowTruncation' items need to be at end of list for correct
  serialisation.
  Function returns one position before
  first 'allowTruncation' EDM item. If there is no 'allowTruncation'
  EDM item, the position returned is the length of the list, i.e. the next
  index in the list.
  """

  insert_idx = len(edm_list)

  # start at end of list
  for i,entry in enumerate(reversed(edm_list)):
      if len(entry) < 4 or allowTruncation not in entry[3]:
          insert_idx -= i
          return insert_idx
  return insert_idx


def addHLTNavigationToEDMList(flags, edmList, allDecisions, hypoDecisions):
    """
    Extend TriggerHLTListRun3 with HLT Navigation objects
    """

    # HLTNav_* object list is built dynamically during job configuration, here we only define its output targets
    HLTNavEDMTargets = ''

    insert_idx = getSafeEDMInsertPosition(edmList)

    if not flags.Trigger.doOnlineNavigationCompactification:
        # If we are not compacting the online EDM, then we must write out all of the individual collections
        # ESD is added for MC support
        HLTNavEDMTargets = 'BS ESD'

    for decisionCollection in allDecisions:
        dynamic = '.-' # Exclude dynamic
        if decisionCollection in hypoDecisions:
            # Include dynamic
            dynamic = '.remap_linkColIndices.remap_linkColKeys'
            if 'PEBInfoWriter' in decisionCollection:
                dynamic += '.PEBROBList.PEBSubDetList'
        typeName = 'xAOD::TrigCompositeContainer#{:s}'.format(decisionCollection)
        typeNameAux = 'xAOD::TrigCompositeAuxContainer#{:s}Aux{:s}'.format(decisionCollection, dynamic)

        # Cost monitoring only requires a sub-set of the navigation collections.
        # (And CANNOT use any slimmed/merged collection, as the container names are important)
        thisCollectionHLTNavEDMTargets = HLTNavEDMTargets
        if decisionCollection.startswith("HLTNav_FStep") or decisionCollection == "HLTNav_Summary" or decisionCollection.startswith("HLTNav_L1"):
            thisCollectionHLTNavEDMTargets += ' CostMonDS'

        edmList[insert_idx:insert_idx] = [
            (typeName,    thisCollectionHLTNavEDMTargets, 'Steer'),
            (typeNameAux, thisCollectionHLTNavEDMTargets, 'Steer')]




