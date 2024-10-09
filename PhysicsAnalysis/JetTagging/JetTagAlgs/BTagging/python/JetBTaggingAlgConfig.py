# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from BTagging.BTagToolConfig import BTagToolCfg
from BTagging.BTagLightSecVertexingConfig import BTagLightSecVtxToolCfg


def JetBTaggingAlgCfg(flags,
                      BTaggingCollection,
                      JetCollection,
                      JetColNoJetsSuffix,
                      PrimaryVertexCollectionName,
                      TaggerList,
                      Tracks,
                      Muons,
                      VxSecVertexInfoNameList,
                      secVtxFinderxAODBaseNameList,
                      secVtxFinderTrackNameList,
                      OutgoingTracks="BTagTrackToJetAssociator",
                      OutgoingMuons="Muons"):

    SetupScheme = flags.BTagging.databaseScheme

    acc = ComponentAccumulator()

    options = {}
    options['BTagTool'] = acc.popToolsAndMerge(BTagToolCfg(
        flags, TaggerList, PrimaryVertexCollectionName, SetupScheme))

    # setup the secondary vertexing tool
    if VxSecVertexInfoNameList:
        options['BTagSecVertexing'] = acc.popToolsAndMerge(
            BTagLightSecVtxToolCfg(flags,
                                   'LightSecVx'+flags.BTagging.GeneralToolSuffix,
                                   JetCollection,
                                   VxSecVertexInfoNameList,
                                   secVtxFinderxAODBaseNameList,
                                   secVtxFinderTrackNameList,
                                   PrimaryVertexCollectionName))

    # Set remaining options
    options['JetCollectionName'] = JetCollection
    options['IncomingTracks'] = Tracks
    options['OutgoingTracks'] = OutgoingTracks
    options['IncomingMuons'] = Muons
    options['OutgoingMuons'] = OutgoingMuons
    options['JetCalibrationName'] = (
        flags.BTagging.forcedCalibrationChannel or JetColNoJetsSuffix
    )

    options['BTaggingLinkName'] = options['JetCollectionName'] + '.btaggingLink'
    options['BTaggingCollectionName'] = BTaggingCollection
    options['JetLinkName'] = options['BTaggingCollectionName'] + '.jetLink'
    options['name'] = 'JetBTaggingAlg' + (options['BTaggingCollectionName'] + "_" + options['JetCollectionName'] + flags.BTagging.GeneralToolSuffix).lower()

    # -- create main BTagging algorithm
    acc.addEventAlgo(CompFactory.Analysis.JetBTaggingAlg(**options))

    return acc
