# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
""" This configuration is for use with MC23 Heavy Ion simulation for oxygen data """


def setupBunchStructure(flags):
    # Set this to the spacing between filled bunch-crossings within the train.
    # there are no trains in this run and bunches are interspaced 
    flags.Beam.BunchSpacing = 25*40
    # This now sets the bunch slot length.
    flags.Digitization.PU.BunchSpacing = 25 # IS THIS CORRECT?
    flags.Digitization.PU.CavernIgnoresBeamInt = False
    #from: https://atlas-triggertool.web.cern.ch/db/run3/bgs/2988/
    paired = [172, 212, 252, 292, 
              673, 713, 753, 793, 895, 935, 975,  1015, 
              1396, 1436, 1476, 1516, 1951, 1991, 2031, 2071, 
              2458, 2498, 2538, 2578, 2674, 2714, 2754, 2794, 
              3181, 3221, 3261, 3301]
    flags.Digitization.PU.BeamIntensityPattern = [ (1.0 if i in paired else 0.0)  for i in range(0, 3564) ]
