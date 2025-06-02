# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
import AthenaCommon.SystemOfUnits as Units


def createPanTauConfigFlags():
    flags = AthConfigFlags()

    flags.addFlag("TauConstituents_UsePionMass", True)
    flags.addFlag("FeatureExtractor_UseEmptySeeds", False)

    flags.addFlag("TauConstituents_Types_DeltaRCore", 0.2)
    flags.addFlag("TauConstituents_MaxEta", 9.9)
    flags.addFlag("TauConstituents_PreselectionMinEnergy", 500.)
    flags.addFlag("DecayModeDeterminator_BDTCutValue_R10X_CellBased", 0.52)
    flags.addFlag("DecayModeDeterminator_BDTCutValue_R11X_CellBased", -0.33)
    flags.addFlag("DecayModeDeterminator_BDTCutValue_R110_CellBased", 0.47)
    flags.addFlag("DecayModeDeterminator_BDTCutValue_R1XX_CellBased", -0.21)
    flags.addFlag("DecayModeDeterminator_BDTCutValue_R30X_CellBased", -0.13)
    flags.addFlag("DecayModeDeterminator_BDTCutValue_R3XX_CellBased", -0.08)
    flags.addFlag("ModeDiscriminator_TMVAMethod", "BDTG")

    flags.addFlag("TauConstituents_BinEdges_Eta", [0.000, 0.800, 1.400, 1.500, 1.900, 9.900])
    flags.addFlag("TauConstituents_Selection_Neutral_EtaBinned_EtCut", [2.1*Units.GeV, 2.5*Units.GeV, 2.6*Units.GeV, 2.4*Units.GeV, 1.9*Units.GeV])
    flags.addFlag("TauConstituents_Selection_Pi0Neut_EtaBinned_EtCut", [2.1*Units.GeV, 2.5*Units.GeV, 2.6*Units.GeV, 2.4*Units.GeV, 1.9*Units.GeV])
    flags.addFlag("TauConstituents_Selection_Charged_EtaBinned_EtCut", [1.0*Units.GeV, 1.0*Units.GeV, 1.0*Units.GeV, 1.0*Units.GeV, 1.0*Units.GeV])
    flags.addFlag("TauConstituents_Selection_OutNeut_EtaBinned_EtCut", [1.0*Units.GeV, 1.0*Units.GeV, 1.0*Units.GeV, 1.0*Units.GeV, 1.0*Units.GeV])
    flags.addFlag("TauConstituents_Selection_OutChrg_EtaBinned_EtCut", [1.0*Units.GeV, 1.0*Units.GeV, 1.0*Units.GeV, 1.0*Units.GeV, 1.0*Units.GeV])
    flags.addFlag("CellBased_BinEdges_Eta", [0.000, 0.800, 1.400, 1.500, 1.900, 9.900])
    flags.addFlag("CellBased_EtaBinned_Pi0MVACut_1prong", [0.46, 0.39, 0.51, 0.47, 0.54])
    flags.addFlag("CellBased_EtaBinned_Pi0MVACut_3prong", [0.47, 0.52, 0.60, 0.55, 0.50])
    flags.addFlag("ModeDiscriminator_BinEdges_Pt", [10*Units.GeV, 100000*Units.GeV])

    flags.addFlag("Names_InputAlgorithms", ["CellBased"])
    flags.addFlag("Names_ModeCases", ["1p0n_vs_1p1n", "1p1n_vs_1pXn", "3p0n_vs_3pXn"])
    flags.addFlag("ModeDiscriminator_BDTVariableNames_CellBased_1p0n_vs_1p1n", ["Neutral_PID_BDTValues_BDTSort_1", "Neutral_Ratio_1stBDTEtOverEtAllConsts", "Combined_DeltaR1stNeutralTo1stCharged", "Charged_JetMoment_EtDRxTotalEt", "Neutral_Shots_NPhotonsInSeed"])
    flags.addFlag("ModeDiscriminator_BDTVariableNames_CellBased_1p1n_vs_1pXn", ["Neutral_PID_BDTValues_BDTSort_2", "Neutral_HLV_SumM", "Neutral_Ratio_EtOverEtAllConsts", "Basic_NNeutralConsts", "Neutral_Shots_NPhotonsInSeed"])
    flags.addFlag("ModeDiscriminator_BDTVariableNames_CellBased_3p0n_vs_3pXn", ["Neutral_Ratio_EtOverEtAllConsts", "Neutral_PID_BDTValues_BDTSort_1", "Charged_StdDev_Et_WrtEtAllConsts", "Neutral_Shots_NPhotonsInSeed", "Charged_HLV_SumM"])

    return flags
