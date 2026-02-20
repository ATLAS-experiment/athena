#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from JetRecConfig import JetRecConfig
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

from TrigEDMConfig.TriggerEDM import recordable

from . import JetRecoCommon
from JetRecConfig.JetDefinition import JetModifier
from JetRecConfig.JetRecConfig import getJetCopyAlg

from AthenaCommon.Logging import logging
logging.getLogger().info("JetHIConfig LOG: Importing %s",__name__)
log = logging.getLogger(__name__)


# For now all duplicated from jetHIRecoSequenceCA -- to be cleaned up by experts
def HeavyIonJetRecoDataDeps(flags, **jetRecoDict):
    # The template unsubtracted jet definition
    jetNamePrefix = JetRecoCommon.getHLTPrefix()
    clustersKey = JetRecoCommon.getClustersKey(jetRecoDict)
    jetDef_unsub = JetRecoCommon.defineHIJets(jetRecoDict,clustersKey=clustersKey,prefix=jetNamePrefix,suffix="_Unsubtracted")
    jetDef_unsub._internalAtt['finalPJContainer'] = "PseudoJet"+clustersKey
    target_jetReco = f"_for_{jetRecoDict['jetDefStr']}"

    # Seed jets for subtraction
    associationName = "%s_DR8Assoc" % (clustersKey)
    jetDef_seed0 = jetDef_unsub.clone()
    jetDef_seed0.suffix = jetDef_unsub.suffix.replace("Unsubtracted", "seed0"+target_jetReco)
    jetDef_seed0.radius = 0.2
    jetsFullName_seed0 = jetDef_seed0.fullname()

    # Add this to the standard modifiers as we use this repeatedly
    stdJetModifiers.update(
        # we give a function as PtMin : it will be evaluated when instantiating the tool (modspec will come alias usage like "Filter:10000" --> PtMin=100000) 
        HLTHIJetAssoc = JetModifier("HIJetDRAssociationTool","HIJetDRAssociation", ContainerKey=clustersKey, DeltaR=0.8, AssociationName=associationName),
    )

    # These mods are custom and used only here
    jetDef_seed0.modifiers=[
        "HLTHIJetAssoc",
        JetModifier("HIJetMaxOverMeanTool","HIJetMaxOverMean"+target_jetReco, JetContainer = jetsFullName_seed0),
        JetModifier("HIJetDiscriminatorTool","HIJetDiscriminator", MaxOverMeanCut = 4, MinimumETMaxCut=3000),
        "Filter:5000"
    ]

    JES_is_data=False
    calib_seq='EtaJES' #only do in situ for R=0.4 jets in data
    if jetRecoDict["jetCalib"].endswith("IS") and not flags.Input.isMC:
         JES_is_data=True
         calib_seq += "_Insitu"

    # Second seed
    SeedPtMin = 25000
    if jetRecoDict["ionopt"] == "ionp":
        SeedPtMin = 8000

    # Copy default jets: seed1
    jetDef_seed1 = jetDef_unsub.clone()
    jetDef_seed1.suffix = jetDef_seed0.suffix.replace("_seed0","_seed1")
    jetDef_seed1.radius = 0.2
    jetDef_seed1.modifiers=["HLTHIJetAssoc", "HLTHIJetConstSub_iter0:iter0", "HLTHIJetSeedCalib:{}___{}".format(calib_seq, JES_is_data), "Filter:{}".format(SeedPtMin)]

    # Final subracted jets
    RecoOutputPtMin = 20000
    if jetRecoDict["ionopt"] == "ionp":
        RecoOutputPtMin = 10000
    jetDef_final = jetDef_unsub.clone()
    jetDef_final.suffix = jetDef_unsub.suffix.replace("_Unsubtracted","")
    jetDef_final.modifiers=["HLTHIJetConstSub_iter1:iter1", "HLTHIJetJetConstMod_iter1", "HLTHIJetCalib:{}___{}".format(calib_seq, JES_is_data), "Sort", "Filter:{}".format(RecoOutputPtMin)]

    jetDefDict = {
        "unsub": (jetDef_unsub.fullname(), jetDef_unsub),
        "seed0": (jetDef_seed0.fullname(), jetDef_seed0),
        "seed1": (jetDef_seed1.fullname(), jetDef_seed1),
        "final": (jetDef_final.fullname(), jetDef_final),
    }
    return jetDefDict

