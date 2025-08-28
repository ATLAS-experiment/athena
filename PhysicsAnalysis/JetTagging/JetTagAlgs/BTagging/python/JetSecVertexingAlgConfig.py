# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from BTagging.MSVVariablesFactoryConfig import MSVVariablesFactoryCfg


def JetSecVertexingAlgCfg(flags, BTagVxSecVertexInfoName, SVAlgName, JetCollection, TrackCollection, PrimaryVertexCollectionName="", SVFinder="", **options):
    """Adds a SecVtxTool instance and registers it.

    input: name:               The tool's name.
           JetCollection       The name of the jet collections.
           ToolSvc:            The ToolSvc instance.
           options:            Python dictionary of options to be passed to the SecVtxTool.
    output: The tool."""

    acc = ComponentAccumulator()

    if SVFinder == 'JetFitter':
        JetSVLink = 'JFVtx'
    if SVFinder == 'JetFitterFlip':#Flip version of jetFitter
        JetSVLink = 'JFVtxFlip'
    if SVFinder == 'SV1':
        JetSVLink = 'SecVtx'
    if SVFinder == 'SV1Flip':
        JetSVLink = 'SecVtxFlip'
    if SVFinder == 'MSV':
        JetSVLink = 'MSecVtx' # Maybe no used

    varFactory = acc.popToolsAndMerge(MSVVariablesFactoryCfg(flags, "MSVVarFactory"))
    jetcol_no_suffix = JetCollection.replace("Jets","")
    BTaggingCollection = f'BTagging_{jetcol_no_suffix}'
    options = {}
    options.setdefault('SecVtxFinderxAODBaseName', SVFinder)
    options.setdefault('vxPrimaryCollectionName', PrimaryVertexCollectionName)
    options['JetCollectionName'] = JetCollection
    options['BTagVxSecVertexInfoName'] = BTagVxSecVertexInfoName
    options['TrackCollectionName'] = TrackCollection
    options['BTagJFVtxCollectionName'] = BTaggingCollection + JetSVLink
    options['BTagSVCollectionName'] = BTaggingCollection + JetSVLink
    options['JetSecVtxLinkName'] = JetCollection + '.' + JetSVLink
    options.setdefault('MSVVariableFactory', varFactory)
    options['name'] = SVAlgName

    # -- create the association algorithm
    acc.addEventAlgo(CompFactory.Analysis.JetSecVertexingAlg(**options))

    return acc
