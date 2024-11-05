# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.Enums import LHCPeriod

def getRecommendedBTagCalib_Run2():
    return "xAODBTaggingEfficiency/13TeV/MC20_2024-10-17_GN2v01_v1.root"

def getRecommendedBTagCalib_Run3():
    return "xAODBTaggingEfficiency/13p6TeV/MC23_2024-10-17_GN2v01_v1.root"

def getRecommendedBTagCalib(geometry):
    """return the recommended FTag calibration files
    for a given LHCPeriod 'geometry'
    """
    if geometry is LHCPeriod.Run2:
        return getRecommendedBTagCalib_Run2()
    elif geometry >= LHCPeriod.Run3:
        return getRecommendedBTagCalib_Run3()
    else:
        raise ValueError(f"LHCPeriod {geometry} does not have a recommended FTag calibration file!")