def jetHIEventShapeSequenceCA(configFlags, clustersKey, towerKey,**jetRecoDict):
    acc = ComponentAccumulator()
    
    #Import the map tool - it will have to harvest configuration along the path
    eventShapeMapToolKey="HLTHIEventShapeMapTool"
    theMapTool=CompFactory.HIEventShapeMapTool(eventShapeMapToolKey)

    #Make new event shape at tower level
    EventShapeKey='HLTHIEventShapeWeighted'
    ESAlg_W=CompFactory.HIEventShapeMaker("ESAlg_W")
    ESAlg_W.OutputContainerKey=EventShapeKey
    ESAlg_W.InputTowerKey=clustersKey
    ESAlg_W.NaviTowerKey=towerKey
        
    #Add filler tool
    ESFiller=CompFactory.HIEventShapeFillerTool("WeightedFiller")
    ESFiller.UseClusters=True
    
    #Add weight tool to filler tool
    TWTool=CompFactory.HITowerWeightTool()
    TWTool.ApplyCorrection={"ion": True, "ionp": False}.get(jetRecoDict["ionopt"])
    TWTool.ConfigDir='HIJetCorrection/'
    from HIJetRec.HIJetRecUtilsCA import getHIClusterGeoWeightFile
    TWTool.InputFile=getHIClusterGeoWeightFile(configFlags)

    ESFiller.TowerWeightTool=TWTool
    ESFiller.EventShapeMapTool=theMapTool
    #Add to top sequence
    ESAlg_W.HIEventShapeFillerTool=ESFiller
    acc.addEventAlgo(ESAlg_W)

    return acc, EventShapeKey, theMapTool


# JetModifier dictionary
from JetRecConfig.StandardJetMods import stdJetModifiers
stdJetModifiers.update(
    HLTHIJetCalib = JetModifier("JetCalibrationTool",
                                "HLTHICalibTool_{modspec}",
                                JetCollection=lambda _, modspec: modspec.split('___')[2] if len(modspec.split('___')) > 2 else "AntiKt4HI",
                                PrimaryVerticesContainerName="",
                                ConfigFile='JES_MC16_HI_Jan2021_5TeV.config',
                                CalibSequence=lambda _, modspec: modspec.split('___')[0],
                                IsData=lambda _, modspec: modspec.split('___')[1] == 'True'),
    )

stdJetModifiers.update(
    HLTHIJetSeedCalib = JetModifier("JetCalibrationTool",
                                "HLTHISeedCalibTool_{modspec}",
                                JetCollection="AntiKt2HI",
                                PrimaryVerticesContainerName="",
                                ConfigFile='JES_MC16_HI_Jan2021_5TeV.config',
                                CalibSequence=lambda _, modspec: modspec.split('___')[0],
                                IsData=lambda _, modspec: modspec.split('___')[1] == 'True'),
    )

