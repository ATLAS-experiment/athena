#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from OutputStreamAthenaPool.OutputStreamConfig import addToAOD, addToESD
from JetRecConfig.JetDefinition import JetInputConstitSeq, JetInputConstit, xAODType, JetDefinition
from JetRecConfig.JetDefinition import JetModifier, JetInputExternal
#from JetRecConfig.JetRecConfig import getJetModifierTools 
from JetRecConfig.StandardJetMods import stdJetModifiers
from JetRecConfig import JetRecConfig
from JetRecConfig.DependencyHelper import solveDependencies
from HIGlobal.HIGlobalConfig import HIEventShapeMakerCfg, HIEventShapeMapToolCfg
from AthenaCommon.Logging import logging
__log = logging.getLogger('HIJetRecConfigCA')


def HIClusterMakerCfg(flags, save=False, **kwargs):
    """Function to equip HI cluster builder from towers and cells, adds to output AOD stream."""

    acc = ComponentAccumulator()

    kwargs.setdefault("CaloCellContainerKey", "AllCalo")
    kwargs.setdefault("OutputContainerKey", flags.HeavyIon.Jet.Internal.ClusterKey)

    # get towers
    from CaloRec.CaloRecoConfig import CaloRecoCfg
    acc.merge(CaloRecoCfg(flags))
    from CaloRec.CaloTowerMakerConfig import CaloTowerMakerCfg
    towerMaker = acc.getPrimaryAndMerge(CaloTowerMakerCfg(flags))
    tower_key = towerMaker.TowerContainerName

    HIClusterMaker = CompFactory.HIClusterMaker("HIClusterMaker",
                                                InputTowerKey=tower_key,
                                                **kwargs)

    if save:
        output = ["xAOD::CaloClusterContainer#"+kwargs["OutputContainerKey"],
                  "xAOD::CaloClusterAuxContainer#"+kwargs["OutputContainerKey"]+"Aux."]
        acc.merge(addToESD(flags, output))
        acc.merge(addToAOD(flags, output))

    acc.addEventAlgo(HIClusterMaker, primary=True)
    return acc


def HICaloJetInputConstitSeq(flags, name="HICaloConstit", **kwargs):
    kwargs.setdefault("objtype", xAODType.CaloCluster)
    kwargs.setdefault("modifiers", [])
    kwargs.setdefault("inputname", flags.HeavyIon.Jet.Internal.ClusterKey)
    kwargs.setdefault("outputname", flags.HeavyIon.Jet.Internal.ClusterKey)
    kwargs.setdefault("label", "HI")

    jetConstit = JetInputConstitSeq(name, **kwargs)

    from JetRecConfig.StandardJetConstits import stdConstitDic
    stdConstitDic.setdefault(name, jetConstit)

    return jetConstit


def HICaloJetDef(flags, jetradius, **kwargs):
    """Returns jet definition for calo jets, with already resolved dependencies."""

    ptmin_dict = {2:7000, 3:7000, 4:7000, 6:7000, 10:50000}
    if jetradius not in ptmin_dict and "ptmin" not in kwargs:
        __log.warning("jetradius "+str(jetradius)+" is not in the dictionary for ptmin; setting ptmin to 7000")
    kwargs.setdefault("ptmin", ptmin_dict.get(jetradius, 7000))
    kwargs.setdefault("radius", float(jetradius)/10.)
    kwargs.setdefault("inputdef", HICaloJetInputConstitSeq(flags))
    kwargs.setdefault("algorithm", "AntiKt")
    kwargs.setdefault("prefix", "")
    kwargs.setdefault("suffix", "")

    return solveDependencies(JetDefinition(**kwargs))


def HITrackJetInputConstit(flags, name="HITrack", **kwargs):
    kwargs.setdefault("objtype", xAODType.TrackParticle)
    kwargs.setdefault("containername", "JetSelectedTracks")

    jetConstit = JetInputConstit(name, **kwargs)
    return jetConstit


def HITrackJetDef(flags, jetradius, **kwargs):
    """Returns jet definition for track jets, with already resolved dependencies."""

    import JetRecTools.JetRecToolsConfig as jrtcfg
    JetInputExternal("JetSelectedTracks",
                     xAODType.TrackParticle,
                     # in std context, this is InDetTrackParticles (see StandardJetContext)
                     prereqs=["InDetTrackParticles"],
                     algoBuilder=lambda jdef, _: jrtcfg.getTrackSelAlg(jdef.context, trackSelOpt=False))
    kwargs.setdefault("ptmin", 5000)
    kwargs.setdefault("inputdef", HITrackJetInputConstit(flags))

    return HICaloJetDef(flags, jetradius=jetradius, **kwargs)


