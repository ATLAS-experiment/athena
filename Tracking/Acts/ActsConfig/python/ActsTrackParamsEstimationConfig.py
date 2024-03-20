#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsTrackParamsEstimationToolCfg(ConfigFlags,
                                     name: str = "ActsTrackParamsEstimationTool",
                                     **kwargs) -> ComponentAccumulator:
    assert isinstance(name, str)

    acc = ComponentAccumulator()

    # eBoundLoc0, eBoundLoc1, eBoundPhi, eBoundTheta, eBoundQOverP, eBoundTime
    kwargs.setdefault('initialVarInflation', [1., 1., 1., 1., 1., 1.])

    acc.setPrivateTools(CompFactory.ActsTrk.TrackParamsEstimationTool(name=name, **kwargs))
    return acc