def jetHIRecoSequenceCA(configFlags, clustersKey, towerKey, **jetRecoDict):
    """This build the standard heavy ion style jet.

    This is similar to JetRecConfig.getJetDefAlgs(). However due to how the alg flow is organized in the
    chain steps, we can't use this function directly.
    Instead we
      - construct a JetDefinition
      - use lower-level function in JetRecConfig with this JetDefinition to get the necessary algs and build our sequence manually.

    """
    acc = ComponentAccumulator()

    if jetRecoDict["ionopt"] == "noion":
         raise ValueError("Jet reco for heavy ion called without a ion option!")

    dataSource = "mc" if configFlags.Input.isMC else "data"

    jetHIEvtShapeSequence, eventShapeKey, eventShapeMapTool = jetHIEventShapeSequenceCA(configFlags, clustersKey=clustersKey, towerKey=towerKey, **jetRecoDict)
    acc.merge(jetHIEvtShapeSequence)

    jetNamePrefix = JetRecoCommon.getHLTPrefix()
    jetDef = JetRecoCommon.defineHIJets(jetRecoDict,clustersKey=clustersKey,prefix=jetNamePrefix,suffix="_Unsubtracted")
    jetsFullName_Unsub = jetDef.fullname()
    
    #This is used for calibration and monitoring
    jet_collection_name = jetsFullName_Unsub.split('_')[1].replace("Jets", "")
    
    # Add the PseudoJetGetter alg to the sequence
    pjgalg = CompFactory.PseudoJetAlgorithm(
        "pjgalg_HI",
        InputContainer = clustersKey,
        OutputContainer = "PseudoJet"+clustersKey,
        Label = "pjgHLTHI",
        SkipNegativeEnergy = False,
        TreatNegativeEnergyAsGhost=True
        )
    acc.addEventAlgo(pjgalg)
    finalpjs = str(pjgalg.OutputContainer)

    # Set the name of the final PseudoJetContainer to be used as input :
    jetDef._internalAtt['finalPJContainer'] = finalpjs

    ## Get online monitoring tool
    from JetRec import JetOnlineMon
    monTool = JetOnlineMon.getMonTool_TrigJetAlgorithm(configFlags, "HLTJets/" + jet_collection_name + "/")

    # Reconstruction 
    jetRecAlg = getHIJetRecAlg(jetDef, jetsFullName_Unsub, monTool=monTool)
    acc.addEventAlgo(jetRecAlg)

    associationName = "%s_DR8Assoc" % (clustersKey)

    jetsInUnsub = jetsFullName_Unsub

    JES_is_data=False
    calib_seq='EtaJES' #only do in situ jets in data
    if jetRecoDict["jetCalib"].endswith("IS") and (dataSource=="data"):
         JES_is_data=True
         calib_seq += "_Insitu"

    target_jetReco = f'_for_{jetRecoDict["jetDefStr"]}'

    SeedPtMin = 25000
    vnharmonics = [2, 3, 4]
    if jetRecoDict["ionopt"] == "ionp":
        SeedPtMin = 8000
        vnharmonics = []

    # Copy unsubtracted jets: seed0
    jetDef_seed0 = jetDef.clone()
    jetDef_seed0.suffix = jetDef.suffix.replace("Unsubtracted", "seed0"+target_jetReco)
    jetDef_seed0.radius = 0.2
    jetsFullName_seed0 = jetDef_seed0.fullname()
    jetDef_seed0.modifiers=[
        JetModifier("HIJetDRAssociationTool","HIJetDRAssociation", ContainerKey=clustersKey, DeltaR=0.8, AssociationName=associationName),
        JetModifier("HIJetMaxOverMeanTool","HIJetMaxOverMean"+target_jetReco, JetContainer = jetsFullName_seed0),
        JetModifier("HIJetDiscriminatorTool","HIJetDiscriminator", MaxOverMeanCut = 4, MinimumETMaxCut=3000),
        "Filter:5000"
    ]

    copySeed0Alg = getJetCopyAlg(jetsin=jetsInUnsub,jetsoutdef=jetDef_seed0,decorations=[],shallowcopy=False,shallowIO=False,monTool=monTool)
    acc.addEventAlgo(copySeed0Alg)

    # First iteration!
    iter0=HLTAddIteration(configFlags, jetsFullName_seed0, eventShapeKey, clustersKey, map_tool=eventShapeMapTool, assoc_name=associationName, suffix="iter0"+target_jetReco, harmonics = vnharmonics) # subtract UE from jets
    acc.addEventAlgo(HLTRunTools([iter0], "jetalgHI_iter0"+target_jetReco))
    modulator0=iter0.Modulator
    subtractor0=iter0.Subtractor

    HLTMakeSubtractionTool(configFlags, iter0.OutputEventShapeKey, Modulator=modulator0, EventShapeMapTool=eventShapeMapTool, Subtractor=subtractor0, label="HLTHIJetConstSub_iter0"+target_jetReco)

    cluster_key_iter0_deep=clustersKey+"_iter0_temp"+target_jetReco
    happy_iter0_Tool = ApplySubtractionToClustersHLT(configFlags, EventShapeKey="HLTHIEventShapeWeighted_iter0"+target_jetReco, ClusterKey=clustersKey, OutClusterKey=cluster_key_iter0_deep, Modulator=modulator0, EventShapeMapTool=eventShapeMapTool, Subtractor=subtractor0, SetMoments=False, ApplyOriginCorrection=False)
    acc.addEventAlgo(HLTRunTools([happy_iter0_Tool], "jetalgHI_clusterSub_iter0"+target_jetReco))

    GetConstituentsModifierToolHLT(configFlags, name="HIJetConstituentModifierTool", ClusterKey=cluster_key_iter0_deep, ApplyOriginCorrection=False, label="HLTHIJetJetConstMod_iter0"+target_jetReco)

    # Copy default jets: seed1
    # Second seed
    jetDef_seed1 = jetDef.clone()
    jetDef_seed1.suffix = jetDef_seed0.suffix.replace("_seed0","_seed1")
    jetDef_seed1.radius = 0.2
    jetDef_seed1.modifiers=["HLTHIJetAssoc", f"HLTHIJetConstSub_iter0{target_jetReco}:iter0", "HLTHIJetSeedCalib:{}___{}".format(calib_seq, JES_is_data), "Filter:{}".format(SeedPtMin)]
    jetsFullName_seed1 = jetDef_seed1.fullname()
    copySeed1Alg = getJetCopyAlg(jetsin=jetsInUnsub,jetsoutdef=jetDef_seed1,decorations=[],shallowcopy=False,shallowIO=False,monTool=monTool)
    acc.addEventAlgo(copySeed1Alg)

    iter1=HLTAddIteration(configFlags, jetsFullName_seed1, eventShapeKey, clustersKey, map_tool=eventShapeMapTool, assoc_name=associationName, sub_tool=subtractor0, suffix="iter1"+target_jetReco, harmonics = vnharmonics)
    iter1.OutputEventShapeKey="HLTHIEventShape_iter1"+target_jetReco
    modulator1=iter1.Modulator
    subtractor1=iter1.Subtractor

    HLTMakeSubtractionTool(configFlags, iter1.OutputEventShapeKey, Modulator=modulator1, EventShapeMapTool=eventShapeMapTool, label="HLTHIJetConstSub_iter1"+target_jetReco)

    acc.addEventAlgo(HLTRunTools([iter1], "jetalgHI_clusterSub_egamma"+target_jetReco))

    # 
    cluster_key_final_deep=clustersKey+"_final"+target_jetReco
    subToClusterTool = ApplySubtractionToClustersHLT(configFlags, EventShapeKey="HLTHIEventShape_iter1"+target_jetReco, ClusterKey=clustersKey, OutClusterKey=cluster_key_final_deep, Modulator=modulator1, EventShapeMapTool=eventShapeMapTool, Subtractor=subtractor1, SetMoments=False, ApplyOriginCorrection=False)
    acc.addEventAlgo(HLTRunTools([subToClusterTool], "jetalgHI_clusterSub"+target_jetReco))

    GetConstituentsModifierToolHLT(configFlags, name="HIJetConstituentModifierTool", ClusterKey=cluster_key_final_deep, ApplyOriginCorrection=False, label="HLTHIJetJetConstMod_iter1"+target_jetReco)

    RecoOutputPtMin = 20000
    if jetRecoDict["ionopt"] == "ionp":
        RecoOutputPtMin = 10000
    jetDef_final = jetDef.clone()
    jetDef_final.suffix = jetDef.suffix.replace("_Unsubtracted","")
    jetDef_final.modifiers=[f"HLTHIJetConstSub_iter1{target_jetReco}:iter1", "HLTHIJetJetConstMod_iter1"+target_jetReco, "HLTHIJetCalib:{}___{}___{}".format(calib_seq, JES_is_data, jet_collection_name), "Sort", "Filter:{}".format(RecoOutputPtMin)]
    copyAlg_final= getJetCopyAlg(jetsin=jetsInUnsub,jetsoutdef=jetDef_final,decorations=[],shallowcopy=False,shallowIO=False,monTool=monTool)
    acc.addEventAlgo(copyAlg_final)

    jetsFinal = recordable(jetDef_final.fullname())

    jetsOut = jetsFinal

    return acc, jetsOut, jetDef_final

