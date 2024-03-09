#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

"""
This script is equivalent to python -m TrkVertexWeightCalculators.TrkVertexWeightCalculatorsConfig
but it is an installed executable script. This simplify the grid submission
since there is no easy way to run CA with pathena / prun.

pathena --trf "TrkVertexWeighCalculatorsExample.py --filesInput=%IN Output.HISTFileName=%OUT.root --evtMax=-1" --inDS mc21_13p6TeV:mc21_13p6TeV.601521.PhPy8EG_PDF4LHC21_ggH_MINLO_gammagamma.merge.AOD.e8472_e8455_s3873_s3874_r13829_r13831 --outDS user.turra.mc21_13p6TeV.601521.PhPy8EG_PDF4LHC21_ggH_MINLO_gammagamma.dumpvertex.NTUP.e8472_s3873_r13829_v9
"""

import sys

from TrkVertexWeightCalculators.TrkVertexWeightCalculatorsConfig import TrkVertexWeightCalculatorBDTDebugRunCfg

acc = TrkVertexWeightCalculatorBDTDebugRunCfg()
status = acc.run()

sys.exit(not status.isSuccess())

