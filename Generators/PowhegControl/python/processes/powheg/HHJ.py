# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from ... import Logging
from ..powheg_RES import PowhegRES

## Get handle to Athena logging
logger = Logging.logging.getLogger("PowhegControl")


class HHJ(PowhegRES):
    """! Default Powheg configuration for Higgs boson pair production plus one jet using MiNNLOPS.

    Create a configurable object with all applicable Powheg options.

    @author Katharina Voss  <katharina.voss@cern.ch>
    """

    def __init__(self, base_directory, **kwargs):
        """! Constructor: all process options are set here.

        @param base_directory: path to PowhegBox code.
        @param kwargs          dictionary of arguments from Generate_tf.
        """
        warnings = super(HHJ, self).hoppet_warning()
        infos = super(HHJ, self).hoppet_info()
        super(HHJ, self).__init__(base_directory, "HHJ", warning_output=warnings, info_output=infos, **kwargs)

        # Add flag for the MiNNLO reweight
        self.reweight_for_MiNNLO = True

        # Add all keywords for this process, overriding defaults if required
        self.add_keyword("ih1")
        self.add_keyword("ih2")
        self.add_keyword("lhans1", self.default_PDFs)
        self.add_keyword("lhans2", self.default_PDFs)
        self.add_keyword("alphas_from_pdf", 1)  # required for running MiNNLO
        self.add_keyword("doublefsr")  # defaults to true internally; recommended for V+J-type processes

        # Scale settings (leave fixedscale=0 and whichscale=2 when MiN(N)LO is used)
        self.add_keyword("fixedscale", 0)
        self.add_keyword("whichscale", 2, description="HHJ: dynamical scale choice if fixedscale=0 [0: sqrt(M_HH^2+pt_HH^2), 1: M_HH, 2: M_HH/2]")
        self.add_keyword("renscfact", self.default_scales[1])
        self.add_keyword("facscfact", self.default_scales[0])

        # MiNLO'/MiNNLO_PS settings
        self.add_keyword("minlo", 1)
        self.add_keyword("minnlo", 1)
        self.add_keyword("modlog_p", -1.0)
        self.add_keyword("kappaQ", 0.5)
        self.add_keyword("Q0", 0)
        self.add_keyword("largeptscales", 1, description="HHJ: scale used at large pT in the fixed-order part [0: muR=muF~Q, 1: muR=muF=pT]")

        # Optimization options for MiNLO/MiNNLO_PS
        self.add_keyword("smartMiNLO", 1)
        self.add_keyword("run_mode", 1)
        self.add_keyword("rwl_group_events", 1)

        # Scale variation & reweighting
        self.add_keyword("storeinfo_rwgt", 1)
        self.add_keyword("rwl_file")
        self.add_keyword("rwl_add")
        self.add_keyword("rwl_format_rwgt")

        # Process parameters
        self.add_keyword("mt-approx", 1, description="HHJ: top-mass treatment [0: full theory (unavailable), 1: HEFT, 2-4: FT-approx0/1/2, 5: Born-improved]")
        self.add_keyword("kl", 1.0)  # modifier of the trilinear Higgs coupling
        self.add_keyword("ewscheme", 1)  # 1 = complex Gmu scheme, 2 = alpha scheme
        self.add_keyword("gfermi")
        self.add_keyword("alpha")  # only used if ewscheme=2
        self.add_keyword("complexGFermi")
        self.add_keyword("zmass")
        self.add_keyword("wmass")
        self.add_keyword("hmass")
        self.add_keyword("tmass")
        self.add_keyword("tmass_phsp")  # top mass used in phase-space generation only; defaults to tmass
        self.add_keyword("bmass", 0.0, description="b-quark mass in GeV; must stay 0 since HHJ uses a 5-flavour massless-b computation")
        self.add_keyword("zwidth")
        self.add_keyword("wwidth")
        self.add_keyword("twidth")
        self.add_keyword("hwidth")

        # Quark masses used unconditionally for momentum reshuffling at event generation level
        # (c and b are active flavours in the HHJ Born/real flavour structures; unlike the lepton
        # reshuffling masses above, these are NOT gated by a massive_leptons-style flag)
        self.add_keyword("c_mass", description="charm mass for momentum reshuffling at event generation level")
        self.add_keyword("b_mass", description="bottom mass for momentum reshuffling at event generation level")

        # Generation cuts
        self.add_keyword("bornktmin", 0.26)

        # Monte Carlo integration (defaults match the process's own suggested-minnlo-run card)
        self.add_keyword("use-old-grid", 1)
        self.add_keyword("ncall1", 500000)
        self.add_keyword("itmx1", 1)
        self.add_keyword("ncall2", 500000)
        self.add_keyword("itmx2", 1)
        self.add_keyword("foldcsi")
        self.add_keyword("foldy")
        self.add_keyword("foldphi")
        self.add_keyword("testplots")

        # Upper bound evaluation
        self.add_keyword("use-old-ubound", 1)
        self.add_keyword("nubound", 30000)
        self.add_keyword("storemintupb", 1)
        self.add_keyword("xupbound", 2.0)
        self.add_keyword("mintupbratlim", 1.0e3)
        self.add_keyword("ubexcess_correct", 1)

        # Instability / stability checks
        self.add_keyword("colltest")
        self.add_keyword("softtest")
        self.add_keyword("withdamp", 1)
        self.add_keyword("hdamp", -1)

        # Optimization options
        self.add_keyword("smartsig", 1)
        self.add_keyword("fastbtlbound", 1)

        # Options to catch instabilities
        self.add_keyword("check_bad_st1", 1)
        self.add_keyword("check_bad_st2", 1)

        # Parallel runs
        self.add_keyword("manyseeds")
        self.add_keyword("parallelstage")
        self.add_keyword("xgriditeration")
        self.add_keyword("maxseeds")
