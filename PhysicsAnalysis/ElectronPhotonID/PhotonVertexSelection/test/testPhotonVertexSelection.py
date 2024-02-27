#!/usr/bin/env python

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

__doc__ = """Script / jobOptions to test PhotonVertexSelectionTool using
Athena"""
__author__ = "Bruno Lenzi"

import sys
from AthenaPython import PyAthena
from AthenaPython.PyAthena import StatusCode


def printMethod(x):
    print(x)


def getViewContainer(container):
    """getViewContainer(container) --> return a view container
    with at most 2 egamma objects from the given container,
    ignoring topo-seeded photons and fwd
    electrons"""
    from xAODEgamma.xAODEgammaParameters import xAOD

    def filterAuthor(x):
        return x.author() not in [
            xAOD.EgammaParameters.AuthorCaloTopo35,
            xAOD.EgammaParameters.AuthorFwdElectron,
        ]

    import ROOT
    egammas = container.__class__(ROOT.SG.VIEW_ELEMENTS)
    for eg in list(filter(filterAuthor, container))[:2]:
        egammas.push_back(eg)
    return egammas


def printOutput(container, tool, printMethod=printMethod):
    """printOutput(egammas, tool) -> print case and MVA output"""
    # TODO: pointing, HPV, zcommon
    if len(container) < 2:
        return
    # ignore conversions if running on electrons
    result = tool.getVertex(container, "Electron" in container.__class__.__name__)
    printMethod("Case: %s" % tool.getCase())
    for vertex, mva in result:
        printMethod("  Vertex %s: %s" % (vertex.index(), mva))


class TestPhotonVertexSelection(PyAthena.Alg):
    """PyAthena test Algorithm"""

    def __init__(self, name="TestPhotonVertexSelection", containerName="Photons", **kw):
        super(TestPhotonVertexSelection, self).__init__(
            name=name, containerName=containerName, **kw
        )

    def initialize(self):
        self.msg.info("initializing [%s]", self.name)
        self.vertexTool = PyAthena.py_tool(
            self.PhotonVertexSelectionTool.getFullJobOptName(),
            iface="CP::IPhotonVertexSelectionTool",
        )
        if not self.vertexTool:
            self.msg.error("Problem retrieving PhotonVertexSelectionTool !!")
            return PyAthena.StatusCode.Failure

        return StatusCode.Success

    def execute(self):
        viewContainer = getViewContainer(self.evtStore[self.containerName])
        printOutput(viewContainer, self.vertexTool, self.msg.info)
        return StatusCode.Success

    def finalize(self):
        return StatusCode.Success


if __name__ == "__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.Enums import ProductionStep

    flags = initConfigFlags()
    flags.Common.ProductionStep = ProductionStep.Simulation

    from AthenaConfiguration.TestDefaults import defaultTestFiles

    flags.Input.Files = defaultTestFiles.AOD_RUN2_MC
    flags.Exec.MaxEvents = 5
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg

    acc = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg

    acc.merge(PoolReadCfg(flags))

    # Configure and PhotonVertexSelectionTool
    from PhotonVertexSelection.PhotonVertexSelectionConfig import (
        PhotonVertexSelectionToolCfg,
    )

    selTool = acc.popToolsAndMerge(PhotonVertexSelectionToolCfg(flags))

    testAlg = TestPhotonVertexSelection(PhotonVertexSelectionTool=selTool)
    acc.addEventAlgo(testAlg)

    sc = acc.run()
    # Success should be 0
    sys.exit(not sc.isSuccess())
