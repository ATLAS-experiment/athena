# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

########################################################################
#                                                                      #
# JetModConfig: A helper module for configuring tools that support     #
# jet reconstruction                                                   #
# Author: TJ Khoo                                                      #
#                                                                      #
########################################################################

from AthenaCommon import Logging
jrtlog = Logging.logging.getLogger('JetRecToolsConfig')

from AthenaConfiguration.ComponentFactory import CompFactory
from JetRecConfig.JetRecConfig import isAnalysisRelease



def getIDTrackSelectionTool(toolname, **toolProps):
    """returns a InDetTrackSelectionTool configured with toolProps
     (except in Analysis releases where some un-used (?) options are explicitely turned off)
    """
    idtracksel = CompFactory.getComp("InDet::InDetTrackSelectionTool")(toolname, **toolProps)

    if not isAnalysisRelease():
        # thes options can not be set in AnalysisBase/AthAnalysis. (but not setting them is equivalent to set them to False)
        idtracksel.UseTrkTrackTools = False
        idtracksel.Extrapolator     = ""
        idtracksel.TrackSummaryTool = ""
    return idtracksel

def getTrackSelAlg(jetdef, trackSelOpt=False):
    trkProperties = jetdef._contextDic
    trkOpt=jetdef.context

    trackToolProps = dict(**trkProperties["trackSelOptions"])

    if not trackSelOpt:
        # track selection from trkOpt but OVERWRITING the CutLevel for e.g. ghosts (typically, only a pt>500MeV cut remains): 
        trackToolProps.update( CutLevel=trkProperties['GhostTrackCutLevel'] )
        outContainerKey = "JetTracks"
        trkOpt= trkOpt+'ghost' # just so it gives a different tool name below
    else:
        outContainerKey = "JetTracksQualityCuts"

    # build the selection alg 
    trkSelAlg = CompFactory.JetTrackSelectionAlg( f"trackselalg_{trkOpt}_{trkProperties[outContainerKey]}",
                                                  TrackSelector = getIDTrackSelectionTool(f"tracksel{trkOpt}",**trackToolProps),
                                                  InputContainer = trkProperties["Tracks"],
                                                  OutputContainer = trkProperties[outContainerKey],
                                                  DecorDeps = ["TTVA_AMVFWeights_forReco", "TTVA_AMVFVertices_forReco"] # Hardcoded for now... we might want to have this context-dependent ??
                                                 )

    return trkSelAlg


def getJetTrackVtxAlg( trkProperties, algname="jetTVA", **ttva_overide):
    """  theSequence and ttva_overide are options used in trigger  (HLT/Jet/JetTrackingConfig.py)"""
    from InDetConfig.TrackVertexAssociationToolConfig import getTTVAToolForReco

    ttva_options = dict(
        TrackContName = trkProperties["Tracks"],
        HardScatterLinkDeco = ""
    )
    # allow client to overide options : 
    ttva_options.update(**ttva_overide)
    
    idtvassoc = getTTVAToolForReco( "jetTVatool", **ttva_options )

    alg = CompFactory.JetTrackVtxAssoAlg(algname,
                                         TrackParticleContainer  = trkProperties["Tracks"],
                                         TrackVertexAssociation  = trkProperties["TVA"],
                                         VertexContainer         = trkProperties["Vertices"],
                                         TrackVertexAssoTool     = idtvassoc                                         
                                         )
    return alg


def getPFlowSelAlg():
    # PFlow objects matched to electrons/muons filtering algorithm 
    return  CompFactory.JetPFlowSelectionAlg( "pflowselalg",
                                              electronID = "LHMedium",
                                              ChargedPFlowInputContainer  = "JetETMissChargedParticleFlowObjects",
                                              NeutralPFlowInputContainer  = "JetETMissNeutralParticleFlowObjects",
                                              ChargedPFlowOutputContainer = "GlobalPFlowChargedParticleFlowObjects",
                                              NeutralPFlowOutputContainer = "GlobalPFlowNeutralParticleFlowObjects"
                                             )
