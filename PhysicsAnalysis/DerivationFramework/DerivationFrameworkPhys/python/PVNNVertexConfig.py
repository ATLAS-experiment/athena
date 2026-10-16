# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#!/usr/bin/env python

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging

logPHYS = logging.getLogger('PHYS')

def addJetContextFlags(flags):
    jetContextName = 'CustomVtxPVNN'
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

    from JetRecConfig.StandardJetConstits import stdInputExtDic, JetInputExternal, JetInputConstitSeq, JetConstitModifier, xAODType
    from JetRecConfig.JetDefinition import JetDefinition 
    from JetRecConfig.StandardSmallRJets import AntiKt4EMTopo
    from JetRecTools import JetRecToolsConfig as jrtcfg
    from JetRecConfig.StandardJetConstits import stdConstitDic, stdContitModifDic
    from JetRecConfig.StandardJetContext import inputsFromContext
    from JetRecConfig.JetInputConfig import buildEventShapeAlg


    #  Get custom jet context
    jetContextName = 'CustomVtxPVNN'
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

        rhoname = "Kt4EMTopoCustomVtxPVNNEventShape" 

        for i, item in enumerate(newList):
            if "Calib" in item:
                calibspecs = item.split(":")
                calib, calibcontext, data_type = calibspecs[:3]
                calibseq=""
                if len(calibspecs)>3: 
                  calibseq = calibspecs[3]
                finalCalibString = f"CalibCustomVtxPVNN:{calibcontext}:{data_type}:{calibseq}:{rhoname}:PrimaryVertices_initial"
                if len(calibspecs)>6: finalCalibString = f"{finalCalibString}:{calibspecs[6]}"
                newList[i] = finalCalibString
                print(finalCalibString)
                return tuple(newList)
        print( "Failed to update calib sequence" )
        return tuple(newList)


    # Create modifier list and JetDefinition 
    modsCustomVtxPVNN = AntiKt4EMTopo.modifiers
    modsCustomVtxPVNN = updateCalibSequence(modsCustomVtxPVNN)
    ghostCustomVtxPVNN = AntiKt4EMTopo.ghostdefs

    # TODO: check this
    stdConstitDic["EMTopoOriginCustomVtxPVNN"]  = JetInputConstitSeq(
        "EMTopoOriginCustomVtxPVNN", xAODType.CaloCluster,
        ["EMCustomVtxPVNN", "OriginCustomVtxPVNN"],
        "CaloCalTopoClusters", "EMOriginCustomVtxPVNNTopoClusters",
        label="EMTopoCustomVtxPVNN")
    stdContitModifDic["OriginCustomVtxPVNN"] = JetConstitModifier(
        "OriginCustomVtxPVNN", "CaloClusterConstituentsOrigin",
        prereqs=[inputsFromContext("Vertices")],
        properties=dict(VertexContainer="PrimaryVertices_initial"))
    stdContitModifDic["EMCustomVtxPVNN"] = JetConstitModifier(
        "EMCustomVtxPVNN", "ClusterAtEMScaleTool")

    AntiKt4EMTopoCustomVtxPVNN = JetDefinition(
        "AntiKt",0.4,stdConstitDic.EMTopoOriginCustomVtxPVNN,
        context = jetContextName,
        ghostdefs = ghostCustomVtxPVNN,
        modifiers = modsCustomVtxPVNN,
        lock = True,
    )

    def getUsedInVertexFitTrackDecoratorAlgCustomVtxPVNN(jetdef, jetmod):
        """ Create the alg  to decorate the used-in-fit information for AMVF """
        context  = jetdef._contextDic

        from InDetUsedInFitTrackDecoratorTool.UsedInVertexFitTrackDecoratorConfig import getUsedInVertexFitTrackDecoratorAlg
        alg = getUsedInVertexFitTrackDecoratorAlg(context['Tracks'], context['Vertices'],
                                                  vertexDeco='TTVA_AMVFVertices_forPVNN',
                                                  weightDeco='TTVA_AMVFWeights_forPVNN')
        return alg

    # Define new input variables for jet configuration
    stdInputExtDic[context['Vertices']] = JetInputExternal( context['Vertices'],   xAODType.Vertex )

    stdInputExtDic["JetSelectedTracksCustomVtxPVNN"] = JetInputExternal("JetSelectedTracksCustomVtxPVNN",     xAODType.TrackParticle,
                                                                                    prereqs= [ f"input:{context['Tracks']}" ], # in std context, this is InDetTrackParticles (see StandardJetContext)
                                                                                    algoBuilder = lambda jdef,_ : jrtcfg.getTrackSelAlg(jdef, trackSelOpt=False,
                                                                                                                                              DecorDeps=["TTVA_AMVFWeights_forPVNN", "TTVA_AMVFVertices_forPVNN"] )
                                                                                 )

    stdInputExtDic["JetTrackUsedInFitDecoCustomVtxPVNN"] = JetInputExternal(
        "JetTrackUsedInFitDecoCustomVtxPVNN", xAODType.TrackParticle,
        prereqs= [ f"input:{context['Tracks']}" , # in std context, this is InDetTrackParticles (see StandardJetContext)
                   f"input:{context['Vertices']}"],
        algoBuilder = getUsedInVertexFitTrackDecoratorAlgCustomVtxPVNN )

    stdInputExtDic["JetTrackVtxAssocCustomVtxPVNN"] = JetInputExternal(
        "JetTrackVtxAssocCustomVtxPVNN",  xAODType.TrackParticle,
        algoBuilder = lambda jdef,_ : jrtcfg.getJetTrackVtxAlg(
            jdef._contextDic, algname="jetTVACustomVtxPVNN",
            WorkingPoint="Nonprompt_All_MaxWeight",
            AMVFVerticesDeco='TTVA_AMVFVertices_forPVNN',
            AMVFWeightsDeco='TTVA_AMVFWeights_forPVNN'),
        prereqs = [ "input:JetTrackUsedInFitDecoCustomVtxPVNN",
                    f"input:{context['Vertices']}" ] )

    stdInputExtDic["EventDensityCustomVtxPVNN"] = JetInputExternal(
        "EventDensity", "EventShape", algoBuilder = buildEventShapeAlg,
        containername = lambda jetdef, _ : "Kt4"+jetdef.inputdef.label+"EventShape",
        prereqs = lambda jetdef : ["input:"+jetdef.inputdef.name] )

    from JetRecConfig.StandardJetMods import stdJetModifiers
    from JetRecConfig.JetDefinition import JetModifier
    from JetCalibTools import JetCalibToolsConfig

    stdJetModifiers.update(

      CalibCustomVtxPVNN = JetModifier("JetCalibrationTool","jetcalib_jetcoll_calibseqCustomVtxPVNN",
                                        createfn=JetCalibToolsConfig.getJetCalibToolFromString,
                                        prereqs=lambda mod,jetdef : JetCalibToolsConfig.getJetCalibToolPrereqs(mod,jetdef)+[f"input:{context['Vertices']}"]),
    )

    from JetRecConfig.JetRecConfig import JetRecCfg

    acc.merge(JetRecCfg(flags,AntiKt4EMTopoCustomVtxPVNN))

    return acc