def HIPseudoJetAlgCfg(flags, **kwargs):
    """Creates a pseudo jet algorithm"""
    acc = ComponentAccumulator()

    kwargs.setdefault("name", "pjcs"+flags.HeavyIon.Jet.Internal.ClusterKey)
    kwargs.setdefault("InputContainer", flags.HeavyIon.Jet.Internal.ClusterKey)
    kwargs.setdefault("OutputContainer", "PseudoJet"+flags.HeavyIon.Jet.Internal.ClusterKey)
    kwargs.setdefault("Label", "LCTopo")
    kwargs.setdefault("SkipNegativeEnergy", False)
    kwargs.setdefault("TreatNegativeEnergyAsGhost", True)

    acc.addEventAlgo(CompFactory.PseudoJetAlgorithm(**kwargs))
    return acc


def HIPseudoTrackJetAlgCfg(flags, name="TrackPseudoJets", **kwargs):
    """Creates a pseudo track jet algorithm."""
    acc = ComponentAccumulator()

    kwargs.setdefault("InputContainer", "HIJetTracks")
    kwargs.setdefault("OutputContainer", "PseudoTracks")
    kwargs.setdefault("Label", "Tracks")
    kwargs.setdefault("SkipNegativeEnergy", True)

    acc.addEventAlgo(CompFactory.PseudoJetAlgorithm(name, **kwargs))
    return acc


def HIJetClustererCfg(flags, name="builder", jetDef=None, **kwargs):
    """Creates a tool for clustering."""
    acc = ComponentAccumulator()

    if jetDef is not None:
        kwargs.setdefault("JetAlgorithm", jetDef.algorithm)
        kwargs.setdefault("JetRadius", jetDef.radius)
        kwargs.setdefault("PtMin", jetDef.ptmin)
    kwargs.setdefault("GhostArea", 0.01)
    kwargs.setdefault("InputPseudoJets", "PseudoJet"+flags.HeavyIon.Jet.Internal.ClusterKey+"_GhostTracks")

    acc.setPrivateTools(CompFactory.JetClusterer(name, **kwargs))
    return acc


def HIJetAlgCfg(flags, jetDef, **kwargs):
    """Creates a jet reconstruction algorithm."""
    acc = ComponentAccumulator()

    if "Provider" not in kwargs:
        jclust = acc.popToolsAndMerge(HIJetClustererCfg(flags, jetDef=jetDef))
        kwargs.setdefault("Provider", jclust)
    if "Modifiers" not in kwargs:
        kwargs.setdefault("Modifiers", JetRecConfig.getJetModifierTools(jetDef))
    if "OutputContainer" not in kwargs:
        kwargs.setdefault("OutputContainer", jetDef.fullname())
    kwargs.setdefault("name", "JRA_build"+kwargs["OutputContainer"])

    acc.addEventAlgo(CompFactory.JetRecAlg(**kwargs))
    return acc


def HIJetCopierCfg(flags, name="builder_copy", **kwargs):
    """Creates a tool to copy jets."""
    acc = ComponentAccumulator()

    kwargs.setdefault("InputJets", "")
    kwargs.setdefault("DecorDeps", [])
    kwargs.setdefault("ShallowCopy", False)
    kwargs.setdefault("ShallowIO", False)

    acc.setPrivateTools(CompFactory.JetCopier(name, **kwargs))
    return acc


def HIJetCopyAlgCfg(flags, jetDef_in, jetDef, **kwargs):
    """Creates an algorithm to copy jets."""
    acc = ComponentAccumulator()

    if "Provider" not in kwargs:
        jcopy = acc.popToolsAndMerge(HIJetCopierCfg(flags, InputJets=jetDef_in.fullname()))
        kwargs.setdefault("Provider", jcopy)

    acc.merge(HIJetAlgCfg(flags, jetDef, **kwargs))
    return acc


