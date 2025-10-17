# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# File: CaloRec/python/KeepTopoClusterCells.py
# Created: August 2025, C.A
# Purpose: To be used as postInclude to Keep
# All Cell associated with TopoClusters


def keepTopoClusterCellsCfg(flags, cfg):
    """Fragment to be used as postInclude:
    CaloRec.keepTopoClusterCellsConfig.keepTopoClusterCellsCfg"""

    from CaloRec.CaloThinCellsByClusterAlgConfig import (
        CaloThinCellsByClusterAlgCfg)
    doLCCalib = flags.Calo.TopoCluster.doTopoClusterLocalCalib
    clustersname = "CaloCalTopoClusters" if doLCCalib else "CaloTopoClusters"
    cellsName = flags.Egamma.Keys.Input.CaloCells

    cfg.merge(CaloThinCellsByClusterAlgCfg(
        flags,
        streamName="StreamAOD",
        clusters=clustersname,
        samplings=[],
        cells=cellsName
    ))

    return cfg