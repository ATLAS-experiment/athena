# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#==============================================================================
# Contains the configuration for customs jet reconstruction + decorations
# used in analysis DAODs
#==============================================================================

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.Enums import LHCPeriod

def addJetContextFlags(flags):
    jetContextName = 'CustomVtx'
    PrefixForHggCollection   = "Hgg"
    HggVertexContainerName   = PrefixForHggCollection+"PrimaryVertices"
    def customVtxContext(prevflags):
        context = prevflags.Jet.Context.default.clone(
            Vertices         = HggVertexContainerName,
            GhostTracks      = "PseudoJetGhostTrack", 
            GhostTracksLabel = "GhostTrack",
            TVA              = "JetTrackVtxAssoc"+jetContextName,
            JetTracks        = "JetSelectedTracks"+jetContextName,
            JetTracksQualityCuts = "JetSelectedTracks"+jetContextName+"_trackSelOpt"
          )
        return context
    flags.addFlag(f"Jet.Context.{jetContextName}", customVtxContext)
    
def HIGG1D1CustomJetsCfg(ConfigFlags):
    """Jet reconstruction needed for HIGG1D1"""

    acc = ComponentAccumulator()

    # Ideally there would be a nice way to configure the PFlowCustomVtx jets  but at the moment 
    # all tools need to be configured manually to ensure that the track to vertex association is done correctly.
    PrefixForHggCollection   = "Hgg"
    HggVertexContainerName   = PrefixForHggCollection+"PrimaryVertices"
    CustomPFJetContainerName = "AntiKt4EMPFlowCustomVtxJets"

    from JetRecConfig.StandardJetConstits import stdInputExtDic, JetInputExternal,JetInputConstit, JetInputConstitSeq, JetConstitModifier, xAODType
    from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlow
    from JetRecConfig.JetDefinition import JetDefinition 
    from JetRecTools import JetRecToolsConfig as jrtcfg
    from JetMomentTools import JetMomentToolsConfig
    from JetRecConfig.StandardJetConstits import stdConstitDic, stdContitModifDic
    from JetRecConfig.StandardJetContext import propFromContext, inputsFromContext
    from JetRecConfig.JetInputConfig import buildEventShapeAlg

    #  Get custom jet context
    jetContextName = 'CustomVtx'
    context = ConfigFlags.Jet.Context[jetContextName]
    
    def replaceItems(tup,orgName,newName):
        newList = list(tup)
        for i, item in enumerate(newList):
            if orgName in item:
                newList[i] = item.replace(orgName,newName)
                print( "Updated ", orgName, " to ", newName )
                return tuple(newList)
        print( "Failed to update ", orgName, " to ", newName )
        return tuple(newList)
    
    def updateCalibSequence(tup):
        newList = list(tup)
        if ConfigFlags.GeoModel.Run is LHCPeriod.Run3:
            rhoname = "Kt4EMPFlowNeutEventShape"
        else:
            rhoname = "Kt4EMPFlowCustomVtxEventShape" 

        for i, item in enumerate(newList):
            if "Calib" in item:
                calibspecs = item.split(":")
                calib, calibcontext, data_type = calibspecs[:3]
                calibseq=""
                if len(calibspecs)>3: 
                  calibseq = calibspecs[3]
                pvname = HggVertexContainerName
                finalCalibString = f"CalibCustomVtx:{calibcontext}:{data_type}:{calibseq}:{rhoname}:{pvname}"
                if len(calibspecs)>6: finalCalibString = f"{finalCalibString}:{calibspecs[6]}"
                newList[i] = finalCalibString
                print(finalCalibString)
                return tuple(newList)
        print( "Failed to update calib sequence" )
        return tuple(newList)


    # Create modifier list and JetDefinition 
    modsCustomVtx = AntiKt4EMPFlow.modifiers
    modsCustomVtx = updateCalibSequence(modsCustomVtx)
    modsCustomVtx = replaceItems(modsCustomVtx,"TrackMoments","TrackMomentsCustomVtx")
    modsCustomVtx = replaceItems(modsCustomVtx,"TrackSumMoments","TrackSumMomentsCustomVtx")
    modsCustomVtx = replaceItems(modsCustomVtx,"JVF","JVFCustomVtx")
    modsCustomVtx = replaceItems(modsCustomVtx,"JVT","JVTCustomVtx")
    modsCustomVtx = replaceItems(modsCustomVtx,"Charge","ChargeCustomVtx")

    ghostCustomVtx = AntiKt4EMPFlow.ghostdefs

    # GPFlow are the same than EMPFlow except they have pflow linked to elec or muons filtered out.
    stdConstitDic["TrackCustomVtx"]  = JetInputConstit("TrackCustomVtx", xAODType.TrackParticle,"JetSelectedTracksCustomVtx" )
    
    
    stdConstitDic["GPFlowCustomVtx"] = JetInputConstitSeq("GPFlowCustomVtx", xAODType.FlowElement,["CorrectPFOCustomVtx", "CHSCustomVtx"] , 'GlobalParticleFlowObjects', 'CHSGCustomVtxParticleFlowObjects',
                        label='EMPFlow')

    stdContitModifDic["CorrectPFOCustomVtx"] = JetConstitModifier("CorrectPFOCustomVtx", "CorrectPFOTool",
                                                                  prereqs=[inputsFromContext("Vertices")],
                                                                  properties=dict(VertexContainerKey=propFromContext("Vertices"),
                                                                                  WeightPFOTool= CompFactory.getComp("CP::WeightPFOTool")("weightPFO") )
                                                                  )

    stdContitModifDic["CHSCustomVtx"] = JetConstitModifier("CHSCustomVtx",    "ChargedHadronSubtractionTool",
                                                          prereqs=  [inputsFromContext("Vertices"),inputsFromContext("TVA")],
                                                          properties=dict(VertexContainerKey=propFromContext("Vertices"),                                       
                                                                          TrackVertexAssociation=propFromContext("TVA"),
                                                                          UseTrackToVertexTool=True, 
                                                                          ))



    AntiKt4EMPFlowCustomVtx = JetDefinition("AntiKt",0.4,stdConstitDic.GPFlowCustomVtx,
                                        infix = "CustomVtx",
                                        context = jetContextName,
                                        ghostdefs = ghostCustomVtx,
                                        modifiers = modsCustomVtx+("JetPtAssociation","QGTaggingCustomVtx","fJVTCustomVtx","NNJVTCustomVtx","CaloEnergiesClus","JetPileupLabel"),
                                        ptmin = 10000,
    )

    def getUsedInVertexFitTrackDecoratorAlgCustomVtx(jetdef, jetmod):
        """ Create the alg  to decorate the used-in-fit information for AMVF """
        context  = jetdef._contextDic

        from InDetUsedInFitTrackDecoratorTool.UsedInVertexFitTrackDecoratorConfig import getUsedInVertexFitTrackDecoratorAlg
        alg = getUsedInVertexFitTrackDecoratorAlg(context['Tracks'], context['Vertices'],
                                                  vertexDeco='TTVA_AMVFVertices_forHiggs',
                                                  weightDeco='TTVA_AMVFWeights_forHiggs')
        return alg


    # Define new input variables for jet configuration
    stdInputExtDic[context['Vertices']] = JetInputExternal( context['Vertices'],   xAODType.Vertex )

    stdInputExtDic["JetSelectedTracksCustomVtx"] = JetInputExternal("JetSelectedTracksCustomVtx",     xAODType.TrackParticle,
                                                                                    prereqs= [ f"input:{context['Tracks']}" ], # in std context, this is InDetTrackParticles (see StandardJetContext)
                                                                                    algoBuilder = lambda jdef,_ : jrtcfg.getTrackSelAlg(jdef, trackSelOpt=False, 
                                                                                                                                              DecorDeps=["TTVA_AMVFWeights_forHiggs", "TTVA_AMVFVertices_forHiggs"] )
                                                                                 )

    stdInputExtDic["JetTrackUsedInFitDecoCustomVtx"] = JetInputExternal("JetTrackUsedInFitDecoCustomVtx", xAODType.TrackParticle,
                                                                        prereqs= [ f"input:{context['Tracks']}" , # in std context, this is InDetTrackParticles (see StandardJetContext)
                                                                                  f"input:{context['Vertices']}"],
                                                                        algoBuilder = getUsedInVertexFitTrackDecoratorAlgCustomVtx
                                                                        )

    stdInputExtDic["JetTrackVtxAssocCustomVtx"] = JetInputExternal("JetTrackVtxAssocCustomVtx",  xAODType.TrackParticle,
                                              algoBuilder = lambda jdef,_ : jrtcfg.getJetTrackVtxAlg(jdef._contextDic, algname="jetTVACustomVtx",
                                                                                                                       WorkingPoint="Nonprompt_All_MaxWeight",
                                                                                                                       AMVFVerticesDeco='TTVA_AMVFVertices_forHiggs',
                                                                                                                       AMVFWeightsDeco='TTVA_AMVFWeights_forHiggs'),
                                              prereqs = [ "input:JetTrackUsedInFitDecoCustomVtx", f"input:{context['Vertices']}" ] )

    stdInputExtDic["EventDensityCustomVtx"] =     JetInputExternal("EventDensityCustomVtx", "EventShape", algoBuilder = buildEventShapeAlg,
                      containername = lambda jetdef, _ : "Kt4"+jetdef.inputdef.label+"CustomVtxEventShape",
                      prereqs = lambda jetdef : ["input:"+jetdef.inputdef.name] )

    from JetRecConfig.StandardJetMods import stdJetModifiers
    from JetRecConfig.JetDefinition import JetModifier
    from JetCalibTools import JetCalibToolsConfig

    stdJetModifiers.update(

      CalibCustomVtx = JetModifier("JetCalibrationTool","jetcalib_jetcoll_calibseqCustomVtx",
                                        createfn=JetCalibToolsConfig.getJetCalibToolFromString,
                                        prereqs=lambda mod,jetdef : JetCalibToolsConfig.getJetCalibToolPrereqs(mod,jetdef)+[f"input:{context['Vertices']}"]),


      JVFCustomVtx =             JetModifier("JetVertexFractionTool", "jvfCustomVtx",
                                        createfn= lambda jdef,_ : JetMomentToolsConfig.getJVFTool(jdef,"CustomVtx"),
                                        modspec = "CustomVtx",
                                        prereqs = ["input:JetTrackVtxAssocCustomVtx", "mod:TrackMomentsCustomVtx", f"input:{context['Vertices']}"] ,
                                             JetContainer = CustomPFJetContainerName),

      JVTCustomVtx =             JetModifier("JetVertexTaggerTool", "jvtCustomVtx",
                                        createfn= lambda jdef,_ : JetMomentToolsConfig.getJVTTool(jdef,"CustomVtx"),
                                        modspec = "CustomVtx",
                                        prereqs = [ "mod:JVFCustomVtx" ],JetContainer = CustomPFJetContainerName),

      NNJVTCustomVtx =           JetModifier("JetVertexNNTagger", "nnjvtCustomVtx",
                                              createfn=lambda jdef,_ :JetMomentToolsConfig.getNNJvtTool(jdef,"CustomVtx"),
                                              prereqs = [ "mod:JVFCustomVtx" ],JetContainer = CustomPFJetContainerName),

      OriginSetPVCustomVtx =     JetModifier("JetOriginCorrectionTool", "origin_setpvCustomVtx",
                                        modspec = "CustomVtx",
                                        prereqs = [ "mod:JVFCustomVtx" ],JetContainer = CustomPFJetContainerName, OnlyAssignPV=True),

      TrackMomentsCustomVtx =    JetModifier("JetTrackMomentsTool", "trkmomsCustomVtx",
                                        createfn= lambda jdef,_ : JetMomentToolsConfig.getTrackMomentsTool(jdef,"CustomVtx"),
                                        modspec = "CustomVtx",
                                        prereqs = [ "input:JetTrackVtxAssocCustomVtx","ghost:Track" ],JetContainer = CustomPFJetContainerName),

      TrackSumMomentsCustomVtx = JetModifier("JetTrackSumMomentsTool", "trksummomsCustomVtx",
                                        createfn=lambda jdef,_ :JetMomentToolsConfig.getTrackSumMomentsTool(jdef,"CustomVtx"),
                                        modspec = "CustomVtx",
                                        prereqs = [ "input:JetTrackVtxAssocCustomVtx","ghost:Track" ],JetContainer = CustomPFJetContainerName),

      ChargeCustomVtx =          JetModifier("JetChargeTool", "jetchargeCustomVtx", 
                                        prereqs = [ "ghost:Track" ]),


      QGTaggingCustomVtx =       JetModifier("JetQGTaggerVariableTool", "qgtaggingCustomVtx",
                                        createfn=lambda jdef,_ :JetMomentToolsConfig.getQGTaggingTool(jdef,"CustomVtx"),
                                        modspec = "CustomVtx",
                                        prereqs = lambda _,jdef :
                                             ["input:JetTrackVtxAssocCustomVtx","mod:TrackMomentsCustomVtx"] +
                                             (["mod:JetPtAssociation"] if not jdef._cflags.Input.isMC else []),
                                        JetContainer = CustomPFJetContainerName),

      fJVTCustomVtx =            JetModifier("JetForwardPFlowJvtTool", "fJVTCustomVtx",
                                        createfn=lambda jdef,_ :JetMomentToolsConfig.getPFlowfJVTTool(jdef,"CustomVtx"),
                                        modspec = "CustomVtx",
                                        prereqs = ["input:JetTrackVtxAssocCustomVtx","input:EventDensityCustomVtx",f"input:{context['Vertices']}","mod:NNJVTCustomVtx"],
                                        JetContainer = CustomPFJetContainerName),
    )

    from JetRecConfig.JetRecConfig import JetRecCfg

    acc.merge(JetRecCfg(ConfigFlags,AntiKt4EMPFlowCustomVtx))
  
    return acc

