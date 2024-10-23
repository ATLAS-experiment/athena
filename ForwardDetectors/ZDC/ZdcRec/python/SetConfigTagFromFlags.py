#!/usr/bin/env python
#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.Enums import LHCPeriod


def SetConfigTag(flags):

    if flags.Input.TriggerStream == "calibration_ZDCInjCalib" or flags.Input.TriggerStream == "calibration_DcmDummyProcessor": # calibration_DcmDummyProcessor is the "trigger stream" in the data we record in the standalone partition that is NOT LED data                
        config = "Injectorpp2024" # default config tag for injector pulse

        if flags.Input.ProjectName == "data24_hi":
            config = "InjectorPbPb2024"
    else:
        config = "PbPb2023" # default config tag
        
        run = flags.GeoModel.Run
        if (run == LHCPeriod.Run3):
            if flags.Input.isMC:
                config = "MonteCarloPbPb2023"
            elif flags.Input.ProjectName == "data22_13p6TeV":
                config = "LHCf2022"
            elif flags.Input.ProjectName == "data23_5p36TeV" or flags.Input.ProjectName == "data23_900GeV" or flags.Input.ProjectName == "data23_13p6TeV":
                config = "pp2023"
            elif flags.Input.ProjectName == "data23_hi" or flags.Input.ProjectName == "data23_comm":
                config = "PbPb2023"
            elif flags.Input.ProjectName == "data24_5p36TeV" or flags.Input.ProjectName == "data24_900GeV" or flags.Input.ProjectName == "data24_13p6TeV":
                config = "pp2024"
            elif flags.Input.ProjectName == "data24_hi":
                config = "PbPb2024"
        elif (run == LHCPeriod.Run2):
            if flags.Input.ProjectName == "data15_hi":
                config = "PbPb2015"
            elif flags.Input.ProjectName == "data17_13TeV":
                config = "PbPb2015"
            elif flags.Input.ProjectName == "data16_hip":
                config = "pPb2016"
            elif flags.Input.ProjectName == "data18_hi":
                config = "PbPb2018"

    return config