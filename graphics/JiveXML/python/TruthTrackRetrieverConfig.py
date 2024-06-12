# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TruthTrackRetrieverCfg(flags, name="TruthTrackRetriever", **kwargs):
    # Based on TruthJiveXML_DataTypes.py (and JiveXML_RecEx_config.py)
    result = ComponentAccumulator()
    from TrkConfig.AtlasExtrapolatorConfig import AtlasExtrapolatorCfg

    extrap = result.popToolsAndMerge(AtlasExtrapolatorCfg(flags))
    result.addPublicTool(extrap)

    kwargs.setdefault("StoreGateKey", "TruthEvent")
    the_tool = CompFactory.JiveXML.TruthTrackRetriever(name, **kwargs)
    result.addPublicTool(the_tool)
    return result
