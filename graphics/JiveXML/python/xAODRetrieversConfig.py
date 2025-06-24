# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def xAODElectronRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODElectronRetriever(
            name="xAODElectronRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODMissingETRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODMissingETRetriever(
        name="xAODMissingETRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODMuonRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODMuonRetriever(
            name="xAODMuonRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODPhotonRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODPhotonRetriever(
            name="xAODPhotonRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODJetRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODJetRetriever(
        name="xAODJetRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODTauRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODTauRetriever(
            name="xAODTauRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODTrackParticleRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODTrackParticleRetriever(
        name="xAODTrackParticleRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODVertexRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODVertexRetriever(
            name="xAODVertexRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODCaloClusterRetrieverCfg(flags, **kwargs):
    result = ComponentAccumulator()
    the_tool = CompFactory.JiveXML.xAODCaloClusterRetriever(
            name="xAODCaloClusterRetriever")
    result.addPublicTool(the_tool, primary=True)
    return result


def xAODRetrieversCfg(flags):
    result = ComponentAccumulator()
    tools = []
    # It's not really necessary to configure these, since nothing depends on flags.
    # We could just make all this the default in cpp (if it is not already)
    tools += [result.getPrimaryAndMerge(xAODElectronRetrieverCfg(flags))]
    tools += [result.getPrimaryAndMerge(xAODMissingETRetrieverCfg(flags))]
    tools += [result.getPrimaryAndMerge(xAODMuonRetrieverCfg(flags))]
    tools += [result.getPrimaryAndMerge(xAODPhotonRetrieverCfg(flags))]
    tools += [result.getPrimaryAndMerge(xAODJetRetrieverCfg(flags))]
    tools += [result.getPrimaryAndMerge(xAODTauRetrieverCfg(flags))]
    tools += [result.getPrimaryAndMerge(xAODTrackParticleRetrieverCfg(flags))]
    tools += [result.getPrimaryAndMerge(xAODVertexRetrieverCfg(flags))]
    tools += [result.getPrimaryAndMerge(xAODCaloClusterRetrieverCfg(flags))]

    return result, tools
