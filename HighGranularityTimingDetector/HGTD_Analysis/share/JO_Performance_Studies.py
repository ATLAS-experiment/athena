#!/usr/bin/env athena
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Driver job options for the HGTD track-time performance studies.
#
# Runs HGTD_TrkTimePerformanceStudies over an AOD and writes
# HGTD_PerformanceStudies_output.root.
#
# The input AOD must have been produced with
#   --preExec 'all:flags.Tracking.writeExtendedHGTDInfo=True'
# otherwise the extended HGTD track decorations the track-time accessor tools
# read (HGTD_cluster_time, HGTD_primary_expected, HGTD_summaryinfo, ...) are
# stripped from the AOD and all histograms come out empty.
#
# Usage:
#   python JO_Performance_Studies.py --filesInput=AOD.pool.root
#   athena.py JO_Performance_Studies.py --filesInput=AOD.pool.root --evtMax=10

if __name__ == "__main__":
    import sys

    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    flags.Exec.MaxEvents = -1
    flags.fillFromArgs()  # set the input with: --filesInput=...

    if not flags.Input.Files:
        sys.exit("ERROR: no input file given, use --filesInput=<AOD.pool.root>")

    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg.merge(PoolReadCfg(flags))

    from HGTD_Analysis.HGTD_AnalysisConfig import HGTD_TrkTimePerformanceStudiesCfg
    cfg.merge(HGTD_TrkTimePerformanceStudiesCfg(flags))

    sys.exit(cfg.run().isFailure())
