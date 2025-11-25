# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def CaloCellDecoratorCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("SGKey_electrons", flags.Egamma.Keys.Output.Electrons)
    kwargs.setdefault("SGKey_photons", flags.Egamma.Keys.Output.Photons)
    acc.setPrivateTools(CompFactory.DerivationFramework.CaloCellDecorator(**kwargs))
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg

    acc.merge(LArOnOffIdMappingCfg(flags))
    return acc


def MaxCellDecoratorCfg(flags, name="MaxCellDecorator", **kwargs):
    acc = ComponentAccumulator()
    electronKey = kwargs.setdefault("SGKey_electrons", flags.Egamma.Keys.Output.Electrons)
    baseDecorations =["maxEcell_time", "maxEcell_energy", "maxEcell_gain",
                      "maxEcell_onlId", "maxEcell_x", "maxEcell_y", "maxEcell_z"]
    electronDecorations = [electronKey + "." + decor for decor in baseDecorations]
    kwargs.setdefault("SGKey_egammaClusters", "")
    if kwargs["SGKey_egammaClusters"] != '':
        electronDecorations += [electronKey + "." + "dR"]
    # FIXME The electronDecorations definition can be simplified after
    # SG::WriteDecorHandleKeyArray is updated.
    kwargs.setdefault("SGKey_electrons_decorations", electronDecorations)
    kwargs.setdefault("SGKey_photons", flags.Egamma.Keys.Output.Photons)
    acc.setPrivateTools(CompFactory.DerivationFramework.MaxCellDecorator(name, **kwargs))
    from LArCabling.LArCablingConfig import LArOnOffIdMappingCfg

    acc.merge(LArOnOffIdMappingCfg(flags))
    return acc


def GainDecoratorCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    decorationPattern = kwargs.pop("decoration_pattern", "{}_Lr{}_{}G")
    kwargs.setdefault("gain_names", { 0 : "Hi", 1 : "Med", 2 : "Low" })
    kwargs.setdefault("layers", [0, 1, 2, 3])
    decorNames = []
    for x, gain in kwargs["gain_names"].items():
        for layer in kwargs["layers"]:
            decorNames += [decorationPattern.format("E", layer, gain)]
            decorNames += [decorationPattern.format("rnoW", layer, gain)]
            decorNames += [decorationPattern.format("nCells", layer, gain)]
    electronKey = kwargs.setdefault("SGKey_electrons", flags.Egamma.Keys.Output.Electrons)
    kwargs.setdefault("SGKey_electrons_decorations", [electronKey + "." + decor for decor in decorNames])
    photonKey = kwargs.setdefault("SGKey_photons", flags.Egamma.Keys.Output.Photons)
    kwargs.setdefault("SGKey_photons_decorations", [photonKey + "." + decor for decor in decorNames])
    #FIXME Decorations can be simplified once SG::WriteDecorHandleKeyArray is updated
    kwargs.setdefault("name", "GainDecor")
    acc.setPrivateTools(CompFactory.DerivationFramework.GainDecorator(**kwargs))
    return acc


def EgammaCoreCellRecoveryCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    # needed for reading cells, do not rely on other config to do that
    from LArGeoAlgsNV.LArGMConfig import LArGMCfg
    acc.merge(LArGMCfg(flags))
    from TileGeoModel.TileGMConfig import TileGMCfg
    acc.merge(TileGMCfg(flags))
    #
    acc.setPrivateTools(
        CompFactory.DerivationFramework.EGammaClusterCoreCellRecovery(**kwargs)
    )
    return acc


def CaloFillRectangularClusterCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    kwargs.setdefault("cells_name", flags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault("fill_cluster", True)
    acc.setPrivateTools(CompFactory.CaloFillRectangularCluster(**kwargs))
    return acc


def ClusterEnergyPerLayerDecoratorCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    electronKey = kwargs.setdefault("SGKey_electrons", flags.Egamma.Keys.Output.Electrons)
    photonKey = kwargs.setdefault("SGKey_photons", flags.Egamma.Keys.Output.Photons)
    kwargs.setdefault("SGKey_caloCells", flags.Egamma.Keys.Input.CaloCells)
    neta = kwargs.pop("neta", 5)
    nphi = kwargs.pop("nphi", 5)
    kwargs.setdefault("layers", [ 0, 1, 2, 3 ])
    decorBase = "E{}x{}_Lr".format(neta, nphi)
    kwargs.setdefault("SGKey_photons_decorations", [photonKey+"."+decorBase+str(layer) for layer in kwargs['layers']])
    kwargs.setdefault("SGKey_electrons_decorations", [electronKey+"."+decorBase+str(layer) for layer in kwargs['layers']])
    #FIXME The above two lines can be simplified when SG::WriteDecorHandleKeyArray is updated
    toolArgs = {}
    toolArgs.update({"eta_size": neta})
    toolArgs.update({"phi_size": nphi})
    kwargs.setdefault(
        "CaloFillRectangularClusterTool",
        acc.popToolsAndMerge(CaloFillRectangularClusterCfg(flags, **toolArgs)),
    )
    acc.setPrivateTools(
        CompFactory.DerivationFramework.ClusterEnergyPerLayerDecorator(**kwargs)
    )
    return acc


def CaloCellDecoratorKernelCfg(flags, name="CaloCellDecoratorKernel", **kwargs):
    acc = ComponentAccumulator()

    augmentationTools = [
        acc.addPublicTool(acc.popToolsAndMerge(CaloCellDecoratorCfg(flags)))
    ]

    kwargs.setdefault("AugmentationTools", augmentationTools)

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(name, **kwargs))
    return acc


