# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import Logging
from ..powheg_RES import PowhegRES

## Get handle to Athena logging
logger = Logging.logging.getLogger("PowhegControl")


class WZj_MiNNLO(PowhegRES):
    """! Default Powheg configuration for W-boson Z-boson pair production including interference for identical leptons.

    Create a configurable object with all applicable Powheg options.

    @author Aonan Wang <aonan.wang@cern.ch>
    """

    def __init__(self, base_directory, **kwargs):
        """! Constructor: all process options are set here.

        @param base_directory: path to PowhegBox code.
        @param kwargs          dictionary of arguments from Generate_tf.
        """
        super(WZj_MiNNLO, self).__init__(base_directory, "WZJ", **kwargs)

        # Add parameter validation functions
        self.validation_functions.append("validate_decays")

        # Add flag for the MiNNLO reweight
        self.reweight_for_MiNNLO = True

        ## List of allowed decay modes
        self.allowed_decay_modes = [
                                    "w- z > e- ve~ e+ e-", "w- z > e- ve~ mu+ mu-", "w- z > e- ve~ tau+ tau-",
                                    "w- z > mu- vm~ e+ e-", "w- z > mu- vm~ mu+ mu-", "w- z > mu- vm~ tau+ tau-", 
                                    "w- z > tau- vt~ e+ e-", "w- z > tau- vt~ mu+ mu-", "w- z > tau- vt~ tau+ tau-", 
                                    "w+ z > e+ ve e+ e-", "w+ z > e+ ve mu+ mu-", "w+ z > e+ ve tau+ tau-",
                                    "w+ z > mu+ vm e+ e-", "w+ z > mu+ vm mu+ mu-", "w+ z > mu+ vm tau+ tau-", 
                                    "w+ z > tau+ vt e+ e-", "w+ z > tau+ vt mu+ mu-", "w+ z > tau+ vt tau+ tau-"]
                                    

        # Add all keywords for this process, overriding defaults if required
        self.add_keyword("ih1")
        self.add_keyword("ih2")
        self.add_keyword("lhans1", self.default_PDFs_nnlo)
        self.add_keyword("lhans2", self.default_PDFs_nnlo)
        self.add_keyword("alphas_from_pdf", 1)
        self.add_keyword("runningscales") # 0 = fixed scale 2m(W), 1=m(WW), 2=mT(W+) + mT(W-)
        self.add_keyword("minlo", 1)
        self.add_keyword("minnlo", 1)
        self.add_keyword("modlog_p", -1)
        self.add_keyword("Q0", 0)
        self.add_keyword("largeptscales", 1)
        self.add_keyword("smartMiNLO", 1)
        self.add_keyword("run_mode", 4)
        self.add_keyword("rwl_group_events", 1)
        self.add_keyword("renscfact", self.default_scales[1])
        self.add_keyword("facscfact", self.default_scales[0])
        self.add_keyword("storeinfo_rwgt", 1)
        self.add_keyword("rwl_file")
        self.add_keyword("rwl_add")
        self.add_keyword("rwl_format_rwgt")
        self.add_keyword("idvecbosW")
        self.add_keyword("Wdecaymode", 1, name="decay_mode", hidden=False)
        self.add_keyword("Zdecaymode")
        self.add_keyword("ewscheme")
        self.add_keyword("gfermi")
        self.add_keyword("hmass")
        self.add_keyword("zmass")
        self.add_keyword("wmass")
        self.add_keyword("tmass")
        self.add_keyword("bmass", 0)
        self.add_keyword("zwidth")
        self.add_keyword("wwidth")
        self.add_keyword("twidth")
        self.add_keyword("hwidth")
        self.add_keyword("massive_leptons")
        self.add_keyword("e_mass")
        self.add_keyword("mu_mass")
        self.add_keyword("tau_mass")
        self.add_keyword("c_mass")
        self.add_keyword("b_mass")
        self.add_keyword("bornktmin")
        self.add_keyword("mllZ_min")
        self.add_keyword("mllZ_max")
        self.add_keyword("mlvW_min")
        self.add_keyword("mlvW_max")
        self.add_keyword("use-old-grid")
        self.add_keyword("ncall1", 30000)
        self.add_keyword("itmx1", 1)
        self.add_keyword("ncall2", 30000)
        self.add_keyword("itmx2",1)
        self.add_keyword("foldcsi")
        self.add_keyword("foldy")
        self.add_keyword("foldphi")
        self.add_keyword("testplots")
        self.add_keyword("withnegweights", 1)
        self.add_keyword("use-old-ubound")
        self.add_keyword("nubound", 50000)
        self.add_keyword("storemintupb")
        self.add_keyword("xupbound", 2)
        self.add_keyword("mintupbratlim")
        self.add_keyword("ubexcess_correct")
        self.add_keyword("compress_upb", 1)
        #  self.add_keyword("btildeviol")
        #  self.add_keyword("corr_btilde")
        #  self.add_keyword("corr_remnant")
        self.add_keyword("colltest", 0)
        self.add_keyword("softtest", 0)
        self.add_keyword("withdamp", 0)
        self.add_keyword("bornsuppfact", 0)
        self.add_keyword("bornsuppfactV")
        self.add_keyword("bornzerodamp")
        self.add_keyword("smartsig")
        self.add_keyword("fastbtlbound")
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
        list(self.parameters_by_keyword("Wdecaymode"))[0].value = 1  # disable the one used as a proxy for decay mode
        __decay_mode_lookup = {
                               "w- z > e- ve~ e+ e-": "WmZevee",
                               "w- z > e- ve~ mu+ mu-": "WmZevmumu",
                               "w- z > e- ve~ tau+ tau-": "WmZevtautau",
                               "w- z > mu- vm~ e+ e-": "WmZmuvee",
                               "w- z > mu- vm~ mu+ mu-": "WmZmuvmumu",
                               "w- z > mu- vm~ tau+ tau-": "WmZmuvtautau",
                               "w- z > tau- vt~ e+ e-": "WmZtauvee",
                               "w- z > tau- vt~ mu+ mu-": "WmZtauvmumu",
                               "w- z > tau- vt~ tau+ tau-": "WmZtauvtautau",
                               "w+ z > e+ ve e+ e-": "WpZevee",
                               "w+ z > e+ ve mu+ mu-": "WpZevmumu",
                               "w+ z > e+ ve tau+ tau-": "WpZevtautau",
                               "w+ z > mu+ vm e+ e-": "WpZmuvee",
                               "w+ z > mu+ vm mu+ mu-": "WpZmuvmumu",
                               "w+ z > mu+ vm tau+ tau-": "WpZmuvtautau",
                               "w+ z > tau+ vt e+ e-": "WpZtauvee",
                               "w+ z > tau+ vt mu+ mu-": "WpZtauvmumu",
                               "w+ z > tau+ vt tau+ tau-": "WpZtauvtautau"}
        if "Wp" in __decay_mode_lookup[self.decay_mode]:
            list(self.parameters_by_keyword("idvecbosW"))[0].value = 24
        else:
            list(self.parameters_by_keyword("idvecbosW"))[0].value = -24

        if "ev" in __decay_mode_lookup[self.decay_mode]:
            list(self.parameters_by_keyword("Wdecaymode"))[0].value = 1
        elif "muv" in __decay_mode_lookup[self.decay_mode]:
            list(self.parameters_by_keyword("Wdecaymode"))[0].value = 2
        else:
            list(self.parameters_by_keyword("Wdecaymode"))[0].value = 3

        if "ee" in __decay_mode_lookup[self.decay_mode]:
            list(self.parameters_by_keyword("Zdecaymode"))[0].value = 1
        elif "mumu" in __decay_mode_lookup[self.decay_mode]:
            list(self.parameters_by_keyword("Zdecaymode"))[0].value = 2
        else:
            list(self.parameters_by_keyword("Zdecaymode"))[0].value = 3
