# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
####################################################################################
# This configuration replicates the beam configuration in the
# following file: 25ns_2748b_2736_2452_2524_288bpi12inj.sch. See the
# following ticket for more details:
# https://its.cern.ch/jira/browse/ATLASSIM-1788
# Any of the filled bunch crossings can be chosen as the central
# bunch crossing, so the timing of the out-of-time pile-up will vary
# significantly from event to event.  Cavern Background is
# independent of the bunch pattern. (Compatible with 25ns cavern
# background.)
####################################################################################


def setupBunchStructure(flags):
    # Set this to the spacing between filled bunch-crossings within the train.
    flags.Beam.BunchSpacing = 50
    # This now sets the bunch slot length.
    flags.Digitization.PU.BunchSpacing = 25

    flags.Digitization.PU.CavernIgnoresBeamInt = True

    flags.Digitization.PU.BeamIntensityPattern = \
        [0.0,  # bunch crossing zero is always empty
         1.0, 0.0, 0.0]
