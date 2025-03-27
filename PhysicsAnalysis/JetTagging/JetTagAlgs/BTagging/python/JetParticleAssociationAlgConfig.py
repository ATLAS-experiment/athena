# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration

# this is just a thin wrapper on ParticleJetTools, I want to
# keep the scope of the MR where we moved this limited. Eventually I
# should move the things that point to this to point to the library
# below.

from ParticleJetTools.JetParticleAssociationAlgConfig import ( # noqa: F401
    JetParticleAssociationAlgCfg)                              # noqa: F401

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from math import inf

def JetParticleAssociationByVertexCfg(ConfigFlags, jetCollName, partcollname, assocname, dzCut, useMinZ0Vertex, **options):

    acc=ComponentAccumulator()

    options["coneSizeFitPar1"] = +0.239
    options["coneSizeFitPar2"] = -1.220
    options["coneSizeFitPar3"] = -1.64e-5
    options["InputParticleContainer"] = partcollname
    options["OutputDecoration"] = assocname
    options["dzCut"] = dzCut
    options["useMinZ0Vertex"] = useMinZ0Vertex
    # -- create the association tool
    acc.setPrivateTools(
    CompFactory.JetParticleOriginVertexAssociation(
        JetContainer=jetCollName, **options))
    

    return acc


def JetParticleAssociationByVertexAlgCfg(
        ConfigFlags,
        JetCollection,
        InputParticleCollection,
        OutputParticleDecoration,
        MinimumJetPt=None,
        MinimumJetPtFlag=None,
        dzCut=10,
        useMinZ0Vertex=True):

    acc=ComponentAccumulator()
    jetcol = JetCollection
    name=(jetcol + "_" + OutputParticleDecoration).lower()
    if useMinZ0Vertex:
        decorName="_" + str(dzCut) + "_inclusive_assoc"
    else:
        decorName="_" + str(dzCut) + "_exclusive_assoc"
    if MinimumJetPt is None:
        MinimumJetPt = ConfigFlags.BTagging.minimumJetPtForTrackAssociation
    if MinimumJetPt > 0.0 and MinimumJetPtFlag is None:
        ptflag = f'{OutputParticleDecoration}OverPtThreshold'
    elif MinimumJetPtFlag is not None:
        ptflag = MinimumJetPtFlag
    else:
        ptflag = ''

    # -- create the association algorithm
    acc.addEventAlgo(CompFactory.JetDecorationAlg(
        name=name+decorName,
        JetContainer=jetcol,
        Decorators=[
            acc.popToolsAndMerge(
                JetParticleAssociationByVertexCfg(
                    ConfigFlags,
                    jetcol,
                    InputParticleCollection,
                    OutputParticleDecoration+decorName,
                    MinimumJetPt=MinimumJetPt,
                    PassPtFlag=ptflag+decorName,
                    dzCut=dzCut,
                    useMinZ0Vertex=useMinZ0Vertex,
                ))
        ]
    ))

    return acc