def updateStdJetModifier(flags, name, **kwargs):
    """Updates the stdJetModifiers dictionary, based on the provided name. 
       Some of the modifiers expect certain kwargs. 
       Some of the modifiers ignore kwargs which makes the code simpler."""

    if "Filter:" in name:
        # already there, do nothing
        return
    if "HIJetCalib:" in name:
        # add generic "HIJetCalib" modifier
        if "HIJetCalib" not in stdJetModifiers:
            updateStdJetModifier(flags, "HIJetCalib", **kwargs)
        return
    if name in stdJetModifiers:
        # already there, do nothing
        return

    if name == "HIJetAssoc":
        stdJetModifiers.update(
            HIJetAssoc=JetModifier("HIJetDRAssociationTool",
                                   "HIJetDRAssociation",
                                   ContainerKey=flags.HeavyIon.Jet.Internal.ClusterKey,
                                   DeltaR=0.8,
                                   AssociationName=flags.HeavyIon.Jet.Internal.ClusterKey+"_DR8Assoc"))
        return

    if name == "HIJetMaxOverMean":
        if "jetDef" not in kwargs:
            __log.warning(
                "HIJetMaxOverMean needs 'jetDef' in its kwargs; HIJetMaxOverMean is not added to stdJetModifiers")
            return
        stdJetModifiers.update(
            HIJetMaxOverMean=JetModifier("HIJetMaxOverMeanTool",
                                         "HIJetMaxOverMean",
                                         JetContainer=kwargs["jetDef"].fullname()))
        return

    if name == "HIJetDiscrim":
        stdJetModifiers.update(
            HIJetDiscrim=JetModifier("HIJetDiscriminatorTool",
                                     "HIJetDiscriminator",
                                     MaxOverMeanCut=4,
                                     MinimumETMaxCut=3000))
        return

    if name == "subtr0":
        if "Modulator" not in kwargs or "EventShapeMapTool" not in kwargs or \
           "Subtractor" not in kwargs or "EventShapeKey" not in kwargs:
            __log.warning(
                "subtr0 needs 'Modulator', 'EventShapeMapTool', 'Subtractor', and 'EventShapeKey' in its kwargs; subtr0 is not added to stdJetModifiers")
            return

        stdJetModifiers.update(
            subtr0=JetModifier("HIJetConstituentSubtractionTool",
                               "HICS_HIEventShapeWeighted_iter0",
                               MomentName="JetSubtractedScaleMomentum",
                               SetMomentOnly=False,
                               ApplyOriginCorrection=True,
                               **kwargs))
        return

    if name == "HIJetCalib":
        stdJetModifiers.update(
            HIJetCalib=JetModifier("JetCalibrationTool",
                                   "HICalibTool_{modspec}",
                                   JetCollection=lambda _, modspec: "AntiKt{}HI".format(modspec.split('___')[0]) if modspec.split('___')[0] in flags.HeavyIon.Jet.CaliRValues else "AntiKt4HI",
                                   PrimaryVerticesContainerName="",
                                   ConfigFile='JES_MC16_HI_Jan2021_5TeV.config', # JES_MC16_HI_Jan2021_5TeV.config JES_MC15c_HI_Nov2016.config
                                   CalibSequence=lambda _, modspec: modspec.split('___')[1],
                                   IsData=lambda _, modspec: modspec.split('___')[2] == 'True'))
        return

    if name == "subtr1":
        if "Modulator" not in kwargs or "EventShapeMapTool" not in kwargs or \
           "Subtractor" not in kwargs or "EventShapeKey" not in kwargs:
            __log.warning(
                "subtr1 needs 'Modulator', 'EventShapeMapTool', 'Subtractor', and 'EventShapeKey' in its kwargs; subtr1 is not added to stdJetModifiers")
            return

        stdJetModifiers.update(
            subtr1=JetModifier("HIJetConstituentSubtractionTool",
                               "HICS_HIEventShapeWeighted_iter1",
                               MomentName="JetSubtractedScaleMomentum",
                               SetMomentOnly=False,
                               ApplyOriginCorrection=True,
                               **kwargs))
        return

    if name == "consmod":
        if "ClusterKey" not in kwargs or "Subtractor" not in kwargs:
            __log.warning(
                "consmod needs 'ClusterKey' and 'Subtractor' in its kwargs; consmod is not added to stdJetModifiers")
            return

        stdJetModifiers.update(
            consmod=JetModifier(
                "HIJetConstituentModifierTool",
                "HIJetConstituentModifierTool_final",
                ApplyOriginCorrection=True,
                **kwargs))
        return

    __log.warning("updateStdJetModifier does not know modifier "+
                  name+"; it is not added to stdJetModifiers")
    return


