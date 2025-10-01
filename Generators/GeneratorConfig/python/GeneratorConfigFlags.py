"""Construct Generator configuration flags

Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
"""

from AthenaConfiguration.AthConfigFlags import AthConfigFlags


def createGeneratorConfigFlags():
    """Return an AthConfigFlags object with required flags"""
    gencf = AthConfigFlags()
        
    # Ignore black list
    gencf.addFlag("Generator.ignoreBlackList", False)
    
    # Input Generator File
    gencf.addFlag("Generator.inputGeneratorFile", '')
    
    # Number of input files per job
    gencf.addFlag("Generator.inputFilesPerJob", 1)
    
    # Events per job
    gencf.addFlag("Generator.nEventsPerJob", 10000)

    # Events per job
    gencf.addFlag("Generator.DSID", 999999)

    # First event
    gencf.addFlag("Generator.firstEvent", -1)
    
    # Number of HepMC events to print
    gencf.addFlag("Generator.printEvts", 0)
    
    # Output yoda file for jobs that require Rivet
    gencf.addFlag("Generator.outputYODAFile", '')
    
    # Output yoda file for jobs that require Rivet
    gencf.addFlag("Generator.rivetAnalyses", '')

    # Default PDG parameter settings
    gencf.addFlag("Generator.PDGparams", True)

    return gencf


def generatorRunArgsToFlags(runArgs, flags):
    """Fill generator configuration flags from run arguments."""
    
    from AthenaCommon.SystemOfUnits import GeV
    
    if hasattr(runArgs, "ecmEnergy"):
        flags.Beam.Energy = runArgs.ecmEnergy/2	* GeV 
    else:
        raise RuntimeError("No center of mass energy provided.") 
        
    if hasattr(runArgs, "ignoreBlackList"):
        flags.Generator.ignoreBlackList = runArgs.ignoreBlackList

    if hasattr(runArgs, "inputGeneratorFile"):
        flags.Generator.inputGeneratorFile = runArgs.inputGeneratorFile

    if hasattr(runArgs, "firstEvent"):
        flags.Generator.firstEvent = runArgs.firstEvent
    
    if hasattr(runArgs, "printEvts"):
        flags.Generator.printEvts = runArgs.printEvts
    
    if hasattr(runArgs, "outputYODAFile"):
        flags.Generator.outputYODAFile = runArgs.outputYODAFile
    
    if hasattr(runArgs, "rivetAnas"):
        flags.Generator.rivetAnalyses = runArgs.rivetAnas
