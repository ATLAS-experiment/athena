#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Minimal test for JetTrackFilteringAlg: build AntiKt4EMPFlowJets (including
# GhostTrack association) from an AOD, then exercise one systematic per
# filter tool in a single job:
#
#   TRK_EFF_LOOSE_GLOBAL   -> InDetTrackTruthFilterTool  (STD tracks)
#   TRK_EFF_LARGED0_GLOBAL -> InclusiveTrackFilterTool   (LRT tracks)
#   TRK_EFF_LOOSE_TIDE     -> JetTrackFilterTool + STD   (TIDE tracks)
#
# Jets are built on the fly from PFOs using JetRecCfg -- AODs do not store
# pre-built jets.  The default CI input is defaultTestFiles.AOD_RUN3_MC
# (an MC23c AOD on CVMFS).  Pass --input to override with a local file.
#
# mc_campaign is read from file metadata before flags.lock() only to apply
# the mc23c -> mc23d remap (MC23c has no dedicated recommendations).

import argparse
import sys

from AthenaConfiguration.AllConfigFlags import initConfigFlags
from AthenaConfiguration.MainServicesConfig import MainServicesCfg
from AthenaConfiguration.TestDefaults import defaultTestFiles
from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
from Campaigns.Utils import Campaign
from InDetTrackSystematicsTools.InDetTrackSystematicsToolsConfig import (
    JetTrackFilteringAlgCfg,
)
from JetRecConfig.JetRecConfig import JetRecCfg
from JetRecConfig.StandardSmallRJets import AntiKt4EMPFlow
from PyUtils.MetaReader import read_metadata


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--input',
        default=defaultTestFiles.AOD_RUN3_MC[0],
        help='Input AOD file (default: defaultTestFiles.AOD_RUN3_MC)',
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
    acc.merge(JetRecCfg(flags, AntiKt4EMPFlow))

    jet = AntiKt4EMPFlow.fullname()
    ghost_in = f'{jet}.GhostTrack'
    for syst, suffix in [
        ('TRK_EFF_LOOSE_GLOBAL',   'GhostTrackSTD'),
        ('TRK_EFF_LARGED0_GLOBAL', 'GhostTrackLRT'),
        ('TRK_EFF_LOOSE_TIDE',     'GhostTrackTIDE'),
    ]:
        acc.merge(JetTrackFilteringAlgCfg(
            flags,
            syst=syst,
            jet_collection=jet,
            in_ghost_tracks=ghost_in,
            out_ghost_tracks=f'{jet}.{suffix}',
        ))

    sys.exit(acc.run(args.events).isFailure())


if __name__ == '__main__':
    main()
