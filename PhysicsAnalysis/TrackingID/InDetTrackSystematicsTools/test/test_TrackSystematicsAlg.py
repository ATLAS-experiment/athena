#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#


import argparse
import sys

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from InDetTrackSystematicsTools.InDetTrackSystematicsToolsConfig import (
    TrackSystematicsAlgCfg,
)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--input',
        default=defaultTestFiles.AOD_RUN3_MC[0],
        help='Input AOD file (default: defaultTestFiles.AOD_RUN3_MC)',
    )
    parser.add_argument('--events', type=int, default=10)
    parser.add_argument(
        '--unknown-syst',
        action='store_true',
        help='Expect failure for a systematic unknown to its filter tool',
    )
    args = parser.parse_args()

    flags = initConfigFlags()
    flags.Input.Files = [args.input]
    flags.lock()

    acc = MainServicesCfg(flags)
    acc.merge(PoolReadCfg(flags))

    # Misspelt efficiency systematic must be rejected in initialize
    configs = [('TRK_EFF_LOOSE_GLOBL', {'SystematicVariationSTD': 'TRK_EFF_LOOSE_GLOBL'})] if args.unknown_syst else [
        # Combined STD + LRT variation in one container, as in LLP1 and IDTR2
        ('TRK_EFF_LARGED0_GLOBAL__1down', {'SystematicVariationSTD': 'TRK_EFF_LOOSE_COMBINED',
                                           'SystematicVariationLRT': 'TRK_EFF_LARGED0_GLOBAL'}),
        ('TRK_EFF_LOOSE_GLOBAL', {'SystematicVariationSTD': 'TRK_EFF_LOOSE_GLOBAL'}),
    ]
    for suffix, kwargs in configs:
        acc.merge(TrackSystematicsAlgCfg(
            flags,
            name=f'InDetTrackSystematicsAlg_{suffix}',
            InputTrackContainer='InDetTrackParticles',
            OutputTrackContainer=f'InDetTrackParticles_{suffix}',
            **kwargs,
        ))

    sc = acc.run(args.events)
    sys.exit(sc.isSuccess() if args.unknown_syst else sc.isFailure())


if __name__ == '__main__':
    main()
