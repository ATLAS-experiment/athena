# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from collections import defaultdict
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

# Create a dictionnary which auto-generates a number for each unique entry it is given
# this is used to generate a unique alg name for different pairs of trk,vtx container names
_trkvtxPairCounter = defaultdict()
_trkvtxPairCounter.default_factory = lambda : str(len(_trkvtxPairCounter))

def idForTrackVtxContainers(trackCont, vtxCont, vtxdeco, wdeco):
    return "InDetUsedInFitDecorator_"+_trkvtxPairCounter[ (trackCont, vtxCont, vtxdeco, wdeco) ]


def getUsedInVertexFitTrackDecoratorAlg(
        trackCont = "InDetTrackParticles", vtxCont = "PrimaryVertices",
        vertexDeco = "TTVA_AMVFVertices_forReco", weightDeco = "TTVA_AMVFWeights_forReco"):
    """ Create the alg  to decorate the used-in-fit information for AMVF """
    
    alg = CompFactory.InDet.InDetUsedInVertexFitTrackDecorator(
        name=idForTrackVtxContainers(trackCont,vtxCont, vertexDeco, weightDeco)+"Alg",
        UsedInFitDecoratorTool=CompFactory.InDet.InDetUsedInFitTrackDecoratorTool(
            AMVFVerticesDecoName=vertexDeco,
            AMVFWeightsDecoName=weightDeco,
            TrackContainer=trackCont,
            VertexContainer=vtxCont)
    )

    return alg

def UsedInVertexFitTrackDecoratorCfg(
        flags, trackCont = 'InDetTrackParticles', vtxCont = 'PrimaryVertices'):
    """ Create the ComponentAccumulator  to decorate the used-in-fit information for AMVF """
    acc = ComponentAccumulator()
    acc.addEventAlgo( getUsedInVertexFitTrackDecoratorAlg( trackCont, vtxCont ) )
    return acc