def HIJetDefCloner(flags, jetDef_in, **kwargs):
    """Clones jet definitions based on the template.
       Updates stdJetModifiers if necessary.
       Overwrites suffix and modifiers, and return jet definition with already resolved dependencies."""

    jetDef_new = jetDef_in.clone()

    if "suffix" in kwargs:
        jetDef_new.suffix = kwargs["suffix"]
    if "modifiers" in kwargs:
        jetDef_new.modifiers = []
        for modifier in kwargs["modifiers"]:
            updateStdJetModifier(flags, modifier, jetDef=jetDef_new)
            jetDef_new.modifiers.append(modifier)

    return solveDependencies(jetDef_new)


def NullModulatorCfg():
    """Provides modulator tool without any modulations."""
    acc = ComponentAccumulator()
    acc.setPrivateTools(CompFactory.HIUEModulatorTool('NullUEModulator',
                                                      EventShapeKey='',
                                                      DoV2=False,
                                                      DoV3=False,
                                                      DoV4=False))
    return acc


def HIModulatorCfg(flags, mod_key, suffix=None, **kwargs):
    """Provides modulator tool."""

    kwargs.setdefault("harmonics", flags.HeavyIon.Jet.HarmonicsForSubtraction)
    if len(kwargs["harmonics"]) == 0:
        return NullModulatorCfg()
    acc = ComponentAccumulator()
    kwargs.setdefault("name", "Modulator_"+mod_key+
                      "".join(["_V"+str(h) for h in kwargs["harmonics"]]))
    if suffix is not None:
        kwargs["name"] += '_'+suffix
    kwargs.setdefault("DoV2", 2 in kwargs["harmonics"])
    kwargs.setdefault("DoV3", 3 in kwargs["harmonics"])
    kwargs.setdefault("DoV4", 4 in kwargs["harmonics"])
    del kwargs["harmonics"]
    kwargs.setdefault("EventShapeKey", mod_key)

    acc.setPrivateTools(CompFactory.HIUEModulatorTool(**kwargs))

    if 'label' in kwargs:
        label = kwargs['label']
        for key in kwargs:
            if key not in ["name", "DoV2", "DoV3", "DoV4", "EventShapeKey"]:
                del kwargs[key]

        stdJetModifiers[label] = JetModifier("HIUEModulatorTool", **kwargs)

    return acc


def HIJetClusterSubtractorCfg(flags, name="HIJetClusterSubtractor", **kwargs):
    """Provides tool for cluster subtraction."""
    acc = ComponentAccumulator()

    kwargs.setdefault("ConfigDir", "HIJetCorrection/")
    kwargs.setdefault("UseSamplings", False)
    if "InputFile" not in kwargs:
        from HIJetRec.HIJetRecUtilsCA import getHIClusterGeoWeightFile
        kwargs.setdefault("InputFile", getHIClusterGeoWeightFile(flags))

    acc.setPrivateTools(CompFactory.HIJetClusterSubtractorTool(name, **kwargs))
    return acc


def HIJetCellSubtractorCfg(flags, name="HIJetCellSubtractor", **kwargs):
    """Provides tool for cell subtraction."""
    acc = ComponentAccumulator()

    acc.setPrivateTools(CompFactory.HIJetCellSubtractorTool(name, **kwargs))
    return acc


def HIJetSubtractorCfg(flags, useClusters, **kwargs):
    """Common function for clsuter and cell subtraction configuration."""

    if useClusters:
        return HIJetClusterSubtractorCfg(flags, **kwargs)
    else:
        return HIJetCellSubtractorCfg(flags, **kwargs)


