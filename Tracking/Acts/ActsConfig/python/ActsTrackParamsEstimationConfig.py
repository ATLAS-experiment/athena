#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory

def ActsTrackParamsEstimationToolCfg(ConfigFlags,
                                     name: str = "ActsTrackParamsEstimationTool",
                                     **kwargs) -> ComponentAccumulator:
    assert isinstance(name, str)

    acc = ComponentAccumulator()

    # eBoundLoc0, eBoundLoc1, eBoundPhi, eBoundTheta, eBoundQOverP, eBoundTime
    kwargs.setdefault('initialVarInflation', ConfigFlags.Acts.initialVarInflation)
    kwargs.setdefault('refitErrInflation', ConfigFlags.Acts.refitErrInflation)

    kwargs.setdefault('allowPropagatorFailure', False)

    kwargs.setdefault('parameterEstimationMode', ConfigFlags.Acts.parameterEstimationMode.value)
    kwargs.setdefault('minDeltaR', ConfigFlags.Acts.minDeltaRParameterEstimation)

    kwargs.setdefault("refitSeeds", ConfigFlags.Acts.refitSeeds)

    if 'FitterTool' not in kwargs:
        from ActsConfig.ActsTrackFittingConfig import ActsFitterCfg
        # This fitter is only used for the seed refit, so restore a finite
        # OutlierChi2Cut only if a refit is scheduled for at least one seed
        # collection (currently the LRT strip seeds), letting the refit
        # potentially downweight bad hits before they bias initial parameters fed to CKF.
        # currently does nothing (flag defaults to inf == disabled)
        if kwargs["refitSeeds"]:
            seedRefitOutlierChi2Cut = ConfigFlags.Acts.SeedRefitOutlierChi2Cut
        else:
            seedRefitOutlierChi2Cut = float('inf')
        kwargs.setdefault(
            'FitterTool',
            acc.popToolsAndMerge(ActsFitterCfg(ConfigFlags,
                                               ReverseFilteringPt=0,
                                               OutlierChi2Cut=seedRefitOutlierChi2Cut))
        )

    acc.setPrivateTools(CompFactory.ActsTrk.TrackParamsEstimationTool(name=name, **kwargs))
    return acc