def MaxCellDecoratorKernelCfg(flags, name="MaxCellDecoratorKernel", **kwargs):
    acc = ComponentAccumulator()

    augmentationTools = [
        acc.addPublicTool(acc.popToolsAndMerge(MaxCellDecoratorCfg(flags)))
    ]

    kwargs.setdefault("AugmentationTools", augmentationTools)

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(name, **kwargs))
    return acc


def CaloDecoratorKernelCfg(flags, name="CaloDecoratorKernel", **kwargs):
    acc = MaxCellDecoratorKernelCfg(flags)

    augmentationTools = [
        acc.addPublicTool(acc.popToolsAndMerge(GainDecoratorCfg(flags)))
    ]

    # might need some modification if cell-level reweighting is implemented
    cluster_sizes = (3, 7), (5, 5), (7, 11)
    for neta, nphi in cluster_sizes:
        cename = "ClusterEnergyPerLayerDecorator_%sx%s" % (neta, nphi)
        ClusterEnergyPerLayerDecorator = acc.popToolsAndMerge(
            ClusterEnergyPerLayerDecoratorCfg(flags, neta=neta, nphi=nphi, name=cename)
        )
        augmentationTools.append(acc.addPublicTool(ClusterEnergyPerLayerDecorator))

    kwargs.setdefault("AugmentationTools", augmentationTools)

    acc.addEventAlgo(CompFactory.DerivationFramework.DerivationKernel(name, **kwargs))
    return acc


def CaloClusterThinningCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    CaloClusterThinning = CompFactory.DerivationFramework.CaloClusterThinning
    acc.addPublicTool(CaloClusterThinning(**kwargs), primary=True)
    return acc

def JetCaloClusterThinningCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    JetCaloClusterThinning = CompFactory.DerivationFramework.JetCaloClusterThinning
    acc.addPublicTool(JetCaloClusterThinning(**kwargs), primary=True)
    return acc

####
# additional utilities to return the list of decorations
# added by the tools


def getGainLayerNames(tool):
    """getGainLayerNames( tool ) -> return a list of names of the decorations added to the
    egamma tool, given the GainDecorator tool"""
    return [
        tool.decoration_pattern.format(info=info, layer=layer, gain=gain)
        for info in ["E", "nCells"]
        for layer in tool.layers
        for gain in tool.gain_names.values()
    ]


def getGainDecorations(
    acc,
    flags,
    kernel,
    collections=None,
    info=["E", "nCells"],
):
    """getGainDecorations( acc, kernel collections=["Electrons", "Photons"] ) ->
    Return a list with the 'ExtraContent' to be added to the decorations to save the gain
    information per layer"""

    if collections is None:
        collections = [flags.Egamma.Keys.Output.Electrons, flags.Egamma.Keys.Output.Photons]

    GainDecoratorTool = None
    for tool in acc.getEventAlgo(kernel).AugmentationTools:
        if tool.getType() == "DerivationFramework::GainDecorator":
            GainDecoratorTool = tool

    if GainDecoratorTool:
        return [
            "{part}.{info}".format(part=part, info=info)
            for part in collections
            for info in getGainLayerNames(GainDecoratorTool)
        ]
    else:
        return ""


def getClusterEnergyPerLayerDecorations(acc, kernel):
    """getClusterEnergyPerLayerDecorationsLegacy( acc, kernel ) -> return a list of names of the
    decorations added to the egamma object, given the ClusterEnergyPerLayerDecorations
    object (e.g. Photons.E7x11_Lr0, ...)"""
    properties = "SGKey_photons", "SGKey_electrons"
    ClusterEnergyPerLayerDecorators = []
    for tool in acc.getEventAlgo(kernel).AugmentationTools:
        if tool.getType() == "DerivationFramework::ClusterEnergyPerLayerDecorator":
            ClusterEnergyPerLayerDecorators.append(tool)

    decorations = []
    for tool in ClusterEnergyPerLayerDecorators:
        collections = filter(bool, (getattr(tool, x) for x in properties))
        for part in collections:
            key = "SGKey_{}_decorations".format(str(part).lower())
            decorations.extend(getattr(tool, key))
    return decorations
