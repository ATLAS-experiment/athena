# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import Logging
from ..powheg_RES import PowhegRES

## Get handle to Athena logging
logger = Logging.logging.getLogger("PowhegControl")


class Zgamj_MiNNLO(PowhegRES):
    """! Default Powheg configuration for Z gamma production plus one jet using MiNNLOPS.

    Create a configurable object with all applicable Powheg options.

    @author Aonan Wang <aonan.wang@cern.ch>
    """

    def __init__(self, base_directory, **kwargs):
        """! Constructor: all process options are set here.

        @param base_directory: path to PowhegBox code.
        @param kwargs          dictionary of arguments from Generate_tf.
        """
        errors = super(Zgamj_MiNNLO, self).openloops_error()
        warnings = super(Zgamj_MiNNLO, self).hoppet_warning()
        infos = super(Zgamj_MiNNLO, self).hoppet_info()
        infos.append("qqvvamp: increasing precision to")
        super(Zgamj_MiNNLO, self).__init__(base_directory, "ZgamJ",warning_output=warnings, info_output=infos, error_output=errors, **kwargs)

        # Add parameter validation functions
        self.validation_functions.append("validate_decays")

        # Add flag for the MiNNLO reweight
        self.reweight_for_MiNNLO = True

        ## List of allowed decay modes
        self.allowed_decay_modes = ["z > e+ e-",
                                    "z > mu+ mu-",
                                    "z > tau+ tau-",
                                    "z > e+ e- / mu+ mu-",
                                    "z > l+ l-",
                                    "z > vl vl~"]

        # Add all keywords for this process, overriding defaults if required
        self.add_keyword("ih1")
        self.add_keyword("ih2")
        self.add_keyword("lhans1", self.default_PDFs_nnlo)
        self.add_keyword("lhans2", self.default_PDFs_nnlo)
        self.add_keyword("alphas_from_pdf", 1)
        self.add_keyword("fixedscale", 0) # if 0 use dynamical scale below (set by whichscale), if 1 scale is fixed to Z-boson mass (leave this to 0 when MiN(N)LO is used)
        self.add_keyword("whichscale", 1) # only used if fixedscale=0, 0 = sqrt(M_Z^2+pt_gamma^2), 1 = M_llgamma, (leave this to 1 when MiN(N)LO is used)
        self.add_keyword("minlo", 1)
        self.add_keyword("minnlo", 1)
        self.add_keyword("modlog_p", -1)
        self.add_keyword("Q0", 0)
        self.add_keyword("largeptscales", 1)
        self.add_keyword("kappaQ", 1)
        self.add_keyword("smartMiNLO", 1)
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
        self.add_keyword("zmass")
        self.add_keyword("wmass")
        self.add_keyword("tmass")
        self.add_keyword("zwidth")
        self.add_keyword("wwidth")
        self.add_keyword("twidth")
        self.add_keyword("vdecaymode", "w+ w- > l+ vl l'- vl'~", name="decay_mode", hidden=False)
        self.add_keyword("e+e-")
        self.add_keyword("mu+mu-")
        self.add_keyword("tau+tau-")
        self.add_keyword("leptonic_notau")
        self.add_keyword("leptonic")
        self.add_keyword("sum_over_families")
        self.add_keyword("massive_leptons")
        self.add_keyword("e_mass")
        self.add_keyword("mu_mass")
        self.add_keyword("tau_mass")
        self.add_keyword("useOL")
        self.add_keyword("OL_CMS")
        self.add_keyword("OL_nf")
        self.add_keyword("OL_onshellphoton")
        self.add_keyword("anomcoup")
        self.add_keyword("hZ1")
        self.add_keyword("hZ2")
        self.add_keyword("hZ3")
        self.add_keyword("hZ4")
        self.add_keyword("hg1")
        self.add_keyword("hg2")
        self.add_keyword("hg3")
        self.add_keyword("hg4")
        self.add_keyword("anommode")
        self.add_keyword("pt_j1_cut")
        self.add_keyword("pt_a_cut")
        self.add_keyword("m_lepg_cut")
        self.add_keyword("invmass_min")
        self.add_keyword("smooth_dyn")
        self.add_keyword("smooth_eps")
        self.add_keyword("smooth_pt0")
        self.add_keyword("smooth_R")
        self.add_keyword("smooth_n")
        self.add_keyword("withdamp", 1)
        self.add_keyword("suppmodel")
        self.add_keyword("bornsuppfact", 1)
        self.add_keyword("remnsuppfact")
        self.add_keyword("ptj_suppfact")
        self.add_keyword("powj_suppfact")
        self.add_keyword("pta_suppfact")
        self.add_keyword("powa_suppfact")
        self.add_keyword("ptnunu_suppfact")
        self.add_keyword("DRal_suppfact")
        self.add_keyword("DRj_suppfact")
        self.add_keyword("powdr_suppfact")
        self.add_keyword("use-old-grid")
        self.add_keyword("ncall1", 500000)
        self.add_keyword("itmx1", 1)
        self.add_keyword("ncall2", 500000)
        self.add_keyword("itmx2",1)
        self.add_keyword("foldcsi")
        self.add_keyword("foldy")
        self.add_keyword("foldphi")
        self.add_keyword("testplots")
        self.add_keyword("use-old-ubound")
        self.add_keyword("nubound", 50000)
        self.add_keyword("storemintupb")
        self.add_keyword("xupbound", 2)
        self.add_keyword("mintupbratlim", 1000)
        self.add_keyword("ubexcess_correct", 1)
        self.add_keyword("iymax", 3)
        self.add_keyword("icsimax", 3)
        #  self.add_keyword("btildeviol")
        #  self.add_keyword("corr_btilde")
        #  self.add_keyword("corr_remnant")
        self.add_keyword("colltest", 0)
        self.add_keyword("softtest", 0)
        self.add_keyword("smartsig")
        self.add_keyword("fastbtlbound")
        self.add_keyword("check_bad_st1")
        self.add_keyword("check_bad_st2")
        self.add_keyword("manyseeds")
        self.add_keyword("parallelstage")
        self.add_keyword("xgriditeration")
        self.add_keyword("maxseeds")

    def validate_decays(self):
        """! Validate the various decay mode keywords."""
        self.expose()  # convenience call to simplify syntax
        self.check_decay_mode(self.decay_mode, self.allowed_decay_modes)
        # Enable appropriate decay mode
        list(self.parameters_by_keyword("vdecaymode"))[0].value = 0
        __decay_mode_lookup = { "z > e+ e-": "e+e-",
                                "z > mu+ mu-": "mu+mu-",
                                "z > tau+ tau-": "tau+tau-",
                                "z > e+ e- / mu+ mu-": "leptonic_notau",
                                "z > l+ l-": "leptonic",
                                "z > vl vl~": "vv"}
        if __decay_mode_lookup[self.decay_mode] == "vv":
            list(self.parameters_by_keyword("vdecaymode"))[0].value = 0
        else:
            list(self.parameters_by_keyword("vdecaymode"))[0].value = 1
            list(self.parameters_by_keyword(__decay_mode_lookup[self.decay_mode]))[0].value = 1
            list(self.parameters_by_keyword("anomcoup"))[0].value = 0


