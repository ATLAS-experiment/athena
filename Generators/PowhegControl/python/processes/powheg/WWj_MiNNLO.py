# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import Logging
from ..powheg_RES import PowhegRES

## Get handle to Athena logging
logger = Logging.logging.getLogger("PowhegControl")


class WWj_MiNNLO(PowhegRES):
    """! Default Powheg configuration for W-boson pair production plus one jet using MiNNLOPS.

    Create a configurable object with all applicable Powheg options.

    @author Aonan Wang <aonan.wang@cern.ch>
    """

    def __init__(self, base_directory, **kwargs):
        """! Constructor: all process options are set here.

        @param base_directory: path to PowhegBox code.
        @param kwargs          dictionary of arguments from Generate_tf.
        """
        errors = super(WWj_MiNNLO, self).openloops_error()
        warnings = super(WWj_MiNNLO, self).hoppet_warning()
        infos = super(WWj_MiNNLO, self).hoppet_info()
        infos.append("qqvvamp: increasing precision to")
        super(WWj_MiNNLO, self).__init__(base_directory, "WWJ",warning_output=warnings, info_output=infos, error_output=errors, **kwargs)

        # Add parameter validation functions
        self.validation_functions.append("validate_decays")

        # Add flag for the MiNNLO reweight
        self.reweight_for_MiNNLO = True

        ## List of allowed decay modes
        self.allowed_decay_modes = ["w+ w- > e+ ve e- ve~",
                                    "w+ w- > mu+ vm mu- vm~",
                                    "w+ w- > tau+ vt tau- vt~",
                                    "w+ w- > e+ ve mu- vm~ / mu+ vm e- ve~",
                                    "w+ w- > l+ vl l'- vl'~",
                                    "w+ w- > l+ vl j j / j j l- vl~",
                                    "w+ w- > e+ ve j j / j j e- ve~ / mu+ vmu j j / j j mu- vmu~",
                                    "w+ w- > j j j j",
                                    "w+ w- > e+ ve mu- vm~",
                                    "w+ w- > mu+ vm e- ve~"]

        # Add all keywords for this process, overriding defaults if required
        self.add_keyword("ih1")
        self.add_keyword("ih2")
        self.add_keyword("lhans1", self.default_PDFs_nnlo_nf_4)
        self.add_keyword("lhans2", self.default_PDFs_nnlo_nf_4)
        self.add_keyword("alphas_from_pdf", 1)
        self.add_keyword("runningscales") # 0 = fixed scale 2m(W), 1=m(WW), 2=mT(W+) + mT(W-)
        self.add_keyword("minlo", 1)
        self.add_keyword("minnlo", 1)
        self.add_keyword("modlog_p", 6)
        self.add_keyword("Q0", 0)
        self.add_keyword("largeptscales", 1)
        self.add_keyword("smartMiNLO", 1)
        self.add_keyword("use_interpolator", 1)
        self.add_keyword("run_mode")
        self.add_keyword("rwl_group_events", 1)
        self.add_keyword("renscfact", self.default_scales[1])
        self.add_keyword("facscfact", self.default_scales[0])
        self.add_keyword("storeinfo_rwgt", 1)
        self.add_keyword("rwl_file")
        self.add_keyword("rwl_add")
        self.add_keyword("rwl_format_rwgt")
        self.add_keyword("clobberlhe")
        self.add_keyword("ewscheme")
        self.add_keyword("gfermi")
        self.add_keyword("hmass")
        self.add_keyword("zmass")
        self.add_keyword("wmass")
        self.add_keyword("tmass")
        self.add_keyword("bmass")
        self.add_keyword("zwidth")
        self.add_keyword("wwidth")
        self.add_keyword("twidth")
        self.add_keyword("hwidth")
        self.add_keyword("e+e-")
        self.add_keyword("mu+mu-")
        self.add_keyword("tau+tau-")
        self.add_keyword("e+mu-")
        self.add_keyword("mu+e-")
        self.add_keyword("leptonic")
        self.add_keyword("leptonic_notau")
        self.add_keyword("hadronic")
        self.add_keyword("semileptonic", "w+ w- > l+ vl l'- vl'~", name="decay_mode", hidden=False)
        self.add_keyword("semileptonic_notau")
        self.add_keyword("bornktmin")
        self.add_keyword("use-old-grid")
        self.add_keyword("ncall1", 30000)
        self.add_keyword("itmx1", 1)
        self.add_keyword("ncall2", 30000)
        self.add_keyword("itmx2",1)
        self.add_keyword("foldcsi")
        self.add_keyword("foldy")
        self.add_keyword("foldphi")
        self.add_keyword("testplots")
        self.add_keyword("use-old-ubound")
        self.add_keyword("nubound", 50000)
        self.add_keyword("storemintupb")
        self.add_keyword("xupbound")
        self.add_keyword("mintupbratlim")
        self.add_keyword("ubexcess_correct")
        self.add_keyword("ptsqmin")
        #  self.add_keyword("btildeviol")
        #  self.add_keyword("corr_btilde")
        #  self.add_keyword("corr_remnant")
        self.add_keyword("colltest")
        self.add_keyword("softtest")
        self.add_keyword("withdamp")
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
        list(self.parameters_by_keyword("semileptonic"))[0].value = 0 # disable the one used as a proxy for decay mode
        __decay_mode_lookup = { "w+ w- > e+ ve e- ve~": "e+e-",
                                "w+ w- > mu+ vm mu- vm~": "mu+mu-",
                                "w+ w- > tau+ vt tau- vt~": "tau+tau-",
                                "w+ w- > e+ ve mu- vm~": "e+mu-",
                                "w+ w- > mu+ vm e- ve~": "mu+e-",
                                "w+ w- > l+ vl l'- vl'~": "leptonic",
                                "w+ w- > e+ ve mu- vm~ / mu+ vm e- ve~": "leptonic_notau",
                                "w+ w- > l+ vl j j / j j l- vl~": "semileptonic",
                                "w+ w- > e+ ve j j / j j e- ve~ / mu+ vmu j j / j j mu- vmu~": "semileptonic_notau",
                                "w+ w- > j j j j": "hadronic"}
        
        list(self.parameters_by_keyword(__decay_mode_lookup[self.decay_mode]))[0].value = 1
