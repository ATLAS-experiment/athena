# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
import os
from AthenaCommon import Logging
from ..powheg_V2 import PowhegV2

## Get handle to Athena logging
logger = Logging.logging.getLogger("PowhegControl")


class ggF_HH(PowhegV2):
    """! Default Powheg configuration for gluon-fusion Higgs boson production with quark mass and EW effects.

    Create a configurable object with all applicable Powheg options.

    @author James Robinson  <james.robinson@cern.ch>
    """

    def __init__(self, base_directory, **kwargs):
        """! Constructor: all process options are set here.

        @param base_directory: path to PowhegBox code.
        @param kwargs          dictionary of arguments from Generate_tf.
        """
        super(ggF_HH, self).__init__(base_directory, "ggHH", **kwargs)

        # Add grid file creation function
        self.validation_functions.append("create_grid_file")

        # Add all keywords for this process, overriding defaults if required
        self.add_keyword("alphas_from_lhapdf")
        self.add_keyword("bornktmin")
        self.add_keyword("bornonly")
        self.add_keyword("bornsuppfact")
        self.add_keyword("bornzerodamp")
        self.add_keyword("bottomthr")
        self.add_keyword("bottomthrpdf")
        self.add_keyword("btildeborn")
        self.add_keyword("btildecoll")
        self.add_keyword("btildereal")
        self.add_keyword("btildevirt")
        self.add_keyword("btlscalect")
        self.add_keyword("btlscalereal")
        self.add_keyword("charmthr")
        self.add_keyword("charmthrpdf")
        self.add_keyword("check_bad_st1")
        self.add_keyword("check_bad_st2")
        self.add_keyword("chhh", 1.0)
        self.add_keyword("ct", 1.0)
        self.add_keyword("ctt", 0.)
        self.add_keyword("cggh", 0.)
        self.add_keyword("cgghh", 0.)
        self.add_keyword("clobberlhe")
        self.add_keyword("colltest")
        self.add_keyword("compress_lhe")
        self.add_keyword("compress_upb")
        self.add_keyword("compute_rwgt")
        self.add_keyword("doublefsr")
        self.add_keyword("evenmaxrat")
        self.add_keyword("facscfact", self.default_scales[0])
        self.add_keyword("fastbtlbound")
        self.add_keyword("fixedgrid")
        self.add_keyword("fixedscale", description="Set renormalisation and factorisation scales to 2*m_H")
        self.add_keyword("flg_debug")
        self.add_keyword("foldcsi", 2)
        self.add_keyword("foldphi", 2)
        self.add_keyword("foldy", 5)
        self.add_keyword("fullrwgt")
        self.add_keyword("gfermi")
        self.add_keyword("hdamp")
        self.add_keyword("hdecaymode")
        self.add_keyword("hfact")
        self.add_keyword("hmass", 125.)
        self.add_keyword("icsimax", 2)
        self.add_keyword("ih1")
        self.add_keyword("ih2")
        self.add_keyword("itmx1", 2)
        self.add_keyword("itmx1rm")
        self.add_keyword("itmx2", 4)
        self.add_keyword("itmx2rm")
        self.add_keyword("iupperfsr")
        self.add_keyword("iupperisr")
        self.add_keyword("iymax", 2)
        self.add_keyword("lhans1", self.default_PDFs)
        self.add_keyword("lhans2", self.default_PDFs)
        self.add_keyword("lhapdf6maxsets")
        self.add_keyword("lhrwgt_descr")
        self.add_keyword("lhrwgt_group_combine")
        self.add_keyword("lhrwgt_group_name")
        self.add_keyword("lhrwgt_id")
        self.add_keyword("LOevents")
        self.add_keyword("manyseeds")
        self.add_keyword("max_io_bufsize")
        self.add_keyword("maxseeds")
        self.add_keyword("minlo", frozen=True)
        self.add_keyword("mintupbratlim")
        self.add_keyword("mintupbxless")
        self.add_keyword("mtdep", 3)
        self.add_keyword("ncall1", 200000)
        self.add_keyword("ncall1rm")
        self.add_keyword("ncall2", 150000)
        self.add_keyword("ncall2rm")
        self.add_keyword("ncallfrominput")
        self.add_keyword("noevents")
        self.add_keyword("novirtual")
        self.add_keyword("nubound", 200000)
        self.add_keyword("olddij")
        self.add_keyword("par_2gsupp")
        self.add_keyword("par_diexp")
        self.add_keyword("par_dijexp")
        self.add_keyword("parallelstage")
        self.add_keyword("pdfreweight")
        self.add_keyword("ptHHcut_CT")
        self.add_keyword("ptHHcut")
        self.add_keyword("ptsqmin")
        self.add_keyword("ptsupp")
        self.add_keyword("radregion")
        self.add_keyword("rand1")
        self.add_keyword("rand2")
        self.add_keyword("renscfact", self.default_scales[1])
        self.add_keyword("rescue_reals")
        self.add_keyword("rwl_add")
        self.add_keyword("rwl_file")
        self.add_keyword("rwl_format_rwgt")
        self.add_keyword("rwl_group_events")
        self.add_keyword("skipextratests")
        self.add_keyword("smartsig")
        self.add_keyword("softtest")
        self.add_keyword("stage2init")
        self.add_keyword("storeinfo_rwgt")
        self.add_keyword("storemintupb")
        self.add_keyword("testplots")
        self.add_keyword("testsuda")
        self.add_keyword("topmass", 173.) # this value is hardcoded in the virtual matrix element, and for consistency has not to be changed when running full theory prediction (i.e. mtdep=3)
        self.add_keyword("ubexcess_correct")
        self.add_keyword("ubsigmadetails")
        self.add_keyword("use-old-grid")
        self.add_keyword("use-old-ubound")
        self.add_keyword("withdamp")
        self.add_keyword("withnegweights")
        self.add_keyword("withsubtr")
        self.add_keyword("Wmass")
        self.add_keyword("Wwidth")
        self.add_keyword("xgriditeration")
        self.add_keyword("xupbound")
        self.add_keyword("zerowidth")
        self.add_keyword("Zmass")
        self.add_keyword("Zwidth")

    def create_grid_file(self):
        """! Creates the .grid file needed by this process."""
        """! This function calls a python script provided by the authors, which is linked to the local directory."""
        """! The code of this function is adapted from the script ${POWHEGPATH}/POWHEG-BOX-V2/ggHH/testrun/run.sh"""
        self.expose()  # convenience call to simplify syntax

        logger.info('Now attempting to link locally the files needed by this Powheg process')
        try:
            processpythondir = os.path.join(os.environ["POWHEGPATH"], "POWHEG-BOX-V2", "ggHH", 'python')
            if os.path.isdir(processpythondir):
                for filename in os.listdir(processpythondir):
                    source_path = os.path.join(processpythondir, filename)
                    if os.path.isfile(source_path):
                        link_name = os.path.join(os.getcwd(), filename)
                        try:
                            os.symlink(source_path, link_name)
                            print(f"Created link: {link_name} -> {source_path}")
                        except FileExistsError:
                            print(f"Link already exists: {link_name}")
            else:
                os.system("ln -s " + os.environ["POWHEGPATH"] + "/POWHEG-BOX-V2/ggHH/Virtual/events.cdf events.cdf")
                os.system("ln -s " + os.environ["POWHEGPATH"] + "/POWHEG-BOX-V2/ggHH/Virtual/creategrid.py creategrid.py")
                os.system("for grid in " + os.environ["POWHEGPATH"] + "/POWHEG-BOX-V2/ggHH/Virtual/Virt_full_*E*.grid; do ln -s $grid ${grid##*/}; done")
        except RuntimeError:
            logger.error('Impossible to link the needed files locally')
            raise

        # handling the parameters of this process
        # these parameters need to be parsed in a specific format
        chhh_str = f'{list(self.parameters_by_keyword("chhh"))[0].value:+.4E}'
        ct_str   = f'{list(self.parameters_by_keyword("ct"))[0].value:+.4E}'
        ctt_str  = f'{list(self.parameters_by_keyword("ctt"))[0].value:+.4E}'
        cggh_str   = f'{list(self.parameters_by_keyword("cggh"))[0].value:+.4E}'
        cgghh_str  = f'{list(self.parameters_by_keyword("cgghh"))[0].value:+.4E}'
        grid_file_name = f'Virt_full_{chhh_str}_{ct_str}_{ctt_str}_{cggh_str}_{cgghh_str}.grid'

        logger.info('Now trying to use creategrid.py to create the Virt_full_*.grid file')
        logger.info(f'File name: {grid_file_name}')
        logger.info(f'Parameters are: chhh={chhh_str}, ct={ct_str}, ctt={ctt_str}, cggh={cggh_str}, cgghh={cgghh_str}')
        try:
            import creategrid as cg
            cg.combinegrids(grid_file_name, float(chhh_str), float(ct_str), float(ctt_str), float(cggh_str), float(cgghh_str))
        except RuntimeError:
            logger.error('Impossible to use creategrid.py to create the Virt_full_*.grid file')
            raise
        logger.info('Although the produced Virt_full_*.grid file now exists in the local directory, Powheg will later try to find it in all directories contained in $PYTHONPATH. This will produce several "not found" info messages which can safely be ignored.')