def HLTRunTools(toollist, algoName):
    
    theAlg = CompFactory.JetAlgorithm(algoName)
    theAlg.Tools = toollist 
   
    return theAlg

def getHIJetRecAlg( jetdef, jetsName, monTool = None):
    """Returns the configured HIJetRecAlg instance corresponding to jetdef

    IMPORTANT : jetdef must have its dependencies solved (i.e. it must result from solveDependencies() )
    """
    pjContNames = jetdef._internalAtt['finalPJContainer']
    jclust = CompFactory.JetClusterer(
        "builder",
        JetAlgorithm = jetdef.algorithm,
        JetRadius = jetdef.radius,
        PtMin = jetdef.ptmin,
        InputPseudoJets = pjContNames,
        GhostArea = 0.0, 
        JetInputType = int(jetdef.inputdef.jetinputtype),
        RandomOption = 1,
    )

    mods = JetRecConfig.getJetModifierTools(jetdef)

    jetname = jetsName
    jra = CompFactory.JetRecAlg(
        "jetrecalg_"+jetname,
        Provider = jclust,
        Modifiers = mods,
        OutputContainer = jetname,
        )
    if monTool:
        # this option can't be set in AnalysisBase -> set only if explicitly asked :
        jra.MonTool = monTool

    return jra

