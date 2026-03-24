# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

def GetJetVariables(container, refcontainer=''):
    vars = []

    vars += ["leadingjettrel"]

    if ( ("Topo" in container or "PFlow" in container) and "Trimmed" not in container) and "SoftDrop" not in container:
        vars += [ "Width", 
            # jet states
            "basickinematics_emscale",
            # track variables
            "JVF[0]", 
            "JVF", 
            "JVFCorr", 
            "Jvt", 
            "JvtRpt",
            # calo variables
            "NegativeE", 
            "Timing",
        ]

    if "Topo" in container or "PFlow" in container or "UFO" in container:
        vars += [
            # jet states
            "basickinematics_constscale",

            #track variables
            "SumPtTrkPt500",
            "SumPtTrkPt500[0]",
            "NumTrkPt500[0]",
            "NumTrkPt1000[0]",
            "TrackWidthPt1000[0]",
            "GhostTrackCount",
            "GhostTruthAssociationFraction",
            "GhostMuonSegmentCount",

            #calo variables
            "EMFrac",
            "HECFrac",
            "EMB2",
            "EMB3",
            "EME2",
            "EME3",
            "HEC2",
            "HEC3",
            "FCAL0",
            "FCAL1",
            "TileBar0",
            "TileBar1",
            "TileExt0",
            "TileExt1",
        ]

    if "PFlow" in container:
        vars += [
            "SumPtChargedPFOPt500[0]",
            "NumChargedPFOPt500[0]",
            "NumChargedPFOPt1000[0]",
            "ChargedPFOWidthPt1000[0]",
            "DFCommonJets_fJvt",
        ]

    if refcontainer != '':
        vars += [
            "effresponse",
        ]
    
    if "SoftDrop" in container:
        vars += [
            "rg",
            "zg",
        ]

    if "Trimmed" in container:
        vars += [
            "NTrimSubjets",
            "TrackSumPt",
            "TrackSumMass",
        ]

    if "Trimmed" in container or "SoftDrop" in container:
        if "Truth" in container:
            vars += [
                "D2",
                "Tau1_wta",
                "Tau2_wta",
                "Tau3_wta",
                "Qw",
            ]
        else:
            vars += [
                "ECF1",
                "ECF2",
                "ECF3",
                "Tau1_wta",
                "Tau2_wta",
                "Tau3_wta",
                "Split12",
                "Split23",
                "Split34",
                "DetectorEta",
                "Qw",
                "PlanarFlow",
                "FoxWolfram2",
                "FoxWolfram0",
                "Angularity",
                "Aplanarity",
                "KtDR",
                "ZCut12",
                "ZCut23",
                "ZCut34",
                "ThrustMin",
                "ThrustMaj",
                "Sphericity",
                "Charge",
            ]

    if 'PV0Track' in container:
        vars += [
            "HadronConeExclTruthLabelID",
            "HadronConeExclExtendedTruthLabelID",
            "HadronGhostTruthLabelID",
            "HadronGhostExtendedTruthLabelID",
        ]
    return vars
