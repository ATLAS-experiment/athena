#   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaCommon import Logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
ufolog = Logging.logging.getLogger('TCCorUFO')

#from AthenaConfiguration import UnifyProperties



def _unifyPV0onlyTrkClustAssoc( vxContName1, vxContName2):
    if vxContName2 == vxContName1 : return vxContName1
    if "" in [vxContName1, vxContName2] : return vxContName1 or vxContName2
    raise Exception(" Error : incompatible VertexContainerName of 2 instance of TrackParticleClusterAssociationAlg : '{}' and '{}'".format(vxContName1, vxContName2))


# we make sure a unique TrackParticleAssociationAlg is well behaved when configured for both TCC and UFOInfo
# in the same job. The function below will be used in the de-duplication and implies all tracks (not only PV0)
# have associated clusters only if TCC is scheduled.
#  !! Doesn't seem to work yet ??!!
#UnifyProperties.setUnificationFunction( "TrackParticleClusterAssociationAlg.VertexContainerName", _unifyPV0onlyTrkClustAssoc)
    






def getDecorationKeyFunc(trackParticleName, assocPostfix):
    """Simple helper returning a function to build decoration keys """
    return lambda d : trackParticleName+'.'+d+assocPostfix

def setupTrackCaloAssoc(flags, caloClusterName="CaloCalTopoClusters",detectorEtaName="default",trackParticleName="InDetTrackParticles", assocPostfix = "TCC", onlyPV0Tracks=False):
    """ Schedule a TrackParticleClusterAssociationAlg in the top sequence, taking as input clusters and tracks defined 
    by the keys caloClusterName and trackParticleName.

    onlyPV0Tracks : calculate associated clusters only for PV0 tracks. Avoids unnecessary calculation (used in the UFO case).
       (IMPORTANT CaloExtensionBuilderAlg does extrapolate all tracks : if too much time consuming, it needs a new option to mask non-PV0 tracks)
    """
    ###################################


    decorKey = getDecorationKeyFunc(trackParticleName,assocPostfix)

    components = ComponentAccumulator()

    from TrackToCalo.CaloExtensionBuilderAlgCfg import CaloExtensionBuilderAlgCfg 
    caloExtAlg =CaloExtensionBuilderAlgCfg( flags )
    caloExtAlg.TrkPartContainerName = trackParticleName

    components.merge(caloExtAlg)    #since its a stack of algorithms

    from InDetConfig.TrackVertexAssociationToolConfig import TTVAToolCfg
    TrackVertexAssoTool = components.popToolsAndMerge(
        TTVAToolCfg(flags, "tvaTool", WorkingPoint="Nonprompt_All_MaxWeight"))

    trackParticleClusterAssociation = CompFactory.TrackParticleClusterAssociationAlg(
        "TrackParticleClusterAssociationAlg"+assocPostfix,
        #ParticleCaloClusterAssociationTool = particleCaloClusterAssociation,
        TrackParticleContainerName = trackParticleName,
        PtCut = 400.,
        CaloExtensionName = (caloExtAlg.getEventAlgos()[0]).ParticleCache, # ParticleCache is a defunct attribute
        CaloClusterLocation = caloClusterName,
        DetectorEtaName = detectorEtaName if detectorEtaName.lower() != "default" else ("DetectorEta" if "Origin" in caloClusterName else ""),
        TrackVertexAssoTool=TrackVertexAssoTool, # will associate trks from PV0 only
        VertexContainerName = "PrimaryVertices" if onlyPV0Tracks else "",
        #VertexContainerName = "PrimaryVertices" if onlyPV0Tracks else "TTVA_AMVFVertices",
        AssociatedClusterDecorKey = decorKey("AssoClusters"),
        UseCovariance = flags.UFO.UseCov,
        DeltaR = flags.UFO.dR,
#        OutputLevel=2
    )


    components.addEventAlgo( trackParticleClusterAssociation )
    return components

    

