# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory


def TruthTrackRetrieverCfg(flags, name="TruthTrackRetriever", **kwargs):
    # Based on TruthJiveXML_DataTypes.py (and JiveXML_RecEx_config.py)
    result = ComponentAccumulator()
    kwargs.setdefault("StoreGateKey", "TruthEvent")
    the_tool = CompFactory.JiveXML.TruthTrackRetriever(name, **kwargs)
    result.addPublicTool(the_tool, primary=True)
    return result
