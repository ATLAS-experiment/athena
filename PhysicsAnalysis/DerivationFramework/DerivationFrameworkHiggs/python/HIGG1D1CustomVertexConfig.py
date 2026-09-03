# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ZeeVertexRefittingToolCfg(
        flags, name="HIGG1D1_ZeeVertexRefitterTool", **kwargs):
    """ PV refitting after removing Z->ee tracks, for vertex studies """
    acc = ComponentAccumulator()

    import AthenaCommon.SystemOfUnits as Units

    if "PrimaryVertexRefitterTool" not in kwargs:
        from JpsiUpsilonTools.JpsiUpsilonToolsConfig import (
            PrimaryVertexRefittingToolCfg)
        kwargs.setdefault("PrimaryVertexRefitterTool", acc.popToolsAndMerge(
            PrimaryVertexRefittingToolCfg(flags)))

    kwargs.setdefault("ObjectRequirements", (
        "(Electrons.DFCommonElectronsLHMedium) && (Electrons.pt > 19.*GeV)"))
    kwargs.setdefault("LowMassCut", 50*Units.GeV)
    kwargs.setdefault("RefittedPVContainerName", "ZeeRefittedPrimaryVertices")
    kwargs.setdefault("MCSamples", [361106, 601189])

    acc.setPrivateTools(
        CompFactory.DerivationFramework.ZeeVertexRefittingTool(name, **kwargs))
    return acc

def ZeeVertexRefitterCfg(flags, name="ZeeVertexRefitKernel"):
    """ PV refitting after removing Z->ee tracks, for vertex studies """

    # Creates a vertex container (ZeeRefittedPrimaryVertices) where the type=1 vertex is refitted
    # after removing tracks that are associated with Z->ee decay candidates
    # Tool runs only for data and Zee MC samples (must be defined in the MCSamples list)

    acc = ComponentAccumulator()
    ZeeVertexRefittingTool = acc.popToolsAndMerge(
        ZeeVertexRefittingToolCfg(flags))
    acc.addPublicTool(ZeeVertexRefittingTool)
    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name, AugmentationTools=[ZeeVertexRefittingTool])) # TODO Migrate public tool to AthReentrantAlgorithm (Uses ExpressionParser)
    return acc

def DiphotonVertexDecoratorCfg(flags, **kwargs):
    acc = ComponentAccumulator()
    if "PhotonVertexSelectionTool" not in kwargs:
        from PhotonVertexSelection.PhotonVertexSelectionConfig import (
            PhotonVertexSelectionToolCfg)
        kwargs.setdefault("PhotonVertexSelectionTool", acc.popToolsAndMerge(
            PhotonVertexSelectionToolCfg(flags)))
    acc.setPrivateTools(
        CompFactory.DerivationFramework.DiphotonVertexDecorator(**kwargs))
    return acc

def DiPhotonVertexDecoratorKernelCfg(flags, name="DiphotonVertexKernel"):
    """ Diphoton vertex decoration tool """

    # Decorator creates a shallow copy of PrimaryVertices (HggPrimaryVertices) for diphoton events
    # Must be created before the jetalg in the sequence as it is input to the modified PFlow jets

    acc = ComponentAccumulator()
    DiphotonVertexDecorator = acc.popToolsAndMerge(
        DiphotonVertexDecoratorCfg(flags))
    acc.addPublicTool(DiphotonVertexDecorator)
    acc.addEventAlgo(CompFactory.DerivationFramework.CommonAugmentation(
        name,AugmentationTools=[DiphotonVertexDecorator])) # TODO Migrate public tool to AthReentrantAlgorithm
    return acc

def DiPhotonVertexCfg(flags):
    from DerivationFrameworkEGamma.EGammaToolsConfig import (
        PhotonVertexSelectionWrapperKernelCfg)
    acc = PhotonVertexSelectionWrapperKernelCfg(flags)
    acc.merge(DiPhotonVertexDecoratorKernelCfg(flags))
    return acc