def runTCCReconstruction(flags, caloClusterName="CaloCalTopoClusters", detectorEtaName = "default", trackParticleName="InDetTrackParticles",
                         assocPostfix="TCC", doCombined=False, doCharged=False, doNeutral=True, outputTCCName="TrackCaloClusters"):
    """
    Create a TrackCaloCluster collection from clusters and tracks (caloClusterName and trackParticleName).
    Depending on options, the collection contains combined, neutral and/or charged TCC.
    This functions schedules 2 TCC specific algs :
       * a TrackCaloClusterInfoAlg to build the TrackCaloClusterInfo object
       * a TrackCaloClusterAlg to build the TCC
    """

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

    decorKey = getDecorationKeyFunc(trackParticleName,assocPostfix)

    components = ComponentAccumulator()    
    
    components.merge(
        setupTrackCaloAssoc(flags, caloClusterName, detectorEtaName, trackParticleName, assocPostfix, onlyPV0Tracks=False)
    )

    ###################################
    # Schedule the TrackCaloClusterInfoAlg to create the weights for clusters/tracks and store them in a TrackCaloClusterInfo object.
    tccInfoAlg = CompFactory.TrackCaloClusterInfoAlg(
        "TCCInfoAlg",
        TCCInfoName = "TCCInfo",
        InputTracks = trackParticleName,
        InputClusters = caloClusterName,
        VertexContainer = "PrimaryVertices",
        AssoClustersDecor = decorKey("AssoClusters"),
    )

    components.addEventAlgo( tccInfoAlg) 
    
    ###################################
    # Create the TCC creator alg. TrackCaloClusterAlg makes use of the TrackCaloClusterInfo object
    # and a list of tools to build the various TCC types.
    tccTools = []

    from InDetConfig.TrackVertexAssociationToolConfig import TTVAToolCfg
    commonArgs=dict(
        TrackVertexAssoTool = components.popToolsAndMerge(
            TTVAToolCfg(flags,"tvaTool",WorkingPoint="Nonprompt_All_MaxWeight")),
        AssoClustersDecor=decorKey("AssoClusters"),            
    )    

    if doCombined:
        tccCombined = CompFactory.TCCCombinedTool("TCCcombined", **commonArgs)
        tccTools.append(tccCombined)
    if doCharged:
        tccCharged = CompFactory.TCCChargedTool("TCCCharged", **commonArgs )
        tccTools.append(tccCharged)
    if doNeutral:
        tccNeutral = CompFactory.TCCNeutralTool("TCCNeutral", **commonArgs )        
        tccTools.append(tccNeutral)

    tccAlg = CompFactory.TrackCaloClusterAlg(name = "TrackCaloClusterAlg",
            OutputTCCName = outputTCCName,
            TCCInfo       = "TCCInfo",
            TCCTools      = tccTools,
    )

    components.addEventAlgo(tccAlg)

    return components


def runUFOReconstruction(flags, constits, caloClusterName="CaloCalTopoClusters", detectorEtaName = "default", assocPostfix="UFO", inputFEcontainerkey=""):
    
    """Create a UFO collection from PFlow and tracks (PFO retrieved from PFOPrefix and tracks directly from trackParticleName). 
    This functions schedules 2 UFO specific algs : 
       * a TrackCaloClusterInfoUFOAlg to build the TrackCaloClusterInfo object
       * a TrackCaloClusterAlg to build the UFO
    """
    from JetRecConfig.JetDefinition import JetDefinition
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    components=ComponentAccumulator()

    if isinstance(constits, JetDefinition):
        jdef = constits 
        constits = jdef.inputdef
        trackParticleName = jdef._contextDic['Tracks'] # defaults to "InDetTrackParticles"
    else:
        trackParticleName = "InDetTrackParticles"
        
    pfoVariant= constits.label.split("PFlow")[-1] # We expect label to be EMPFlowXYZ or LCPFlowXYZ -> extrat XYZ

    decorKey = getDecorationKeyFunc(trackParticleName,assocPostfix)    
    
    

    components.merge(
        setupTrackCaloAssoc(flags, caloClusterName, detectorEtaName, trackParticleName, assocPostfix, onlyPV0Tracks=False) #onlyPV0Tracks True was original option
    )
    

    from InDetConfig.TrackVertexAssociationToolConfig import TTVAToolCfg
    commonArgs=dict(
        TrackVertexAssoTool = components.popToolsAndMerge(
            TTVAToolCfg(flags,"tvaTool",WorkingPoint="Nonprompt_All_MaxWeight")),
        AssoClustersDecor=decorKey("AssoClusters"),
    )    
    
    
    inputFEcontainerkey = inputFEcontainerkey or constits.containername
            
        
    UFOInfoAlg = CompFactory.TrackCaloClusterInfoUFOAlg(f"UFOInfoAlg{pfoVariant}",
                                                        TCCInfoName = pfoVariant+"UFOInfo",
                                                        InputTracks = trackParticleName,
                                                        InputClusters = caloClusterName,
                                                        VertexContainer = "PrimaryVertices",
                                                        InputPFO=inputFEcontainerkey, 
                                                        OriginPFO='originalObjectLink',
                                                        ClusterECut = 0.,
                                                        **commonArgs
    )
        
        
    components.addEventAlgo( UFOInfoAlg) 

    tccUFO = CompFactory.UFOTool(f"UFOtool{pfoVariant}",
                                 ClusterECut = UFOInfoAlg.ClusterECut,                     
                                 InputPFO=inputFEcontainerkey, 
                                 OriginPFO='originalObjectLink',
                                 **commonArgs
                                 )    

    UFOAlg = CompFactory.TrackCaloClusterAlg(name = f"TrackCaloClusterAlgUFO{pfoVariant}",
                                             OutputTCCName = f"UFO{pfoVariant}",
                                             TCCInfo = UFOInfoAlg.TCCInfoName ,
                                             TCCTools = [tccUFO,],
    )

    

    components.addEventAlgo( UFOAlg)
    return components
