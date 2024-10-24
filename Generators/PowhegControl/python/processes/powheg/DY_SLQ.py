# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import Logging
from ...parameters import powheg_atlas_common
from ..powheg_V2 import PowhegV2

## Get handle to Athena logging
logger = Logging.logging.getLogger("PowhegControl")


class DY_SLQ(PowhegV2):
    """! Default Powheg configuration for Vector LeptoQuark production.
    
    https://arxiv.org/abs/2207.00356
    
    Create a configurable object with all applicable Powheg options.

    @author Timothee Theveneaux-Pelzer  <tpelzer@cern.ch>
    """

    def __init__(self, base_directory, **kwargs):
        """! Constructor: all process options are set here.

        @param base_directory: path to PowhegBox code.
        @param kwargs          dictionary of arguments from Generate_tf.
        """
        super(DY_SLQ, self).__init__(base_directory, "DY_SLQ_NLO", **kwargs)

        # Add parameter validation functions
        self.validation_functions.append("validate_decays")

        ## List of allowed decay modes
        self.allowed_decay_modes = ["ta+ ta-", "e+ e-", "mu+ mu-"]

        # Add all keywords for this process, overriding defaults if required
        self.add_keyword("angcorr_damp") # using angular correlations aware damp function
        self.add_keyword("bornktmin")
        self.add_keyword("bornonly")
        self.add_keyword("bornsuppfact")
        self.add_keyword("bornzerodamp")
        self.add_keyword("bottommass")
        self.add_keyword("bottomthr")
        self.add_keyword("bottomthrpdf")
        self.add_keyword("btildeborn")
        self.add_keyword("btildecoll")
        self.add_keyword("btildereal")
        self.add_keyword("btildevirt")
        self.add_keyword("btlscalect")
        self.add_keyword("btlscalereal")
        self.add_keyword("charmmass")
        self.add_keyword("charmthr")
        self.add_keyword("charmthrpdf")
        self.add_keyword("check_bad_st1")
        self.add_keyword("check_bad_st2")
        self.add_keyword("clobberlhe")
        self.add_keyword("colltest")
        self.add_keyword("compress_lhe")
        self.add_keyword("compress_upb")
        self.add_keyword("compute_rwgt")
        self.add_keyword("doublefsr")
        self.add_keyword("evenmaxrat",0)
        self.add_keyword("ew")
        self.add_keyword("facscfact", self.default_scales[0])
        self.add_keyword("fastbtlbound")
        self.add_keyword("fixedgrid")
        self.add_keyword("flg_debug")
        self.add_keyword("foldcsi")
        self.add_keyword("foldphi")
        self.add_keyword("foldy")
        self.add_keyword("fullrwgt")
        self.add_keyword("fullrwgtmode")
        self.add_keyword("gfermi")
        self.add_keyword("hdamp")
        self.add_keyword("hdecaymode")
        self.add_keyword("hdecaywidth")
        self.add_keyword("hfact", 104.16)
        self.add_keyword("hmass")
        self.add_keyword("hnew_damp")
        self.add_keyword("hwidth")
        self.add_keyword("icsimax")
        self.add_keyword("ih1")
        self.add_keyword("ih2")
        self.add_keyword("itmx1", 2)
        self.add_keyword("itmx1rm")
        self.add_keyword("itmx2", 2)
        self.add_keyword("itmx2rm")
        self.add_keyword("iupperfsr")
        self.add_keyword("iupperisr")
        self.add_keyword("iymax")
        self.add_keyword("lhans1", self.default_PDFs)
        self.add_keyword("lhans2", self.default_PDFs)
        self.add_keyword("lhapdf6maxsets")
        self.add_keyword("lhrwgt_descr")
        self.add_keyword("lhrwgt_group_combine")
        self.add_keyword("lhrwgt_group_name")
        self.add_keyword("lhrwgt_id")
        self.add_keyword("manyseeds")
        self.add_keyword("maxseeds")
        self.add_keyword("mass_low") # lower limit for dilepton mass
        self.add_keyword("mass_high") # upper limit for dilepton mass
        self.add_keyword("mt", powheg_atlas_common.mass.t, name="mass_t", description="top quark mass in GeV")
        self.add_keyword("ncall1", 30000)
        self.add_keyword("ncall1rm")
        self.add_keyword("ncall2", 50000)
        self.add_keyword("ncall2rm")
        self.add_keyword("ncallfrominput")
        self.add_keyword("new_damp") # using new, better default damp function
        self.add_keyword("nubound", 50000)
        self.add_keyword("parallelstage")
        self.add_keyword("pdfreweight")
        self.add_keyword("rand1")
        self.add_keyword("rand2")
        self.add_keyword("renscfact", self.default_scales[1])
        self.add_keyword("runningscale")
        self.add_keyword("rwl_add")
        self.add_keyword("rwl_file")
        self.add_keyword("rwl_format_rwgt")
        self.add_keyword("rwl_group_events")
        self.add_keyword("smartsig")
        self.add_keyword("softtest")
        self.add_keyword("stage2init")
        self.add_keyword("storeinfo_rwgt")
        self.add_keyword("storemintupb")
        self.add_keyword("testplots")
        self.add_keyword("testsuda")
        self.add_keyword("use-old-grid")
        self.add_keyword("use-old-ubound")
        self.add_keyword("vdecaymode", self.allowed_decay_modes[0], name="decay_mode")
        self.add_keyword("withbtilde",1)
        self.add_keyword("withdamp")
        self.add_keyword("withremnants",1)
        self.add_keyword("withnegweights")
        self.add_keyword("withsubtr")
        self.add_keyword("xgriditeration")
        self.add_keyword("xupbound")
        # General Leptoquark (LQ) Parameters
        self.add_keyword("SM", 1) # Include SM contribution
        self.add_keyword("LQ", 0) # Include basic LQ contributions
        self.add_keyword("LQ-EW", 0, "LQ_EW") # Include LQ corrections to photon/Z couplings
        self.add_keyword("LQ-Int", 0, "LQ_Int") # Include the interference between the SM and the LQ contributions
        # Yukawa couplings to down-type quarks (S1t RR)
        self.add_keyword("YSD1x1")
        self.add_keyword("YSD1x2")
        self.add_keyword("YSD1x3")
        self.add_keyword("YSD2x1")
        self.add_keyword("YSD2x2")
        self.add_keyword("YSD2x3")
        self.add_keyword("YSD3x1")
        self.add_keyword("YSD3x2")
        self.add_keyword("YSD3x3")
        self.add_keyword("MSD") # mass
        # Yukawa couplings to up-type quarks (S1 RR)
        self.add_keyword("YSU1x1")
        self.add_keyword("YSU1x2")
        self.add_keyword("YSU1x3")
        self.add_keyword("YSU2x1")
        self.add_keyword("YSU2x2")
        self.add_keyword("YSU2x3")
        self.add_keyword("YSU3x1")
        self.add_keyword("YSU3x2")
        self.add_keyword("YSU3x3")
        self.add_keyword("MSU") # mass
    

    def validate_decays(self):
        """! Validate semileptonic and topdecaymode keywords."""
        self.expose()  # convenience call to simplify syntax
        if self.decay_mode not in self.allowed_decay_modes:
            logger.warning("Decay mode {} not recognised!".format(self.decay_mode))
            raise ValueError("Decay mode {} not recognised!".format(self.decay_mode))
        # Calculate appropriate decay mode numbers
        __decay_mode_lookup = { "ta+ ta-": "3", "e+ e-": "1", "mu+ mu-": "2"}
        self.parameters_by_keyword("vdecaymode")[0].value = __decay_mode_lookup[self.decay_mode]
