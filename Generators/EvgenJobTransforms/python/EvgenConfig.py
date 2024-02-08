#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from PyJobTransformsCore.TransformConfig import TransformConfig, String, ListOfStrings, Boolean, Integer, AllowedExpression, TransformConfigError
from GeneratorConfig.GenConfigHelpers import KnownGenerators

class EvgenConfig(TransformConfig):
    __slots__ = ()
    generators = ListOfStrings("List of used generators", allowedValues=KnownGenerators)
    description = String("Human readable description of the dataset content")
    process = String("Specific information about the physics process being generated")
    notes = String("Extra information about this process e.g. known current problems")
    contact = ListOfStrings("Contact person for this dataset. Leave empty for 'MC group'")
    keywords = ListOfStrings("Search keywords for this dataset, e.g. 'QCD', 'EW', 'minbias', ...")
    categories = ListOfStrings("Category keywords for this dataset, e.g. 'L1:Top', 'L2:RareTop'")
    inputfilecheck = String("A regex to check that the input file needed for some generators has a valid name")
    inputconfcheck = String("A regex to check that the config file needed for some generators has a valid name")
    specialConfig = String("Special configuration for subsequent prod steps")
    tune = String("Generator shower/hadronisation/MPI tune name")
    saveJets = Boolean("Save truth jet collections in the output file if they are available", False)
    savePileupTruthParticles = Boolean("Save truth particle collections, if available, for pileup-overlay output", False)
    findJets = Boolean("Schedule jet finding algorithms for each defined jet container", False)
    doNotSaveItems = ListOfStrings("List of StreamEVGEN items to NOT save in output file - note occurs BEFORE extraSaveItems are added")
    extraSaveItems = ListOfStrings("List of extra StreamEVGEN items to save in output file - note occurs AFTER doNotSaveItems are removed")
    inputFilesPerJob = Integer("number of input files per job",0, AllowedExpression("value >= 0"))
    nEventsPerJob = Integer("number of input events per job",0, AllowedExpression("value >= 0"))
    obsolete = Boolean("Are JOs/common fragment obsolete", False)
    PDGparams = Boolean("Do we use the standard PDG values for masses, widths etc. ", False)

    def __init__(self, name="evgenConfig"):
        TransformConfig.__init__(self, name)
        self.contact = ["MC group"]
        self.auxfiles = ["PDGTABLE.MeV", "pdt.table", "DECAY.DEC", "Bdecays0.dat", "Bs2Jpsiphi.DEC","G4particle_acceptlist.txt","susyParticlePdgid.txt"]
        self.nEventsPerJob = 10000
        self.maxeventsstrategy = "ABORT"
        self.specialConfig = "NONE"
# for the sake of Generate_tf leave minevents for a while
        self.minevents = 0

    ## Explicitly block MC11/12 settings of efficiency, input*base, or weighting attrs
    def __setattr__(self, name, value):
        if name in ["efficiency", "inputfilebase", "inputconfbase", "weighting"]:
            msg = "evgenConfig.%s is not used in MC14 production. " % name + \
                "Please update your JO file, having read the docs at " + \
                "https://twiki.cern.ch/twiki/bin/viewauth/AtlasProtected/AtlasMcProductionMC14"
            raise TransformConfigError(msg)
        object.__setattr__(self, name, value)


evgenConfig = EvgenConfig()
