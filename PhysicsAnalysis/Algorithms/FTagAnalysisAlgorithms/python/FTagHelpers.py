# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.Enums import LHCPeriod

def getRecommendedBTagCalib_Run2():
    return "xAODBTaggingEfficiency/13TeV/MC20_2025-06-17_GN2v01_v4.root"

def getRecommendedBTagCalib_Run3():
    return "xAODBTaggingEfficiency/13p6TeV/MC23_2025-06-17_GN2v01_v4.root"

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

def getRecommendedBTagTrigCalib(geometry):
    """return the recommended bjet trigger calibration files
    for a given LHCPeriod 'geometry'
    """
    if geometry is LHCPeriod.Run3:
        return "xAODBTaggingEfficiency/13p6TeV-Online/online-MC23_2025-11-26_v2_smooth.root"
    else:
        raise ValueError(f"LHCPeriod {geometry} does not have a recommended bjet trigger calibration file!")

def getReadFromBTaggingObject(flags, jetCollection, defaultReadFromBTaggingObject):
    
    if defaultReadFromBTaggingObject is not None: 
        # In that case the user has provided a value through the readFromBTaggingObject flag 
        # Thus follow user requirements 
        # to read tagging probabilities
        return defaultReadFromBTaggingObject
    
    
    # If reaching this point it means we determine for the user 
    # what should be the reading strategy
    
    # Define the default value (since default is None) to be used 
    # in case we do not have enough information at our disposal 
    # to determine the strategy to adopt
    defaultReadFromBTaggingObject = False
    
    # No auto configuration flag thus using default value 
    if flags is None: 
        return defaultReadFromBTaggingObject
    
    # Otherwise autoconfiguration flags exist 
    # Let's use directly information from meta data 
    # The list contains container names 
    # NB: Only for AnalysisBase the list also contains branch names
    listContainersInputFiles = flags.Input.Collections
    
    # Make sure list is not empty 
    # otherwise use default value
    if not listContainersInputFiles: 
        return defaultReadFromBTaggingObject
    
    # For AthAnalysis and Athena the list contains string of type: cppyy.gbl.std.string 
    # hence transforming it to a list of string always applying str() function
    listContainersInputFiles = [ str(theStr) for theStr in listContainersInputFiles ]
    
    # Remove "Jets" from the jetCollection string 
    if jetCollection.endswith("Jets"):
        jetCollection = jetCollection[:-4]
    
    # e.g. BTaggingJetContainerName="BTagging_AntiKt4EMPFlow"
    BTaggingJetContainerName =  f"BTagging_{jetCollection}"
    
    # If the BTagging object is into the list then 
    # we can read the BTagging object (old DAODs) 
    # Otherwise we should read jet probabilities directly 
    # (new DAODs without BTagging object) 
    return (BTaggingJetContainerName in listContainersInputFiles)