# Same as AddIteration in Reconstruction/HeavyIonRec/HIJetRec/HIJetRecUtils.py but without jtm
def HLTAddIteration(configFlags, seed_container,shape_name,clustersKey, **kwargs) :
    out_shape_name=shape_name
    if 'suffix' in kwargs.keys() : out_shape_name+='_' + kwargs['suffix']
    mod_shape_key=out_shape_name+'_Modulate'
    remodulate=True
    if remodulate :
        if 'modulator' in kwargs.keys() : mod_tool=kwargs['modulator']
        else :
            log.info( "In HLTAddIteration function, HIUEModulatorTool is created using HLTMakeModulatorTool with mod_shape_key = {}".format(mod_shape_key) )
            mod_tool=HLTMakeModulatorTool(mod_shape_key,**kwargs)

    if 'map_tool' in kwargs.keys() : map_tool=kwargs['map_tool']
    else :
        map_tool=CompFactory.HIEventShapeMapTool()

    if 'sub_tool' in kwargs.keys() : sub_tool=kwargs['sub_tool']
    else :
        from HIJetRec.HIJetRecUtilsCA import getHIClusterGeoWeightFile
        weightInputFile=getHIClusterGeoWeightFile(configFlags)

        HIJetClusterSubtractorTool=CompFactory.HIJetClusterSubtractorTool
        sub_tool=HIJetClusterSubtractorTool("HLTHIJetClusterSubtractor", ConfigDir='HIJetCorrection/', InputFile=weightInputFile)
        sub_tool.UseSamplings=False

    if 'assoc_name' in kwargs.keys() : assoc_name=kwargs['assoc_name']
    else :
        log.info( "In HLTAddIteration function, HIJetDRAssociationTool is created with clustersKey= {}".format(clustersKey) )
        assoc=CompFactory.HIJetDRAssociationTool("HIJetDRAssociation")
        assoc.ContainerKey=clustersKey
        assoc.DeltaR=0.8
        assoc.AssociationName="%s_DR8Assoc" % (clustersKey)
        assoc_name=assoc.AssociationName

    HIEventShapeJetIteration=CompFactory.HIEventShapeJetIteration
    iter_tool=HIEventShapeJetIteration('HLTHIJetIteration_%s' % out_shape_name )

    iter_tool.InputEventShapeKey=shape_name
    iter_tool.OutputEventShapeKey=out_shape_name
    iter_tool.AssociationKey=assoc_name
    iter_tool.CaloJetSeedContainerKey=seed_container
    iter_tool.Subtractor=sub_tool
    iter_tool.ModulationScheme=1
    iter_tool.RemodulateUE=True
    iter_tool.Modulator=mod_tool
    iter_tool.ShallowCopy=False
    iter_tool.ModulationEventShapeKey=mod_shape_key
    iter_tool.EventShapeMapTool=map_tool

    return iter_tool

def HLTBuildHarmonicName(shape_key, **kwargs) :
    tname=shape_key
    if 'harmonics' in kwargs.keys() :
        for n in kwargs['harmonics'] :
            tname = str(tname) + str('_V%d' % n)
    return tname

def HLTGetNullModulator() :
    tname='NullUEModulator'
    HIUEModulatorTool=CompFactory.HIUEModulatorTool
    mod=HIUEModulatorTool(tname)
    mod.EventShapeKey=''
    for n in [2,3,4] : setattr(mod,'DoV%d' % n,False)
    return mod

def HLTMakeModulatorTool(mod_key, **kwargs):
    harmonics = kwargs.pop('harmonics', [2, 3, 4])
    tname = kwargs.pop('mod_name', 'Modulator_{}'.format(HLTBuildHarmonicName(mod_key, harmonics=harmonics)))

    if 'suffix' in kwargs.keys():
        tname += '_' + kwargs['suffix']

    if len(harmonics) == 0:
        return HLTGetNullModulator()

    HIUEModulatorTool=CompFactory.HIUEModulatorTool
    mod=HIUEModulatorTool(tname)
    mod.EventShapeKey=mod_key

    for n in [2,3,4] :
        val=(n in harmonics)
        attr_name='DoV%d' % n
        setattr(mod,attr_name,val)  
    
    if 'label' in kwargs.keys():
        label = kwargs['label']
        stdJetModifiers[label] = JetModifier(
            "HIUEModulatorTool",
            tname,
            DoV2 = mod.DoV2,
            DoV3 = mod.DoV3,
            DoV4 = mod.DoV4,
            EventShapeKey=mod_key)

    return mod

