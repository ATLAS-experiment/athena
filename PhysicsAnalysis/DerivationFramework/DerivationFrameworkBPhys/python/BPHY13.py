# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# ====================================================================
# BPHY13.py for 4-muon resonance search
# Contact: xin.chen@cern.ch
# ====================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import MetadataCategory

BPHYDerivationName = "BPHY13"
streamName = "StreamDAOD_BPHY13"


def BPHY13JpsiFinderCfg(flags, name, **kwargs):
    acc = ComponentAccumulator()

    kwargs.setdefault("TrackAndTrack", False)
    kwargs.setdefault("assumeDiMuons", True)  # If True, will assume dimu hypothesis and use PDG value for mu mass
    kwargs.setdefault("muonThresholdPt", 2400)
    kwargs.setdefault("trackThresholdPt", 2400)
    kwargs.setdefault("Chi2Cut", 50)
    kwargs.setdefault("oppChargesOnly", True)
    kwargs.setdefault("useCombinedMeasurement", False)  # Only takes effect if combOnly=True
    kwargs.setdefault("muonCollectionKey", "Muons")
    kwargs.setdefault("TrackParticleCollection", "InDetTrackParticles")
    kwargs.setdefault("useMCPCuts", False)

    acc.setPrivateTools(CompFactory.Analysis.JpsiFinder(name, **kwargs))
    return acc


