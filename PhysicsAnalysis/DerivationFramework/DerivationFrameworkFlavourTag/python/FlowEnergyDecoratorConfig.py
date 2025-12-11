"""
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def FlowEnergyDecoratorCfg(**kwargs):
    """
    Function to configure FlowEnergyDecorator for Athena.
    FlowEnergyDecorator decorates UFO's (UFOCSSK) with energy cluster's information.
    """

    ca = ComponentAccumulator()
    kwargs.setdefault("UFOContainer", "UFOCSSK")
    kwargs.setdefault("PFlowContainer", "GlobalNeutralParticleFlowObjects")
    layerEnergiesEM  = kwargs.pop("layerEnergiesEM", [ "PreSamplerB", "PreSamplerE", "EMB1", "EMB2", "EMB3", "EME1", "EME2", "EME3", "FCAL0" ])
    layerEnergiesHAD = kwargs.pop("layerEnergiesHAD", [ "TileBar0", "TileBar1", "TileBar2", "TileExt0", "TileExt1", "TileExt2",
                         "TileGap1", "TileGap2", "TileGap3", "FCAL1", "FCAL2", "HEC1", "HEC2", "HEC3" ])
    accessorLayerEnergiesEM = ["LAYERENERGY_" + layer for layer in layerEnergiesEM]
    decorLayerEnergiesEM = ["e" + layer for layer in layerEnergiesEM]
    accessorLayerEnergiesHAD = ["LAYERENERGY_" + layer for layer in layerEnergiesHAD]
    decorLayerEnergiesHAD = ["e" + layer for layer in layerEnergiesHAD]
    kwargs.setdefault("LayerEnergyAccessorsEM", accessorLayerEnergiesEM)
    kwargs.setdefault("LayerEnergyDecoratorsEM", decorLayerEnergiesEM)
    kwargs.setdefault("LayerEnergyAccessorsHAD", accessorLayerEnergiesHAD)
    kwargs.setdefault("LayerEnergyDecoratorsHAD", decorLayerEnergiesHAD)
    decorator = CompFactory.FlowEnergyDecorator(
        name="FlowEnergyDecorator", **kwargs
    )

    ca.addEventAlgo(decorator)

    return ca
