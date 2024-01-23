"""
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

Define sets of standard variables to save in output files.
The variable lists returned by these functions are used by the smart slimming
service to determine which variables to save in derivations.
"""

from AthenaConfiguration.Enums import LHCPeriod


# ---------------------------------------------------------------------
# Convenience functions
# ---------------------------------------------------------------------
def _getBtagging(jetcol):
    """Convenience function for getting btagging names"""
    btaggingtmp = "BTagging_" + jetcol.split('Jets')[0]
    if 'BTagging' in jetcol:
         stamp = jetcol.split('BTagging')[1]
         btaggingtmp += '_'+stamp
    # deal with name mismatch between PV0TrackJets and BTagging_Track
    btagging = btaggingtmp.replace("PV0Track", "Track")
    return btagging

def _isRun4(ConfigFlags):
    """Convenience function for checking if we are in Run4"""
    return ConfigFlags is not None and ConfigFlags.GeoModel.Run >= LHCPeriod.Run4

def _getVariableList(collection, aux_list):
    """Convenience function for getting variable list"""
    return [collection] + [".".join( [ collection + "Aux" ] + aux_list )]

def _getVars(name, extra_flavours=None, flip_modes=None):
    """Convenience function for getting output variable names"""
    if extra_flavours is None:
        extra_flavours = []
    if flip_modes is None:
        flip_modes = [""]
    flavors = list("cub") + extra_flavours
    variants = [""] + flip_modes
    return [f'{name}{v}_p{f}' for v in variants for f in flavors]

def _getTruthVars():
    vals = ['ID', 'Pt', 'Lxy', 'DR', 'PdgId', 'Barcode']
    algs = ['HadronConeExcl', 'HadronGhost']
    base = [f'{a}TruthLabel{v}' for v in vals for a in algs + ['PartonTruthLabel'] ]
    extended = [f'{a}ExtendedTruthLabelID' for a in algs]
    return base + extended

# ---------------------------------------------------------------------
# Variable lists
# ---------------------------------------------------------------------
# some jet variables we always want to save
fold_hashes = ['jetFoldHash', 'jetFoldHash_noHits']
JetStandardAux = fold_hashes + [
    "pt",
    "eta",
    "btaggingLink",
    "GhostTrack",
    "ConeExclBHadronsFinal",
    "ConeExclCHadronsFinal",
    "PartonTruthLabelID",
    *_getTruthVars(),
]

JetExtendedAux = [
    "GhostBHadronsFinalCount",
    "GhostBHadronsFinalPt",
    "GhostCHadronsFinalCount",
    "GhostCHadronsFinalPt",
    "GhostTausFinalCount",
    "GhostTausFinalPt",
    "PartonTruthLabelEnergy",
]

# standard outputs for Run 3
BTaggingRun3Aux = ["SV1_NGTinSvx", "SV1_masssvx",]
BTaggingRun3Aux += _getVars("DL1dv01", flip_modes=['Flip']) # 202 r22 pre-rec tagger
BTaggingRun3Aux += _getVars("GN2v00", flip_modes=['Simple']) # preliminary GN2 tagger
BTaggingRun3Aux += _getVars("GN2v01", extra_flavours=['tau'], flip_modes=['Simple']) # planned GN2 tagger for 2024 recommendations

# standard outputs for Run 4
BTaggingRun4Aux = [
    "SV1_NGTinSvx",
    "SV1_masssvx",
    "dipsrun420221008_pu",
    "dipsrun420221008_pc",
    "dipsrun420221008_pb",
    "DL1drun420221017_pu",
    "DL1drun420221017_pc",
    "DL1drun420221017_pb",
    "GN1run420221010_pu",
    "GN1run420221010_pc",
    "GN1run420221010_pb"
]

# more involved outputs we might not want to save (ExpertContent)
BTaggingHighLevelAux = [
    "softMuon_dR",
    "softMuon_pTrel",
    "softMuon_scatteringNeighbourSignificance",
    "softMuon_momentumBalanceSignificance",
    "softMuon_qOverPratio",
    "softMuon_ip3dD0",
    "softMuon_ip3dD0Significance",
    "softMuon_ip3dZ0",
    "softMuon_ip3dZ0Significance",
    "JetFitter_mass",
    "JetFitter_isDefaults",
    "JetFitter_energyFraction",
    "JetFitter_significance3d",
    "JetFitter_nVTX",
    "JetFitter_nSingleTracks",
    "JetFitter_nTracksAtVtx",
    "JetFitter_N2Tpair",
    "JetFitter_deltaR",
    "SV1_isDefaults",
    "SV1_N2Tpair",
    "SV1_efracsvx",
    "SV1_deltaR",
    "SV1_Lxy",
    "SV1_L3d",
    "SV1_significance3d",
    "IP3D_bu",
    "IP3D_isDefaults",
    "IP3D_bc",
    "IP3D_cu",
    "JetFitterSecondaryVertex_nTracks",
    "JetFitterSecondaryVertex_isDefaults",
    "JetFitterSecondaryVertex_mass",
    "JetFitterSecondaryVertex_energy",
    "JetFitterSecondaryVertex_energyFraction",
    "JetFitterSecondaryVertex_displacement3d",
    "JetFitterSecondaryVertex_displacement2d",
    "JetFitterSecondaryVertex_maximumTrackRelativeEta",
    "JetFitterSecondaryVertex_minimumTrackRelativeEta",
    "JetFitterSecondaryVertex_averageTrackRelativeEta",
    "JetFitterDMeson_mass",
    "JetFitterDMeson_isDefaults",
    "maximumTrackRelativeEta",
    "minimumTrackRelativeEta",
    "averageTrackRelativeEta",
    "softMuon_pb",
    "softMuon_pc",
    "softMuon_pu",
    "softMuon_isDefaults",
    "BTagTrackToJetAssociator"
]

# ---------------------------------------------------------------------
# Functions which define smart slimming content for different use cases
# ---------------------------------------------------------------------
def BTaggingExpertContent(jetcol, ConfigFlags = None):
    btagging = _getBtagging(jetcol)

    # jet variables
    jetcontent = _getVariableList(jetcol, JetStandardAux + JetExtendedAux)

    # b-tagging variables
    isRun4 = _isRun4(ConfigFlags)
    aux = BTaggingRun4Aux if isRun4 else BTaggingRun3Aux
    aux += BTaggingHighLevelAux
    btagcontent = _getVariableList(btagging, aux)

    return jetcontent + btagcontent


def BTaggingStandardContent(jetcol, ConfigFlags = None):
    btagging = _getBtagging(jetcol)

    # jet variables
    jetcontent = _getVariableList(jetcol, JetStandardAux)

    # b-tagging variables
    isRun4 = _isRun4(ConfigFlags)
    aux = BTaggingRun4Aux if isRun4 else BTaggingRun3Aux
    btagcontent = _getVariableList(btagging, aux)

    return jetcontent + btagcontent


def BTaggingXbbContent(jetcol, ConfigFlags = None):
    btagging = _getBtagging(jetcol)

    # jet variables
    jetAllAux = JetStandardAux + JetExtendedAux
    jetcontent = _getVariableList(jetcol, jetAllAux)

    # b-tagging variables
    isRun4 = _isRun4(ConfigFlags)
    aux = BTaggingRun4Aux if isRun4 else BTaggingRun3Aux
    aux += BTaggingHighLevelAux
    btagcontent = _getVariableList(btagging, aux)

    return jetcontent + btagcontent
