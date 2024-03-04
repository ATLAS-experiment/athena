"""
Configuration for transfering SFGen output from Lhe to ENVT.
"""

import SFGen_i.EventFiller as EF

ef = EF.LheEVNTFiller(runArgs.ecmEnergy)
genSeq += ef
