"""
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

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
    return "BTagging_" + jetcol.split('Jets')[0]

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

def _getVarsXbb(name, extra_flavours=None):
    """Convenience function for getting output variable names, here for Xbb tagging (typically GN2X)"""
    if extra_flavours is None:
        extra_flavours = []
    flavors = ["hbb", "hcc", "top", "qcd"] + extra_flavours
    return [f'{name}_p{f}' for f in flavors]

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
    "jetRank",
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

# standard largeR jets truth outputs 
LargeRJetStandardAux = [
    "R10TruthLabel_R22v1",
    "R10TruthLabel_R22v1_TruthJetMass",
    "R10TruthLabel_R22v1_TruthJetPt"
]

# standard outputs for Run 3
BTaggingRun3Aux = ["SV1_NGTinSvx", "SV1_masssvx",]
BTaggingRun3Aux += _getVars("DL1dv01", flip_modes=['Flip']) # 202 r22 pre-rec tagger
BTaggingRun3Aux += _getVars("GN2v01", extra_flavours=['tau'], flip_modes=['SimpleFlip']) # planned GN2 tagger for 2024 recommendations

# Xbb taggers outputs 
BTaggingLargeRAux = []
# Note that GN2Xv01 is buggy and should only be used in very specialized cases, see
# https://its.cern.ch/jira/browse/AFT-794 for the details of the only suppored uses
BTaggingLargeRAux += _getVarsXbb("GN2Xv01")
# Note that GN2Xv02 isn't supported, and is only kept around for the sake of one analysis
# https://atlas-glance.cern.ch/atlas/analysis/analyses/details?ref_code=ANA-HIGP-2024-01
# please contact the analsyis contacts before removing (but please remove at some point)
BTaggingLargeRAux += _getVarsXbb("GN2Xv02")
BTaggingLargeRAux += _getVarsXbb("GN2XTauV00", extra_flavours=['htautauhad'])
BTaggingLargeRAux += [f'GN3XV00_p{x}' for x in ["htautauhad", "hbb", "hcc", "top", "qcdbb", "qcdbx", "qcdcx", "qcdll", "Wqq"]]

# standard outputs for Run 4
BTaggingRun4Aux = [
    "SV1_NGTinSvx",
    "SV1_masssvx",
    "GN2HL_pu",
    "GN2HL_pc",
    "GN2HL_pb",
    "GN2HL_ptau"
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


def BTaggingLargeRContent(jetcol, ConfigFlags = None):
    jetcontent = _getVariableList(jetcol, LargeRJetStandardAux)
    # b-tagging variables
    aux = BTaggingLargeRAux
    btagcontent = _getVariableList(jetcol, aux)
    return jetcontent + btagcontent

def BTaggingVRContent(jetcol, ConfigFlags = None):
    aux = JetStandardAux + [
        "SV1_NGTinSvx", 
        "SV1_masssvx",
        "SV1_TrackParticleLinks"
    ]
    jetcontent = _getVariableList(jetcol, aux)
    return jetcontent



def BTagginglessContent(jetcol, ConfigFlags=None):
    # GN2v01 was the recommended tagger as of 30-06-2025
    BTaggingRun3AuxVar = _getVars("GN2v01", extra_flavours=['tau'], flip_modes=['SimpleFlip'])
    BTaggingRun3AuxVar += ["SV1_NGTinSvx", "SV1_masssvx",]

    # GN3 models were experimental as of 30-06-2025
    # they are saved to phys for the sake of
    # https://its.cern.ch/jira/browse/AFT-779
    gn3v00_models = [
        "GN3V00",
        "GN3PflowV00",
        "GN3MuonsV00",
        "GN3PflowMuonsV00"
    ]
    for gn3_dev in gn3v00_models:
        extra_flavours = ["tau",]
        if gn3_dev in {"GN3PflowMuonsV00"}:
            extra_flavours = ["tau", "ud", "g", "s", "quark"]
            BTaggingRun3AuxVar += [f"{gn3_dev}_ptFromTruthDressedWZJet"]
        BTaggingRun3AuxVar += _getVars(gn3_dev, extra_flavours=extra_flavours, flip_modes=["SimpleFlip"])

    isRun4 = _isRun4(ConfigFlags)
    aux = BTaggingRun3AuxVar if not isRun4 else []
    btagcontent = _getVariableList(jetcol, aux)
    return btagcontent
