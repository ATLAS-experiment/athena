# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""
This module defines the standard JetModifier tools used in jet reco

Definitions are grouped in a dictionary of tool configurations using the helpers defined
in package configs.
This dict maps a modifier alias to the JetModifier config object
that in turn will be responsible for generating a configured tool.

The JetModifier config class is defined in JetDefinition.py 

 Args to the JetModifier constructor are:
   1. Tool Type (ignored if the helper is a custom one)
   2. Tool Name (ignored if the helper is a custom one)
   3. createfn : helper function which build the actual tool. If none, we just instantiate the tool type. 
   4. prereqs : Prerequisites  (default to []). Can also be a function which returns a list
   X. all other keyword arguments are directly interpreted as Property of the tool. 
       for ex, passing 'PtMin=10.' will configure the tool as in 'tool.PtMin = 10'
       we can pass function as the value : 
         'JetContainerName=nameFunc' will configure as in 'tool.JetContainerName=nameFunc(jetdef, modspec)'

        --> should this be by default? prefer to avoid ignored args
"""
from .JetDefinition import JetModifier
from .Utilities import ldict
from AthenaConfiguration.ComponentFactory import CompFactory
from JetRecConfig.JetConfigFlags import jetInternalFlags
from .StandardJetConstits import inputsFromContext

stdJetModifiers = ldict()

########################################################################
# Define the simple modifier setups here -- those defined in JetRec.
stdJetModifiers.update( 
    Sort   = JetModifier("JetSorter","jetsort"),
    Filter = JetModifier("JetFilterTool","jetptfilter_{modspec}",
                         # we give a function as PtMin : it will be evaluated when instantiating the tool (modspec is specified with this tool
                         # alias like "Filter:10000" --> PtMin=100000).
                         PtMin = lambda jdef,modspec: int(modspec) 
                         ),
    Filter_ifnotESD = JetModifier("JetFilterTool","jetptfilter_{modspec}",
                                 PtMin = lambda _,modspec: 1 if jetInternalFlags.isRecoJob else int(modspec),
                                 ),
    # Filter that can be easily turned off via pre-exec for e.g. PHYSVAL
    Filter_calibThreshold = JetModifier("JetFilterTool","jetptfilter_{modspec}",
                                        PtMin = lambda jetdef,modspec: 1 if not jetdef._cflags.Jet.useCalibJetThreshold else int(modspec),
                                       )
)

########################################################################
# Below, we populate the stdJetModifiers with modifier definitions for tools
# that are defined in other packages.
# When necessary, the helper functions 'createfn' for tools from package 'PackageName' live in modules called
# PackageName.PackageConfig (modules to be moved)

# Calibration
try:
    from JetCalibTools import JetCalibToolsConfig
    stdJetModifiers.update(
        Calib = JetModifier("JetCalibrationTool","jetcalib_jetcoll_calibseq",
                            createfn=JetCalibToolsConfig.getJetCalibToolFromString,
                            prereqs=lambda mod,jetdef : JetCalibToolsConfig.getJetCalibToolPrereqs(mod,jetdef)+[inputsFromContext("Vertices")])
    )
except ModuleNotFoundError:
    # In some releases (AthGeneration) JetCalibTools is not existing
    pass

# TBD:
# All items below in principle will support decoration mode, rather
# than only non-const modification. Mode of operation should be
# determined by interface called from parent tool/alg.


# Many JetMoment tools need to know the name of the container they operate on.
# We set the function below as the 'JetContainer' property so the config system
# can assign the right name to the c++ tool.
def _jetname(jetdef,modspec):
    return jetdef.fullname()

def isMC(flags):
    """A simple filter function for  testing if we're running in MC
    returns (bool, str) where the str contains an explanation of why the bool is False.
    (probably worth re-allocating somehere else)"""
    return flags.Input.isMC or flags.Overlay.DataOverlay, "Input file is not MC"


# Standard jet moments
try:
    from JetMomentTools import JetMomentToolsConfig
    stdJetModifiers.update(

        # Easy cases, no special config or prereqs, just default tool config 
        ClusterMoments =  JetModifier("JetClusterMomentsTool", "clsmoms", JetContainer = _jetname),
        ECPSFrac =        JetModifier("JetECPSFractionTool", "ecpsfrac", JetContainer = _jetname),
        Width =           JetModifier("JetWidthTool", "width", JetContainer = _jetname),

        # More complex cases here
        CaloEnergies =    JetModifier("JetCaloEnergies", "jetens", 
                                      prereqs=["mod:EMScaleMom"], 
                                      Calculations=["EMFrac", "HECFrac", "PSFrac", "FracSamplingMax"], JetContainer = _jetname,
                                      ),
        
        CaloEnergiesLargeR =    JetModifier("JetCaloEnergies", "jetenslargeR", 
                                      prereqs=["mod:EMScaleMom"], 
                                      Calculations=["EMFrac", "HECFrac", "PSFrac", "EM3Frac", "Tile0Frac", "EffNClusts"], JetContainer = _jetname,
                                      calcClusterBasedVars = True,
                                      ),

        # CaloEnergiesClus is only relevant for FE-based jet collections
        CaloEnergiesClus = JetModifier("JetCaloEnergies", "jetensclus",
                                       prereqs=["mod:EMScaleMom"], 
                                       Calculations=["EMFrac", "HECFrac", "PSFrac", "FracSamplingMax"], JetContainer = _jetname,
                                       calcClusterBasedVars = True),

        CaloQuality =     JetModifier("JetCaloQualityTool", "caloqual",
                                      TimingCuts = [5,10],
                                      Calculations = ["LArQuality", "N90Constituents", "NegativeE", "Timing", "HECQuality", "Centroid", "AverageLArQF", "BchCorrCell"],JetContainer = _jetname),

        CaloQualityFE =   JetModifier("JetCaloQualityToolFE", "caloqualFE",
                                      TimingCuts = [5,10],
                                      ThresholdCuts = [90],
                                      Calculations = ["LArQuality", "NegativeE", "Timing", "HECQuality", "Centroid", "AverageLArQF", "BchCorrCell"],JetContainer = _jetname),

        N90 =             JetModifier("JetCaloQualityTool", "n90",
                                      Calculations = ["N90Constituents"],JetContainer = _jetname),

        ConstitFourMom =  JetModifier("JetConstitFourMomTool", "constitfourmom_basename",
                                      createfn=JetMomentToolsConfig.getConstitFourMomTool,),
        EMScaleMom =      JetModifier("JetEMScaleMomTool", "emscalemom_basename",
                                      createfn=JetMomentToolsConfig.getEMScaleMomTool,
                                      JetContainer = _jetname),

        JVF =             JetModifier("JetVertexFractionTool", "jvf",
                                      createfn=JetMomentToolsConfig.getJVFTool,
                                      prereqs = [inputsFromContext("TVA"), "mod:TrackMoments", inputsFromContext("Vertices")],
                                      JetContainer = _jetname),
        JVT =             JetModifier("JetVertexTaggerTool", "jvt",
                                       createfn=JetMomentToolsConfig.getJVTTool,
                                       prereqs = [ "mod:JVF" ],JetContainer = _jetname),
        NNJVT =           JetModifier("JetVertexNNTagger", "nnjvt",
                                       createfn=JetMomentToolsConfig.getNNJvtTool,
                                       prereqs = [ "mod:JVF" ],JetContainer = _jetname),
        LArHVCorr =       JetModifier("JetLArHVTool", "larhvcorr",
                                       prereqs = ["mod:EMScaleMom"],JetContainer = _jetname),
        OriginSetPV =     JetModifier("JetOriginCorrectionTool", "origin_setpv",
                                       prereqs = [ "mod:JVF" ],JetContainer = _jetname, OnlyAssignPV=True),
        TrackMoments =    JetModifier("JetTrackMomentsTool", "trkmoms",
                                      createfn=JetMomentToolsConfig.getTrackMomentsTool,
                                      prereqs = [ inputsFromContext("TVA"),"ghost:Track" ],JetContainer = _jetname),

        TrackSumMoments = JetModifier("JetTrackSumMomentsTool", "trksummoms",
                                      createfn=JetMomentToolsConfig.getTrackSumMomentsTool,
                                      prereqs = [ inputsFromContext("TVA"),"ghost:Track" ],JetContainer = _jetname),
        Charge =          JetModifier("JetChargeTool", "jetcharge", 
                                      prereqs = [ "ghost:Track" ]),

        QGTagging =       JetModifier("JetQGTaggerVariableTool", "qgtagging",
                                      createfn=JetMomentToolsConfig.getQGTaggingTool,
                                      prereqs = lambda _,jetdef :
                                      [inputsFromContext("TVA"),"mod:TrackMoments"] +
                                      (["mod:JetPtAssociation"] if not isMC(jetdef._cflags) else []),
                                      JetContainer = _jetname),

        fJVT =           JetModifier("JetForwardPFlowJvtTool", "fJVT",
                                     createfn=JetMomentToolsConfig.getPFlowfJVTTool,
                                     prereqs = [inputsFromContext("TVA"),inputsFromContext("EventDensity"),inputsFromContext("Vertices"),"mod:NNJVT"],
                                     JetContainer = _jetname),

        bJVT =           JetModifier("JetBalancePFlowJvtTool", "bJVT",
                                     createfn=JetMomentToolsConfig.getPFlowbJVTTool,
                                     prereqs = [inputsFromContext("EventDensity"),inputsFromContext("Vertices"),"mod:NNJVT"],
                                     JetContainer = _jetname),

        ConstitFrac =    JetModifier("JetConstituentFrac", "constitFrac",
                                     JetContainer = _jetname,
                                     ),
        
        groomMRatio =    JetModifier("JetGroomMRatio", "groomMRatio",
                                     JetContainer = _jetname,
                                     ),

        JetPtAssociation = JetModifier("JetPtAssociationTool", "jetPtAssociation",
                                       filterfn=isMC,
                                       createfn=JetMomentToolsConfig.getJetPtAssociationTool,
                                       prereqs=["ghost:Truth"],
                                       JetContainer = _jetname
                                       ),

    )
except ModuleNotFoundError:
    # In some releases (AthGeneration) JetMomentTools is not existing
    pass

# Truth labelling moments
from ParticleJetTools import ParticleJetToolsConfig
stdJetModifiers.update(
    # Easy cases, no special config or prereqs, just default tool config
    PartonTruthLabel = JetModifier("Analysis::JetPartonTruthLabel","partontruthlabel",
                                    prereqs=["ghost:Partons"]),

    # More complex cases here
    TruthPartonDR =    JetModifier("Analysis::JetConeLabeling","truthpartondr",
                                   filterfn=isMC,
                                   JetTruthMatchTool = lambda *l : CompFactory.getComp("Analysis::JetQuarkLabel")("jetquarklabel", McEventCollection='TruthEvents'),
                                   ),

                                    
    JetDeltaRLabel =   JetModifier("ParticleJetDeltaRLabelTool","jetdrlabeler_jetptmin",
                                   createfn=ParticleJetToolsConfig.getJetDeltaRLabelTool,
                                   prereqs=["ghost:BHadronsFinal",
                                            "ghost:CHadronsFinal",
                                            "ghost:TausFinal"]
                                   ),


    JetGhostLabel =    JetModifier("ParticleJetGhostLabelTool","jetghostlabeler",
                                   createfn=ParticleJetToolsConfig.getJetGhostLabelTool,
                                   prereqs=["ghost:BHadronsFinal",
                                            "ghost:CHadronsFinal",
                                            "ghost:TausFinal"]
                                   ),


    JetTaggingTruthLabel = JetModifier("JetTaggingTruthLabel", "truthlabeler_{mods}",
                                       filterfn=isMC,
                                       createfn=ParticleJetToolsConfig.getJetTruthLabelTool,
                                      ),

    JetPileupLabel = JetModifier("JetPileupLabel", "pileuplabeler_{mods}",
                                 filterfn=isMC,
                                 createfn=ParticleJetToolsConfig.getJetPileupLabelTool,
                                 prereqs=["input:AntiKt4TruthDressedWZJets"]
                                 ),
)



# Substructure tools 
stdJetModifiers.update( 
    nsubjettiness = JetModifier( "NSubjettinessTool", "nsubjettiness",Alpha = 1.0),
    nsubjettinessR = JetModifier( "NSubjettinessRatiosTool", "nsubjettinessR",),

    
    ktdr       = JetModifier("KtDeltaRTool", "ktdr", JetRadius = 0.4),

    ktsplitter = JetModifier( "KTSplittingScaleTool", "ktsplitter"),
    
    angularity = JetModifier( "AngularityTool", "angularity"),
    
    dipolarity = JetModifier( "DipolarityTool", "dipolarity",SubJetRadius = 0.3),
    
    planarflow = JetModifier( "PlanarFlowTool", "planarflow"),

    ktmassdrop = JetModifier( "KtMassDropTool", "ktmassdrop"),

    ecorr      = JetModifier( "EnergyCorrelatorTool", "ecorr", Beta = 1.0),
    ecorrR     = JetModifier( "EnergyCorrelatorRatiosTool", "ecorrR", ),

    ecorrgeneral = JetModifier( "EnergyCorrelatorGeneralizedTool", "ecorrgeneral", DoLSeries = True),
    ecorrgeneralratios = JetModifier( "EnergyCorrelatorGeneralizedRatiosTool", "ecorrgeneralratios",  DoLSeries = True),

    comshapes = JetModifier( "CenterOfMassShapesTool","comshapes"),

    pull      = JetModifier("JetPullTool", "pull",  UseEtaInsteadOfY = False, IncludeTensorMoments = True ),

    charge    = JetModifier( "JetChargeTool", "charge", K=1.0),

    qw = JetModifier( "QwTool", "qw"),

    softdropobs = JetModifier("SoftDropObservablesTool", "softdropobs"),

)

# VR track-jet decorations
stdJetModifiers.update(
    vr = JetModifier( "FlavorTagDiscriminants::VRJetOverlapDecoratorTool", "vr")
)
