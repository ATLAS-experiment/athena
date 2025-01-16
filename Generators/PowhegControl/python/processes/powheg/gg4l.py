# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

import os
from AthenaCommon import Logging
from ..powheg_RES import PowhegRES

## Get handle to Athena logging
logger = Logging.logging.getLogger("PowhegControl")

class gg4l(PowhegRES):
    """! Default Powheg configuration for 
    NLO QCD corrections to 4 lepton production in gluon fusion, 
    including the Higgs-mediated contribution, the diboson background and their interference.

    Create a configurable object with all applicable Powheg options.

    @author Andrej Saibel <Andrej.Saibel@cern.ch>
    """

    def __init__(self, base_directory, **kwargs):
        """! Constructor: all process options are set here.

        @param base_directory: path to PowhegBox code.
        @param kwargs          dictionary of arguments from Generate_tf.
        """

        # list of message patterns to be treated as warnings
        warnings = [
            "ggvvamp: increasing precision to"
            ]
        super(gg4l, self).__init__(base_directory, "gg4l", **kwargs)
        self.warning_output = warnings

        # defining gg4l environment variable to bypass file path issues in QCDLoop-*/ff/ffinit.f
        os.environ['gg4lPATH'] = os.path.dirname(self.executable)
        logger.info("gg4lPATH defined as = {0}".format(os.getenv('gg4lPATH')))

        # This is a hacky fix that's needed at the moment...
        self.manually_set_openloops_gnu_paths()

        # Adding external libraries to LD_LIBRARY_PATH for gg4l
        self.link_external_powheg_libraries("/POWHEG-BOX-RES/gg4l/amplitudes/obj-gnu/")
        self.link_external_powheg_libraries("/POWHEG-BOX-RES/gg4l/QCDLoop*/ff/obj-gnu/")

        # Add parameter validation functions
        self.validation_functions.append("validate_process_contrib")


        self.allowed_process_modes = ["\'WW\'", "\'ZZ\'"]
        self.allowed_contrib_modes = ["\'full\'", "\'only_h\'", "\'no_h\'", "\'interf_h\'"]

        # Add all keywords for this process, overriding defaults if required
        self.add_keyword("allrad", 1)
        self.add_keyword("alpha")
        self.add_keyword("bmass")
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
        self.add_keyword("charmthr")
        self.add_keyword("charmthrpdf")
        self.add_keyword("check_bad_st1")
        self.add_keyword("check_bad_st2")
        self.add_keyword("chklimseed")
        self.add_keyword("clobberlhe")
        self.add_keyword("colltest")
        self.add_keyword("complexGFermi")
        self.add_keyword("compress_lhe")
        self.add_keyword("compress_upb")
        self.add_keyword("compute_rwgt")
        self.add_keyword("contr", "\'full\'")       # 'full', 'only_h', 'no_h' or 'interf_h'
        self.add_keyword("dorwgt", 1)
        self.add_keyword("doublefsr")
        self.add_keyword("enhancereg")
        self.add_keyword("evenmaxrat")
        self.add_keyword("ewscheme")
        self.add_keyword("facscfact", self.default_scales[0])
        self.add_keyword("fastbtlbound")
        self.add_keyword("fixedscale")
        self.add_keyword("flg_debug")
        self.add_keyword("foldcsi", 2)
        self.add_keyword("foldphi", 5)
        self.add_keyword("foldy", 5)
        self.add_keyword("for_reweighting")
        self.add_keyword("fullrwgt")
        self.add_keyword("gamcut", 60)
        self.add_keyword("hdamp", 172.5)
        self.add_keyword("hfact")
        self.add_keyword("hmass")
        self.add_keyword("hwidth")
        self.add_keyword("icsimax")
        self.add_keyword("ih1")
        self.add_keyword("ih2")
        self.add_keyword("itmx1")
        self.add_keyword("itmx1btl")
        self.add_keyword("itmx1btlbrn")
        self.add_keyword("itmx1rm")
        self.add_keyword("itmx2", 6)
        self.add_keyword("itmx2btl")
        self.add_keyword("itmx2btlbrn")
        self.add_keyword("itmx2rm")
        self.add_keyword("iupperfsr")
        self.add_keyword("iupperisr")
        self.add_keyword("iymax")
        self.add_keyword("lhans1", self.default_PDFs)
        self.add_keyword("lhans2", self.default_PDFs)
        self.add_keyword("lhrwgt_descr")
        self.add_keyword("lhrwgt_group_combine")
        self.add_keyword("lhrwgt_group_name")
        self.add_keyword("lhrwgt_id")
        self.add_keyword("LOevents")
        self.add_keyword("m4l_sampling", 2)     # 0: flat   1: exponential   2:Breit-Wigner
        self.add_keyword("m4lmax", 100)
        self.add_keyword("m4lmin", 1)
        self.add_keyword("m4lwindow", 4)
        self.add_keyword("manyseeds")
        self.add_keyword("massiveloops", 0)
        self.add_keyword("mass_rwg_frac", 0.) # gg4l treatment of heavy-flavour
        self.add_keyword("max_io_bufsize")
        self.add_keyword("maxseeds")
        self.add_keyword("minlo")
        self.add_keyword("mint_density_map")
        self.add_keyword("mintupbratlim")
        self.add_keyword("mllmax", 100)         # default 0.1 GeV this is maximum invar mass for Z leptons
        self.add_keyword("mllmin", 0.)          # default 0.1 GeV this is minimum invar mass for Z leptons
        self.add_keyword("MSbarscheme")
        self.add_keyword("mt_expansion", 1)
        self.add_keyword("ncall1", 10000)
        self.add_keyword("ncall1btl")
        self.add_keyword("ncall1btlbrn", 50000)
        self.add_keyword("ncall1rm")
        self.add_keyword("ncall2", 10000)
        self.add_keyword("ncall2btl")
        self.add_keyword("ncall2btlbrn",100000)
        self.add_keyword("ncall2rm")
        self.add_keyword("ncallfrominput")
        self.add_keyword("noevents")
        self.add_keyword("nores")
        self.add_keyword("novirtual")
        self.add_keyword("nubound", 50000)
        self.add_keyword("ol_nf", 6)
        self.add_keyword("ol_verbose", 1)
        self.add_keyword("ol_preset", 3)
        self.add_keyword("ol_notri", 0)
        self.add_keyword("ol_stability_kill", 0.01)
        self.add_keyword("olpreset")
        self.add_keyword("olverbose")
        self.add_keyword("openloops-stability")
        self.add_keyword("openloopsreal")
        self.add_keyword("openloopsvirtual")
        self.add_keyword("par_2gsupp")
        self.add_keyword("par_diexp")
        self.add_keyword("par_dijexp")
        self.add_keyword("parallelstage")
        self.add_keyword("pdfreweight")
        self.add_keyword("proc","\'WW\'" )          # 'ZZ' or 'WW'
        self.add_keyword("ptllmin", 0.1)        # this is hard-coded for virtual corrections with massive loops
        self.add_keyword("ptsqmin")
        self.add_keyword("ptsupp")
        self.add_keyword("ptVVcut_CT")
        self.add_keyword("ptVVcut")
        self.add_keyword("radregion")
        self.add_keyword("rand1")
        self.add_keyword("rand2")
        self.add_keyword("regridfix")
        self.add_keyword("renscfact", self.default_scales[1])
        self.add_keyword("rwl_add")
        self.add_keyword("rwl_file")
        self.add_keyword("rwl_format_rwgt")
        self.add_keyword("rwl_group_events")
        self.add_keyword("select_real", 0) #do gg qg qq channels
        self.add_keyword("smartsig")
        self.add_keyword("softmismch")
        self.add_keyword("softonly")
        self.add_keyword("softtest")
        self.add_keyword("stage2init")
        self.add_keyword("storeinfo_rwgt")
        self.add_keyword("storemintupb")
        self.add_keyword("testplots")
        self.add_keyword("testsuda")
        self.add_keyword("tmass_phsp")
        self.add_keyword("tmass")
        self.add_keyword("twidth",-1)
        self.add_keyword("twidth_phsp",-1)
        self.add_keyword("ubexcess_correct")
        self.add_keyword("ubsigmadetails", -1)  # disable cross-section output to avoid Fortran crash
        self.add_keyword("use-old-grid")
        self.add_keyword("use-old-ubound")
        self.add_keyword("useoldphsp", 0)       # use the old phase space parametrization from ggZZ
        self.add_keyword("user_reshists_sep")
        self.add_keyword("vdecaymodeV1")  # decay mode of first vector boson
        self.add_keyword("vdecaymodeV2")    # decay mode of second vector boson
        self.add_keyword("verytinypars")
        self.add_keyword("virtonly")
        self.add_keyword("which_as",2)
        self.add_keyword("whichpwhgevent")
        self.add_keyword("withbtilde")
        self.add_keyword("withdamp", 1)
        self.add_keyword("withnegweights")
        self.add_keyword("withremnants")
        self.add_keyword("withsubtr")
        self.add_keyword("wmass")
        self.add_keyword("wwidth")
        self.add_keyword("xgriditeration")
        self.add_keyword("xupbound", 2)
        self.add_keyword("zerowidth")
        self.add_keyword("zmass")
        self.add_keyword("zwidth")


    def validate_process_contrib(self):
        """! Validate Process and Contribution modes"""
        self.expose()  # convenience call to simplify syntax

        #adding char qualication to input
        if "\'" not in self.proc:
            self.proc = "\'"+self.proc+"\'"
        if "\'" not in self.contr:
            self.contr = "\'"+self.contr+"\'"

        #Modify unsupported decay configuration in ZZ production
        if (self.proc == "'ZZ'" and ((self.vdecaymodeV1 == self.vdecaymodeV2) or self.vdecaymodeV1 == 15 or self.vdecaymodeV2 == 15)): 
            logger.warning("Powheg/gg4l does support directly 4e, 4mu or tau final states.")
            if(self.vdecaymodeV1 == 11 and self.vdecaymodeV2 == 11):
                logger.warning("Ask to generate 2e2mu decays and hack the LHE files to have 4e final states - make sure to validate!")
                self.add_algorithm("mu2e")
            elif(self.vdecaymodeV1 == 13 and self.vdecaymodeV2 == 13):
                logger.warning("Ask to generate 2e2mu decays and hack the LHE files to have 4mu final states - make sure to validate!")
                self.add_algorithm("e2mu")
            elif(self.vdecaymodeV1 == 15 and self.vdecaymodeV2 == 15):
                logger.warning("Ask to generate 2e2mu decays and hack the LHE files to have 4tau final states - make sure to validate!")
                self.add_algorithm("e2tau")
                self.add_algorithm("mu2tau")
            elif(self.vdecaymodeV1 == 11 and self.vdecaymodeV2 == 15) or (self.vdecaymodeV1 == 15 and self.vdecaymodeV2 == 11):
                logger.warning("Ask to generate 2e2mu decays and hack the LHE files to have 2e2tau final states - make sure to validate!")
                self.add_algorithm("mu2tau")
            elif(self.vdecaymodeV1 == 13 and self.vdecaymodeV2 == 15) or (self.vdecaymodeV1 == 15 and self.vdecaymodeV2 == 13):
                logger.warning("Ask to generate 2e2mu decays and hack the LHE files to have 2mu2tau final states - make sure to validate!")
                self.add_algorithm("e2tau")
            elif(self.vdecaymodeV1 == 'll' and self.vdecaymodeV2 == 'll'):
                logger.warning("Ask to generate 2e2mu decays and hack the LHE files to have inclusive 4l decays - make sure to validate!")
                self.add_algorithm("gg4l_emu2all")
            
            self.vdecaymodeV1 = 11
            self.vdecaymodeV2 = 13
            self.parameters_by_keyword("vdecaymodeV1")[0].value = self.vdecaymodeV1
            self.parameters_by_keyword("vdecaymodeV2")[0].value = self.vdecaymodeV2

        #check if the setting is allowed
        if self.proc not in self.allowed_process_modes:
            logger.warning("Process mode {} not recognised!".format(self.proc))
            raise ValueError("Process mode {} not recognised!".format(self.proc))

        if self.contr not in self.allowed_contrib_modes:
            logger.warning("Contribution mode {} not recognised!".format(self.contr))
            raise ValueError("Contribution mode {} not recognised!".format(self.contr))

        self.parameters_by_keyword("proc")[0].value    = self.proc
        self.parameters_by_keyword("contr")[0].value   = self.contr