def BPHY13Cfg(flags):
    from DerivationFrameworkBPhys.CommonBPhysMethodsConfig import (BPHY_V0ToolCfg, BPHY_InDetDetailedTrackSelectorToolCfg, BPHY_VertexPointEstimatorCfg, BPHY_TrkVKalVrtFitterCfg, Reco_VertexCfg, ReVertexCfg, PsiPlusPsiSingleVertexCfg)
    from JpsiUpsilonTools.JpsiUpsilonToolsConfig import PrimaryVertexRefittingToolCfg
    acc = ComponentAccumulator()
    isSimulation = flags.Input.isMC

    # Configure common Public Tools
    V0Tools = acc.addPublicTool(acc.popToolsAndMerge(BPHY_V0ToolCfg(flags, BPHYDerivationName)))
    vkalvrt = acc.addPublicTool(acc.popToolsAndMerge(BPHY_TrkVKalVrtFitterCfg(flags, BPHYDerivationName)))  # VKalVrt vertex fitter
    trackselect = acc.addPublicTool(acc.popToolsAndMerge(BPHY_InDetDetailedTrackSelectorToolCfg(flags, BPHYDerivationName)))
    vpest = acc.addPublicTool(acc.popToolsAndMerge(BPHY_VertexPointEstimatorCfg(flags, BPHYDerivationName)))
    PVrefit = acc.addPublicTool(acc.popToolsAndMerge(PrimaryVertexRefittingToolCfg(flags)))

    # mass bounds and constants used in the following
    Phi_lo = 770.0
    Phi_hi = 1300.0
    Jpsi_lo = 2600.0
    Jpsi_hi = 3600.0
    Psi_lo = 3300.0
    Psi_hi = 4200.0
    Upsi_lo = 8800.0
    Upsi_hi = 10000.0
    Dimu_lo = 5000.0
    Dimu_hi = 9100.0

    Mumass = 105.658
    Phimass = 1019.461
    Jpsimass = 3096.916
    Psi2Smass = 3686.10
    Upsimass = 9460.30

    augmentation_tools = []

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name="BPHY13_Reco_Phimumu",
                       VertexSearchTool=acc.popToolsAndMerge(
                           BPHY13JpsiFinderCfg(flags,
                                               name="BPHY13PhiFinder_mumu",
                                               muAndMu=True,
                                               muAndTrack=False,
                                               invMassLower=Phi_lo,
                                               invMassUpper=Phi_hi,
                                               atLeastOneComb=True,
                                               TrkVertexFitterTool=vkalvrt,  # PublicToolHandle
                                               TrackSelectorTool=trackselect,  # PublicToolHandle
                                               VertexPointEstimator=vpest,  # PublicToolHandle
                                               )),
                       OutputVtxContainerName="BPHY13Phimumu",
                       PVContainerName="PrimaryVertices",
                       RefPVContainerName="SHOULDNOTBEUSED",
                       V0Tools=V0Tools,  # PublicToolHandle
                       PVRefitter=PVrefit,  # PublicToolHandle
                       DoVertexType=1)
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name="BPHY13_Reco_Jpsimumu",
                       VertexSearchTool=acc.popToolsAndMerge(
                           BPHY13JpsiFinderCfg(flags,
                                               name="BPHY13JpsiFinder_mumu",
                                               muAndMu=True,
                                               muAndTrack=False,
                                               invMassLower=Jpsi_lo,
                                               invMassUpper=Jpsi_hi,
                                               atLeastOneComb=True,
                                               TrkVertexFitterTool=vkalvrt,  # PublicToolHandle
                                               TrackSelectorTool=trackselect,  # PublicToolHandle
                                               VertexPointEstimator=vpest,  # PublicToolHandle
                                               )),
                       OutputVtxContainerName="BPHY13Jpsimumu",
                       PVContainerName="PrimaryVertices",
                       RefPVContainerName="SHOULDNOTBEUSED",
                       V0Tools=V0Tools,  # PublicToolHandle
                       PVRefitter=PVrefit,  # PublicToolHandle
                       DoVertexType=1)
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name="BPHY13_Reco_Psimumu",
                       VertexSearchTool=acc.popToolsAndMerge(
                           BPHY13JpsiFinderCfg(flags,
                                               name="BPHY13PsiFinder_mumu",
                                               muAndMu=True,
                                               muAndTrack=False,
                                               invMassLower=Psi_lo,
                                               invMassUpper=Psi_hi,
                                               atLeastOneComb=True,
                                               TrkVertexFitterTool=vkalvrt,  # PublicToolHandle
                                               TrackSelectorTool=trackselect,  # PublicToolHandle
                                               VertexPointEstimator=vpest,  # PublicToolHandle
                                               )),
                       OutputVtxContainerName="BPHY13Psimumu",
                       PVContainerName="PrimaryVertices",
                       RefPVContainerName="SHOULDNOTBEUSED",
                       V0Tools=V0Tools,  # PublicToolHandle
                       PVRefitter=PVrefit,  # PublicToolHandle
                       DoVertexType=1)
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name="BPHY13_Reco_Upsimumu",
                       VertexSearchTool=acc.popToolsAndMerge(
                           BPHY13JpsiFinderCfg(flags,
                                               name="BPHY13UpsiFinder_mumu",
                                               muAndMu=True,
                                               muAndTrack=False,
                                               invMassLower=Upsi_lo,
                                               invMassUpper=Upsi_hi,
                                               atLeastOneComb=True,
                                               TrkVertexFitterTool=vkalvrt,  # PublicToolHandle
                                               TrackSelectorTool=trackselect,  # PublicToolHandle
                                               VertexPointEstimator=vpest,  # PublicToolHandle
                                               )),
                       OutputVtxContainerName="BPHY13Upsimumu",
                       PVContainerName="PrimaryVertices",
                       RefPVContainerName="SHOULDNOTBEUSED",
                       V0Tools=V0Tools,  # PublicToolHandle
                       PVRefitter=PVrefit,  # PublicToolHandle
                       DoVertexType=1)
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name="BPHY13_Reco_Dimumumu",
                       VertexSearchTool=acc.popToolsAndMerge(
                           BPHY13JpsiFinderCfg(flags,
                                               name="BPHY13DimuFinder_mumu",
                                               muAndMu=True,
                                               muAndTrack=False,
                                               invMassLower=Dimu_lo,
                                               invMassUpper=Dimu_hi,
                                               atLeastOneComb=True,
                                               TrkVertexFitterTool=vkalvrt,  # PublicToolHandle
                                               TrackSelectorTool=trackselect,  # PublicToolHandle
                                               VertexPointEstimator=vpest,  # PublicToolHandle
                                               )),
                       OutputVtxContainerName="BPHY13Dimumumu",
                       PVContainerName="PrimaryVertices",
                       RefPVContainerName="SHOULDNOTBEUSED",
                       V0Tools=V0Tools,  # PublicToolHandle
                       PVRefitter=PVrefit,  # PublicToolHandle
                       DoVertexType=1)
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name="BPHY13_Reco_Phimutrk",
                       VertexSearchTool=acc.popToolsAndMerge(
                           BPHY13JpsiFinderCfg(flags,
                                               name="BPHY13PhiFinder_mutrk",
                                               muAndMu=False,
                                               muAndTrack=True,
                                               invMassLower=Phi_lo,
                                               invMassUpper=Phi_hi,
                                               atLeastOneComb=False,
                                               TrkVertexFitterTool=vkalvrt,  # PublicToolHandle
                                               TrackSelectorTool=trackselect,  # PublicToolHandle
                                               VertexPointEstimator=vpest,  # PublicToolHandle
                                               doTagAndProbe=True)),
                       OutputVtxContainerName="BPHY13Phimutrk",
                       PVContainerName="PrimaryVertices",
                       RefPVContainerName="SHOULDNOTBEUSED",
                       V0Tools=V0Tools,  # PublicToolHandle
                       PVRefitter=PVrefit,  # PublicToolHandle
                       DoVertexType=1)
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name="BPHY13_Reco_Jpsimutrk",
                       VertexSearchTool=acc.popToolsAndMerge(
                           BPHY13JpsiFinderCfg(flags,
                                               name="BPHY13JpsiFinder_mutrk",
                                               muAndMu=False,
                                               muAndTrack=True,
                                               invMassLower=Jpsi_lo,
                                               invMassUpper=Jpsi_hi,
                                               atLeastOneComb=False,
                                               TrkVertexFitterTool=vkalvrt,  # PublicToolHandle
                                               TrackSelectorTool=trackselect,  # PublicToolHandle
                                               VertexPointEstimator=vpest,  # PublicToolHandle
                                               useMCPCuts=False,
                                               doTagAndProbe=True)),
                       OutputVtxContainerName="BPHY13Jpsimutrk",
                       PVContainerName="PrimaryVertices",
                       RefPVContainerName="SHOULDNOTBEUSED",
                       V0Tools=V0Tools,  # PublicToolHandle
                       PVRefitter=PVrefit,  # PublicToolHandle
                       DoVertexType=1)
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name="BPHY13_Reco_Psimutrk",
                       VertexSearchTool=acc.popToolsAndMerge(
                           BPHY13JpsiFinderCfg(flags,
                                               name="BPHY13PsiFinder_mutrk",
                                               muAndMu=False,
                                               muAndTrack=True,
                                               invMassLower=Psi_lo,
                                               invMassUpper=Psi_hi,
                                               atLeastOneComb=False,
                                               TrkVertexFitterTool=vkalvrt,  # PublicToolHandle
                                               TrackSelectorTool=trackselect,  # PublicToolHandle
                                               VertexPointEstimator=vpest,  # PublicToolHandle
                                               doTagAndProbe=True)),
                       OutputVtxContainerName="BPHY13Psimutrk",
                       PVContainerName="PrimaryVertices",
                       RefPVContainerName="SHOULDNOTBEUSED",
                       V0Tools=V0Tools,  # PublicToolHandle
                       PVRefitter=PVrefit,  # PublicToolHandle
                       DoVertexType=1)
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name="BPHY13_Reco_Upsimutrk",
                       VertexSearchTool=acc.popToolsAndMerge(
                           BPHY13JpsiFinderCfg(flags,
                                               name="BPHY13UpsiFinder_mutrk",
                                               muAndMu=False,
                                               muAndTrack=True,
                                               invMassLower=Upsi_lo,
                                               invMassUpper=Upsi_hi,
                                               atLeastOneComb=False,
                                               TrkVertexFitterTool=vkalvrt,  # PublicToolHandle
                                               TrackSelectorTool=trackselect,  # PublicToolHandle
                                               VertexPointEstimator=vpest,  # PublicToolHandle
                                               doTagAndProbe=True)),
                       OutputVtxContainerName="BPHY13Upsimutrk",
                       PVContainerName="PrimaryVertices",
                       RefPVContainerName="SHOULDNOTBEUSED",
                       V0Tools=V0Tools,  # PublicToolHandle
                       PVRefitter=PVrefit,  # PublicToolHandle
                       DoVertexType=1)
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        Reco_VertexCfg(flags,
                       name="BPHY13_Reco_Dimumutrk",
                       VertexSearchTool=acc.popToolsAndMerge(
                           BPHY13JpsiFinderCfg(flags,
                                               name="BPHY13DimuFinder_mutrk",
                                               muAndMu=False,
                                               muAndTrack=True,
                                               invMassLower=Dimu_lo,
                                               invMassUpper=Dimu_hi,
                                               atLeastOneComb=False,
                                               TrkVertexFitterTool=vkalvrt,  # PublicToolHandle
                                               TrackSelectorTool=trackselect,  # PublicToolHandle
                                               VertexPointEstimator=vpest,  # PublicToolHandle
                                               doTagAndProbe=True)),
                       OutputVtxContainerName="BPHY13Dimumutrk",
                       PVContainerName="PrimaryVertices",
                       RefPVContainerName="SHOULDNOTBEUSED",
                       V0Tools=V0Tools,  # PublicToolHandle
                       PVRefitter=PVrefit,  # PublicToolHandle
                       DoVertexType=1)
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        ReVertexCfg(flags,
                    name="BPHY13_Rev_Phimumu",
                    InputVtxContainerName="BPHY13Phimumu",
                    TrackIndices=[0, 1],
                    RefitPV=False,
                    UseMassConstraint=True,
                    VertexMass=Phimass,
                    MassInputParticles=[Mumass, Mumass],
                    Chi2Cut=50.,
                    PVRefitter=PVrefit,  # PublicToolHandle
                    V0Tools=V0Tools,  # PublicToolHandle
                    TrkVertexFitterTool=vkalvrt,  # PublicToolHandle
                    OutputVtxContainerName="BPHY13Phimumu_revtx")
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        ReVertexCfg(flags,
                    name="BPHY13_Rev_Jpsimumu",
                    InputVtxContainerName="BPHY13Jpsimumu",
                    TrackIndices=[0, 1],
                    RefitPV=False,
                    UseMassConstraint=True,
                    VertexMass=Jpsimass,
                    MassInputParticles=[Mumass, Mumass],
                    Chi2Cut=50.,
                    PVRefitter=PVrefit,  # PublicToolHandle
                    V0Tools=V0Tools,  # PublicToolHandle
                    TrkVertexFitterTool=vkalvrt,  # PublicToolHandle
                    OutputVtxContainerName="BPHY13Jpsimumu_revtx")
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        ReVertexCfg(flags,
                    name="BPHY13_Rev_Psimumu",
                    InputVtxContainerName="BPHY13Psimumu",
                    TrackIndices=[0, 1],
                    RefitPV=False,
                    UseMassConstraint=True,
                    VertexMass=Psi2Smass,
                    MassInputParticles=[Mumass, Mumass],
                    Chi2Cut=50.,
                    PVRefitter=PVrefit,  # PublicToolHandle
                    V0Tools=V0Tools,  # PublicToolHandle
                    TrkVertexFitterTool=vkalvrt,  # PublicToolHandle
                    OutputVtxContainerName="BPHY13Psimumu_revtx")
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        ReVertexCfg(flags,
                    name="BPHY13_Rev_Upsimumu",
                    InputVtxContainerName="BPHY13Upsimumu",
                    TrackIndices=[0, 1],
                    RefitPV=False,
                    UseMassConstraint=True,
                    VertexMass=Upsimass,
                    MassInputParticles=[Mumass, Mumass],
                    Chi2Cut=50.,
                    PVRefitter=PVrefit,  # PublicToolHandle
                    V0Tools=V0Tools,  # PublicToolHandle
                    TrkVertexFitterTool=vkalvrt,  # PublicToolHandle
                    OutputVtxContainerName="BPHY13Upsimumu_revtx")
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        ReVertexCfg(flags,
                    name="BPHY13_Rev_Phimutrk",
                    InputVtxContainerName="BPHY13Phimutrk",
                    TrackIndices=[0, 1],
                    RefitPV=False,
                    UseMassConstraint=True,
                    VertexMass=Phimass,
                    MassInputParticles=[Mumass, Mumass],
                    Chi2Cut=50.,
                    PVRefitter=PVrefit,  # PublicToolHandle
                    V0Tools=V0Tools,  # PublicToolHandle
                    TrkVertexFitterTool=vkalvrt,  # PublicToolHandle
                    OutputVtxContainerName="BPHY13Phimutrk_revtx")
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        ReVertexCfg(flags,
                    name="BPHY13_Rev_Jpsimutrk",
                    InputVtxContainerName="BPHY13Jpsimutrk",
                    TrackIndices=[0, 1],
                    RefitPV=False,
                    UseMassConstraint=True,
                    VertexMass=Jpsimass,
                    MassInputParticles=[Mumass, Mumass],
                    Chi2Cut=50.,
                    PVRefitter=PVrefit,  # PublicToolHandle
                    V0Tools=V0Tools,  # PublicToolHandle
                    TrkVertexFitterTool=vkalvrt,  # PublicToolHandle
                    OutputVtxContainerName="BPHY13Jpsimutrk_revtx")
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        ReVertexCfg(flags,
                    name="BPHY13_Rev_Psimutrk",
                    InputVtxContainerName="BPHY13Psimutrk",
                    TrackIndices=[0, 1],
                    RefitPV=False,
                    UseMassConstraint=True,
                    VertexMass=Psi2Smass,
                    MassInputParticles=[Mumass, Mumass],
                    Chi2Cut=50.,
                    PVRefitter=PVrefit,  # PublicToolHandle
                    V0Tools=V0Tools,  # PublicToolHandle
                    TrkVertexFitterTool=vkalvrt,  # PublicToolHandle
                    OutputVtxContainerName="BPHY13Psimutrk_revtx")
    )))

    augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
        ReVertexCfg(flags,
                    name="BPHY13_Rev_Upsimutrk",
                    InputVtxContainerName="BPHY13Upsimutrk",
                    TrackIndices=[0, 1],
                    RefitPV=False,
                    UseMassConstraint=True,
                    VertexMass=Upsimass,
                    MassInputParticles=[Mumass, Mumass],
                    Chi2Cut=50.,
                    PVRefitter=PVrefit,  # PublicToolHandle
                    V0Tools=V0Tools,  # PublicToolHandle
                    TrkVertexFitterTool=vkalvrt,  # PublicToolHandle
                    OutputVtxContainerName="BPHY13Upsimutrk_revtx")
    )))

    OutputCollections = []
    RefPVContainers = []
    RefPVAuxContainers = []
    passedCandidates = []

    list_4mu_hypo = ["UpsiUpsi_4mu", "UpsiPsi_4mu", "UpsiJpsi_4mu", "UpsiPhi_4mu",
                     "PsiPsi_4mu", "PsiJpsi_4mu", "PsiPhi_4mu",
                     "JpsiJpsi_4mu", "JpsiPhi_4mu", "PhiPhi_4mu",
                     "UpsiDimu_4mu"]
    list_4mu_psi1Input = ["BPHY13Upsimumu_revtx", "BPHY13Upsimumu_revtx", "BPHY13Upsimumu_revtx", "BPHY13Upsimumu_revtx",
                          "BPHY13Psimumu_revtx", "BPHY13Psimumu_revtx", "BPHY13Psimumu_revtx",
                          "BPHY13Jpsimumu_revtx", "BPHY13Jpsimumu_revtx", "BPHY13Phimumu_revtx",
                          "BPHY13Upsimumu_revtx"]
    list_4mu_psi2Input = ["BPHY13Upsimumu_revtx", "BPHY13Psimumu_revtx", "BPHY13Jpsimumu_revtx", "BPHY13Phimumu_revtx",
                          "BPHY13Psimumu_revtx", "BPHY13Jpsimumu_revtx", "BPHY13Phimumu_revtx",
                          "BPHY13Jpsimumu_revtx", "BPHY13Phimumu_revtx", "BPHY13Phimumu_revtx",
                          "BPHY13Dimumumu"]
    list_4mu_jpsi1lo = [Upsi_lo, Upsi_lo, Upsi_lo, Upsi_lo,
                        Psi_lo, Psi_lo, Psi_lo,
                        Jpsi_lo, Jpsi_lo, Phi_lo,
                        Upsi_lo]
    list_4mu_jpsi1hi = [Upsi_hi, Upsi_hi, Upsi_hi, Upsi_hi,
                        Psi_hi, Psi_hi, Psi_hi,
                        Jpsi_hi, Jpsi_hi, Phi_hi,
                        Upsi_hi]
    list_4mu_jpsi1mass = [Upsimass, Upsimass, Upsimass, Upsimass,
                          Psi2Smass, Psi2Smass, Psi2Smass,
                          Jpsimass, Jpsimass, Phimass,
                          Upsimass]
    list_4mu_jpsi2lo = [Upsi_lo, Psi_lo, Jpsi_lo, Phi_lo,
                        Psi_lo, Jpsi_lo, Phi_lo,
                        Jpsi_lo, Phi_lo, Phi_lo,
                        Dimu_lo]
    list_4mu_jpsi2hi = [Upsi_hi, Psi_hi, Jpsi_hi, Phi_hi,
                        Psi_hi, Jpsi_hi, Phi_hi,
                        Jpsi_hi, Phi_hi, Phi_hi,
                        Dimu_hi]
    list_4mu_jpsi2mass = [Upsimass, Psi2Smass, Jpsimass, Phimass,
                          Psi2Smass, Jpsimass, Phimass,
                          Jpsimass, Phimass, Phimass]

    for i, hypo in enumerate(list_4mu_hypo):
        FourMuArgs = {}
        FourMuArgs.setdefault("HypothesisName", hypo)
        FourMuArgs.setdefault("Psi1Vertices", list_4mu_psi1Input[i])
        FourMuArgs.setdefault("Psi2Vertices", list_4mu_psi2Input[i])
        FourMuArgs.setdefault("NumberOfPsi1Daughters", 2)
        FourMuArgs.setdefault("NumberOfPsi2Daughters", 2)
        FourMuArgs.setdefault("Jpsi1MassLowerCut", list_4mu_jpsi1lo[i])
        FourMuArgs.setdefault("Jpsi1MassUpperCut", list_4mu_jpsi1hi[i])
        FourMuArgs.setdefault("Jpsi2MassLowerCut", list_4mu_jpsi2lo[i])
        FourMuArgs.setdefault("Jpsi2MassUpperCut", list_4mu_jpsi2hi[i])
        FourMuArgs.setdefault("MassLowerCut", 0.)
        FourMuArgs.setdefault("MassUpperCut", 31000.)
        FourMuArgs.setdefault("Jpsi1Mass", list_4mu_jpsi1mass[i])
        FourMuArgs.setdefault("ApplyJpsi1MassConstraint", True)
        if i == len(list_4mu_hypo) - 1:
            FourMuArgs.setdefault("ApplyJpsi2MassConstraint", False)
        else:
            FourMuArgs.setdefault("Jpsi2Mass", list_4mu_jpsi2mass[i])
            FourMuArgs.setdefault("ApplyJpsi2MassConstraint", True)
        FourMuArgs.setdefault("Chi2Cut", 25.)
        FourMuArgs.setdefault("PVRefitter", PVrefit)
        FourMuArgs.setdefault("TrkVertexFitterTool", vkalvrt)
        FourMuArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        FourMuArgs.setdefault("OutputVertexCollections", ["BPHY13_" + hypo + "_SubVtx1", "BPHY13_" + hypo + "_SubVtx2", "BPHY13_" + hypo + "_MainVtx"])
        FourMuArgs.setdefault("RefPVContainerName", "BPHY13_" + hypo + "_RefPrimaryVertices")
        FourMuArgs.setdefault("RefitPV", True)
        FourMuArgs.setdefault("MaxnPV", 100)
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            PsiPlusPsiSingleVertexCfg(flags, name="BPHY13_" + hypo, **FourMuArgs)
        )))
        OutputCollections += FourMuArgs.get("OutputVertexCollections")
        RefPVContainers += ["xAOD::VertexContainer#BPHY13_" + hypo + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY13_" + hypo + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY13_" + hypo + "_MainVtx"]

    list_3mu1trk_hypo = ["UpsiUpsi_3mu1trk", "UpsiPsi_3mu1trk", "UpsiJpsi_3mu1trk", "UpsiPhi_3mu1trk",
                         "PsiPsi_3mu1trk", "PsiJpsi_3mu1trk", "PsiPhi_3mu1trk",
                         "JpsiJpsi_3mu1trk", "JpsiPhi_3mu1trk", "PhiPhi_3mu1trk",
                         "PsiUpsi_3mu1trk", "JpsiUpsi_3mu1trk", "PhiUpsi_3mu1trk",
                         "JpsiPsi_3mu1trk", "PhiPsi_3mu1trk", "PhiJpsi_3mu1trk",
                         "UpsiDimu_3mu1trk", "DimuUpsi_3mu1trk"]
    list_3mu1trk_psi1Input = ["BPHY13Upsimumu_revtx", "BPHY13Upsimumu_revtx", "BPHY13Upsimumu_revtx", "BPHY13Upsimumu_revtx",
                              "BPHY13Psimumu_revtx", "BPHY13Psimumu_revtx", "BPHY13Psimumu_revtx",
                              "BPHY13Jpsimumu_revtx", "BPHY13Jpsimumu_revtx", "BPHY13Phimumu_revtx",
                              "BPHY13Psimumu_revtx", "BPHY13Jpsimumu_revtx", "BPHY13Phimumu_revtx",
                              "BPHY13Jpsimumu_revtx", "BPHY13Phimumu_revtx", "BPHY13Phimumu_revtx",
                              "BPHY13Upsimumu_revtx", "BPHY13Dimumumu"]
    list_3mu1trk_psi2Input = ["BPHY13Upsimutrk_revtx", "BPHY13Psimutrk_revtx", "BPHY13Jpsimutrk_revtx", "BPHY13Phimutrk_revtx",
                              "BPHY13Psimutrk_revtx", "BPHY13Jpsimutrk_revtx", "BPHY13Phimutrk_revtx",
                              "BPHY13Jpsimutrk_revtx", "BPHY13Phimutrk_revtx", "BPHY13Phimutrk_revtx",
                              "BPHY13Upsimutrk_revtx", "BPHY13Upsimutrk_revtx", "BPHY13Upsimutrk_revtx",
                              "BPHY13Psimutrk_revtx", "BPHY13Psimutrk_revtx", "BPHY13Jpsimutrk_revtx",
                              "BPHY13Dimumutrk", "BPHY13Upsimutrk_revtx"]
    list_3mu1trk_jpsi1lo = [Upsi_lo, Upsi_lo, Upsi_lo, Upsi_lo,
                            Psi_lo, Psi_lo, Psi_lo,
                            Jpsi_lo, Jpsi_lo, Phi_lo,
                            Psi_lo, Jpsi_lo, Phi_lo,
                            Jpsi_lo, Phi_lo, Phi_lo,
                            Upsi_lo, Dimu_lo]
    list_3mu1trk_jpsi1hi = [Upsi_hi, Upsi_hi, Upsi_hi, Upsi_hi,
                            Psi_hi, Psi_hi, Psi_hi,
                            Jpsi_hi, Jpsi_hi, Phi_hi,
                            Psi_hi, Jpsi_hi, Phi_hi,
                            Jpsi_hi, Phi_hi, Phi_hi,
                            Upsi_hi, Dimu_hi]
    list_3mu1trk_jpsi1mass = [Upsimass, Upsimass, Upsimass, Upsimass,
                              Psi2Smass, Psi2Smass, Psi2Smass,
                              Jpsimass, Jpsimass, Phimass,
                              Psi2Smass, Jpsimass, Phimass,
                              Jpsimass, Phimass, Phimass,
                              Upsimass, 0.0]  # 0.0 is dummy
    list_3mu1trk_jpsi2lo = [Upsi_lo, Psi_lo, Jpsi_lo, Phi_lo,
                            Psi_lo, Jpsi_lo, Phi_lo,
                            Jpsi_lo, Phi_lo, Phi_lo,
                            Upsi_lo, Upsi_lo, Upsi_lo,
                            Psi_lo, Psi_lo, Jpsi_lo,
                            Dimu_lo, Upsi_lo]
    list_3mu1trk_jpsi2hi = [Upsi_hi, Psi_hi, Jpsi_hi, Phi_hi,
                            Psi_hi, Jpsi_hi, Phi_hi,
                            Jpsi_hi, Phi_hi, Phi_hi,
                            Upsi_hi, Upsi_hi, Upsi_hi,
                            Psi_hi, Psi_hi, Jpsi_hi,
                            Dimu_hi, Upsi_lo]
    list_3mu1trk_jpsi2mass = [Upsimass, Psi2Smass, Jpsimass, Phimass,
                              Psi2Smass, Jpsimass, Phimass,
                              Jpsimass, Phimass, Phimass,
                              Upsimass, Upsimass, Upsimass,
                              Psi2Smass, Psi2Smass, Jpsimass,
                              0.0, Upsimass]  # 0.0 is dummy

    for i, hypo in enumerate(list_3mu1trk_hypo):
        ThreeMuArgs = {}
        ThreeMuArgs.setdefault("HypothesisName", hypo)
        ThreeMuArgs.setdefault("Psi1Vertices", list_3mu1trk_psi1Input[i])
        ThreeMuArgs.setdefault("Psi2Vertices", list_3mu1trk_psi2Input[i])
        ThreeMuArgs.setdefault("NumberOfPsi1Daughters", 2)
        ThreeMuArgs.setdefault("NumberOfPsi2Daughters", 2)
        ThreeMuArgs.setdefault("MaxCandidates", 20)
        ThreeMuArgs.setdefault("Jpsi1MassLowerCut", list_3mu1trk_jpsi1lo[i])
        ThreeMuArgs.setdefault("Jpsi1MassUpperCut", list_3mu1trk_jpsi1hi[i])
        ThreeMuArgs.setdefault("Jpsi2MassLowerCut", list_3mu1trk_jpsi2lo[i])
        ThreeMuArgs.setdefault("Jpsi2MassUpperCut", list_3mu1trk_jpsi2hi[i])
        ThreeMuArgs.setdefault("MassLowerCut", 0.)
        ThreeMuArgs.setdefault("MassUpperCut", 31000.)
        if i == len(list_3mu1trk_hypo) - 2:
            ThreeMuArgs.setdefault("Jpsi1Mass", list_3mu1trk_jpsi1mass[i])
            ThreeMuArgs.setdefault("ApplyJpsi1MassConstraint", True)
            ThreeMuArgs.setdefault("ApplyJpsi2MassConstraint", False)
        elif i == len(list_3mu1trk_hypo) - 1:
            ThreeMuArgs.setdefault("Jpsi2Mass", list_3mu1trk_jpsi2mass[i])
            ThreeMuArgs.setdefault("ApplyJpsi1MassConstraint", False)
            ThreeMuArgs.setdefault("ApplyJpsi2MassConstraint", True)
        else:
            ThreeMuArgs.setdefault("Jpsi1Mass", list_3mu1trk_jpsi1mass[i])
            ThreeMuArgs.setdefault("Jpsi2Mass", list_3mu1trk_jpsi2mass[i])
            ThreeMuArgs.setdefault("ApplyJpsi1MassConstraint", True)
            ThreeMuArgs.setdefault("ApplyJpsi2MassConstraint", True)
        ThreeMuArgs.setdefault("Chi2Cut", 25.)
        ThreeMuArgs.setdefault("PVRefitter", PVrefit)
        ThreeMuArgs.setdefault("TrkVertexFitterTool", vkalvrt)
        ThreeMuArgs.setdefault("VxPrimaryCandidateName", "PrimaryVertices")
        ThreeMuArgs.setdefault("OutputVertexCollections", ["BPHY13_" + hypo + "_SubVtx1", "BPHY13_" + hypo + "_SubVtx2", "BPHY13_" + hypo + "_MainVtx"])
        ThreeMuArgs.setdefault("RefPVContainerName", "BPHY13_" + hypo + "_RefPrimaryVertices")
        ThreeMuArgs.setdefault("RefitPV", True)
        ThreeMuArgs.setdefault("MaxnPV", 100)
        augmentation_tools.append(acc.addPublicTool(acc.popToolsAndMerge(
            PsiPlusPsiSingleVertexCfg(flags, name="BPHY13_" + hypo, **ThreeMuArgs)
        )))
        OutputCollections += ThreeMuArgs.get("OutputVertexCollections")
        RefPVContainers += ["xAOD::VertexContainer#BPHY13_" + hypo + "_RefPrimaryVertices"]
        RefPVAuxContainers += ["xAOD::VertexAuxContainer#BPHY13_" + hypo + "_RefPrimaryVerticesAux."]
        passedCandidates += ["BPHY13_" + hypo + "_MainVtx"]

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(
        "BPHY13Kernel",
        AugmentationTools=augmentation_tools,
        SkimmingTools=[acc.addPublicTool(CompFactory.DerivationFramework.AnyVertexSkimmingTool(name="BPHY13_SelectEvent", VertexContainerNames=passedCandidates))]
    ))

    from DerivationFrameworkCore.SlimmingHelper import SlimmingHelper
    from OutputStreamAthenaPool.OutputStreamConfig import OutputStreamCfg
    from xAODMetaDataCnv.InfileMetaDataConfig import SetupMetaDataForStreamCfg
    BPHY13SlimmingHelper = SlimmingHelper("BPHY13SlimmingHelper", NamesAndTypes=flags.Input.TypedCollections, flags=flags)
    from DerivationFrameworkBPhys.CommonBPhysMethodsConfig import getDefaultAllVariables
    BPHY13_AllVariables = getDefaultAllVariables()
    BPHY13_StaticContent = []

    # Needed for trigger objects
    BPHY13SlimmingHelper.IncludeMuonTriggerContent = True
    BPHY13SlimmingHelper.IncludeBPhysTriggerContent = True

    # primary vertices
    BPHY13_AllVariables += ["PrimaryVertices"]
    BPHY13_StaticContent += RefPVContainers
    BPHY13_StaticContent += RefPVAuxContainers

    # ID track particles
    BPHY13_AllVariables += ["InDetTrackParticles"]

    # combined / extrapolated muon track particles
    # (note: for tagged muons there is no extra TrackParticle collection since the ID tracks
    # are store in InDetTrackParticles collection)
    BPHY13_AllVariables += ["CombinedMuonTrackParticles", "ExtrapolatedMuonTrackParticles"]

    # muon container
    BPHY13_AllVariables += ["Muons", "MuonSegments"]

    # we have to disable vxTrackAtVertex branch since it is not xAOD compatible
    for output in OutputCollections:
        BPHY13_StaticContent += ["xAOD::VertexContainer#%s" % output]
        BPHY13_StaticContent += ["xAOD::VertexAuxContainer#%sAux.-vxTrackAtVertex" % output]

    # Truth information for MC only
    if isSimulation:
        BPHY13_AllVariables += ["TruthEvents", "TruthParticles", "TruthVertices", "MuonTruthParticles"]

    BPHY13SlimmingHelper.SmartCollections = ["Muons", "PrimaryVertices", "InDetTrackParticles"]
    BPHY13SlimmingHelper.AllVariables = BPHY13_AllVariables
    BPHY13SlimmingHelper.StaticContent = BPHY13_StaticContent

    BPHY13ItemList = BPHY13SlimmingHelper.GetItemList()
    acc.merge(OutputStreamCfg(flags, "DAOD_BPHY13", ItemList=BPHY13ItemList, AcceptAlgs=["BPHY13Kernel"]))
    acc.merge(SetupMetaDataForStreamCfg(flags, "DAOD_BPHY13", AcceptAlgs=["BPHY13Kernel"], createMetadata=[MetadataCategory.CutFlowMetaData]))
    acc.printConfig(withDetails=True, summariseProps=True, onlyComponents=[], printDefaults=True)
    return acc
