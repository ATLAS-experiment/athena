# Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
import os
import glob
from AthenaCommon import Logging
from ..powheg_V2 import PowhegV2

## Get handle to Athena logging
logger = Logging.logging.getLogger("PowhegControl")


class ggF_HH_SMEFT(PowhegV2):
    """! Default Powheg configuration for gluon-fusion Higgs boson production with quark mass and EW effects.

    Create a configurable object with all applicable Powheg options.

    @author Timothée Theveneaux-Pelzer  <tpelzer@cern.ch>
    """

    def __init__(self, base_directory, **kwargs):
        """! Constructor: all process options are set here.

        @param base_directory: path to PowhegBox code.
        @param kwargs          dictionary of arguments from Generate_tf.
        """
        super(ggF_HH_SMEFT, self).__init__(base_directory, "ggHH_SMEFT", **kwargs)

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
        self.add_keyword("CHbox", 0.)
        self.add_keyword("CHD", 0.)
        self.add_keyword("CH", 0.)
        self.add_keyword("CuH", 0.)
        self.add_keyword("CHG", 0.)
        self.add_keyword("CtG", 0.)
        self.add_keyword("CQt", 0.)
        self.add_keyword("CQt8", 0.)
        self.add_keyword("CQQtt", 0.)
        self.add_keyword("CQQ8", 0.)
        self.add_keyword("GAMMA5BMHV", 0)
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
        self.add_keyword("hmass", 125.) # Value is hardcoded to 125 in the virtual matrix element, modification only possible in bornonly mode.
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
        self.add_keyword("includesubleading", 0)
        self.add_keyword("Lambda", 1.0)
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
        self.add_keyword("multiple-insertion", 1)
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
        self.add_keyword("topmass", 173.) # Value is hardcoded to 173 in the virtual matrix element, modification only possible in bornonly mode.
        self.add_keyword("ubexcess_correct")
        self.add_keyword("ubsigmadetails")
        self.add_keyword("use-old-grid")
        self.add_keyword("use-old-ubound")
        self.add_keyword("usesmeft", 1)
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
            os.system("ln -s " + os.environ["POWHEGPATH"] + "/POWHEG-BOX-V2/ggHH_SMEFT/Virtual/events.cdf events.cdf")
            os.system("ln -s " + os.environ["POWHEGPATH"] + "/POWHEG-BOX-V2/ggHH_SMEFT/Virtual/creategrid.py creategrid.py")
            os.system("ln -s " + os.environ["POWHEGPATH"] + "/POWHEG-BOX-V2/ggHH_SMEFT/shell/warmup_smeft.py warmup_smeft.py")
            os.system("for grid in " + os.environ["POWHEGPATH"] + "/POWHEG-BOX-V2/ggHH_SMEFT/Virtual/Virt_full_*E*.grid; do ln -s $grid ${grid##*/}; done")
        except RuntimeError:
            logger.error('Impossible to link the needed files locally')
            raise

         # need to override lhapdf python path while the powheg process has been compiled in a different platform
        py_path_save = os.environ["PYTHONPATH"]
        base_path = os.environ["LHAPDF_INSTAL_PATH"]

        # Search for the Python version in the lib folder
        python_lib_path = glob.glob(os.path.join(base_path, "lib", "python*"))

        # Ensure at least one matching path is found
        if python_lib_path:
            python_lib_path = python_lib_path[0]
        else:
            raise ValueError("No Python version found in lib folder")

        # Build the temporary path
        py_path_temp = python_lib_path + "/site-packages" + ":" + py_path_save

        os.environ["PYTHONPATH"] = py_path_temp
        logger.debug(f'Temporarily setting PYTHONPATH to:\n{py_path_temp}')

        # handling the parameters of this process
        # these parameters need to be parsed in a specific format
        usesmeft_str = str(list(self.parameters_by_keyword("usesmeft"))[0].value)
        #EFTcount_str = ""
        if usesmeft_str == "0":
            EFTcount = 3
        else:
            EFTcount = list(self.parameters_by_keyword("multiple-insertion"))[0].value
        EFTcount_str = f'{EFTcount:+n}'
        # need to handle the case where we have different scales or pdfs in the joboption, provided as a list, or just one value
        renfact = list(self.parameters_by_keyword("renscfact"))[0].value
        renfac_str = f'{renfact[0]:+.2f}' if type(renfact) is list else f'{renfact:+.2f}'
        lhapdfid = list(self.parameters_by_keyword("lhans1"))[0].value
        lhapdfid_str = str(lhapdfid[0]) if type(lhapdfid) is list else str(lhapdfid)

        if usesmeft_str == "1":
            GF_str = f'{list(self.parameters_by_keyword("gfermi"))[0].value:+.4E}'
            Lambda_str = f'{list(self.parameters_by_keyword("Lambda"))[0].value:+.4E}'
            CHbox_str = f'{list(self.parameters_by_keyword("CHbox"))[0].value:+.4E}'
            CHD_str = f'{list(self.parameters_by_keyword("CHD"))[0].value:+.4E}'
            CH_str = f'{list(self.parameters_by_keyword("CH"))[0].value:+.4E}'
            CuH_str = f'{list(self.parameters_by_keyword("CuH"))[0].value:+.4E}'
            CHG_str = f'{list(self.parameters_by_keyword("CHG"))[0].value:+.4E}'

            logger.info('Now trying to use warmup_smeft.py to create the Virt_full_*.grid file')
            logger.info(f'Parameters are: GF={GF_str}, Lambda={Lambda_str}, CHbox={CHbox_str}, CHD={CHD_str}, CH={CH_str}, CuH={CuH_str}, CHG={CHG_str}, EFTcount={EFTcount_str}, usesmeft={usesmeft_str}, lhapdfid={lhapdfid_str}, renfac={renfac_str}')
            try:
                #import creategrid as cg
                #cg.combinegrids(grid_file_name, chhh_str, ct_str, ctt_str, cggh_str, cgghh_str)
                pythoncmd=f"import warmup_smeft as ws; ws.combinegrids_smeft({Lambda_str}, {CHbox_str}, {CHD_str}, {CH_str}, {CuH_str}, {CHG_str}, {GF_str}, {EFTcount_str}, {usesmeft_str}, {lhapdfid_str}, {renfac_str})"
                os.system("python3 -c \""+pythoncmd+"\"")
            except RuntimeError:
                logger.error('Impossible to use warmup_smeft.py to create the Virt_full_*.grid file')
                raise

        else:
            chhh_str = f'{list(self.parameters_by_keyword("chhh"))[0].value:+.4E}'
            ct_str   = f'{list(self.parameters_by_keyword("ct"))[0].value:+.4E}'
            ctt_str  = f'{list(self.parameters_by_keyword("ctt"))[0].value:+.4E}'
            cggh_str   = f'{list(self.parameters_by_keyword("cggh"))[0].value:+.4E}'
            cgghh_str  = f'{list(self.parameters_by_keyword("cgghh"))[0].value:+.4E}'
            grid_file_name = f'Virt_full_{chhh_str}_{ct_str}_{ctt_str}_{cggh_str}_{cgghh_str}-HEFT{EFTcount_str}.grid'

            logger.info('Now trying to use creategrid.py to create the Virt_full_*.grid file')
            logger.info(f'File name: {grid_file_name}')
            logger.info(f'Parameters are: chhh={chhh_str}, ct={ct_str}, ctt={ctt_str}, cggh={cggh_str}, cgghh={cgghh_str}, EFTcount={EFTcount_str}, usesmeft={usesmeft_str}, lhapdfid={lhapdfid_str}, renfac={renfac_str}')
            try:
                #import creategrid as cg
                #cg.combinegrids(grid_file_name, chhh_str, ct_str, ctt_str, cggh_str, cgghh_str)
                pythoncmd=f"import creategrid as cg; cg.combinegrids('{grid_file_name}', {chhh_str}, {ct_str}, {ctt_str}, {cggh_str}, {cgghh_str}, {EFTcount_str}, {lhapdfid_str}, {renfac_str})"
                os.system("python3 -c \""+pythoncmd+"\"")
            except RuntimeError:
                logger.error('Impossible to use creategrid.py to create the Virt_full_*.grid file')
                raise

        # setting PYTHONPATH back to its original value
        os.environ["PYTHONPATH"] = py_path_save
        logger.debug(f'Setting PYTHONPATH back to:\n{py_path_save}')

        logger.info('Although the produced Virt_full_*.grid file now exists in the local directory, Powheg will later try to find it in all directories contained in $PYTHONPATH. This will produce several "not found" info messages which can safely be ignored.')

