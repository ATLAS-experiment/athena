# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def HGTD_AlignDBToolCfg(flags, name="HGTD_AlignDBTool", **kwargs):
    """
    Configure the HGTD alignment database tool.

    This tool is responsible for creating and managing HGTD alignment
    payloads that can later be written to the Conditions Database
    (SQLite/COOL).
    """

    acc = ComponentAccumulator()

    kwargs.setdefault("DetectorManager", "HGTD")
    kwargs.setdefault("DBRoot", "/HGTD/Align")

    acc.setPrivateTools(
        CompFactory.HGTD_AlignDBTool(name, **kwargs)
    )

    return acc