def HLTHIJetClusterSubtractorGetter(configFlags):
    from HIJetRec.HIJetRecUtilsCA import getHIClusterGeoWeightFile
    weightInputFile=getHIClusterGeoWeightFile(configFlags)

    HIJetClusterSubtractorTool = CompFactory.HIJetClusterSubtractorTool
    sub_tool = HIJetClusterSubtractorTool("HLTHIJetClusterSubtractor", ConfigDir='HIJetCorrection/', InputFile=weightInputFile)
    sub_tool.UseSamplings = False

    return sub_tool

#same as MakeSubtractionTool in Reconstruction/HeavyIonRec/HIJetRec/HIJetRecUtils.py but without jtm
def HLTMakeSubtractionTool(configFlags, shapeKey, moment_name='', momentOnly=False, **kwargs) :

    alg_props = {
        'EventShapeKey': shapeKey,
        'Modulator': HLTGetNullModulator(),
        'EventShapeMapTool': CompFactory.HIEventShapeMapTool(),
        'Subtractor': HLTHIJetClusterSubtractorGetter(configFlags),
        'MomentName': 'HLTJetSubtractedScale{}Momentum'.format(moment_name),
        'SetMomentOnly': momentOnly,
        'ApplyOriginCorrection': True,
    }

    suffix = shapeKey.toStringProperty()
    if momentOnly is True:
        suffix += '_' + moment_name

    alg_props.update(kwargs)

    label = alg_props.pop('label', None)

    HIJetConstituentSubtractionTool = CompFactory.HIJetConstituentSubtractionTool
    subtr = HIJetConstituentSubtractionTool("HLTHICS_" + suffix, **alg_props)

    stdJetModifiers[label] = JetModifier(
        "HIJetConstituentSubtractionTool",
        "HLTHICS_HLTHIEventShapeWeighted_{modspec}",
        Modulator=subtr.Modulator,
        EventShapeMapTool=subtr.EventShapeMapTool,
        Subtractor=subtr.Subtractor,
        EventShapeKey=subtr.EventShapeKey,
        MomentName=subtr.MomentName,
        SetMomentOnly=subtr.SetMomentOnly,
        ApplyOriginCorrection=True)

    return subtr

def HLTHIClusterGetter(dummyFlags, tower_key="CombinedTower", cell_key="AllCalo", cluster_key="HLT_HIClusters") :
    """Function to equip HLT HI cluster builder from towers and cells, adds to output AOD stream"""

    HIClusterMaker=CompFactory.HIClusterMaker
    theAlg=HIClusterMaker()
    theAlg.InputTowerKey=tower_key
    theAlg.CaloCellContainerKey=cell_key
    theAlg.OutputContainerKey=cluster_key

    return theAlg


def ApplySubtractionToClustersHLT(configFlags, **kwargs) :

    alg_props = {
        'EventShapeKey': "HLTHIEventShape_iter1",
        'ClusterKey': "HLT_HIClusters",
        'Modulator': HLTGetNullModulator(),
        'EventShapeMapTool': CompFactory.HIEventShapeMapTool(),
        'UpdateOnly': False,
        'ApplyOriginCorrection': True,
        'Subtractor': HLTHIJetClusterSubtractorGetter(configFlags),
        'SetMoments': False,
    }

    alg_props.update(kwargs)

    toolName = alg_props.pop('toolName', 'HIClusterSubtraction')

    HIClusterSubtraction = CompFactory.HIClusterSubtraction
    theAlg = HIClusterSubtraction(toolName, **alg_props)

    return theAlg

def GetConstituentsModifierToolHLT(configFlags, **kwargs) :
    #For the cluster key, same exact logic as used for ApplySubtractionToClusters

    alg_props = {
        'ClusterKey': "HLT_HIClusters",
        'ApplyOriginCorrection': True,
        'Subtractor': HLTHIJetClusterSubtractorGetter(configFlags),
    }

    alg_props.update(kwargs)

    toolName = alg_props.pop('name', 'HIJetConstituentModifierTool')
    label = alg_props.pop('label', None)

    HIJetConstituentModifierTool = CompFactory.HIJetConstituentModifierTool
    cmod = HIJetConstituentModifierTool(toolName, **alg_props)

    stdJetModifiers[label] = JetModifier(
        "HIJetConstituentModifierTool",
        "HLTHIJetConstituentModifierTool_{modspec}",
        ClusterKey=cmod.ClusterKey,
        Subtractor=cmod.Subtractor,
        ApplyOriginCorrection=cmod.ApplyOriginCorrection)

    return cmod
