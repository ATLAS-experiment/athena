# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import Logging
from ..powheg_V2 import PowhegV2
from ..external import ExternalMadSpin

## Get handle to Athena logging
logger = Logging.logging.getLogger("PowhegControl")



# Dictionary to convert the PowhegControl decay mode names to the appropriate
# decay mode numbers understood by Powheg
_decay_mode_lookup = {
    "t t~ z > all [MadSpin]" : "00000", # switch off decays in Powheg and let MadSpin handle them!
    't t~ z > all' : '22222',
    't t~ z > b j j b~ j j emu+ emu-' : '00022',
    't t~ z > b l+ vl b~ l- vl~ emu+ emu-' : '22200',
    "t t~ z > b emu+ vemu b~ emu- vemu~ emu+ emu-": "22000",
    't t~ z > semileptonic + emu+ emu-': '11111', # additionally need to set the "semileptonic" parameter
    't t~ z > undecayed' : '00000',
}



class ttZ(PowhegV2):
    """
    Powheg interface for ttZ production.

    Reference for this process: https://arxiv.org/abs/2112.08892

    @author Timothee Theveneaux-Pelzer  <tpelzer@cern.ch>
    """

    def __init__(self, base_directory, **kwargs):
        """! Constructor: all process options are set here.

        @param base_directory: path to PowhegBox code.
        @param kwargs          dictionary of arguments from Gen_tf.
        """
        super(ttZ, self).__init__(base_directory, "ttZ", **kwargs)

        # List of allowed decay modes
        # (The sorting of the list is just to increase readability when it's printed)
        self.allowed_decay_modes = sorted(_decay_mode_lookup.keys())

        # Add algorithms to the sequence
        self.add_algorithm(ExternalMadSpin(process="generate p p > t t~ e+ e- [QCD]\nadd process p p > t t~ mu+ mu- [QCD]"))

        self.validation_functions.append("validate_decays")
        self.validation_functions.append("get_nlox_params_file")

        # Add all keywords for this process, overriding defaults if required
        self.add_keyword("bornonly")
        self.add_keyword("elbranching")
        self.add_keyword("btlscalect", 1)
        self.add_keyword("btlscalereal", 1)
        self.add_keyword("bmass")
        self.add_keyword("CKM_Vcd")
        self.add_keyword("CKM_Vcs")
        self.add_keyword("CKM_Vud")
        self.add_keyword("CKM_Vus")
        self.add_keyword("clobberlhe")
        self.add_keyword("colltest")
        self.add_keyword("compress_lhe")
        self.add_keyword("compress_upb")
        self.add_keyword("compute_rwgt")
        self.add_keyword("delta_mttmin")
        self.add_keyword("dynamic_hdamp")
        self.add_keyword("facscfact", self.default_scales[0])
        self.add_keyword("fakevirt")
        self.add_keyword("fastbtlbound")
        self.add_keyword("foldcsi", 2)
        self.add_keyword("foldphi", 2)
        self.add_keyword("foldy", 2)
        self.add_keyword("for_reweighting")
        self.add_keyword("gfermi")
        self.add_keyword("hbzd")
        self.add_keyword("hdamp")
        self.add_keyword("hmass")
        self.add_keyword("hwidth")
        self.add_keyword("icsimax", 3)
        self.add_keyword("ih1")
        self.add_keyword("ih2")
        self.add_keyword("itmx1", 5)
        self.add_keyword("itmx2", 5)
        self.add_keyword("iymax", 3)
        self.add_keyword("lhans1", self.default_PDFs)
        self.add_keyword("lhans2", self.default_PDFs)
        self.add_keyword("lhfm/bmass")
        self.add_keyword("lhfm/cmass")
        self.add_keyword("lhfm/emass")
        self.add_keyword("lhfm/mumass")
        self.add_keyword("lhfm/smass")
        self.add_keyword("lhfm/taumass")
        self.add_keyword("lhfm/umass")
        self.add_keyword("lhrwgt_descr")
        self.add_keyword("lhrwgt_group_combine")
        self.add_keyword("lhrwgt_group_name")
        self.add_keyword("lhrwgt_id")
        self.add_keyword("LOevents")
        self.add_keyword("manyseeds")
        self.add_keyword("maxseeds", 1000)
        self.add_keyword("ncall1", 100000)
        self.add_keyword("ncall2", 100000)
        self.add_keyword("ncall2rm")
        self.add_keyword("nubound", 100000)
        self.add_keyword("parallelstage")
        self.add_keyword("renscfact", self.default_scales[1])
        self.add_keyword("runningscales", 0) # 0: no running scales; 1: use running scales
        self.add_keyword("rwl_add")
        self.add_keyword("rwl_file")
        self.add_keyword("rwl_format_rwgt")
        self.add_keyword("rwl_group_events")
        self.add_keyword("semileptonic")
        self.add_keyword("storeinfo_rwgt", 1)
        self.add_keyword("storemintupb")
        self.add_keyword("tmass")
        self.add_keyword("topdecaymode", "t t~ z > all", name="decay_mode")
        self.add_keyword("twidth")
        self.add_keyword("use-old-grid", 1)
        self.add_keyword("use-old-ubound", 1)
        self.add_keyword("withdamp", 1)
        self.add_keyword("wmass")
        self.add_keyword("wwidth")
        self.add_keyword("xgriditeration")
        self.add_keyword("xupbound", 2)
        self.add_keyword("zerowidth")
        self.add_keyword("zmass")
        self.add_keyword("zwidth")

    def get_nlox_params_file(self):
        """
        Retrieve nlox parameters file, copy it into the local directory, and edit some of the parameters
        """
        import os
        import shutil

        # First attempt: look in testrun under executable directory
        original_path = os.path.join(os.path.dirname(self.executable), 'test', 'nlox_parameters.par')

        # Fallback: try one directory above executable
        fallback_path = os.path.join(os.path.dirname(os.path.dirname(self.executable)), 'test', 'nlox_parameters.par')

        # Determine which file exists
        if os.path.exists(original_path):
            source_file = original_path
        elif os.path.exists(fallback_path):
            source_file = fallback_path
        else:
            raise FileNotFoundError("No valid nlox_parameters.par found in expected locations.")

        # Copy the file
        shutil.copyfile(source_file, './nlox_parameters.par-old')

        # edit some of the parameters
        lines = []
        with open('./nlox_parameters.par-old') as f:
            lines = f.readlines()
        update_dict = {
            '# mb =':'mb = {}'.format(self.parameters_by_keyword('bmass')[0].value),
            '# mt =':'mt = {}'.format(self.parameters_by_keyword('tmass')[0].value),
            '# wt =':'wt = {}'.format(self.parameters_by_keyword('twidth')[0].value),
            '# mW =':'mW = {}'.format(self.parameters_by_keyword('wmass')[0].value),
            '# wW =':'wW = {}'.format(self.parameters_by_keyword('wwidth')[0].value),
            '# mZ =':'mZ = {}'.format(self.parameters_by_keyword('zmass')[0].value),
            '# wZ =':'wZ = {}'.format(self.parameters_by_keyword('zwidth')[0].value),
            '# mH =':'mH = {}'.format(self.parameters_by_keyword('hmass')[0].value),
            '# wH =':'wH = {}'.format(self.parameters_by_keyword('hwidth')[0].value),
            '# GF =':'GF = {}'.format(self.parameters_by_keyword('gfermi')[0].value),
            }
        with open('./nlox_parameters.par','w') as f:
            for line in lines:
                for pattern, replacement in update_dict.items():
                    if pattern in line:
                        line = replacement+'\n'
                        break
                f.write(line)
        logger.info('New file ./nlox_parameters.par created locally with updated parameters')

    def validate_decays(self):
        """
        Validate decay_mode keywords and translate them from ATLAS input to Powheg input
        """
        self.expose() # convenience call to simplify syntax
        if self.decay_mode not in self.allowed_decay_modes:
            error_message = "Decay mode '{given}' not recognised, valid choices are: '{choices}'!".format(given=self.decay_mode, choices="', '".join(self.allowed_decay_modes))
            logger.warning(error_message)
            raise ValueError(error_message)

        # Check if MadSpin decays are requested.
        # Accordingly, MadSpin will run or not run.
        if "MadSpin" in self.decay_mode:
            self.externals["MadSpin"].parameters_by_keyword("powheg_top_decays_enabled")[0].value = False
            self.externals["MadSpin"].parameters_by_keyword("MadSpin_model")[0].value = "loop_sm-no_b_mass"
            self.externals["MadSpin"].parameters_by_keyword("MadSpin_nFlavours")[0].value = 5

        self.parameters_by_keyword("topdecaymode")[0].value = _decay_mode_lookup[self.decay_mode]
        if self.decay_mode == 't t~ z > semileptonic emu+ emu-':
            # Parameter semileptonic must be set to 1
            self.parameters_by_keyword("semileptonic")[0].value = 1
