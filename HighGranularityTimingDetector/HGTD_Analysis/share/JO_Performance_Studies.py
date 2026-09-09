#!/usr/bin/env athena
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#
# Driver job options for the HGTD track-time performance studies.
#
# Runs HGTD_TrkTimePerformanceStudies over an AOD and writes
# HGTD_PerformanceStudies_output.root. It is a thin wrapper around
# HGTD_Analysis.HGTD_AnalysisConfig.main(), so this file and
# `python -m HGTD_Analysis.HGTD_AnalysisConfig` do exactly the same thing.
#
# Usage:
#   python JO_Performance_Studies.py --filesInput=AOD.pool.root
#   athena.py JO_Performance_Studies.py --filesInput=AOD.pool.root --evtMax=10

if __name__ == "__main__":
    import sys

    from HGTD_Analysis.HGTD_AnalysisConfig import main
    sys.exit(main())
