#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Minimal test for TrackSmearingAlg: read InDetTrackParticles from any xAOD
# (AOD or DAOD) and exercise one systematic per smearing/biasing tool in a
# single job:
#
#   TRK_RES_D0_MEAS  -> InDetTrackSmearingTool
#   TRK_BIAS_D0_WM   -> InDetTrackBiasingTool
#
# Only reads InDetTrackParticles directly -- no element-link dereference --
# so any format that stores tracks works (including DAOD_PHYS).

import argparse
import sys

from AsgAnalysisAlgorithms.PileupReweightingAlgConfig import (
    PileupReweightingAlgCfg,
)
from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from Campaigns.Utils import Campaign
from InDetTrackSystematicsTools.InDetTrackSystematicsToolsConfig import (
    TrackSmearingAlgCfg,
)
from PyUtils.MetaReader import read_metadata


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--input',
        default=defaultTestFiles.AOD_RUN3_MC[0],
        help='Input AOD/xAOD file (default: defaultTestFiles.AOD_RUN3_MC)',
    )
    parser.add_argument('--events', type=int, default=10)
    args = parser.parse_args()

    # mc23c has no dedicated recommendations; treat as mc23d for testing.
    _meta = read_metadata(args.input, mode='lite')[args.input]
    _campaign = _meta.get('mc_campaign')

    flags = initConfigFlags()
    flags.Input.Files = [args.input]
    if _campaign == 'mc23c':
        flags.Input.MCCampaign = Campaign.MC23d
    flags.lock()

    acc = MainServicesCfg(flags)
    acc.merge(PoolReadCfg(flags))
    acc.merge(PileupReweightingAlgCfg(flags))

    for syst in ['TRK_RES_D0_MEAS', 'TRK_BIAS_D0_WM']:
        acc.merge(TrackSmearingAlgCfg(
            flags,
            syst=syst,
            input_tracks='InDetTrackParticles',
            output_tracks=f'InDetTrackParticles_{syst}',
        ))

    sys.exit(acc.run(args.events).isFailure())


if __name__ == '__main__':
    main()
