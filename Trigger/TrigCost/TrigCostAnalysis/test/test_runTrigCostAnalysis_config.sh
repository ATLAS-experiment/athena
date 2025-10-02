#!/bin/sh

# This only tests configuration of the RunTrigCostAnalysis.py script
# No events need to be read, so a generic input file with Trigger metadata suffices

RunTrigCostAnalysis.py --evtMax 0 --filesInput /cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test/data24_13p6TeV.00475321.physics_Main.daq.RAW._lb0247._SFO-11._0006.data_150evt
