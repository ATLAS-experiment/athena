#  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def IsolationSelectionToolCfg(flags, name="IsolationSelectionTool", **kwargs):
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.CP.IsolationSelectionTool(name, **kwargs))
    return acc

def MuonPhysValIsolationSelCfg(flags, **kwargs):
    return IsolationSelectionToolCfg(flags,MuonWP="PflowTight_FixedRad")

def IsoCloseByCorrectionToolCfg(flags, name="IsoCloseByCorrectionTool", ttva_wp = "", **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("IsolationSelectionTool", acc.popToolsAndMerge(MuonPhysValIsolationSelCfg(flags)))
    from InDetConfig.InDetTrackSelectionToolConfig import isoTrackSelectionToolCfg
    kwargs.setdefault("TrackSelectionTool", acc.popToolsAndMerge(isoTrackSelectionToolCfg(flags, minPt=500)) )
    if len(ttva_wp):
        from TrackVertexAssociationTool.TrackVertexAssociationToolConfig import isoTTVAToolCfg
        kwargs.setdefault("TTVASelectionTool", acc.popToolsAndMerge(isoTTVAToolCfg(flags, WorkingPoint = ttva_wp)))
    from TrackToCalo.TrackToCaloConfig import ParticleCaloExtensionToolCfg
    kwargs.setdefault("ParticleCaloExtensionTool", acc.popToolsAndMerge(ParticleCaloExtensionToolCfg(flags)))   
    the_tool = CompFactory.CP.IsolationCloseByCorrectionTool(name, **kwargs)
    acc.setPrivateTools(the_tool)
    return acc


def IsoCloseByCorrSkimmingAlgCfg(flags, suff = "", name="IsoCloseByCorrSkimmingAlg", ttva_wp = 'Nonprompt_All_MaxWeight',  **kwargs):
    result = ComponentAccumulator()
    from ElectronPhotonSelectorTools.AsgElectronLikelihoodToolsConfig import AsgElectronLikelihoodToolCfg
    from ElectronPhotonSelectorTools.ElectronLikelihoodToolMapping import  electronLHmenu
    from ElectronPhotonSelectorTools.LikelihoodEnums import LikeEnum
    from AthenaConfiguration.Enums import LHCPeriod
    kwargs.setdefault("ElectronSelectionTool", result.popToolsAndMerge(AsgElectronLikelihoodToolCfg(flags,
                                                                           name= "ElectronSelTool",
                                                                           quality = LikeEnum.VeryLoose,
                                                                           menu=electronLHmenu.offlineMC21 if flags.GeoModel.Run >= LHCPeriod.Run3 else electronLHmenu.offlineMC20)))

    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    kwargs.setdefault("MuonSelectionTool", result.popToolsAndMerge(MuonSelectionToolCfg(flags, 
                                                                MaxEta=2.7,
                                                                DisablePtCuts=True,
                                                                MuQuality=2, ### Select the loose working point
                                                                )))
    kwargs.setdefault("IsoCloseByCorrectionTool", result.popToolsAndMerge(IsoCloseByCorrectionToolCfg(flags)))

    ### Photon selection needs to be still defined
    # kwargs.setdefault("PhotonSelectionTool", <blah>)   
    kwargs.setdefault("PhotContainer", "")
    the_alg = CompFactory.CP.IsoCloseByCorrectionTrkSelAlg(name+ttva_wp+suff, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def IsoCloseByCorrAlgCfg(flags, name="IsoCloseByCorrAlg", suff = "", isPhysLite = False, containerNames = [ "Muons", "Electrons", "Photons"], useSelTools = False, isoDecSuffix = "CloseByCorr", hasLRT = False, **kwargs):

    result = ComponentAccumulator()

    # Configure the CloseBy isolation correction alg - only need two WPs each for all iso variables
    elIsoWPs   = [ "Loose_VarRad", "TightTrackOnly_FixedRad" ]
    muIsoWPs   = [ "PflowLoose_VarRad", "Loose_VarRad" ] 
    phIsoWPs   = [ "FixedCutLoose", "TightCaloOnly" ]

    isoTool   = result.popToolsAndMerge( IsolationSelectionToolCfg( flags,
                                                                    ElectronWPVec = elIsoWPs,
                                                                    MuonWPVec     = muIsoWPs,
                                                                    PhotonWPVec   = phIsoWPs))
    # Set suffix for writing corrected isolation values
    selectionDecorator = "isoSelIsOK" + suff
    kwargs.setdefault("IsoCloseByCorrectionTool", 
                       result.popToolsAndMerge(IsoCloseByCorrectionToolCfg(flags, 
                                                                           IsolationSelectionTool = isoTool,
                                                                           SelectionDecorator     = selectionDecorator,
                                                                           IsoDecSuffix           = isoDecSuffix,
                                                                           EleContainers = [ x for x in containerNames if x.find("Ele") != -1],
                                                                           MuoContainers = [ x for x in containerNames if x.find("Muo") != -1],
                                                                           PhoContainers = [ x for x in containerNames if x.find("Pho") != -1])))  
    
    # Need muon selection tool to apply Loose - no pt cuts
    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    kwargs.setdefault("MuonSelectionTool", result.popToolsAndMerge(MuonSelectionToolCfg(flags, 
                                                                MaxEta        = 2.7,
                                                                DisablePtCuts = True,
                                                                MuQuality     = 2, ### Select the loose working point
                                                                UseLRT        = hasLRT,
                                                                )))  

    # Define selectors for electron and photon - different for LLP1 as compared to PHYS and PHYSLITE
    kwargs.setdefault("PhotSelectionKey",  "Photons.DFCommonPhotonsIsEMLoose")
    if hasLRT:
        kwargs.setdefault("ElecSelectionKey",  "Electrons.DFCommonElectronsLHVeryLooseNoPix")
    else:
        kwargs.setdefault("ElecSelectionKey",  "Electrons.DFCommonElectronsLHVeryLoose")

    # Set selection for muons, electrons and photons to contribute to overlap
    kwargs.setdefault("ParticleContainerKeys",    containerNames)
    
    # No default pt cuts for the moment
    kwargs.setdefault("MinElecPt", 0.)
    kwargs.setdefault("MinMuonPt", 0.)
    kwargs.setdefault("MinPhotPt", 0.)

      
    the_alg = CompFactory.CP.IsoCloseByCorrectionAlg(name + suff,
                                                     SelectionDecorator     = selectionDecorator,
                                                     **kwargs)
    result.addEventAlgo(the_alg)
    return result

def IsoCloseByCaloDecorCfg(flags, name="IsoCloseByCaloDecor", suff = "", containers =[], **kwargs):
    result = ComponentAccumulator()
    ## Configure the tool such that the calo & pflow clusters are decorated
    ## https://gitlab.cern.ch/atlas/athena/-/blob/master/PhysicsAnalysis/AnalysisCommon/IsolationSelection/IsolationSelection/IsolationCloseByCorrectionTool.h#L35-39
    kwargs.setdefault("IsoCloseByCorrectionTool", result.popToolsAndMerge(
                      IsoCloseByCorrectionToolCfg(flags,
                                                  CaloCorrectionModel = -1
                                                  )))    
    for cont in containers:
        result.addEventAlgo(CompFactory.CP.IsoCloseByCaloDecorAlg(name = name + cont + suff, 
                                                                  PrimaryContainer = cont,
                                                                  **kwargs))
    return result
    
def TestIsoCloseByCorrectionCfg(flags, name="TestIsoCloseByAlg", suff = "", **kwargs):
    result = ComponentAccumulator()
    from ElectronPhotonSelectorTools.AsgElectronLikelihoodToolsConfig import AsgElectronLikelihoodToolCfg
    from ElectronPhotonSelectorTools.ElectronLikelihoodToolMapping import  electronLHmenu
    from ElectronPhotonSelectorTools.LikelihoodEnums import LikeEnum
    from AthenaConfiguration.Enums import LHCPeriod
    kwargs.setdefault("ElectronSelectionTool", result.popToolsAndMerge(AsgElectronLikelihoodToolCfg(flags,
                                                                           name= "ElectronSelTool",
                                                                           quality = LikeEnum.VeryLoose,
                                                                           menu=electronLHmenu.offlineMC21 if flags.GeoModel.Run >= LHCPeriod.Run3 else electronLHmenu.offlineMC20)))

    from MuonSelectorTools.MuonSelectorToolsConfig import MuonSelectionToolCfg
    kwargs.setdefault("MuonSelectionTool", result.popToolsAndMerge(MuonSelectionToolCfg(flags, 
                                                                MaxEta=2.7,
                                                                DisablePtCuts=True,
                                                                MuQuality=2, ### Select the loose working point
                                                                )))  
    the_alg = CompFactory.CP.TestIsolationCloseByCorrAlg(name + suff, **kwargs)
    result.addEventAlgo(the_alg, primary = True)
    return result

def IsoCloseByAlgsCfg(flags, suff = "", isPhysLite = False, containerNames = [ "Muons", "Electrons", "Photons"], stream_name="", ttva_wp = "Nonprompt_All_MaxWeight", useSelTools = False, isoDecSuffix = "CloseByCorr", hasLRT = False):

    # Add in two ways to do IsoCloseBy correction:
    #   - use IsoCloseByCorrAlg to modify the <iso_value>s for close by lepton/photon. 
    #     These can be used directly reading a derivation.
    #   - Also add in extra information to run IsolationCloseByTool on the derivation
    # For closeByIso correction, only one way is needed. The other way can be a cross check.
    # The second way will be eventually deprecated and is not used for PhysLite to minimize the 
    # information on PhysLite.
    acc = ComponentAccumulator()

    # REMOVING THE FOLLOWING UNTIL THE CLASH BETWEEN THESE ALGS AND THE CLOSEBY CORRECTION IS FIXED
    # ## Temporarily comment out for parent/child augmentation tests
    # # Add additional information to derivation output to be able to run IsoCloseByCorrectionTool on it 
    # if not isPhysLite:
    #     from IsolationSelection.IsolationSelectionConfig import IsoCloseByCorrSkimmingAlgCfg, IsoCloseByCaloDecorCfg
    #     ### Add the tracks that potentially pollute the isolation cones of others to the collection. 
    #     ### Question: Is the list of recommended TTVA working points used for isolation available somewhere?
    #     acc.merge(IsoCloseByCorrSkimmingAlgCfg(flags, suff = suff, ttva_wp = "Nonprompt_All_MaxWeight",
    #                                                         OutputStream = stream_name))
        
    #     ### Associate the close-by pflow objects and the calorimeter clusters
    #     acc.merge(IsoCloseByCaloDecorCfg(flags, suff = suff,
    #                                      containers = containerNames ))

    # Setup the isolation close-by correction algorithm sequence to correct the isolation of near-by el, mu, ph
    from IsolationSelection.IsolationSelectionConfig import IsoCloseByCorrAlgCfg
    acc.merge(IsoCloseByCorrAlgCfg(flags, suff = suff, isPhysLite = isPhysLite, containerNames = containerNames, useSelTools = useSelTools, isoDecSuffix = isoDecSuffix, hasLRT = hasLRT))


    return acc

def setupIsoCloseBySlimmingVariables(slimmingHelper, isLLP1 = False):

    ### Extra variables needed to make the isolation close-by correction work when running on derivation
    ### https://gitlab.cern.ch/atlas/athena/-/blob/master/PhysicsAnalysis/AnalysisCommon/IsolationSelection/Root/IsolationCloseByCorrectionTool.cxx#L23-30
    iso_corr_vars = [ "IsoCloseByCorr_assocClustEta", "IsoCloseByCorr_assocClustPhi", "IsoCloseByCorr_assocClustEnergy",
                "IsoCloseByCorr_assocClustDecor", "IsoCloseByCorr_assocPflowEta", "IsoCloseByCorr_assocPflowPhi", "IsoCloseByCorr_assocPflowEnergy",
                "IsoCloseByCorr_assocPflowDecor"]

    if isLLP1:
        slimmingHelper.ExtraVariables += ["Electrons."+(".".join(iso_corr_vars)),
                                          "LRTElectrons."+(".".join(iso_corr_vars)),
                                          "Muons."+(".".join(iso_corr_vars)),
                                          "MuonsLRT."+(".".join(iso_corr_vars)) ]
    else:
        slimmingHelper.ExtraVariables += ["Electrons."+(".".join(iso_corr_vars)),
                                          "Muons."+(".".join(iso_corr_vars)) ]