def HIEventShapeJetIterationCfg(flags, suffix=None, useClusters=True, **kwargs):
    """Provides tool for event shape iteration.
       Also saves some tool, so they can be used later."""
    acc = ComponentAccumulator()

    kwargs.setdefault("InputEventShapeKey", "HIEventShape_Weighted")
    kwargs.setdefault("CaloJetSeedContainerKey", "")

    out_shape_name = kwargs["InputEventShapeKey"]
    if suffix is not None:
        out_shape_name += '_'+suffix
    mod_shape_key = out_shape_name+'_Modulate'

    if 'Modulator' not in kwargs:
        modulator = acc.popToolsAndMerge(HIModulatorCfg(
            flags, mod_key=mod_shape_key, suffix=suffix))
        kwargs.setdefault('Modulator', modulator)
    if "Subtractor" not in kwargs:
        sub_tool = acc.popToolsAndMerge(HIJetSubtractorCfg(flags, useClusters))
        kwargs.setdefault("Subtractor", sub_tool)
    if "EventShapeMapTool" not in kwargs:
        map_tool = acc.popToolsAndMerge(HIEventShapeMapToolCfg(flags))
        kwargs.setdefault("EventShapeMapTool", map_tool)
    kwargs.setdefault("OutputEventShapeKey", out_shape_name)
    kwargs.setdefault("AssociationKey", flags.HeavyIon.Jet.Internal.ClusterKey+"_DR8Assoc")
    kwargs.setdefault("ModulationScheme", 1)
    kwargs.setdefault("RemodulateUE", True)
    kwargs.setdefault("ShallowCopy", False)
    kwargs.setdefault("ModulationEventShapeKey", mod_shape_key)
    kwargs.setdefault("TrackJetSeedContainerKey", "")

    acc.setPrivateTools(CompFactory.HIEventShapeJetIteration(
        "HIJetIteration_"+out_shape_name, **kwargs))

    # save some tools for later
    jm_dict = {"Modulator": kwargs["Modulator"],
               "EventShapeMapTool": kwargs["EventShapeMapTool"],
               "Subtractor": kwargs["Subtractor"],
               "EventShapeKey": kwargs["OutputEventShapeKey"]}

    return jm_dict, acc


def HITrackSelAlgCfg(flags, name="TrackSelAlgHI", **kwargs):
    """Provides track selection algorithm for track jet reconstruction."""
    acc = ComponentAccumulator()

    if "TrackSelector" not in kwargs:
        from InDetConfig.InDetTrackSelectionToolConfig import HI_InDetTrackSelectionToolForHITrackJetsCfg
        tracksel = acc.popToolsAndMerge(HI_InDetTrackSelectionToolForHITrackJetsCfg(flags))
        kwargs.setdefault("TrackSelector", tracksel)
    kwargs.setdefault("InputContainer", "InDetTrackParticles")
    kwargs.setdefault("OutputContainer", "HIJetTracks")
    kwargs.setdefault("DecorDeps", [])

    acc.addEventAlgo(CompFactory.JetTrackSelectionAlg(name, **kwargs))
    return acc


def HICaloClusterMomentsCfg(flags, name="HIClusterMoments", **kwargs):
    """Provides tool for cluster moments."""
    acc = ComponentAccumulator()

    kwargs.setdefault("MinBadLArQuality", 4000)
    kwargs.setdefault("MomentsNames", ["CENTER_MAG",
                                       "LONGITUDINAL",
                                       "FIRST_ENG_DENS",
                                       "SECOND_ENG_DENS",
                                       "ENG_FRAC_EM",
                                       "ENG_FRAC_MAX",
                                       "ENG_FRAC_CORE",
                                       "ENG_BAD_CELLS",
                                       "N_BAD_CELLS",
                                       "N_BAD_CELLS_CORR",
                                       "BAD_CELLS_CORR_E",
                                       "BADLARQ_FRAC",
                                       "ENG_POS",
                                       "SIGNIFICANCE",
                                       "CELL_SIGNIFICANCE",
                                       "CELL_SIG_SAMPLING",
                                       "AVG_LAR_Q",
                                       "AVG_TILE_Q",
                                       "ENG_BAD_HV_CELLS",
                                       "N_BAD_HV_CELLS"])

    acc.setPrivateTools(CompFactory.CaloClusterMomentsMaker(name, **kwargs))
    return acc


def HISubtractionToCellsCfg(flags, name="HIClusterSubtraction", **kwargs):
    """Provides tool for cell subtraction.
       Re-uses tool for cluster subtraction."""

    return HISubtractionToClustersCfg(flags, name, useClusters=False, **kwargs)


