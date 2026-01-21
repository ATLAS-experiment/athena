# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

ExtraElectronShowerShapes = [
    ".".join(
        [
            "Electrons",
            "Rhad.Rhad1.e277.Reta.Rphi.weta2.f1.Eratio.DeltaE.weta1.fracs1",
            "wtots1.f3.f3core.deltaEta1.deltaPhi1.deltaPhi2",
            "deltaPhiRescaled2.deltaPhiFromLastMeasurement",
        ]
    )
]

ExtraElectronTruthInfo = [
    ".".join(
        [
            "Electrons",
            "lastEgMotherTruthClassification.lastEgMotherTruthType.lastEgMotherTruthOrigin",
            "lastEgMotherTruthParticleLink.lastEgMotherPdgId",
        ]
    )
]


ElectronsCPDetailedContent = ExtraElectronShowerShapes + ExtraElectronTruthInfo

ExtraElectronGSFVar = [
    ".".join(
        [
            "GSFTrackParticles",
            "parameterX.parameterPX.parameterPY.parameterPZ.parameterPosition",
            "numberOfTRTHits.numberOfTRTOutliers",
            "numberOfTRTHighThresholdHits.numberOfTRTHighThresholdOutliers",
            "numberOfTRTXenonHits",
            "eProbabilityComb.eProbabilityHT.eProbabilityNN",
        ]
    )
]

GSFTracksCPDetailedContent = ExtraElectronGSFVar

ElectronsAddAmbiguityContent = [
    ".".join(
        [
            "Electrons",
            "DFCommonSimpleConvRadius.DFCommonSimpleConvPhi.DFCommonSimpleMee",
            "DFCommonSimpleMeeAtVtx.DFCommonSimpleSeparation",
            "DFCommonProdTrueRadius.DFCommonProdTruePhi.DFCommonProdTrueZ",
        ]
    )
]
