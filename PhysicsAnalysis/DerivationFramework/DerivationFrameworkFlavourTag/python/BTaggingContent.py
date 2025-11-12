"""
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

Define sets of standard variables to save in output files.
The variable lists returned by these functions are used by the smart slimming
service to determine which variables to save in derivations.
"""
from AthenaConfiguration.Enums import LHCPeriod

def _isRun4(flags):
    """Convenience function for checking if we are in Run4"""
    return flags is not None and flags.GeoModel.Run >= LHCPeriod.Run4

def _getVariableList(collection, aux_list):
    """Convenience function for getting variable list"""
    return [collection] + [".".join( [ collection + "Aux" ] + aux_list )]

def _getSmallRDiscriminantVars(name, extra_flavours=None, flip_modes=None):
    """Convenience function for getting output variable names"""
    if extra_flavours is None:
        extra_flavours = []
    if flip_modes is None:
        flip_modes = [""]
    flavors = list("cub") + extra_flavours
    variants = [""] + flip_modes
    return [f'{name}{v}_p{f}' for v in variants for f in flavors]

def _getLargeRDiscriminantVars(name, extra_flavours=None):
    """Convenience function for getting output variable names, here for Xbb tagging (typically GN2X)"""
    if extra_flavours is None:
        extra_flavours = []
    flavors = ["hbb", "hcc", "top", "qcd"] + extra_flavours
    return [f'{name}_p{f}' for f in flavors]

def _getStandardSmallRVars():
    vals = ['ID', 'Pt', 'Lxy', 'DR', 'PdgId', 'Barcode']
    algs = ['HadronConeExcl', 'HadronGhost']
    base = [f'{a}TruthLabel{v}' for v in vals for a in algs + ['PartonTruthLabel'] ]
    extended = [f'{a}ExtendedTruthLabelID' for a in algs]
    truth_vars = base + extended
    reco_vars = [
        # FoldHash 
        'jetFoldHash', 
        'jetFoldHash_noHits',
        # basic kinematics
        "pt",
        "eta",
        "GhostTrack",
        "jetRank",
        "ConeExclBHadronsFinal",
        "ConeExclCHadronsFinal",
        "PartonTruthLabelID",
    ]
    return truth_vars + reco_vars


def BTaggingLargeRContent(flags, jetcol):
    # large-R jet truth variables
    LargeRJetTruthAux = [
        "R10TruthLabel_R22v1",
        "R10TruthLabel_R22v1_TruthJetMass",
        "R10TruthLabel_R22v1_TruthJetPt"
    ]
    jetContent = _getVariableList(jetcol, LargeRJetTruthAux)

    XBBAuxVar = []
    XBBAuxVar += _getLargeRDiscriminantVars('GN2Xv01',      extra_flavours=[])
    XBBAuxVar += _getLargeRDiscriminantVars('GN2Xv02',      extra_flavours=[])
    XBBAuxVar += _getLargeRDiscriminantVars('GN2XTauV00',   extra_flavours=['htautauhad'])
    XBBAuxVar += _getLargeRDiscriminantVars('GN3XPV01',     extra_flavours=['htautauhad', "qcdbb", "qcdbx", "qcdcx", "qcdll", "Wqq"])
    btagContent = _getVariableList(jetcol, XBBAuxVar)
    return jetContent + btagContent

def BTaggingVRContent(flags, jetcol):
    aux = _getStandardSmallRVars() + [ "SV1_NGTinSvx",  "SV1_masssvx", "SV1_TrackParticleLinks" ]
    jetcontent = _getVariableList(jetcol, aux)
    return jetcontent

def BTaggingStandardContent(flags, jetcol):

    # basic jet variables
    jetBasicContent = _getVariableList(jetcol, _getStandardSmallRVars())
    # b-tagging variables
    if not _isRun4(flags):
        # standard outputs for Run 3
        BTaggingRun3Aux = []
        BTaggingRun3Aux += _getSmallRDiscriminantVars(
            'GN2v01',           
            extra_flavours=['tau'], 
            flip_modes=['SimpleFlip']
        )
        BTaggingRun3Aux += _getSmallRDiscriminantVars(
            'GN3V00',           
            extra_flavours=['tau'], 
            flip_modes=['SimpleFlip']
        )
        BTaggingRun3Aux += _getSmallRDiscriminantVars(
            'GN3PflowMuonsV00', 
            extra_flavours=['tau', 'ud', 'g', 's', 'quark'], 
            flip_modes=['SimpleFlip']
        )
        BTaggingRun3Aux += _getSmallRDiscriminantVars(
            'GN3EPCLV01',
            extra_flavours=['tau', 'ud', 'g', 's', 'bquark', 'antibquark', 'cquark', 'anticquark', 'other'],
            flip_modes=['SimpleFlip']
        )
        BTaggingRun3Aux += ["SV1_NGTinSvx", "SV1_masssvx"] # GN2v01 extra vars
        BTaggingRun3Aux += ['GN3PflowMuonsV00_ptFromTruthDressedWZJet'] # GN3PflowMuonsV00 extra vars
        BTaggingRun3Aux += ['GN3EPCLV01_ptFromTruthDressedWZJet'] # GN3EPCLV01 extra vars
        btagContent = _getVariableList(jetcol, BTaggingRun3Aux)
    else:
        # standard outputs for Run 4
        BTaggingRun4Aux = [ 
            "SV1_NGTinSvx", 
            "SV1_masssvx", 
            "GN2HL_pu", 
            "GN2HL_pc", 
            "GN2HL_pb", 
            "GN2HL_ptau",
        ]
        btagContent = _getVariableList(jetcol, BTaggingRun4Aux)

    return btagContent + jetBasicContent

def BTaggingExpertContent(flags, jetcol):
    standardContent = BTaggingStandardContent(flags, jetcol)
    JetExtendedAux = [
        "GhostBHadronsFinalCount",
        "GhostBHadronsFinalPt",
        "GhostCHadronsFinalCount",
        "GhostCHadronsFinalPt",
        "GhostTausFinalCount",
        "GhostTausFinalPt",
        "PartonTruthLabelEnergy",
    ]
    extendedContent = _getVariableList(jetcol, JetExtendedAux)
    #! see https://ftag.docs.cern.ch/reco_algs/taggers/overview/ for additional expert-level variables that are omitted here.
    return standardContent + extendedContent
