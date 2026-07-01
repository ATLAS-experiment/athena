# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.SystemOfUnits import GeV
from ROOT import CaloCell_ID


# Electrons
def CaloRingsElectronBuilderCfg(flags, name="CaloRingsElectronBuilder", **kwargs):
    kwargs.setdefault("EtaWidth", [0.025, 0.003125, 0.025, 0.05, 0.1, 0.1, 0.2])
    kwargs.setdefault(
        "PhiWidth",
        [
            0.098174770424681,
            0.098174770424681,
            0.024543692606170,
            0.024543692606170,
            0.098174770424681,
            0.098174770424681,
            0.098174770424681,
        ],
    )
    kwargs.setdefault("NRings", [8, 64, 8, 8, 4, 4, 4])
    kwargs.setdefault("CellMaxDEtaDist", 0.2)
    kwargs.setdefault("CellMaxDPhiDist", 0.2)
    kwargs.setdefault(
        "Layers",
        [
            CaloCell_ID.PreSamplerB,
            CaloCell_ID.PreSamplerE,
            CaloCell_ID.EMB1,
            CaloCell_ID.EME1,
            CaloCell_ID.EMB2,
            CaloCell_ID.EME2,
            CaloCell_ID.EMB3,
            CaloCell_ID.EME3,
            CaloCell_ID.HEC0,
            CaloCell_ID.TileBar0,
            CaloCell_ID.TileGap3,
            CaloCell_ID.TileExt0,
            CaloCell_ID.HEC1,
            CaloCell_ID.HEC2,
            CaloCell_ID.TileBar1,
            CaloCell_ID.TileGap1,
            CaloCell_ID.TileExt1,
            CaloCell_ID.HEC3,
            CaloCell_ID.TileBar2,
            CaloCell_ID.TileGap2,
            CaloCell_ID.TileExt2,
        ],
    )
    kwargs.setdefault("RingSetNLayers", [2, 2, 2, 2, 4, 5, 4])
    kwargs.setdefault(
        "useShowerShapeBarycenter", flags.CaloRinger.useShowerShapeBarycenter
    )
    kwargs.setdefault("CellsContainerName", flags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault("CaloRingsContainerName", "ElectronCaloRings")
    kwargs.setdefault("RingSetContainerName", "ElectronRingSets")
    kwargs.setdefault("MinPartEnergy", flags.CaloRinger.minElectronEnergy * GeV)
    kwargs.setdefault("doTransverseEnergy", flags.CaloRinger.doTransverseEnergy)

    tool = CompFactory.Ringer.CaloRingsBuilder(name, **kwargs)
    return tool


def CaloRingsAsymElectronBuilderCfg(
    flags, name="CaloRingsAsymElectronBuilder", **kwargs
):
    NRings = [8, 64, 8, 8, 4, 4, 4]
    kwargs.setdefault("EtaWidth", [0.025, 0.003125, 0.025, 0.05, 0.1, 0.1, 0.2])
    kwargs.setdefault(
        "PhiWidth",
        [
            0.098174770424681,
            0.098174770424681,
            0.024543692606170,
            0.024543692606170,
            0.098174770424681,
            0.098174770424681,
            0.098174770424681,
        ],
    )
    kwargs.setdefault("NRings", [(rings - 1) * 4 + 1 for rings in NRings])
    kwargs.setdefault("CellMaxDEtaDist", 0.2)
    kwargs.setdefault("CellMaxDPhiDist", 0.2)
    kwargs.setdefault(
        "Layers",
        [
            CaloCell_ID.PreSamplerB,
            CaloCell_ID.PreSamplerE,
            CaloCell_ID.EMB1,
            CaloCell_ID.EME1,
            CaloCell_ID.EMB2,
            CaloCell_ID.EME2,
            CaloCell_ID.EMB3,
            CaloCell_ID.EME3,
            CaloCell_ID.HEC0,
            CaloCell_ID.TileBar0,
            CaloCell_ID.TileGap3,
            CaloCell_ID.TileExt0,
            CaloCell_ID.HEC1,
            CaloCell_ID.HEC2,
            CaloCell_ID.TileBar1,
            CaloCell_ID.TileGap1,
            CaloCell_ID.TileExt1,
            CaloCell_ID.HEC3,
            CaloCell_ID.TileBar2,
            CaloCell_ID.TileGap2,
            CaloCell_ID.TileExt2,
        ],
    )
    kwargs.setdefault("RingSetNLayers", [2, 2, 2, 2, 4, 5, 4])
    kwargs.setdefault(
        "useShowerShapeBarycenter", flags.CaloRinger.useShowerShapeBarycenter
    )
    kwargs.setdefault("CellsContainerName", flags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault("CaloRingsContainerName", "ElectronCaloAsymRings")
    kwargs.setdefault("RingSetContainerName", "ElectronAsymRingSets")
    kwargs.setdefault("MinPartEnergy", flags.CaloRinger.minElectronEnergy * GeV)
    kwargs.setdefault("doTransverseEnergy", flags.CaloRinger.doTransverseEnergy)
    kwargs.setdefault("doEtaAxesDivision", True)
    kwargs.setdefault("doPhiAxesDivision", True)

    tool = CompFactory.Ringer.CaloAsymRingsBuilder(name, **kwargs)
    return tool


def CaloRingsStripsElectronBuilderCfg(
    flags, name="CaloRingsStripsElectronBuilder", **kwargs
):
    kwargs.setdefault("EtaWidth", [0.025, 0.003125, 0.025, 0.05, 0.1, 0.1, 0.2])
    kwargs.setdefault(
        "PhiWidth",
        [
            0.098174770424681,
            0.098174770424681,
            0.024543692606170,
            0.024543692606170,
            0.098174770424681,
            0.098174770424681,
            0.098174770424681,
        ],
    )
    kwargs.setdefault("NRings", [28, 252, 28, 14, 8, 8, 4])
    kwargs.setdefault("Axis", 0)
    kwargs.setdefault("CellMaxDEtaDist", 0.2)
    kwargs.setdefault("CellMaxDPhiDist", 0.2)
    kwargs.setdefault(
        "Layers",
        [
            CaloCell_ID.PreSamplerB,
            CaloCell_ID.PreSamplerE,
            CaloCell_ID.EMB1,
            CaloCell_ID.EME1,
            CaloCell_ID.EMB2,
            CaloCell_ID.EME2,
            CaloCell_ID.EMB3,
            CaloCell_ID.EME3,
            CaloCell_ID.HEC0,
            CaloCell_ID.TileBar0,
            CaloCell_ID.TileGap3,
            CaloCell_ID.TileExt0,
            CaloCell_ID.HEC1,
            CaloCell_ID.HEC2,
            CaloCell_ID.TileBar1,
            CaloCell_ID.TileGap1,
            CaloCell_ID.TileExt1,
            CaloCell_ID.HEC3,
            CaloCell_ID.TileBar2,
            CaloCell_ID.TileGap2,
            CaloCell_ID.TileExt2,
        ],
    )
    kwargs.setdefault("RingSetNLayers", [2, 2, 2, 2, 4, 5, 4])
    kwargs.setdefault(
        "useShowerShapeBarycenter", flags.CaloRinger.useShowerShapeBarycenter
    )
    kwargs.setdefault("CellsContainerName", flags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault("CaloRingsContainerName", "ElectronCaloStripsRings")
    kwargs.setdefault("RingSetContainerName", "ElectronStripsRingSets")
    kwargs.setdefault("MinPartEnergy", flags.CaloRinger.minElectronEnergy * GeV)
    kwargs.setdefault("doTransverseEnergy", flags.CaloRinger.doTransverseEnergy)
    kwargs.setdefault("doEtaAxesDivision", True)
    kwargs.setdefault("doPhiAxesDivision", True)

    tool = CompFactory.Ringer.CaloStripsRingsBuilder(name, **kwargs)
    return tool


def CaloRingsCornerElectronBuilderCfg(
    flags, name="CaloRingsCornerElectronBuilder", **kwargs
):
    NRings = [8, 64, 8, 8, 4, 4, 4]
    kwargs.setdefault(
        "EtaWidth",
        [
            0.025,
            0.003125,
            0.025,
            0.05,
            0.1,
            0.1,
            0.2,
        ],
    )
    kwargs.setdefault(
        "PhiWidth",
        [
            0.098174770424681,
            0.098174770424681,
            0.024543692606170,
            0.024543692606170,
            0.098174770424681,
            0.098174770424681,
            0.098174770424681,
        ],
    )
    kwargs.setdefault("NRings", [ring * 5 for ring in NRings])
    kwargs.setdefault(
        "Layers",
        [
            CaloCell_ID.PreSamplerB,
            CaloCell_ID.PreSamplerE,
            CaloCell_ID.EMB1,
            CaloCell_ID.EME1,
            CaloCell_ID.EMB2,
            CaloCell_ID.EME2,
            CaloCell_ID.EMB3,
            CaloCell_ID.EME3,
            CaloCell_ID.HEC0,
            CaloCell_ID.TileBar0,
            CaloCell_ID.TileGap3,
            CaloCell_ID.TileExt0,
            CaloCell_ID.HEC1,
            CaloCell_ID.HEC2,
            CaloCell_ID.TileBar1,
            CaloCell_ID.TileGap1,
            CaloCell_ID.TileExt1,
            CaloCell_ID.HEC3,
            CaloCell_ID.TileBar2,
            CaloCell_ID.TileGap2,
            CaloCell_ID.TileExt2,
        ],
    )
    kwargs.setdefault("RingSetNLayers", [2, 2, 2, 2, 4, 5, 4])
    kwargs.setdefault(
        "useShowerShapeBarycenter", flags.CaloRinger.useShowerShapeBarycenter
    )
    kwargs.setdefault("CellsContainerName", flags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault("CaloRingsContainerName", "ElectronCaloCornerRings")
    kwargs.setdefault("RingSetContainerName", "ElectronCornerRingSets")
    kwargs.setdefault("MinPartEnergy", flags.CaloRinger.minElectronEnergy * GeV)
    kwargs.setdefault("doTransverseEnergy", flags.CaloRinger.doTransverseEnergy)
    kwargs.setdefault("CornerShift", flags.CaloRinger.cornerShift)
    kwargs.setdefault("CellMaxDEtaDist", 0.2)
    kwargs.setdefault("CellMaxDPhiDist", 0.2)

    tool = CompFactory.Ringer.CaloCornerRingsBuilder(name, **kwargs)
    return tool


# Photons
def CaloRingsPhotonBuilderCfg(flags, name="CaloRingsPhotonBuilder", **kwargs):
    kwargs.setdefault("EtaWidth", [0.025, 0.003125, 0.025, 0.05, 0.1, 0.1, 0.2])
    kwargs.setdefault(
        "PhiWidth",
        [
            0.098174770424681,
            0.098174770424681,
            0.024543692606170,
            0.024543692606170,
            0.098174770424681,
            0.098174770424681,
            0.098174770424681,
        ],
    )
    kwargs.setdefault("NRings", [8, 64, 8, 8, 4, 4, 4])
    kwargs.setdefault("CellMaxDEtaDist", 0.2)
    kwargs.setdefault("CellMaxDPhiDist", 0.2)
    kwargs.setdefault(
        "Layers",
        [
            CaloCell_ID.PreSamplerB,
            CaloCell_ID.PreSamplerE,
            CaloCell_ID.EMB1,
            CaloCell_ID.EME1,
            CaloCell_ID.EMB2,
            CaloCell_ID.EME2,
            CaloCell_ID.EMB3,
            CaloCell_ID.EME3,
            CaloCell_ID.HEC0,
            CaloCell_ID.TileBar0,
            CaloCell_ID.TileGap3,
            CaloCell_ID.TileExt0,
            CaloCell_ID.HEC1,
            CaloCell_ID.HEC2,
            CaloCell_ID.TileBar1,
            CaloCell_ID.TileGap1,
            CaloCell_ID.TileExt1,
            CaloCell_ID.HEC3,
            CaloCell_ID.TileBar2,
            CaloCell_ID.TileGap2,
            CaloCell_ID.TileExt2,
        ],
    )
    kwargs.setdefault("RingSetNLayers", [2, 2, 2, 2, 4, 5, 4])
    kwargs.setdefault(
        "useShowerShapeBarycenter", flags.CaloRinger.useShowerShapeBarycenter
    )
    kwargs.setdefault("CellsContainerName", flags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault("CaloRingsContainerName", "PhotonCaloRings")
    kwargs.setdefault("RingSetContainerName", "PhotonRingSets")
    kwargs.setdefault("MinPartEnergy", flags.CaloRinger.minPhotonEnergy * GeV)
    kwargs.setdefault("doTransverseEnergy", flags.CaloRinger.doTransverseEnergy)

    tool = CompFactory.Ringer.CaloRingsBuilder(name, **kwargs)
    return tool


def CaloRingsAsymPhotonBuilderCfg(flags, name="CaloRingsAsymPhotonBuilder", **kwargs):
    NRings = [8, 64, 8, 8, 4, 4, 4]
    kwargs.setdefault("EtaWidth", [0.025, 0.003125, 0.025, 0.05, 0.1, 0.1, 0.2])
    kwargs.setdefault(
        "PhiWidth",
        [
            0.098174770424681,
            0.098174770424681,
            0.024543692606170,
            0.024543692606170,
            0.098174770424681,
            0.098174770424681,
            0.098174770424681,
        ],
    )
    kwargs.setdefault("NRings", [(rings - 1) * 4 + 1 for rings in NRings])
    kwargs.setdefault("CellMaxDEtaDist", 0.2)
    kwargs.setdefault("CellMaxDPhiDist", 0.2)
    kwargs.setdefault(
        "Layers",
        [
            CaloCell_ID.PreSamplerB,
            CaloCell_ID.PreSamplerE,
            CaloCell_ID.EMB1,
            CaloCell_ID.EME1,
            CaloCell_ID.EMB2,
            CaloCell_ID.EME2,
            CaloCell_ID.EMB3,
            CaloCell_ID.EME3,
            CaloCell_ID.HEC0,
            CaloCell_ID.TileBar0,
            CaloCell_ID.TileGap3,
            CaloCell_ID.TileExt0,
            CaloCell_ID.HEC1,
            CaloCell_ID.HEC2,
            CaloCell_ID.TileBar1,
            CaloCell_ID.TileGap1,
            CaloCell_ID.TileExt1,
            CaloCell_ID.HEC3,
            CaloCell_ID.TileBar2,
            CaloCell_ID.TileGap2,
            CaloCell_ID.TileExt2,
        ],
    )
    kwargs.setdefault("RingSetNLayers", [2, 2, 2, 2, 4, 5, 4])
    kwargs.setdefault(
        "useShowerShapeBarycenter", flags.CaloRinger.useShowerShapeBarycenter
    )
    kwargs.setdefault("CellsContainerName", flags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault("CaloRingsContainerName", "PhotonCaloAsymRings")
    kwargs.setdefault("RingSetContainerName", "PhotonAsymRingSets")
    kwargs.setdefault("MinPartEnergy", flags.CaloRinger.minPhotonEnergy * GeV)
    kwargs.setdefault("doTransverseEnergy", flags.CaloRinger.doTransverseEnergy)
    kwargs.setdefault("doEtaAxesDivision", True)
    kwargs.setdefault("doPhiAxesDivision", True)

    tool = CompFactory.Ringer.CaloAsymRingsBuilder(name, **kwargs)
    return tool


def CaloRingsStripsPhotonBuilderCfg(
    flags, name="CaloRingsStripsPhotonBuilder", **kwargs
):
    kwargs.setdefault("EtaWidth", [0.025, 0.003125, 0.025, 0.05, 0.1, 0.1, 0.2])
    kwargs.setdefault(
        "PhiWidth",
        [
            0.098174770424681,
            0.098174770424681,
            0.024543692606170,
            0.024543692606170,
            0.098174770424681,
            0.098174770424681,
            0.098174770424681,
        ],
    )
    kwargs.setdefault("NRings", [28, 252, 28, 14, 8, 8, 4])
    kwargs.setdefault("Axis", 0)
    kwargs.setdefault("CellMaxDEtaDist", 0.2)
    kwargs.setdefault("CellMaxDPhiDist", 0.2)
    kwargs.setdefault(
        "Layers",
        [
            CaloCell_ID.PreSamplerB,
            CaloCell_ID.PreSamplerE,
            CaloCell_ID.EMB1,
            CaloCell_ID.EME1,
            CaloCell_ID.EMB2,
            CaloCell_ID.EME2,
            CaloCell_ID.EMB3,
            CaloCell_ID.EME3,
            CaloCell_ID.HEC0,
            CaloCell_ID.TileBar0,
            CaloCell_ID.TileGap3,
            CaloCell_ID.TileExt0,
            CaloCell_ID.HEC1,
            CaloCell_ID.HEC2,
            CaloCell_ID.TileBar1,
            CaloCell_ID.TileGap1,
            CaloCell_ID.TileExt1,
            CaloCell_ID.HEC3,
            CaloCell_ID.TileBar2,
            CaloCell_ID.TileGap2,
            CaloCell_ID.TileExt2,
        ],
    )
    kwargs.setdefault("RingSetNLayers", [2, 2, 2, 2, 4, 5, 4])
    kwargs.setdefault(
        "useShowerShapeBarycenter", flags.CaloRinger.useShowerShapeBarycenter
    )
    kwargs.setdefault("CellsContainerName", flags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault("CaloRingsContainerName", "PhotonCaloStripsRings")
    kwargs.setdefault("RingSetContainerName", "PhotonStripsRingSets")
    kwargs.setdefault("MinPartEnergy", flags.CaloRinger.minPhotonEnergy * GeV)
    kwargs.setdefault("doTransverseEnergy", flags.CaloRinger.doTransverseEnergy)
    kwargs.setdefault("doEtaAxesDivision", True)
    kwargs.setdefault("doPhiAxesDivision", True)

    tool = CompFactory.Ringer.CaloStripsRingsBuilder(name, **kwargs)
    return tool


def CaloRingsCornerPhotonBuilderCfg(
    flags, name="CaloRingsCornerPhotonBuilder", **kwargs
):
    NRings = [8, 64, 8, 8, 4, 4, 4]
    kwargs.setdefault(
        "EtaWidth",
        [
            0.025,
            0.003125,
            0.025,
            0.05,
            0.1,
            0.1,
            0.2,
        ],
    )
    kwargs.setdefault(
        "PhiWidth",
        [
            0.098174770424681,
            0.098174770424681,
            0.024543692606170,
            0.024543692606170,
            0.098174770424681,
            0.098174770424681,
            0.098174770424681,
        ],
    )
    kwargs.setdefault("NRings", [ring * 5 for ring in NRings])
    kwargs.setdefault(
        "Layers",
        [
            CaloCell_ID.PreSamplerB,
            CaloCell_ID.PreSamplerE,
            CaloCell_ID.EMB1,
            CaloCell_ID.EME1,
            CaloCell_ID.EMB2,
            CaloCell_ID.EME2,
            CaloCell_ID.EMB3,
            CaloCell_ID.EME3,
            CaloCell_ID.HEC0,
            CaloCell_ID.TileBar0,
            CaloCell_ID.TileGap3,
            CaloCell_ID.TileExt0,
            CaloCell_ID.HEC1,
            CaloCell_ID.HEC2,
            CaloCell_ID.TileBar1,
            CaloCell_ID.TileGap1,
            CaloCell_ID.TileExt1,
            CaloCell_ID.HEC3,
            CaloCell_ID.TileBar2,
            CaloCell_ID.TileGap2,
            CaloCell_ID.TileExt2,
        ],
    )
    kwargs.setdefault("RingSetNLayers", [2, 2, 2, 2, 4, 5, 4])
    kwargs.setdefault(
        "useShowerShapeBarycenter", flags.CaloRinger.useShowerShapeBarycenter
    )
    kwargs.setdefault("CellsContainerName", flags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault("CaloRingsContainerName", "PhotonCaloCornerRings")
    kwargs.setdefault("RingSetContainerName", "PhotonCornerRingSets")
    kwargs.setdefault("MinPartEnergy", flags.CaloRinger.minPhotonEnergy * GeV)
    kwargs.setdefault("doTransverseEnergy", flags.CaloRinger.doTransverseEnergy)
    kwargs.setdefault("CornerShift", flags.CaloRinger.cornerShift)
    kwargs.setdefault("CellMaxDEtaDist", 0.2)
    kwargs.setdefault("CellMaxDPhiDist", 0.2)

    tool = CompFactory.Ringer.CaloCornerRingsBuilder(name, **kwargs)
    return tool


# Jets
def CaloRingsJetBuilderCfg(flags, name="CaloRingsJetBuilder", **kwargs):
    kwargs.setdefault("EtaWidth", [0.025, 0.003125, 0.025, 0.05, 0.1, 0.1, 0.2])
    kwargs.setdefault(
        "PhiWidth",
        [
            0.098174770424681,
            0.098174770424681,
            0.024543692606170,
            0.024543692606170,
            0.098174770424681,
            0.098174770424681,
            0.098174770424681,
        ],
    )
    kwargs.setdefault("NRings", [8, 64, 8, 8, 4, 4, 4])
    kwargs.setdefault("CellMaxDEtaDist", 0.2)
    kwargs.setdefault("CellMaxDPhiDist", 0.2)
    kwargs.setdefault(
        "Layers",
        [
            CaloCell_ID.PreSamplerB,
            CaloCell_ID.PreSamplerE,
            CaloCell_ID.EMB1,
            CaloCell_ID.EME1,
            CaloCell_ID.EMB2,
            CaloCell_ID.EME2,
            CaloCell_ID.EMB3,
            CaloCell_ID.EME3,
            CaloCell_ID.HEC0,
            CaloCell_ID.TileBar0,
            CaloCell_ID.TileGap3,
            CaloCell_ID.TileExt0,
            CaloCell_ID.HEC1,
            CaloCell_ID.HEC2,
            CaloCell_ID.TileBar1,
            CaloCell_ID.TileGap1,
            CaloCell_ID.TileExt1,
            CaloCell_ID.HEC3,
            CaloCell_ID.TileBar2,
            CaloCell_ID.TileGap2,
            CaloCell_ID.TileExt2,
        ],
    )
    kwargs.setdefault("RingSetNLayers", [2, 2, 2, 2, 4, 5, 4])
    kwargs.setdefault(
        "useShowerShapeBarycenter", flags.CaloRinger.useShowerShapeBarycenter
    )
    kwargs.setdefault("CellsContainerName", flags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault("CaloRingsContainerName", "JetCaloRings")
    kwargs.setdefault("RingSetContainerName", "JetRingSets")
    kwargs.setdefault("MinPartEnergy", flags.CaloRinger.minJetEnergy * GeV)
    kwargs.setdefault("doTransverseEnergy", flags.CaloRinger.doTransverseEnergy)

    tool = CompFactory.Ringer.CaloRingsBuilder(name, **kwargs)
    return tool


def CaloRingsAsymJetBuilderCfg(flags, name="CaloRingsAsymJetBuilder", **kwargs):
    NRings = [8, 64, 8, 8, 4, 4, 4]
    kwargs.setdefault("EtaWidth", [0.025, 0.003125, 0.025, 0.05, 0.1, 0.1, 0.2])
    kwargs.setdefault(
        "PhiWidth",
        [
            0.098174770424681,
            0.098174770424681,
            0.024543692606170,
            0.024543692606170,
            0.098174770424681,
            0.098174770424681,
            0.098174770424681,
        ],
    )
    kwargs.setdefault("NRings", [(rings - 1) * 4 + 1 for rings in NRings])
    kwargs.setdefault("CellMaxDEtaDist", 0.2)
    kwargs.setdefault("CellMaxDPhiDist", 0.2)
    kwargs.setdefault(
        "Layers",
        [
            CaloCell_ID.PreSamplerB,
            CaloCell_ID.PreSamplerE,
            CaloCell_ID.EMB1,
            CaloCell_ID.EME1,
            CaloCell_ID.EMB2,
            CaloCell_ID.EME2,
            CaloCell_ID.EMB3,
            CaloCell_ID.EME3,
            CaloCell_ID.HEC0,
            CaloCell_ID.TileBar0,
            CaloCell_ID.TileGap3,
            CaloCell_ID.TileExt0,
            CaloCell_ID.HEC1,
            CaloCell_ID.HEC2,
            CaloCell_ID.TileBar1,
            CaloCell_ID.TileGap1,
            CaloCell_ID.TileExt1,
            CaloCell_ID.HEC3,
            CaloCell_ID.TileBar2,
            CaloCell_ID.TileGap2,
            CaloCell_ID.TileExt2,
        ],
    )
    kwargs.setdefault("RingSetNLayers", [2, 2, 2, 2, 4, 5, 4])
    kwargs.setdefault(
        "useShowerShapeBarycenter", flags.CaloRinger.useShowerShapeBarycenter
    )
    kwargs.setdefault("CellsContainerName", flags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault("CaloRingsContainerName", "JetCaloAsymRings")
    kwargs.setdefault("RingSetContainerName", "JetAsymRingSets")
    kwargs.setdefault("MinPartEnergy", flags.CaloRinger.minJetEnergy * GeV)
    kwargs.setdefault("doTransverseEnergy", flags.CaloRinger.doTransverseEnergy)
    kwargs.setdefault("doEtaAxesDivision", True)
    kwargs.setdefault("doPhiAxesDivision", True)

    tool = CompFactory.Ringer.CaloAsymRingsBuilder(name, **kwargs)
    return tool


def CaloRingsStripsJetBuilderCfg(flags, name="CaloRingsStripsJetBuilder", **kwargs):
    kwargs.setdefault(
        "EtaWidth",
        [
            0.025,
            0.003125,
            0.025,
            0.05,
            0.1,
            0.1,
            0.2,
        ],
    )
    kwargs.setdefault(
        "PhiWidth",
        [
            0.098174770424681,
            0.098174770424681,
            0.024543692606170,
            0.024543692606170,
            0.098174770424681,
            0.098174770424681,
            0.098174770424681,
        ],
    )
    kwargs.setdefault("NRings", [28, 252, 28, 14, 8, 8, 4])
    kwargs.setdefault("Axis", 0)
    kwargs.setdefault("CellMaxDEtaDist", 0.2)
    kwargs.setdefault("CellMaxDPhiDist", 0.2)
    kwargs.setdefault(
        "Layers",
        [
            CaloCell_ID.PreSamplerB,
            CaloCell_ID.PreSamplerE,
            CaloCell_ID.EMB1,
            CaloCell_ID.EME1,
            CaloCell_ID.EMB2,
            CaloCell_ID.EME2,
            CaloCell_ID.EMB3,
            CaloCell_ID.EME3,
            CaloCell_ID.HEC0,
            CaloCell_ID.TileBar0,
            CaloCell_ID.TileGap3,
            CaloCell_ID.TileExt0,
            CaloCell_ID.HEC1,
            CaloCell_ID.HEC2,
            CaloCell_ID.TileBar1,
            CaloCell_ID.TileGap1,
            CaloCell_ID.TileExt1,
            CaloCell_ID.HEC3,
            CaloCell_ID.TileBar2,
            CaloCell_ID.TileGap2,
            CaloCell_ID.TileExt2,
        ],
    )
    kwargs.setdefault("RingSetNLayers", [2, 2, 2, 2, 4, 5, 4])
    kwargs.setdefault(
        "useShowerShapeBarycenter", flags.CaloRinger.useShowerShapeBarycenter
    )
    kwargs.setdefault("CellsContainerName", flags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault("CaloRingsContainerName", "JetCaloStripsRings")
    kwargs.setdefault("RingSetContainerName", "JetStripsRingSets")
    kwargs.setdefault("MinPartEnergy", flags.CaloRinger.minJetEnergy * GeV)
    kwargs.setdefault("doTransverseEnergy", flags.CaloRinger.doTransverseEnergy)
    kwargs.setdefault("doEtaAxesDivision", True)
    kwargs.setdefault("doPhiAxesDivision", True)

    tool = CompFactory.Ringer.CaloStripsRingsBuilder(name, **kwargs)
    return tool


def CaloRingsCornerJetBuilderCfg(flags, name="CaloRingsCornerJetBuilder", **kwargs):
    NRings = [8, 64, 8, 8, 4, 4, 4]
    kwargs.setdefault(
        "EtaWidth",
        [
            0.025,
            0.003125,
            0.025,
            0.05,
            0.1,
            0.1,
            0.2,
        ],
    )
    kwargs.setdefault(
        "PhiWidth",
        [
            0.098174770424681,
            0.098174770424681,
            0.024543692606170,
            0.024543692606170,
            0.098174770424681,
            0.098174770424681,
            0.098174770424681,
        ],
    )
    kwargs.setdefault("NRings", [ring * 5 for ring in NRings])
    kwargs.setdefault(
        "Layers",
        [
            CaloCell_ID.PreSamplerB,
            CaloCell_ID.PreSamplerE,
            CaloCell_ID.EMB1,
            CaloCell_ID.EME1,
            CaloCell_ID.EMB2,
            CaloCell_ID.EME2,
            CaloCell_ID.EMB3,
            CaloCell_ID.EME3,
            CaloCell_ID.HEC0,
            CaloCell_ID.TileBar0,
            CaloCell_ID.TileGap3,
            CaloCell_ID.TileExt0,
            CaloCell_ID.HEC1,
            CaloCell_ID.HEC2,
            CaloCell_ID.TileBar1,
            CaloCell_ID.TileGap1,
            CaloCell_ID.TileExt1,
            CaloCell_ID.HEC3,
            CaloCell_ID.TileBar2,
            CaloCell_ID.TileGap2,
            CaloCell_ID.TileExt2,
        ],
    )
    kwargs.setdefault("RingSetNLayers", [2, 2, 2, 2, 4, 5, 4])
    kwargs.setdefault(
        "useShowerShapeBarycenter", flags.CaloRinger.useShowerShapeBarycenter
    )
    kwargs.setdefault("CellsContainerName", flags.Egamma.Keys.Input.CaloCells)
    kwargs.setdefault("CaloRingsContainerName", "JetCaloCornerRings")
    kwargs.setdefault("RingSetContainerName", "JetCornerRingSets")
    kwargs.setdefault("MinPartEnergy", flags.CaloRinger.minJetEnergy * GeV)
    kwargs.setdefault("doTransverseEnergy", flags.CaloRinger.doTransverseEnergy)
    kwargs.setdefault("CornerShift", flags.CaloRinger.cornerShift)
    kwargs.setdefault("CellMaxDEtaDist", 0.2)
    kwargs.setdefault("CellMaxDPhiDist", 0.2)

    tool = CompFactory.Ringer.CaloCornerRingsBuilder(name, **kwargs)
    return tool


# Readers
def CaloRingerElectronsInputReaderCfg(
    flags, name="CaloRingerElectronsReader", **kwargs
):
    if "Asym" in name:
        builderTool = CaloRingsAsymElectronBuilderCfg(flags)
    elif "Strips" in name:
        builderTool = CaloRingsStripsElectronBuilderCfg(flags)
    elif "Corner" in name:
        builderTool = CaloRingsCornerElectronBuilderCfg(flags)
    else:
        builderTool = CaloRingsElectronBuilderCfg(flags)

    kwargs.setdefault("crBuilder", builderTool)
    kwargs.setdefault("inputKey", flags.Egamma.Keys.Output.Electrons)
    kwargs.setdefault("builderAvailable", True)
    inputReaderTool = CompFactory.Ringer.CaloRingerElectronsReader(name, **kwargs)

    return inputReaderTool, builderTool


def CaloRingerPhotonsInputReaderCfg(flags, name="CaloRingerPhotonsReader", **kwargs):
    if "Asym" in name:
        builderTool = CaloRingsAsymPhotonBuilderCfg(flags)
    elif "Strips" in name:
        builderTool = CaloRingsStripsPhotonBuilderCfg(flags)
    elif "Corner" in name:
        builderTool = CaloRingsCornerPhotonBuilderCfg(flags)
    else:
        builderTool = CaloRingsPhotonBuilderCfg(flags)

    kwargs.setdefault("crBuilder", builderTool)
    kwargs.setdefault("inputKey", flags.Egamma.Keys.Output.Photons)
    kwargs.setdefault("builderAvailable", True)
    inputReaderTool = CompFactory.Ringer.CaloRingerPhotonsReader(name, **kwargs)

    return inputReaderTool, builderTool


def CaloRingerJetsInputReaderCfg(flags, name="CaloRingerJetsReader", **kwargs):
    if "Asym" in name:
        builderTool = CaloRingsAsymJetBuilderCfg(flags)
    elif "Strips" in name:
        builderTool = CaloRingsStripsJetBuilderCfg(flags)
    elif "Corner" in name:
        builderTool = CaloRingsCornerJetBuilderCfg(flags)
    else:
        builderTool = CaloRingsJetBuilderCfg(flags)

    kwargs.setdefault("crBuilder", builderTool)
    kwargs.setdefault(
        "inputKey", "AntiKt4EMPFlowJets"
    )  # need to get the jet container name
    kwargs.setdefault("builderAvailable", True)
    inputReaderTool = CompFactory.Ringer.CaloRingerJetsReader(name, **kwargs)

    return inputReaderTool, builderTool


# Algs Electrons
def CaloRingerElectronAlgsCfg(flags, name="CaloRingerElectronAlgorithm", **kwargs):
    electronInputReaderTool, electronBuilderTool = CaloRingerElectronsInputReaderCfg(
        flags
    )
    acc = ComponentAccumulator()
    acc.addPublicTool(electronBuilderTool)
    acc.addPublicTool(electronInputReaderTool)
    kwargs.setdefault("inputReaderTools", [electronInputReaderTool])

    CaloRingerAlgorithm = CompFactory.Ringer.CaloRingerAlgorithm(
        name="CaloRingerElectronAlgorithm", **kwargs
    )
    acc.addEventAlgo(CaloRingerAlgorithm)
    return acc


def CaloRingerAsymElectronAlgsCfg(
    flags, name="CaloRingerAsymElectronAlgorithm", **kwargs
):
    electronAsymInputReaderTool, electronAsymBuilderTool = (
        CaloRingerElectronsInputReaderCfg(flags, name="CaloRingerAsymElectronAlgorithm")
    )
    acc = ComponentAccumulator()
    acc.addPublicTool(electronAsymBuilderTool)
    acc.addPublicTool(electronAsymInputReaderTool)
    kwargs.setdefault("inputReaderTools", [electronAsymInputReaderTool])

    CaloRingerAsymAlgorithm = CompFactory.Ringer.CaloRingerAlgorithm(
        name="CaloRingerAsymAlgorithm", **kwargs
    )
    acc.addEventAlgo(CaloRingerAsymAlgorithm)
    return acc


def CaloRingerStripsElectronAlgsCfg(
    flags, name="CaloRingerStripsElectronAlgorithm", **kwargs
):
    electronStripsInputReaderTool, electronStripsBuilderTool = (
        CaloRingerElectronsInputReaderCfg(flags, name="CaloRingerStripsElectronReader")
    )
    acc = ComponentAccumulator()
    acc.addPublicTool(electronStripsBuilderTool)
    acc.addPublicTool(electronStripsInputReaderTool)
    kwargs.setdefault("inputReaderTools", [electronStripsInputReaderTool])

    CaloRingerStripsAlgorithm = CompFactory.Ringer.CaloRingerAlgorithm(
        name="CaloRingerStripsElectronAlgorithm", **kwargs
    )
    acc.addEventAlgo(CaloRingerStripsAlgorithm)
    return acc


def CaloRingerCornerElectronAlgsCfg(flags, name="CaloRingerCornerAlgorithm", **kwargs):
    electronCornerInputReaderTool, electronCornerBuilderTool = (
        CaloRingerElectronsInputReaderCfg(flags, name="CaloCornerRingerElectronReader")
    )
    acc = ComponentAccumulator()
    acc.addPublicTool(electronCornerBuilderTool)
    acc.addPublicTool(electronCornerInputReaderTool)
    kwargs.setdefault("inputReaderTools", [electronCornerInputReaderTool])

    CaloRingerCornerAlgorithm = CompFactory.Ringer.CaloRingerAlgorithm(
        name="CaloRingerCornerAlgorithm", **kwargs
    )
    acc.addEventAlgo(CaloRingerCornerAlgorithm)
    return acc


# Algs Photons
def CaloRingerPhotonAlgsCfg(flags, name="CaloRingerPhotonAlgorithm", **kwargs):
    photonInputReaderTool, photonBuilderTool = CaloRingerPhotonsInputReaderCfg(flags)
    acc = ComponentAccumulator()
    acc.addPublicTool(photonBuilderTool)
    acc.addPublicTool(photonInputReaderTool)
    kwargs.setdefault("inputReaderTools", [photonInputReaderTool])

    CaloRingerAlgorithm = CompFactory.Ringer.CaloRingerAlgorithm(
        name="CaloRingerPhotonAlgorithm", **kwargs
    )
    acc.addEventAlgo(CaloRingerAlgorithm)
    return acc


def CaloRingerAsymPhotonAlgsCfg(flags, name="CaloAsymRingerAlgorithm", **kwargs):
    photonAsymInputReaderTool, photonAsymBuilderTool = CaloRingerPhotonsInputReaderCfg(
        flags, name="CaloAsymRingerPhotonReader"
    )
    acc = ComponentAccumulator()
    acc.addPublicTool(photonAsymBuilderTool)
    acc.addPublicTool(photonAsymInputReaderTool)
    kwargs.setdefault("inputReaderTools", [photonAsymInputReaderTool])

    CaloAsymRingerAlgorithm = CompFactory.Ringer.CaloRingerAlgorithm(
        name="CaloAsymRingerAlgorithm", **kwargs
    )
    acc.addEventAlgo(CaloAsymRingerAlgorithm)
    return acc


def CaloRingerStripsPhotonAlgsCfg(flags, name="CaloRingerStripsAlgorithm", **kwargs):
    photonStripsInputReaderTool, photonStripsBuilderTool = (
        CaloRingerPhotonsInputReaderCfg(flags, name="CaloRingerStripsPhotonReader")
    )
    acc = ComponentAccumulator()
    acc.addPublicTool(photonStripsBuilderTool)
    acc.addPublicTool(photonStripsInputReaderTool)
    kwargs.setdefault("inputReaderTools", [photonStripsInputReaderTool])

    CaloRingerStripsAlgorithm = CompFactory.Ringer.CaloRingerAlgorithm(
        name="CaloRingerStripsAlgorithm", **kwargs
    )
    acc.addEventAlgo(CaloRingerStripsAlgorithm)
    return acc


def CaloRingerCornerPhotonAlgsCfg(flags, name="CaloRingerCornerAlgorithm", **kwargs):
    photonCornerInputReaderTool, photonCornerBuilderTool = (
        CaloRingerPhotonsInputReaderCfg(flags, name="CaloCornerRingerPhotonReader")
    )
    acc = ComponentAccumulator()
    acc.addPublicTool(photonCornerBuilderTool)
    acc.addPublicTool(photonCornerInputReaderTool)
    kwargs.setdefault("inputReaderTools", [photonCornerInputReaderTool])

    CaloRingerCornerAlgorithm = CompFactory.Ringer.CaloRingerAlgorithm(
        name="CaloRingerCornerAlgorithm", **kwargs
    )
    acc.addEventAlgo(CaloRingerCornerAlgorithm)
    return acc


# Algs Jets
def CaloRingerJetAlgsCfg(flags, name="CaloRingerJetAlgorithm", **kwargs):
    jetInputReaderTool, jetBuilderTool = CaloRingerJetsInputReaderCfg(flags)
    acc = ComponentAccumulator()
    acc.addPublicTool(jetBuilderTool)
    acc.addPublicTool(jetInputReaderTool)
    kwargs.setdefault("inputReaderTools", [jetInputReaderTool])

    CaloRingerAlgorithm = CompFactory.Ringer.CaloRingerAlgorithm(
        name="CaloRingerJetAlgorithm", **kwargs
    )
    acc.addEventAlgo(CaloRingerAlgorithm)
    return acc


def CaloRingerAsymJetAlgsCfg(flags, name="CaloAsymRingerAlgorithm", **kwargs):
    jetAsymInputReaderTool, jetAsymBuilderTool = CaloRingerJetsInputReaderCfg(
        flags, name="CaloAsymRingerJetReader"
    )
    acc = ComponentAccumulator()
    acc.addPublicTool(jetAsymBuilderTool)
    acc.addPublicTool(jetAsymInputReaderTool)
    kwargs.setdefault("inputReaderTools", [jetAsymInputReaderTool])

    CaloAsymRingerAlgorithm = CompFactory.Ringer.CaloRingerAlgorithm(
        name="CaloAsymRingerAlgorithm", **kwargs
    )
    acc.addEventAlgo(CaloAsymRingerAlgorithm)
    return acc


def CaloRingerStripsJetAlgsCfg(
    flags,
    name="CaloRingerStripsAlgorithm",
    **kwargs,
):
    jetStripsInputReaderTool, jetStripsBuilderTool = CaloRingerJetsInputReaderCfg(
        flags, name="CaloStripsRingerJetReader"
    )
    acc = ComponentAccumulator()
    acc.addPublicTool(jetStripsBuilderTool)
    acc.addPublicTool(jetStripsInputReaderTool)
    kwargs.setdefault("inputReaderTools", [jetStripsInputReaderTool])

    CaloRingerStripsAlgorithm = CompFactory.Ringer.CaloRingerAlgorithm(
        name="CaloRingerStripsAlgorithm", **kwargs
    )
    acc.addEventAlgo(CaloRingerStripsAlgorithm)
    return acc


def CaloRingerCornerJetAlgsCfg(flags, name="CaloRingerCornerAlgorithm", **kwargs):
    jetCornerInputReaderTool, jetCornerBuilderTool = CaloRingerJetsInputReaderCfg(
        flags, name="CaloCornerRingerJetReader"
    )
    acc = ComponentAccumulator()
    acc.addPublicTool(jetCornerBuilderTool)
    acc.addPublicTool(jetCornerInputReaderTool)
    kwargs.setdefault("inputReaderTools", [jetCornerInputReaderTool])

    CaloRingerCornerAlgorithm = CompFactory.Ringer.CaloRingerAlgorithm(
        name="CaloRingerCornerAlgorithm", **kwargs
    )
    acc.addEventAlgo(CaloRingerCornerAlgorithm)
    return acc


# Config Ringer
def CaloRingerOutputCfg(flags, name="CaloRingerOutputList"):
    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD, addToESD

    acc = ComponentAccumulator()

    toOutput = []
    if flags.CaloRinger.buildElectronRings:
        toOutput += [
            "xAOD::RingSetContainer#ElectronRingSets",
            "xAOD::RingSetAuxContainer#ElectronRingSetsAux.",
            "xAOD::CaloRingsContainer#ElectronCaloRings",
            "xAOD::CaloRingsAuxContainer#ElectronCaloRingsAux.",
        ]
    if flags.CaloRinger.buildElectronAsymRings:
        toOutput += [
            "xAOD::RingSetContainer#ElectronAsymRingSets",
            "xAOD::RingSetAuxContainer#ElectronAsymRingSetsAux.",
            "xAOD::CaloRingsContainer#ElectronCaloAsymRings",
            "xAOD::CaloRingsAuxContainer#ElectronCaloAsymRingsAux.",
        ]
    if flags.CaloRinger.buildElectronStripsRings:
        toOutput += [
            "xAOD::RingSetContainer#ElectronStripsRingSets",
            "xAOD::RingSetAuxContainer#ElectronStripsRingSetsAux.",
            "xAOD::CaloRingsContainer#ElectronCaloStripsRings",
            "xAOD::CaloRingsAuxContainer#ElectronCaloStripsRingsAux.",
        ]
    if flags.CaloRinger.buildElectronCornerRings:
        toOutput += [
            "xAOD::RingSetContainer#ElectronCornerRingSets",
            "xAOD::RingSetAuxContainer#ElectronCornerRingSetsAux.",
            "xAOD::CaloRingsContainer#ElectronCaloCornerRings",
            "xAOD::CaloRingsAuxContainer#ElectronCaloCornerRingsAux.",
        ]
    if flags.CaloRinger.buildPhotonRings:
        toOutput += [
            "xAOD::RingSetContainer#PhotonRingSets",
            "xAOD::RingSetAuxContainer#PhotonRingSetsAux.",
            "xAOD::CaloRingsContainer#PhotonCaloRings",
            "xAOD::CaloRingsAuxContainer#PhotonCaloRingsAux.",
        ]
    if flags.CaloRinger.buildPhotonAsymRings:
        toOutput += [
            "xAOD::RingSetContainer#PhotonAsymRingSets",
            "xAOD::RingSetAuxContainer#PhotonAsymRingSetsAux.",
            "xAOD::CaloRingsContainer#PhotonCaloAsymRings",
            "xAOD::CaloRingsAuxContainer#PhotonCaloAsymRingsAux.",
        ]
    if flags.CaloRinger.buildPhotonStripsRings:
        toOutput += [
            "xAOD::RingSetContainer#PhotonStripsRingSets",
            "xAOD::RingSetAuxContainer#PhotonStripsRingSetsAux.",
            "xAOD::CaloRingsContainer#PhotonCaloStripsRings",
            "xAOD::CaloRingsAuxContainer#PhotonCaloStripsRingsAux.",
        ]
    if flags.CaloRinger.buildPhotonCornerRings:
        toOutput += [
            "xAOD::RingSetContainer#PhotonCornerRingSets",
            "xAOD::RingSetAuxContainer#PhotonCornerRingSetsAux.",
            "xAOD::CaloRingsContainer#PhotonCaloCornerRings",
            "xAOD::CaloRingsAuxContainer#PhotonCaloCornerRingsAux.",
        ]
    if flags.CaloRinger.buildJetRings:
        toOutput += [
            "xAOD::RingSetContainer#JetRingSets",
            "xAOD::RingSetAuxContainer#JetRingSetsAux.",
            "xAOD::CaloRingsContainer#JetCaloRings",
            "xAOD::CaloRingsAuxContainer#JetCaloRingsAux.",
        ]
    if flags.CaloRinger.buildJetAsymRings:
        toOutput += [
            "xAOD::RingSetContainer#JetAsymRingSets",
            "xAOD::RingSetAuxContainer#JetAsymRingSetsAux.",
            "xAOD::CaloRingsContainer#JetCaloAsymRings",
            "xAOD::CaloRingsAuxContainer#JetCaloAsymRingsAux.",
        ]
    if flags.CaloRinger.buildJetStripsRings:
        toOutput += [
            "xAOD::RingSetContainer#JetStripsRingSets",
            "xAOD::RingSetAuxContainer#JetStripsRingSetsAux.",
            "xAOD::CaloRingsContainer#JetCaloStripsRings",
            "xAOD::CaloRingsAuxContainer#JetCaloStripsRingsAux.",
        ]
    if flags.CaloRinger.buildJetCornerRings:
        toOutput += [
            "xAOD::RingSetContainer#JetCornerRingSets",
            "xAOD::RingSetAuxContainer#JetCornerRingSetsAux.",
            "xAOD::CaloRingsContainer#JetCaloCornerRings",
            "xAOD::CaloRingsAuxContainer#JetCaloCornerRingsAux.",
        ]

    if flags.Output.doWriteAOD:
        acc.merge(addToAOD(flags, toOutput))
    if flags.Output.doWriteESD:
        acc.merge(addToESD(flags, toOutput))

    return acc


def CaloRingerSteeringCfg(flags, name="CaloRingerSteering"):
    acc = ComponentAccumulator()

    if flags.CaloRinger.buildElectronRings:
        acc.merge(CaloRingerElectronAlgsCfg(flags))
    if flags.CaloRinger.buildElectronAsymRings:
        acc.merge(CaloRingerAsymElectronAlgsCfg(flags))
    if flags.CaloRinger.buildElectronStripsRings:
        acc.merge(CaloRingerStripsElectronAlgsCfg(flags))
    if flags.CaloRinger.buildElectronCornerRings:
        acc.merge(CaloRingerCornerElectronAlgsCfg(flags))

    if flags.CaloRinger.buildPhotonRings:
        acc.merge(CaloRingerPhotonAlgsCfg(flags))
    if flags.CaloRinger.buildPhotonAsymRings:
        acc.merge(CaloRingerAsymPhotonAlgsCfg(flags))
    if flags.CaloRinger.buildPhotonStripsRings:
        acc.merge(CaloRingerStripsPhotonAlgsCfg(flags))
    if flags.CaloRinger.buildPhotonCornerRings:
        acc.merge(CaloRingerCornerPhotonAlgsCfg(flags))

    if flags.CaloRinger.buildJetRings:
        acc.merge(CaloRingerJetAlgsCfg(flags))
    if flags.CaloRinger.buildJetAsymRings:
        acc.merge(CaloRingerAsymJetAlgsCfg(flags))
    if flags.CaloRinger.buildJetStripsRings:
        acc.merge(CaloRingerStripsJetAlgsCfg(flags))
    if flags.CaloRinger.buildJetCornerRings:
        acc.merge(CaloRingerCornerJetAlgsCfg(flags))

    acc.merge(CaloRingerOutputCfg(flags))
    return acc


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags = initConfigFlags()
    flags.Input.Files = defaultTestFiles.RDO_RUN2
    flags.Output.doWriteAOD = True
    flags.Output.ESDFileName = "testing.ESD.root"
    flags.lock()
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    cfg = MainServicesCfg(flags)
    cfg.merge(CaloRingerSteeringCfg(flags))
