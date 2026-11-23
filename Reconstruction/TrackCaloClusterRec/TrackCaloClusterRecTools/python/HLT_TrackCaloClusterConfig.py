#   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
from AthenaCommon import Logging
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.CFElements import parOR

ufolog = Logging.logging.getLogger('TCCorUFO')

def getDecorationKeyFunc(trackParticleName, assocPostfix):
    """Simple helper returning a function to build decoration keys """
    return lambda d : trackParticleName+'.'+d+assocPostfix

def setupTrackCaloAssoc(flags, caloClusterName="CaloCalTopoClusters",detectorEtaName="default",trackParticleName="InDetTrackParticles", vtxName="HLT_IDVertex_FS",assocPostfix = "TCC", onlyPV0Tracks=False, seqname="HLTUFO_ftf"):
    """ Schedule a TrackParticleClusterAssociationAlg in the top sequence, taking as input clusters and tracks defined
    by the keys caloClusterName and trackParticleName.

    onlyPV0Tracks : calculate associated clusters only for PV0 tracks. Avoids unnecessary calculation (used in the UFO case).
       (IMPORTANT CaloExtensionBuilderAlg does extrapolate all tracks : if too much time consuming, it needs a new option to mask non-PV0 tracks)
    """
    ###################################

    components = ComponentAccumulator()
    components.addSequence(parOR(seqname))

    from TrackToCalo.CaloExtensionBuilderAlgCfg import CaloExtensionBuilderAlgCfg
    caloExtAlg = CaloExtensionBuilderAlgCfg( flags, TrkPartContainerName=trackParticleName)
    components.merge(caloExtAlg)    #since its a stack of algorithms

    from TrackVertexAssociationTool.TrackVertexAssociationToolConfig import TTVAToolCfg
    TrackVertexAssoTool = components.popToolsAndMerge(TTVAToolCfg(flags,
                                                                  "tvaTool",
                                                                  TrackContName=trackParticleName,
                                                                  VertexContName=vtxName,
                                                                  WorkingPoint="Nonprompt_All_MaxWeight",
                                                                  HardScatterLinkDeco=""))
    
    trackParticleClusterAssociation = CompFactory.TrackParticleClusterAssociationAlg(
        "TrackParticleClusterAssociationAlg"+assocPostfix,
        TrackParticleContainerName = trackParticleName,
        PtCut = 400.,
        CaloExtensionName = (caloExtAlg.getEventAlgos()[0]).ParticleCache, # ParticleCache is a defunct attribute
        CaloClusterLocation = caloClusterName,
        DetectorEtaName = detectorEtaName if detectorEtaName.lower() != "default" else ("DetectorEta" if "Origin" in caloClusterName else ""),
        TrackVertexAssoTool=TrackVertexAssoTool, # will associate trks from PV0 only
        VertexContainerName = vtxName if onlyPV0Tracks else "",
        AssociatedClusterDecorKey = "AssoClusters"+assocPostfix,
        UseCovariance = flags.UFO.UseCov,
        DeltaR = flags.UFO.dR,
    )

    components.addEventAlgo( trackParticleClusterAssociation, seqname)
    return components



def runUFOReconstruction(flags, constits, trackParticleName, vtxName, caloClusterName="CaloCalTopoClusters", detectorEtaName = "default", assocPostfix="HLT_UFO", inputFEcontainerkey="", trackingType=""):
    
    """Create a UFO collection from PFlow and tracks (PFO retrieved from PFOPrefix and tracks directly from trackParticleName). 
    This functions schedules 2 UFO specific algs : 
       * a TrackCaloClusterInfoUFOAlg to build the TrackCaloClusterInfo object
       * a TrackCaloClusterAlg to build the UFO
    """
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    components=ComponentAccumulator()

    pfoVariant = constits.label.split("PFlow")[-1] # We expect label to be EMPFlowXYZ or LCPFlowXYZ -> extrat XYZ
    
    seqname = f'HLTUFO{pfoVariant}_ftf'
    components.addSequence(parOR(seqname))
    
    decorKey = getDecorationKeyFunc(trackParticleName,assocPostfix)    

    components.merge(setupTrackCaloAssoc(flags, caloClusterName, detectorEtaName, trackParticleName, vtxName, assocPostfix, onlyPV0Tracks=True, seqname=seqname),seqname)

    from TrackVertexAssociationTool.TrackVertexAssociationToolConfig import TTVAToolCfg
    commonArgs=dict(
        TrackVertexAssoTool = components.popToolsAndMerge(TTVAToolCfg(flags,"tvaTool",
                                                                      TrackContName=trackParticleName,VertexContName=vtxName,
                                                                      WorkingPoint="Nonprompt_All_MaxWeight",HardScatterLinkDeco="")),
        AssoClustersDecor=decorKey("AssoClusters"),
    )

    inputFEcontainerkey = inputFEcontainerkey or constits.containername

    UFOInfoAlg = CompFactory.TrackCaloClusterInfoUFOAlg(f"UFOInfoAlg{pfoVariant}",
                                                        TCCInfoName = pfoVariant+"UFOInfo",
                                                        InputTracks = trackParticleName,
                                                        InputClusters = caloClusterName,
                                                        VertexContainer = vtxName,
                                                        InputPFO=inputFEcontainerkey, 
                                                        OriginPFO='originalObjectLink',
                                                        ClusterECut = 0.,
                                                        **commonArgs
    )        
        
    components.addEventAlgo( UFOInfoAlg, seqname) 

    tccUFO = CompFactory.UFOTool(f"UFOtool{pfoVariant}",
                                 ClusterECut = UFOInfoAlg.ClusterECut,                     
                                 InputPFO=inputFEcontainerkey, 
                                 OriginPFO='originalObjectLink',
                                 **commonArgs
                                 )    
    
    UFOAlg = CompFactory.TrackCaloClusterAlg(name = f"TrackCaloClusterAlgUFO{pfoVariant}",
                                             OutputTCCName = f"HLT_{trackingType}UFO{pfoVariant}",
                                             TCCInfo = UFOInfoAlg.TCCInfoName ,
                                             TCCTools = [tccUFO,],
    )

    

    components.addEventAlgo( UFOAlg, seqname)
    return components
