# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging

logPHYS = logging.getLogger('PHYS')

def addJetContextFlags(flags):
    jetContextName = 'CustomVtxGNN'
    def customVtxContext(prevflags):
        context = prevflags.Jet.Context.default.clone(
            Vertices         = "PrimaryVertices_initial",
            GhostTracks      = "PseudoJetGhostTrack", 
            GhostTracksLabel = "GhostTrack",
            TVA              = "JetTrackVtxAssoc"+jetContextName,
            JetTracks        = "JetSelectedTracks"+jetContextName,
            JetTracksQualityCuts = "JetSelectedTracks"+jetContextName+"_trackSelOpt"
          )
        return context
    flags.addFlag(f"Jet.Context.{jetContextName}", customVtxContext)


def CustomJetsCfg(flags):

    acc = ComponentAccumulator()

    CustomJetContainerName = "AntiKt4EMTopoCustomVtxGNNJets"

    from JetRecConfig.StandardJetConstits import stdInputExtDic, JetInputExternal, JetInputConstitSeq, JetConstitModifier, xAODType
    from JetRecConfig.JetDefinition import JetDefinition 
    from JetRecConfig.StandardSmallRJets import AntiKt4EMTopo
    from JetRecTools import JetRecToolsConfig as jrtcfg
    from JetMomentTools import JetMomentToolsConfig
    from JetRecConfig.StandardJetConstits import stdConstitDic, stdContitModifDic
    from JetRecConfig.StandardJetContext import inputsFromContext
    from JetRecConfig.JetInputConfig import buildEventShapeAlg


    #  Get custom jet context
    jetContextName = 'CustomVtxGNN'
    context = flags.Jet.Context[jetContextName]

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

        rhoname = "Kt4EMTopoCustomVtxGNNEventShape" 

        for i, item in enumerate(newList):
            if "Calib" in item:
                calibspecs = item.split(":")
                calib, calibcontext, data_type = calibspecs[:3]
                calibseq=""
                if len(calibspecs)>3: 
                  calibseq = calibspecs[3]
                finalCalibString = f"CalibCustomVtxGNN:{calibcontext}:{data_type}:{calibseq}:{rhoname}:PrimaryVertices_initial"
                if len(calibspecs)>6: finalCalibString = f"{finalCalibString}:{calibspecs[6]}"
                newList[i] = finalCalibString
                print(finalCalibString)
                return tuple(newList)
        print( "Failed to update calib sequence" )
        return tuple(newList)


    # Create modifier list and JetDefinition 
    modsCustomVtxGNN = AntiKt4EMTopo.modifiers
    modsCustomVtxGNN = updateCalibSequence(modsCustomVtxGNN)
    modsCustomVtxGNN = replaceItems(modsCustomVtxGNN,"TrackMoments","TrackMomentsCustomVtxGNN")
    modsCustomVtxGNN = replaceItems(modsCustomVtxGNN,"TrackSumMoments","TrackSumMomentsCustomVtxGNN")
    modsCustomVtxGNN = replaceItems(modsCustomVtxGNN,"JVF","JVFCustomVtxGNN")
    modsCustomVtxGNN = replaceItems(modsCustomVtxGNN,"JVT","JVTCustomVtxGNN")
    modsCustomVtxGNN = replaceItems(modsCustomVtxGNN,"Charge","ChargeCustomVtxGNN")
    modsCustomVtxGNN = replaceItems(modsCustomVtxGNN,"jetiso","jetisoCustomVtxGNN")

    ghostCustomVtxGNN = AntiKt4EMTopo.ghostdefs

    # TODO: check this
    stdConstitDic["EMTopoOriginCustomVtxGNN"]  = JetInputConstitSeq("EMTopoOriginCustomVtxGNN", xAODType.CaloCluster, ["EMCustomVtxGNN","OriginCustomVtxGNN"],
                                                    "CaloCalTopoClusters", "EMOriginCustomVtxGNNTopoClusters", label="EMTopo")
    stdContitModifDic["OriginCustomVtxGNN"] = JetConstitModifier("OriginCustomVtxGNN", "CaloClusterConstituentsOrigin", prereqs=[inputsFromContext("Vertices")], 
                                                properties=dict(VertexContainer="PrimaryVertices_initial"))
    stdContitModifDic["EMCustomVtxGNN"] = JetConstitModifier("EMCustomVtxGNN",     "ClusterAtEMScaleTool", )

    AntiKt4EMTopoCustomVtxGNN = JetDefinition("AntiKt",0.4,stdConstitDic.EMTopoOriginCustomVtxGNN,
                                           infix = "CustomVtxGNN",
                                           context = jetContextName,
                                           ghostdefs = ghostCustomVtxGNN,
                                           modifiers = modsCustomVtxGNN,
                                           lock = True,
    )

    def getUsedInVertexFitTrackDecoratorAlgCustomVtxGNN(jetdef, jetmod):
        """ Create the alg  to decorate the used-in-fit information for AMVF """
        context  = jetdef._contextDic

        from InDetUsedInFitTrackDecoratorTool.UsedInVertexFitTrackDecoratorConfig import getUsedInVertexFitTrackDecoratorAlg
        alg = getUsedInVertexFitTrackDecoratorAlg(context['Tracks'], context['Vertices'],
                                                  vertexDeco='TTVA_AMVFVertices_forGNN',
                                                  weightDeco='TTVA_AMVFWeights_forGNN')
        return alg

    # Define new input variables for jet configuration
    stdInputExtDic[context['Vertices']] = JetInputExternal( context['Vertices'],   xAODType.Vertex )

    stdInputExtDic["JetSelectedTracksCustomVtxGNN"] = JetInputExternal("JetSelectedTracksCustomVtxGNN",     xAODType.TrackParticle,
                                                                                    prereqs= [ f"input:{context['Tracks']}" ], # in std context, this is InDetTrackParticles (see StandardJetContext)
                                                                                    algoBuilder = lambda jdef,_ : jrtcfg.getTrackSelAlg(jdef, trackSelOpt=False,
                                                                                                                                              DecorDeps=["TTVA_AMVFWeights_forGNN", "TTVA_AMVFVertices_forGNN"] )
                                                                                 )

    stdInputExtDic["JetTrackUsedInFitDecoCustomVtxGNN"] = JetInputExternal("JetTrackUsedInFitDecoCustomVtxGNN", xAODType.TrackParticle,
                                                                        prereqs= [ f"input:{context['Tracks']}" , # in std context, this is InDetTrackParticles (see StandardJetContext)
                                                                                  f"input:{context['Vertices']}"],
                                                                        algoBuilder = getUsedInVertexFitTrackDecoratorAlgCustomVtxGNN
                                                                        )

    stdInputExtDic["JetTrackVtxAssocCustomVtxGNN"] = JetInputExternal("JetTrackVtxAssocCustomVtxGNN",  xAODType.TrackParticle,
                                              algoBuilder = lambda jdef,_ : jrtcfg.getJetTrackVtxAlg(jdef._contextDic, algname="jetTVACustomVtxGNN",
                                                                                                                       WorkingPoint="Nonprompt_All_MaxWeight",
                                                                                                                       AMVFVerticesDeco='TTVA_AMVFVertices_forGNN',
                                                                                                                       AMVFWeightsDeco='TTVA_AMVFWeights_forGNN'),
                                              prereqs = [ "input:JetTrackUsedInFitDecoCustomVtxGNN", f"input:{context['Vertices']}" ] )

    stdInputExtDic["EventDensityCustomVtxGNN"] =     JetInputExternal("EventDensity", "EventShape", algoBuilder = buildEventShapeAlg,
                      containername = lambda jetdef, _ : "Kt4"+jetdef.inputdef.label+"EventShape",
                      prereqs = lambda jetdef : ["input:"+jetdef.inputdef.name] )



    from JetRecConfig.StandardJetMods import stdJetModifiers
    from JetRecConfig.JetDefinition import JetModifier
    from JetCalibTools import JetCalibToolsConfig

    stdJetModifiers.update(

      CalibCustomVtxGNN = JetModifier("JetCalibrationTool","jetcalib_jetcoll_calibseqCustomVtxGNN",
                                        createfn=JetCalibToolsConfig.getJetCalibToolFromString,
                                        prereqs=lambda mod,jetdef : JetCalibToolsConfig.getJetCalibToolPrereqs(mod,jetdef)+[f"input:{context['Vertices']}"]),


      JVFCustomVtxGNN =             JetModifier("JetVertexFractionTool", "jvfCustomVtxGNN",
                                        createfn= lambda jdef,_ : JetMomentToolsConfig.getJVFTool(jdef,"CustomVtxGNN"),
                                        modspec = "CustomVtxGNN",
                                        prereqs = ["input:JetTrackVtxAssocCustomVtxGNN", "mod:TrackMomentsCustomVtxGNN", f"input:{context['Vertices']}"] ,
                                             JetContainer = CustomJetContainerName),

      JVTCustomVtxGNN =             JetModifier("JetVertexTaggerTool", "jvtCustomVtxGNN",
                                        createfn= lambda jdef,_ : JetMomentToolsConfig.getJVTTool(jdef,"CustomVtxGNN"),
                                        modspec = "CustomVtxGNN",
                                        prereqs = [ "mod:JVFCustomVtxGNN" ],JetContainer = CustomJetContainerName),

      NNJVTCustomVtxGNN =           JetModifier("JetVertexNNTagger", "nnjvtCustomVtxGNN",
                                              createfn=lambda jdef,_ :JetMomentToolsConfig.getNNJvtTool(jdef,"CustomVtxGNN"),
                                              prereqs = [ "mod:JVFCustomVtxGNN" ],JetContainer = CustomJetContainerName),

      OriginSetPVCustomVtxGNN =     JetModifier("JetOriginCorrectionTool", "origin_setpvCustomVtxGNN",
                                        modspec = "CustomVtxGNN",
                                        prereqs = [ "mod:JVFCustomVtxGNN" ],JetContainer = CustomJetContainerName, OnlyAssignPV=True),

      TrackMomentsCustomVtxGNN =    JetModifier("JetTrackMomentsTool", "trkmomsCustomVtxGNN",
                                        createfn= lambda jdef,_ : JetMomentToolsConfig.getTrackMomentsTool(jdef,"CustomVtxGNN"),
                                        modspec = "CustomVtxGNN",
                                        prereqs = [ "input:JetTrackVtxAssocCustomVtxGNN","ghost:Track" ],JetContainer = CustomJetContainerName),

      TrackSumMomentsCustomVtxGNN = JetModifier("JetTrackSumMomentsTool", "trksummomsCustomVtxGNN",
                                        createfn=lambda jdef,_ :JetMomentToolsConfig.getTrackSumMomentsTool(jdef,"CustomVtxGNN"),
                                        modspec = "CustomVtxGNN",
                                        prereqs = [ "input:JetTrackVtxAssocCustomVtxGNN","ghost:Track" ],JetContainer = CustomJetContainerName),

      ChargeCustomVtxGNN =          JetModifier("JetChargeTool", "jetchargeCustomVtxGNN", 
                                        prereqs = [ "ghost:Track" ]),


      QGTaggingCustomVtxGNN =       JetModifier("JetQGTaggerVariableTool", "qgtaggingCustomVtxGNN",
                                        createfn=lambda jdef,_ :JetMomentToolsConfig.getQGTaggingTool(jdef,"CustomVtxGNN"),
                                        modspec = "CustomVtxGNN",
                                        prereqs = lambda _,jdef :
                                             ["input:JetTrackVtxAssocCustomVtxGNN","mod:TrackMomentsCustomVtxGNN"] +
                                             (["mod:JetPtAssociation"] if not jdef._cflags.Input.isMC else []),
                                        JetContainer = CustomJetContainerName),

      fJVTCustomVtxGNN =            JetModifier("JetForwardPFlowJvtTool", "fJVTCustomVtxGNN",
                                        createfn=lambda jdef,_ :JetMomentToolsConfig.getPFlowfJVTTool(jdef,"CustomVtxGNN"),
                                        modspec = "CustomVtxGNN",
                                        prereqs = ["input:JetTrackVtxAssocCustomVtxGNN","input:EventDensityCustomVtxGNN",f"input:{context['Vertices']}","mod:NNJVTCustomVtxGNN"],
                                        JetContainer = CustomJetContainerName),
      jetisoCustomVtxGNN = JetModifier("JetIsolationTool","isoCustomVtxGNN",
                           JetContainer=CustomJetContainerName,
                           InputConstitContainer = "EMOriginCustomVtxGNNTopoClusters",
                           IsolationCalculations = ["IsoFixedCone:5:Pt",   "IsoFixedCone:5:PtPUsub",],
                           RhoKey = lambda jetdef, specs : "Kt4"+jetdef.inputdef.label+"CustomVtxGNNEventShape" ,
                           prereqs= ["input:EventDensityCustomVtxGNN"], #lambda spec,jetdef : ["input:Kt4"+jetdef.inputdef.label+"EventShape",],
                           ),

    )


    from JetRecConfig.JetRecConfig import JetRecCfg

    acc.merge(JetRecCfg(flags,AntiKt4EMTopoCustomVtxGNN))

    return acc


# Main algorithm config
def GNNVertexCfg(flags, **kwargs):
    acc = ComponentAccumulator()

    from SGComps.AddressRemappingConfig import InputRenameCfg
    acc.merge(InputRenameCfg("xAOD::VertexContainer", "PrimaryVertices", "PrimaryVertices_initial"))
    acc.merge(InputRenameCfg("xAOD::VertexAuxContainer", "PrimaryVerticesAux.", "PrimaryVertices_initialAux."))

    acc.merge(CustomJetsCfg(flags))

    ## Hard-scatter vertex selection
    from InDetConfig.InDetGNNHardScatterSelectionConfig import GNNSequenceCfg
    acc.merge(GNNSequenceCfg(flags))

    from TrkConfig.TrkVertexToolsConfig import GNNVertexCollectionSortingToolCfg
    vxsort_gnn = GNNVertexCollectionSortingToolCfg(flags)
    from InDetPriVxFinder.ResortVerticesConfig import ResortVerticesCfg
    acc.merge(ResortVerticesCfg(flags, "PrimaryVertices_initial", "PrimaryVertices", vxsort_gnn))

    return acc

