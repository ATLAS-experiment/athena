# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import Logging
from ..powheg_RES import PowhegRES

## Get handle to Athena logging
logger = Logging.logging.getLogger("PowhegControl")


class ZZj_MiNNLO(PowhegRES):
    """! Default Powheg configuration for Z-boson pair production plus one jet using MiNNLOPS.

    Create a configurable object with all applicable Powheg options.

    @author Guglielmo Frattari <guglielmo.frattari@cern.ch>
    """

    def __init__(self, base_directory, **kwargs):
        """! Constructor: all process options are set here.

        @param base_directory: path to PowhegBox code.
        @param kwargs          dictionary of arguments from Generate_tf.
        """
        super(ZZj_MiNNLO, self).__init__(base_directory, "ZZJ", **kwargs)

        # Add parameter validation functions
        self.validation_functions.append("validate_decays")

        # Add flag for the MiNNLO reweight
        self.reweight_for_MiNNLO = True

        ## List of allowed decay modes
        self.allowed_decay_modes = ["z z > e- e+ mu- mu+",
                                    "z z > e- e+ tau- tau+",
                                    "z z > mu- mu+ tau- tau+",
                                    "z z > l- l+ l'- l'+",
                                    "z z > e- e+ e- e+",
                                    "z z > mu- mu+ mu- mu+",
                                    "z z > tau- tau+ tau- tau+",
                                    "z z > l- l+ l- l+",
                                    "z z > e- e+ e- e+ / mu- mu+ mu- mu+", 
                                    "z z > e- e+ vl vl~",
                                    "z z > mu- mu+ vl vl~",
                                    "z z > tau- tau+ vl vl~",
                                    "z z > l- l+ vl vl~",
                                    "z z > e- e+ vl vl~ / mu- mu+ vl vl~",
                                    "z z (+ w w) > e- e+ vl vl~",
                                    "z z (+ w w) > mu- mu+ vl vl~",
                                    "z z (+ w w) > tau- tau+ vl vl~",
                                    "z z (+ w w) > l- l+ vl vl~",
                                    "z z (+ w w) > e- e+ vl vl~ / mu- mu+ vl vl~",
                                    "z z > j j j' j'",
                                    "z z > j j j j",
                                    "z z > l- l+ j j",
                                    "z z > j j vl vl~"]

        # Add all keywords for this process, overriding defaults if required
        self.add_keyword("ih1")
        self.add_keyword("ih2")
        self.add_keyword("lhans1", self.default_PDFs)
        self.add_keyword("lhans2", self.default_PDFs)
        self.add_keyword("alphas_from_pdf", 1)
        self.add_keyword("fixedscale", 0) 
        self.add_keyword("whichscale", 1) 
        self.add_keyword("minlo", 1)
        self.add_keyword("minnlo", 1)
        self.add_keyword("modlog_p", "-1d0")
        self.add_keyword("Q0", "0d0")
        self.add_keyword("largeptscales", 1)
        self.add_keyword("smartMiNLO", 1)
        self.add_keyword("run_mode", 4)
        self.add_keyword("rwl_group_events", 1)
        self.add_keyword("renscfact", self.default_scales[1])
        self.add_keyword("facscfact", self.default_scales[0])
        self.add_keyword("storeinfo_rwgt", 1)
        self.add_keyword("rwl_file")
        self.add_keyword("ewscheme")
        self.add_keyword("gfermi")
        self.add_keyword("zmass")
        self.add_keyword("wmass")
        self.add_keyword("hmass")
        self.add_keyword("tmass")
        self.add_keyword("bmass", 0)
        self.add_keyword("zwidth")
        self.add_keyword("wwidth")
        self.add_keyword("twidth")
        self.add_keyword("hwidth")
        self.add_keyword("e-e+mu-mu+", 1, name="decay_mode", hidden=False)
        self.add_keyword("e-e+tau-tau+")
        self.add_keyword("mu-mu+tau-tau+")
        self.add_keyword("4l_DF")
        self.add_keyword("e-e+e-e+")
        self.add_keyword("mu-mu+mu-mu+")
        self.add_keyword("tau-tau+tau-tau+")
        self.add_keyword("4l_SF")
        self.add_keyword("4l_notau_SF")
        self.add_keyword("e-e+nunu_DF")
        self.add_keyword("mu-mu+nunu_DF")
        self.add_keyword("tau-tau+nunu_DF")
        self.add_keyword("2l2nu_DF")
        self.add_keyword("2l2nu_notau_DF")
        self.add_keyword("n_neutrinos_DF")
        self.add_keyword("e-e+nunu_SF")
        self.add_keyword("mu-mu+nunu_SF")
        self.add_keyword("tau-tau+nunu_SF")
        self.add_keyword("2l2nu_SF")
        self.add_keyword("2l2nu_notau_SF")
        self.add_keyword("4q_DF")
        self.add_keyword("4q_SF")
        self.add_keyword("2l2q")
        self.add_keyword("2q2nu")
        self.add_keyword("massive_leptons", 0)
        self.add_keyword("e_mass", 0.000511)
        self.add_keyword("mu_mass", 0.1057)
        self.add_keyword("tau_mass", 1.777)
        self.add_keyword("c_mass", 1.40)
        self.add_keyword("b_mass", 4.92)
        self.add_keyword("bornktmin", 0.26)
        self.add_keyword("mll_min", 60)
        self.add_keyword("mll_max", 120)
        self.add_keyword("m4lcut", 4)
        self.add_keyword("use-old-grid", 1)
        self.add_keyword("ncall1", 30000)
        self.add_keyword("itmx1", 1)
        self.add_keyword("ncall2", 30000)
        self.add_keyword("itmx2", 1)
        self.add_keyword("foldcsi", 1)
        self.add_keyword("foldy", 1)
        self.add_keyword("foldphi", 1)
        self.add_keyword("testplots", 0)
        self.add_keyword("use-old-ubound", 1)
        self.add_keyword("nubound", 50000)
        self.add_keyword("storemintupb", 1)
        self.add_keyword("xupbound", 2)
        self.add_keyword("mintupbratlim", 1000)
        self.add_keyword("ubexcess_correct", 1)
        #  self.add_keyword("btildeviol")
        #  self.add_keyword("corr_btilde")
        #  self.add_keyword("corr_remnant")
        self.add_keyword("colltest", 0)
        self.add_keyword("softtest", 0)
        self.add_keyword("withdamp", 1)
        self.add_keyword("smartsig", 1)
        self.add_keyword("fastbtlbound", 1)
        self.add_keyword("check_bad_st1")
        self.add_keyword("check_bad_st2")
        self.add_keyword("manyseeds")
        self.add_keyword("parallelstage")
        self.add_keyword("maxseeds")
        self.add_keyword("xgriditeration")
        
    def validate_decays(self):
        """! Validate the various decay mode keywords."""
        self.expose()  # convenience call to simplify syntax
        self.check_decay_mode(self.decay_mode, self.allowed_decay_modes)
        # Enable appropriate decay mode
        list(self.parameters_by_keyword("e-e+mu-mu+"))[0].value = 0 
        __decay_mode_lookup = { 
                            "z z > l- l+ l'- l'+" : "4l_DF",
                            "z z > e- e+ mu- mu+":                          "e-e+mu-mu+",
                            "z z > e- e+ tau- tau+":                        "e-e+tau-tau+",
                            "z z > mu- mu+ tau- tau+":                      "mu-mu+tau-tau+",
                            "z z > e- e+ e- e+":                            "e-e+e-e+",
                            "z z > mu- mu+ mu- mu+":                        "mu-mu+mu-mu+",
                            "z z > tau- tau+ tau- tau+":                    "tau-tau+tau-tau+",
                            "z z > l- l+ l- l+":                            "4l_SF",
                            "z z > e- e+ e- e+ / mu- mu+ mu- mu+":          "4l_notau_SF",
                            "z z > e- e+ vl vl~":                           "e-e+nunu_DF",
                            "z z > mu- mu+ vl vl~":                         "mu-mu+nunu_DF",
                            "z z > tau- tau+ vl vl~":                       "tau-tau+nunu_DF",
                            "z z > l- l+ vl vl~":                           "2l2nu_DF",
                            "z z > e- e+ vl vl~ / mu- mu+ vl vl~":          "2l2nu_notau_DF",
                            "z z (+ w w) > e- e+ vl vl~":                   "e-e+nunu_SF",
                            "z z (+ w w) > mu- mu+ vl vl~":                 "mu-mu+nunu_SF",
                            "z z (+ w w) > tau- tau+ vl vl~":               "tau-tau+nunu_SF",
                            "z z (+ w w) > l- l+ vl vl~":                   "2l2nu_SF",
                            "z z (+ w w) > e- e+ vl vl~ / mu- mu+ vl vl~":  "2l2nu_notau_SF",
                            "z z > j j j' j'":                              "4q_DF",
                            "z z > j j j j":                                "4q_SF",
                            "z z > l- l+ j j":                              "2l2q",
                            "z z > j j vl vl~":                             "2q2nu"}
        
        list(self.parameters_by_keyword(__decay_mode_lookup[self.decay_mode]))[0].value = 1