def HIGG1D1CustomJetsCleaningCfg(ConfigFlags):
    """Event cleaning and jet cleaning for HIGG1D1"""

    acc = ComponentAccumulator()

    from DerivationFrameworkJetEtMiss.JetCommonConfig import AddJvtDecorationAlgCfg
    acc.merge(AddJvtDecorationAlgCfg(ConfigFlags, algName = "JvtPassDecorCustomVtxAlg", jetContainer='AntiKt4EMPFlowCustomVtx'))

    from DerivationFrameworkTau.TauCommonConfig import AddTauAugmentationCfg
    acc.merge(AddTauAugmentationCfg(ConfigFlags, prefix="JetCommon", doRNNLoose=True))

    # Decorate if jet passes OR and save decoration DFCommonJets_passOR
    # Use modified OR that does not check overlaps with tauls
    from AssociationUtils.AssociationUtilsConfig import OverlapRemovalToolCfg

    # Sequences for event cleaning and decorator locking.
    # See comments in JetCommonConfig.AddEventCleanFlagsCfg.
    acc.addSequence(CompFactory.AthSequencer('EventCleanSeq', Sequential=True))
    acc.addSequence(CompFactory.AthSequencer('EventCleanLockSeq', Sequential=True))

    outputLabel = 'DFCommonJets_passOR'
    bJetLabel = '' #default
    tauLabel = 'DFTauRNNLoose'
    orTool = acc.popToolsAndMerge(OverlapRemovalToolCfg(ConfigFlags, outputLabel=outputLabel, bJetLabel=bJetLabel))
    algOR = CompFactory.OverlapRemovalGenUseAlg('OverlapRemovalGenUseAlg_CustomVtx',
                                                JetKey="AntiKt4EMPFlowCustomVtxJets",
                                                OverlapLabel=outputLabel,
                                                OverlapRemovalTool=orTool,
                                                TauLabel=tauLabel,
                                                BJetLabel=bJetLabel)
    acc.addEventAlgo(algOR, 'EventCleanSeq')

    # Explictly lock the decorations produced by overlap removal.
    lockOR = CompFactory.DerivationFramework.LockDecorations \
        ('OverlapRemovalLockDecorAlg_CustomVtx',
         Decorations = [
             'Electrons.selected',
             'Electrons.' + outputLabel,
             'Muons.selected',
             'Muons.' + outputLabel,
             'Photons.selected',
             'Photons.' + outputLabel,
             'AntiKt4EMPFlowCustomVtxJets.selected',
             'AntiKt4EMPFlowCustomVtxJets.' + outputLabel,
         ])
    acc.addEventAlgo(lockOR, 'EventCleanLockSeq')

    from JetSelectorTools.JetSelectorToolsConfig import EventCleaningToolCfg, JetCleaningToolCfg
    workingPoints = ['Loose', 'Tight']

    for wp in workingPoints:
        cleaningLevel = wp + "Bad"

        jetCleaningTool = acc.popToolsAndMerge(
            JetCleaningToolCfg(
                ConfigFlags,
                name="JetCleaningCustomVtxTool_" + cleaningLevel,
                jetdef="AntiKt4EMPFlowCustomVtxJets",
                cleaningLevel=cleaningLevel,
                useDecorations=False,
            )
        )
        acc.addPublicTool(jetCleaningTool)

        ecTool = acc.popToolsAndMerge(
            EventCleaningToolCfg(ConfigFlags, "EventCleaningCustomVtxTool_" + wp, cleaningLevel)
        )
        ecTool.JetCleanPrefix = "DFCommonJets_"
        ecTool.JetContainer = "AntiKt4EMPFlowCustomVtxJets"
        ecTool.JetCleaningTool = jetCleaningTool
        acc.addPublicTool(ecTool)

        # Alg to calculate event-level and jet-level cleaning variables
        # Only store event-level flags for Loose* WPs
        eventCleanAlg = CompFactory.EventCleaningTestAlg(
            "EventCleaningCustomVtxTestAlg_" + wp,
            EventCleaningTool=ecTool,
            JetCollectionName="AntiKt4EMPFlowCustomVtxJets",
            EventCleanPrefix="DFCommonJetsCustomVtx_",
            CleaningLevel=cleaningLevel,
            doEvent=True,
        )
        acc.addEventAlgo(eventCleanAlg, 'EventCleanSeq')
    
    return acc
