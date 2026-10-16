# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaCommon.Logging import logging

logPHYS = logging.getLogger('PHYS')


def PVNNVertexCfg(flags):
    acc = ComponentAccumulator()

    from SGComps.AddressRemappingConfig import InputRenameCfg
    acc.merge(InputRenameCfg("xAOD::VertexContainer", "PrimaryVertices", "PrimaryVertices_initial"))
    acc.merge(InputRenameCfg("xAOD::VertexAuxContainer", "PrimaryVerticesAux.", "PrimaryVertices_initialAux."))

    from DerivationFrameworkPhys.PVNNVertexConfig import CustomJetsCfg
    acc.merge(CustomJetsCfg(flags))

    ## Hard-scatter vertex selection
    from InDetConfig.InDetPVNNSelectionConfig import PVNNSequenceCfg
    acc.merge(PVNNSequenceCfg(flags))

    from TrkConfig.TrkVertexToolsConfig import VertexCollectionSortingToolCfg
    vxsort = VertexCollectionSortingToolCfg(flags)
    from InDetPriVxFinder.ResortVerticesConfig import ResortVerticesCfg
    acc.merge(ResortVerticesCfg(flags, "PrimaryVertices_initial", "PrimaryVertices", vxsort))

    return acc


def HGamNNVertexCfg(flags):
    acc = ComponentAccumulator()

    from SGComps.AddressRemappingConfig import InputRenameCfg
    acc.merge(InputRenameCfg("xAOD::VertexContainer", "PrimaryVertices", "PrimaryVertices_old"))
    acc.merge(InputRenameCfg("xAOD::VertexAuxContainer", "PrimaryVerticesAux.", "PrimaryVertices_oldAux."))

    from DerivationFrameworkHiggs.HIGG1D1CustomVertexConfig import DiPhotonVertexCfg
    acc.merge(DiPhotonVertexCfg(flags, VertexContainer="PrimaryVertices_old"))

    from TrkConfig.TrkVertexToolsConfig import VertexCollectionSortingToolCfg
    vxsort = VertexCollectionSortingToolCfg(flags)
    from InDetPriVxFinder.ResortVerticesConfig import ResortVerticesCfg
    acc.merge(ResortVerticesCfg(flags, "HggPrimaryVertices", "PrimaryVertices", vxsort))

    return acc