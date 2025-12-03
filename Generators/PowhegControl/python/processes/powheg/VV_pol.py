# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import Logging
from ..powheg_RES import PowhegRES

## Get handle to Athena logging
logger = Logging.logging.getLogger("PowhegControl")


class VV_pol(PowhegRES):
    """! Default Powheg configuration for W-boson pair production plus one jet using MiNNLOPS.

    Create a configurable object with all applicable Powheg options.

    @author Aonan Wang <aonan.wang@cern.ch>
    """

    def __init__(self, base_directory, **kwargs):
        """! Constructor: all process options are set here.

        @param base_directory: path to PowhegBox code.
        @param kwargs          dictionary of arguments from Generate_tf.
        """
        super(VV_pol, self).__init__(base_directory, "VV_pol", **kwargs)

        # Add parameter validation functions
        self.validation_functions.append("validate_process")
        self.validation_functions.append("validate_polarization")

        # Add flag for the MiNNLO reweight
        self.reweight_for_MiNNLO = True

        ## List of allowed decay modes
        self.allowed_VVprocess = ["w+ w- > e+ ve mu- vm~",
                                    "w+ w- > mu+ vm e- ve~",
                                    "w+ w- > tau+ vt e- ve~",
                                    "w+ w- > e+ ve tau- vt~",
                                    "w+ w- > tau+ vt mu- vm~",
                                    "w+ w- > mu+ vm tau- vt~",
                                    "w+ w- > e+ ve e- ve~",
                                    "w+ w- > mu+ vm mu- vm~",
                                    "w+ w- > tau+ vt tau- vt~",
                                    "w+ z > e+ ve mu+ mu-",
                                    "w+ z > e+ ve tau+ tau-",
                                    "w+ z > mu+ vm e+ e-",
                                    "w+ z > mu+ vm tau+ tau-",
                                    "w+ z > tau+ vt e+ e-",
                                    "w+ z > tau+ vt mu+ mu-",
                                    "w- z > e- ve~ mu+ mu-",
                                    "w- z > e- ve~ tau+ tau-",
                                    "w- z > mu- vm~ e+ e-",
                                    "w- z > mu- vm~ tau+ tau-",
                                    "w- z > tau- vt~ e+ e-",
                                    "w- z > tau- vt~ mu+ mu-",
                                    "z z > e+ e- mu+ mu-",
                                    "z z > e+ e- tau+ tau-",
                                    "z z > mu+ mu- e+ e-",
                                    "z z > mu+ mu- tau+ tau-",
                                    "z z > tau+ tau- e+ e-",
                                    "z z > tau+ tau- mu+ mu-",
                                ]
        self.allowed_polarization = ["unpol-unpol",
                                     "unpol-transv",
                                     "transv-unpol",
                                     "unpol-longit",
                                     "longit-unpol",
                                     "longit-longit",
                                     "transv-transv",
                                     "longit-transv",
                                     "transv-longit",
                                     "unpol-left",
                                     "left-unpol",
                                     "longit-left",
                                     "left-longit",
                                     "transv-left",
                                     "left-transv",
                                     "unpol-right",
                                     "right-unpol",
                                     "longit-right",
                                     "right-longit",
                                     "transv-right",
                                     "right-transv",
                                     "right-left",
                                     "left-right",
                                     "left-left",
                                     "right-right",
                                     ]
        # Add all keywords for this process, overriding defaults if required
        self.add_keyword("ih1")
        self.add_keyword("ih2")
        self.add_keyword("lhans1", self.default_PDFs)
        self.add_keyword("lhans2", self.default_PDFs)
        self.add_keyword("renscfact", self.default_scales[1])
        self.add_keyword("facscfact", self.default_scales[0])
        self.add_keyword("procVV")
        self.add_keyword("idvecbos")
        self.add_keyword("decayV1", "w+ w- > e+ ve mu- vm~", name="VVprocess", hidden=False)
        self.add_keyword("decayV2")
        self.add_keyword("dpa", 1)
        self.add_keyword("pol1", 4, name="polarization")
        self.add_keyword("pol2", 4)
        self.add_keyword("NP_POWER")
        self.add_keyword("SUM_AMP")
        self.add_keyword("CHBD6")
        self.add_keyword("CHWD6")
        self.add_keyword("CHWBD6")
        self.add_keyword("CWD6")
        self.add_keyword("CHBtilD6")
        self.add_keyword("CHWtilD6")
        self.add_keyword("CHWBtilD6")
        self.add_keyword("CWtilD6")
        self.add_keyword("CWWWL2")
        self.add_keyword("CWL2")
        self.add_keyword("CBL2")
        self.add_keyword("CPWWWL2")
        self.add_keyword("CPWL2")
        self.add_keyword("whichphsp", 2)
        self.add_keyword("qcdonly", 0)
        self.add_keyword("qedonly", 0)
        self.add_keyword("numberofquarks", 4)
        self.add_keyword("alphas_from_pdf", 1)
        self.add_keyword("scheme", 1)
        self.add_keyword("runningscale", 0)
        self.add_keyword("improvedrclrunning")
        self.add_keyword("useOSmass-mu")
        self.add_keyword("mllcut")
        self.add_keyword("mllmax", "1d10")
        self.add_keyword("ewsimplecuts")
        self.add_keyword("ncall1", 1000000)
        self.add_keyword("itmx1", 1)
        self.add_keyword("fakevirt", 0)
        self.add_keyword("ncall2", 1000000)
        self.add_keyword("itmx2", 1)
        self.add_keyword("foldcsi", 1)
        self.add_keyword("foldy", 1)
        self.add_keyword("foldphi", 1)
        self.add_keyword("use-old-grid")
        self.add_keyword("testplots")
        self.add_keyword("nubound", 500000)
        self.add_keyword("xupbound", 2)
        self.add_keyword("use-old-ubound")
        self.add_keyword("icsimax", 1)
        self.add_keyword("iymax", 1)
        self.add_keyword("ubexcess_correct")
        self.add_keyword("storeinfo_rwgt")
        self.add_keyword("rwl_group_events", 1)
        self.add_keyword("rwl_file")
        self.add_keyword("rwl_add")
        self.add_keyword("rwl_format_rwgt")
        self.add_keyword("Zmass")
        self.add_keyword("Zwidth")
        self.add_keyword("Wmass")
        self.add_keyword("Wwidth")
        self.add_keyword("Tmass")
        self.add_keyword("Twidth")
        self.add_keyword("Hmass")
        self.add_keyword("Hwidth")
        self.add_keyword("Mumass", 0)
        self.add_keyword("Elmass", 0)
        self.add_keyword("Taumass", 0)
        self.add_keyword("gmu")
        self.add_keyword("CKM_Vud",1)
        self.add_keyword("CKM_Vus",0)
        self.add_keyword("CKM_Vub",0)
        self.add_keyword("CKM_Vcd",0)
        self.add_keyword("CKM_Vcs",1)
        self.add_keyword("CKM_Vcb",0)
        self.add_keyword("CKM_Vtd",0)
        self.add_keyword("CKM_Vts",0)
        self.add_keyword("CKM_Vtb",1)
        self.add_keyword("nondiagCKM")
        self.add_keyword("bornzerodamp")
        self.add_keyword("bornsuppfact")
        self.add_keyword("bornsuppfact-pt")
        self.add_keyword("bornonly")
        self.add_keyword("allrad")
        self.add_keyword("check_bad_st1")
        self.add_keyword("check_bad_st2")
        self.add_keyword("manyseeds")
        self.add_keyword("parallelstage")
        self.add_keyword("maxseeds")
        self.add_keyword("xgriditeration")

    def validate_process(self):
        """! Validate the various process keywords."""
        self.expose()  # convenience call to simplify syntax
        self.check_decay_mode(self.VVprocess, self.allowed_VVprocess)
        # Enable appropriate decay mode
        list(self.parameters_by_keyword("decayV1"))[0].value = 1 # adjust the one used as a proxy for process
        
        if "w+ w-" in self.VVprocess:
            list(self.parameters_by_keyword("procVV"))[0].value = 1
            if "e+ ve mu- vm~" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 1
                list(self.parameters_by_keyword("decayV2"))[0].value = 2
            elif "mu+ vm e- ve~" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 2
                list(self.parameters_by_keyword("decayV2"))[0].value = 1
            elif "tau+ vt e- ve~" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 3
                list(self.parameters_by_keyword("decayV2"))[0].value = 1
            elif "e+ ve tau- vt~" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 1
                list(self.parameters_by_keyword("decayV2"))[0].value = 3
            elif "tau+ vt mu- vm~" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 3
                list(self.parameters_by_keyword("decayV2"))[0].value = 2
            elif "mu+ vm tau- vt~" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 2
                list(self.parameters_by_keyword("decayV2"))[0].value = 3
            elif "e+ ve e- ve~" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 1
                list(self.parameters_by_keyword("decayV2"))[0].value = 1
            elif "mu+ vm mu- vm~" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 2
                list(self.parameters_by_keyword("decayV2"))[0].value = 2
            elif "tau+ vt tau- vt~" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 3
                list(self.parameters_by_keyword("decayV2"))[0].value = 3
                
        elif "z z" in self.VVprocess:
            list(self.parameters_by_keyword("procVV"))[0].value = 2
            list(self.parameters_by_keyword("numberofquarks"))[0].value = 5
            if "e+ ve mu+ mu-" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 1
                list(self.parameters_by_keyword("decayV2"))[0].value = 2
            elif "e+ e- tau+ tau-" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 1
                list(self.parameters_by_keyword("decayV2"))[0].value = 3
            elif "mu+ mu- e+ e-" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 2
                list(self.parameters_by_keyword("decayV2"))[0].value = 1
            elif "mu+ mu- tau+ tau-" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 2
                list(self.parameters_by_keyword("decayV2"))[0].value = 3
            elif "tau+ tau- e+ e-" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 3
                list(self.parameters_by_keyword("decayV2"))[0].value = 1
            elif "tau+ tau- mu+ mu-" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 3
                list(self.parameters_by_keyword("decayV2"))[0].value = 2

        elif "w+ z" in self.VVprocess:
            list(self.parameters_by_keyword("idvecbos"))[0].value = 24
            list(self.parameters_by_keyword("procVV"))[0].value = 3
            if "e+ ve mu+ mu-" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 1
                list(self.parameters_by_keyword("decayV2"))[0].value = 2
            elif "e+ ve tau+ tau-" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 1
                list(self.parameters_by_keyword("decayV2"))[0].value = 3
            elif "mu+ vm e+ e-" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 2
                list(self.parameters_by_keyword("decayV2"))[0].value = 1
            elif "mu+ vm tau+ tau-" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 2
                list(self.parameters_by_keyword("decayV2"))[0].value = 3
            elif "tau+ vt e+ e-" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 3
                list(self.parameters_by_keyword("decayV2"))[0].value = 1
            elif "tau+ vt mu+ mu-" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 3
                list(self.parameters_by_keyword("decayV2"))[0].value = 2

        elif "w- z" in self.VVprocess:
            list(self.parameters_by_keyword("idvecbos"))[0].value = -24
            list(self.parameters_by_keyword("procVV"))[0].value = 3
            if "e- ve~ mu+ mu-" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 1
                list(self.parameters_by_keyword("decayV2"))[0].value = 2
            elif "e- ve~ tau+ tau-" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 1
                list(self.parameters_by_keyword("decayV2"))[0].value = 3
            elif "mu- vm~ e+ e-" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 2
                list(self.parameters_by_keyword("decayV2"))[0].value = 1
            elif "mu- vm~ tau+ tau-" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 2
                list(self.parameters_by_keyword("decayV2"))[0].value = 3
            elif "tau- vt~ e+ e-" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 3
                list(self.parameters_by_keyword("decayV2"))[0].value = 1
            elif "tau- vt~ mu+ mu-" in self.VVprocess:
                list(self.parameters_by_keyword("decayV1"))[0].value = 3
                list(self.parameters_by_keyword("decayV2"))[0].value = 2

    def validate_polarization(self):
        """! Validate the various polarization keywords."""
        self.expose()  # convenience call to simplify syntax
        self.check_decay_mode(self.polarization, self.allowed_polarization)
        # Enable appropriate decay mode
        list(self.parameters_by_keyword("pol1"))[0].value = 4 # adjust the one used as a proxy for polarization
        list(self.parameters_by_keyword("pol2"))[0].value = 4

        if "unpol-unpol" not in self.polarization:
            list(self.parameters_by_keyword("dpa"))[0].value = 1
            list(self.parameters_by_keyword("qcdonly"))[0].value = 1
            if "transv-" in self.polarization:
                list(self.parameters_by_keyword("pol1"))[0].value = 3
            elif "longit-" in self.polarization:
                list(self.parameters_by_keyword("pol1"))[0].value = 0
            elif "left-" in self.polarization:
                list(self.parameters_by_keyword("pol1"))[0].value = -1
            elif "right-" in self.polarization:
                list(self.parameters_by_keyword("pol1"))[0].value = 1
            if "-transv" in self.polarization:
                list(self.parameters_by_keyword("pol2"))[0].value = 3
            elif "-longit" in self.polarization:
                list(self.parameters_by_keyword("pol2"))[0].value = 0
            elif "-left" in self.polarization:
                list(self.parameters_by_keyword("pol2"))[0].value = -1
            elif "-right" in self.polarization:
                list(self.parameters_by_keyword("pol2"))[0].value = 1