def HISubtractionToClustersCfg(flags, name="HIClusterSubtraction", useClusters=True, **kwargs):
    """Provides tool for cluster subtraction."""
    acc = ComponentAccumulator()

    kwargs.setdefault('EventShapeKey', 'EventShapeKey')
    kwargs.setdefault('ClusterKey', flags.HeavyIon.Jet.Internal.ClusterKey)
    kwargs.setdefault('OutClusterKey', 'ClusterKey_deep')
    kwargs.setdefault('UpdateOnly', False)
    kwargs.setdefault('ApplyOriginCorrection', True)
    kwargs.setdefault('SetMoments', False)
    if 'Modulator' not in kwargs:
        modulator = acc.popToolsAndMerge(NullModulatorCfg())
        kwargs.setdefault('Modulator', modulator)
    if "EventShapeMapTool" not in kwargs:
        map_tool = acc.popToolsAndMerge(HIEventShapeMapToolCfg(flags))
        kwargs.setdefault("EventShapeMapTool", map_tool)
    if "Subtractor" not in kwargs:
        sub_tool = acc.popToolsAndMerge(HIJetSubtractorCfg(flags, useClusters))
        kwargs.setdefault("Subtractor", sub_tool)
    if kwargs["SetMoments"] and "ClusterCorrectionTools" not in kwargs:
        clusterCorrectionTools = acc.popToolsAndMerge(
            HICaloClusterMomentsCfg(flags, "HIClusterMoments"))
        kwargs.setdefault("ClusterCorrectionTools", [clusterCorrectionTools])

    acc.setPrivateTools(CompFactory.HIClusterSubtraction(name, **kwargs))
    return acc


def HIJetRecCfg(flags):
    """Configures Heavy Ion Jet reconstruction."""
    acc = ComponentAccumulator()

    # get HIClusters
    acc.merge(HIClusterMakerCfg(flags))

    # get weighted event shape
    eventshapeKey = "HIEventShapeWeighted"
    acc.merge(HIEventShapeMakerCfg(flags,
                                   name="HIEventShapeMaker_Weighted",
                                   doWeighted=True,
                                   InputTowerKey=flags.HeavyIon.Jet.Internal.ClusterKey,
                                   OutputContainerKey=eventshapeKey))

    # get jet definition
    # R=0.2 calojets are use as seeds for UE subtraction
    jetDef2 = HICaloJetDef(flags, jetradius=2, suffix="_Unsubtracted")

    # get jet definitions for physics
    jetDef = []
    jetRlist = flags.HeavyIon.Jet.RValues #Default [0.2,0.4], Others R's should be build in Derivations or pass in preExec
    for jetR in jetRlist:
        jetDef.append(HICaloJetDef(flags, jetradius=jetR, suffix="_Unsubtracted"))
        __log.info("HI Jet Collection for Reco: "+jetDef[-1].fullname())

    # get calo pseudojets
    acc.merge(HIPseudoJetAlgCfg(flags))

    # HIJetTracks are used for GhostTracks association and later to build TrackJets seeds
    pseudoTrkJetCont = "HIJetTracks"
    pseudoGhostTrks = "PseudoJetGhostTracks"

    acc.merge(HITrackSelAlgCfg(flags, OutputContainer=pseudoTrkJetCont))
    acc.merge(HIPseudoTrackJetAlgCfg(
        flags, name="GhostTrackPseudoJets", InputContainer=pseudoTrkJetCont, OutputContainer=pseudoGhostTrks, Label="GhostTrack"))

    ## merge between PJHICluster and PJTracks

    pjContNames = ["PseudoJet"+flags.HeavyIon.Jet.Internal.ClusterKey,pseudoGhostTrks]

    mergeAlg = CompFactory.PseudoJetMerger(
        "PJmerge_HIGhostTrack",
        InputPJContainers = pjContNames,
        OutputContainer = "PseudoJet"+flags.HeavyIon.Jet.Internal.ClusterKey+"_GhostTracks"
    )
    acc.addEventAlgo(mergeAlg)

    # build jets
    acc.merge(HIJetAlgCfg(flags, jetDef=jetDef2))
    for jd in jetDef:
        acc.merge(HIJetAlgCfg(flags, jetDef=jd))

    # copy unsubtracted jets; create seed0
    jetDef_seed0 = HIJetDefCloner(flags, jetDef_in=jetDef2,
                                  suffix="_seed0",
                                  modifiers=["HIJetAssoc", "HIJetMaxOverMean", "HIJetDiscrim", "Filter:5000"])
    acc.merge(HIJetCopyAlgCfg(flags, jetDef2, jetDef_seed0))

    # first iteration, iter0
    jm_dict0, acc_iter0 = HIEventShapeJetIterationCfg(flags,
                                                      suffix="iter0",
                                                      InputEventShapeKey=eventshapeKey,
                                                      CaloJetSeedContainerKey=jetDef_seed0.fullname())
    iter0 = acc.popToolsAndMerge(acc_iter0)
    acc.addEventAlgo(CompFactory.JetAlgorithm("jetalgHI_iter0", Tools=[iter0]))

    # jet modifier from the first iteration
    updateStdJetModifier(flags, "subtr0", **jm_dict0)

    # set jet energy scale configuration
    calib_seq = "EtaJES"
    if not flags.Input.isMC:
        calib_seq += "_Insitu"

    # copy jets from the first iteration; create seed1
    jetDef_seed1 = HIJetDefCloner(flags, jetDef_in=jetDef2,
                                  suffix="_seed1",
                                  modifiers=["HIJetAssoc", "subtr0", "HIJetCalib:{}___{}___{}".format(2, calib_seq, not flags.Input.isMC), "Filter:{}".format(flags.HeavyIon.Jet.SeedPtMin)])
    acc.merge(HIJetCopyAlgCfg(flags, jetDef2, jetDef_seed1))

    # configuring track jets, seeds for second iteration
    if flags.HeavyIon.Jet.doTrackJetSeed:
        pseudoTrks = "PseudoTracks"

        acc.merge(HIPseudoTrackJetAlgCfg(
            flags, InputContainer=pseudoTrkJetCont, OutputContainer=pseudoTrks))

        jetDef_trk = HITrackJetDef(flags,
                                      jetradius=4,
                                      modifiers=["HIJetAssoc", "Filter:{}".format(flags.HeavyIon.Jet.TrackJetPtMin),"Sort"])
        trkJetSeedCont = jetDef_trk.fullname()

        trkJetClust = acc.popToolsAndMerge(HIJetClustererCfg(flags, 
                                                             jetDef=jetDef_trk, 
                                                             InputPseudoJets=pseudoTrks))
        acc.merge(HIJetAlgCfg(flags, jetDef_trk, Provider=trkJetClust))
    else:
        trkJetSeedCont = ""

    # second iteration, iter1
    jm_dict1, acc_iter1 = HIEventShapeJetIterationCfg(flags,
                                                      suffix="iter1",
                                                      InputEventShapeKey=eventshapeKey,
                                                      CaloJetSeedContainerKey=jetDef_seed1.fullname(),
                                                      TrackJetSeedContainerKey=trkJetSeedCont)
    iter1 = acc.popToolsAndMerge(acc_iter1)
    acc.addEventAlgo(CompFactory.JetAlgorithm("jetalgHI_iter1", Tools=[iter1]))

    # event shape for egamma
    jm_dict1_eg, acc_iter1_eg = HIEventShapeJetIterationCfg(flags,
                                                            suffix="iter_egamma",
                                                            useClusters=False,
                                                            InputEventShapeKey=flags.HeavyIon.Global.EventShape,
                                                            OutputEventShapeKey=flags.HeavyIon.Egamma.EventShape,
                                                            CaloJetSeedContainerKey=jetDef_seed1.fullname())
    iter1_eg = acc.popToolsAndMerge(acc_iter1_eg)
    acc.addEventAlgo(CompFactory.JetAlgorithm("jetalgHI_iter1_egamma", Tools=[iter1_eg]))

    # constituents subtraction for egamma, cell-level
    cluster_key_eGamma_deep = flags.HeavyIon.Jet.Internal.ClusterKey+"_eGamma_deep"
    subtrToCelltool = acc.popToolsAndMerge(
        HISubtractionToCellsCfg(flags,
                                name="HIClusterSubtraction_egamma",
                                EventShapeKey=jm_dict1_eg["EventShapeKey"],
                                OutClusterKey=cluster_key_eGamma_deep,
                                Modulator=jm_dict1["Modulator"],
                                EventShapeMapTool=jm_dict1["EventShapeMapTool"],
                                SetMoments=True,
                                ApplyOriginCorrection=False)
    )
    acc.addEventAlgo(CompFactory.JetAlgorithm("jetalgHI_subtrToCellTool", Tools=[subtrToCelltool]))

    # jet modifier from the second iteration
    updateStdJetModifier(flags, "subtr1", **jm_dict1)

    # constituents subtraction for jets, tower-level
    subtrToClusterTool = acc.popToolsAndMerge(
        HISubtractionToClustersCfg(flags,
                                   name="HIClusterSubtraction_final",
                                   EventShapeKey=jm_dict1["EventShapeKey"],
                                   ClusterKey=cluster_key_eGamma_deep,
                                   OutClusterKey=flags.HeavyIon.Jet.ClusterKey,
                                   Modulator=jm_dict1["Modulator"],
                                   EventShapeMapTool=jm_dict1["EventShapeMapTool"],
                                   ApplyOriginCorrection=False)
    )
    acc.addEventAlgo(CompFactory.JetAlgorithm(
        "jetalgHI_subtrToClusterTool", Tools=[subtrToClusterTool]))

    # jet modifier from the tower-level subtraction
    updateStdJetModifier(flags, "consmod",
                         ClusterKey=flags.HeavyIon.Jet.ClusterKey,
                         Subtractor=jm_dict1["Subtractor"])

    # store final version of HIClusters
    if flags.HeavyIon.Jet.WriteHIClusters:
        output_hicluster = ["xAOD::CaloClusterContainer#"+flags.HeavyIon.Jet.ClusterKey,
                            "xAOD::CaloClusterAuxContainer#"+flags.HeavyIon.Jet.ClusterKey+"Aux."]
        acc.merge(addToESD(flags, output_hicluster))
        acc.merge(addToAOD(flags, output_hicluster))

    # configure final jets and store them
    extramods = ["Sort","Width","CaloEnergies","LArHVCorr","CaloQuality","TrackMoments","JVF","JVT"]# adding modifiers to final jets
    for jd in jetDef:
        jetDef_final = HIJetDefCloner(flags,
                                      jetDef_in=jd,
                                      suffix="",
                                      modifiers=["subtr1", "consmod", "HIJetCalib:{}___{}___{}".format(str(float(jd.radius)*10).replace('.0',''),calib_seq, not flags.Input.isMC), "Filter:{}".format(flags.HeavyIon.Jet.RecoOutputPtMin)]+extramods)
        acc.merge(HIJetCopyAlgCfg(flags, jd, jetDef_final))

        output = ["xAOD::JetContainer#"+jetDef_final.fullname(),
                  "xAOD::JetAuxContainer#"+jetDef_final.fullname()+"Aux.-PseudoJet"]
        acc.merge(addToESD(flags, output))
        acc.merge(addToAOD(flags, output))

    # store track jets
    if flags.HeavyIon.Jet.doTrackJetSeed:
        output = ["xAOD::JetContainer#"+jetDef_trk.fullname(),
                  "xAOD::JetAuxContainer#"+jetDef_trk.fullname()+"Aux.-PseudoJet"]
        acc.merge(addToESD(flags, output))
        acc.merge(addToAOD(flags, output))

    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles, defaultGeometryTags
    flags = initConfigFlags()
    flags.Input.Files = [defaultTestFiles.d + "/RecJobTransformTests/data18_hi.00367384.physics_HardProbes.daq.RAW._lb0145._SFO-8._0001.data"]
    flags.Exec.MaxEvents = 5
    flags.Concurrency.NumThreads = 1

    # enable unit tests to switch only parts of reco such as (note the absence of spaces around equal sign):
    ### python -m HIRecConfig.HIRecConfig HeavyIon.doGlobal="False" GeoModel.AtlasVersion="ATLAS-R3S-2021-03-01-00"
    flags.fillFromArgs()
    
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    acc = MainServicesCfg(flags)
    from InDetConfig.TrackRecoConfig import InDetTrackRecoCfg
    acc.merge(InDetTrackRecoCfg(flags))

    ## To avoid error coming from modifiers
    from JetRecConfig.JetConfigFlags import jetInternalFlags
    jetInternalFlags.isRecoJob = True

    from JetRecConfig.JetRecConfig import JetRecCfg
    from JetRecConfig.StandardSmallRJets import AntiKt4EMTopo
    EMjet = AntiKt4EMTopo.clone()
    EMjet.ghostdefs = ["Track"]
    acc.merge(JetRecCfg(flags,EMjet))
    ##
    from HIGlobal.HIGlobalConfig import HIGlobalRecCfg
    acc.merge(HIGlobalRecCfg(flags))

    acc.merge(HIJetRecCfg(flags))

    acc.printConfig(withDetails=True, summariseProps=True)
    flags.dump()

    import sys
    sys.exit(acc.run().isFailure())
