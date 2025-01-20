# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaCommon import Logging
from .configurable import Configurable
from ..utility import check_svn_revision, FileParser
import math
import os
import glob

## Get handle to Athena logging
logger = Logging.logging.getLogger("PowhegControl")


class PowhegBase(Configurable):
    """! Base class for PowhegBox processes.

    All process types inherit from this class.

    @author James Robinson  <james.robinson@cern.ch>
    @author Stefan Richter  <stefan.richter@cern.ch>
    """

    def hoppet_info(self):
        '''
        Returns a list of strings to be treated as info messages in the log
        They otherwise throw an error with HOPPET v. 1.2.0
        Cf. AGENE-2016
        '''
        return ["-----------------------------------------------------------",
                "Welcome to HOPPET v. 1.2.0",
                "Higher Order Perturbative Parton Evolution Toolkit",
                "Written by Gavin P. Salam (2001-2012)",
                "with contributions from Juan Rojo",
                "Frederic Dreyer and Alexander Karlberg",
                "It is made available under the GNU public license,",
                "with the additional request that if you use it or any",
                "derivative of it in scientific work then you should cite:",
                "G.P. Salam & J. Rojo, CPC 180(2009)120 (arXiv:0804.3755).",
                "You are also encouraged to cite the original references,",
                "for LO, NLO and NNLO splitting functions, the QCD",
                "1, 2 and 3 loop beta functions and the coupling and",
                "PDF and coupling mass threshold matching functions."]

    def hoppet_warning(self):
        '''
        Returns a list of strings to be treated as warning messages in the log
        They otherwise throw an error
        '''
        return ["WARNING in InitMTMNNLO: using parametrisation  (less accuracte) for A2PShg"]

    def openloops_error(self):
        '''
        Returns a list of strings to be treated as error messages in the log
        They otherwise do not throw an error
        '''
        return ["[POWHEG-BOX+OpenLoops] Process not found!"]

    def __init__(self, base_directory, version, executable_name, cores, powheg_executable="pwhg_main", is_reweightable=True, warning_output = [], info_output = [], error_output = [], **kwargs):
        """! Constructor.

        @param base_directory  path to PowhegBox code.
        @param version         PowhegBox version.
        @param executable_name folder containing appropriate PowhegBox executable.
        @param powheg_executable name of the powheg executable.
        @param warning_output list of patterns which if found in the output will be treated as warning in the log.
        @param error_output list of patterns which if found in the output will be treated as error in the log.
        @param info_output list of patterns which if found in the output will be treated as info in the log.
        """
        super(PowhegBase, self).__init__()

        ## Powheg executable that will be used
        self.executable = os.path.join(base_directory, version, executable_name, powheg_executable)

        ## Add to Python path "python" directory on POWHEG process directory
        os.environ["PYTHONPATH"] += ":" + os.path.join(base_directory, version, executable_name, "python")

        ## Add other locations of the libraries
        os.environ["LD_LIBRARY_PATH"] += ":" + os.path.join(base_directory, version, executable_name, "amplitudes", "obj-gnu")
        os.environ["LD_LIBRARY_PATH"] += ":" + os.path.join(base_directory, version, executable_name, "QCDLoop-1.95", "ff", "obj-gnu")
        os.environ["LD_LIBRARY_PATH"] += ":" + os.path.join(base_directory, version, executable_name, "Virtuals", "obj-gnu")
        os.environ["LD_LIBRARY_PATH"] += ":" + os.path.join(base_directory, version, executable_name, "obj-gfortran")
        os.environ["LD_LIBRARY_PATH"] += ":" + os.path.join(base_directory, version, executable_name, "obj-gfortran", "proclib")
        os.environ["LD_LIBRARY_PATH"] += ":" + os.path.join(base_directory, version, executable_name, "obj-gnu")
        os.environ["LD_LIBRARY_PATH"] += ":" + os.path.join(base_directory, version, executable_name, "obj-gnu", "proclib")

        ##OpenLoops
        directories = ['obj-gfortran', 'OpenLoops2', 'obj-gnu']
        logger.info("OpenLoopsPath (before) = {0}".format(os.getenv('OpenLoopsPath')))
        for directory in directories:
          if os.path.isdir(os.path.join(base_directory, version, executable_name, directory, 'proclib')):
             OLPath = os.path.join(base_directory, version, executable_name, directory)
             os.environ['OpenLoopsPath'] = OLPath
             logger.info("OpenLoopsPath (after) = {0}".format(os.getenv('OpenLoopsPath')))
             break

        ## SVN revision of process code
        self.process_revision = check_svn_revision(os.path.dirname(self.executable))

        ## Log the PowhegBox version and process-code revision for AMI etc.
        ## Also set environment variable POWHEGVER to this contain this information
        one_character_version = self.powheg_version.replace('V', '')[0]
        POWHEGVER = '{v}_r{rev}'.format(v=one_character_version, rev=self.process_revision)
        logger.info('MetaData: POWHEGVER = {0}'.format(POWHEGVER))
        os.environ["POWHEGVER"] = POWHEGVER # does not export to parent shell, but might be sufficient inside job?

        ## SVN revision of PowhegBox code
        self.powhegbox_revision = check_svn_revision(os.path.dirname(os.path.dirname(self.executable)))

        ## Number of cores to use
        self.cores = cores

        ## List of additional algorithms to schedule
        self.algorithms = []

        ## List of external processes to schedule
        self.externals = {}

        ## List of validation functions to run before preparing runcard
        self.validation_functions = []

        # Universal keywords that are set from the run arguments
        self.add_keyword("ebeam1", kwargs.get("beam_energy", None))
        self.add_keyword("ebeam2", kwargs.get("beam_energy", None))
        self.add_keyword("iseed", int(kwargs.get("random_seed", None)))
        self.add_keyword("numevts", kwargs.get("nEvents", None))

        # Add parameter validation functions
        self.validation_functions.append("validate_integration_parameters")

        ## Switch to determine whether reweighting is allowed
        self.is_reweightable = is_reweightable

        ## Switch to determine whether XML reweighting should be used
        self.use_XML_reweighting = None

        ## Switch to determine if the #rwgt and #pdf comments should be kept in lhe files despite using xml reweighting
        self.remove_oldStyle_rwt_comments = None

        ## Special treatment for some log messages
        self.warning_output = warning_output
        self.info_output = info_output
        self.error_output = error_output

        ## Dictionary used to change parameters of the Powheg input.
        ## The structure is: {"parameter" : [default_value, changed_value, parallelstage_to_changed_value]}
        self.parameterStageDict = {}

    def add_algorithm(self, alg_or_process):
        """! Add an algorithm or external process to the sequence.

        @param process  Algorithm or external process to add.
        """
        # Either add to the list of algorithms to schedule
        if isinstance(alg_or_process, str):
            self.algorithms.append(alg_or_process)
        # ... or add as an external process
        else:
            self.externals[alg_or_process.name] = alg_or_process

    @property
    def files_for_cleanup(self):
        """! Wildcarded list of files created by this process that can be deleted."""
        raise AttributeError("Names of unneeded files are not known for this process!")

    @property
    def integration_file_names(self):
        """! Wildcarded list of integration files that might be created by this process."""
        raise AttributeError("Integration file names are not known for this process!")

    @property
    def mandatory_integration_file_names(self):
        """! Wildcarded list of integration files that are needed for this process."""
        raise AttributeError("Integration file names are not known for this process!")

    @property
    def powheg_version(self):
        """! Version of PowhegBox process."""
        raise AttributeError("Powheg version is not known!")

    @property
    def default_PDFs(self):
        """! Default PDFs for this process."""
        raise AttributeError("Default PDFs are not known for this process!")

    @property
    def default_scales(self):
        """! Default scale variations for this process."""
        raise AttributeError("Default scales are not known for this process!")

    def prepare_to_parallelise(self, n_cores):
        """! Scale calculation parameters by n_cores."""
        __nEvents_unscaled = self.parameters_by_keyword("numevts")[0].value
        for keyword in ["ncall1", "ncall1rm", "ncall2", "ncall2rm", "nubound", "numevts"]:
            for parameter in self.parameters_by_keyword(keyword):
                if int(parameter.value) > 0:
                    parameter.value = int(math.ceil(float(parameter.value) / n_cores))
        __nEvents_scaled = self.parameters_by_keyword("numevts")[0].value
        logger.info("Scaling number of events per job from {} down to {}".format(__nEvents_unscaled, __nEvents_scaled))
        # Freeze parallelstage parameters before printing list for user
        [parameter.freeze() for parameter in self.parameters_by_name("parallelstage")]

    def stage_is_completed(self, stage):
        """! Set whether the specified POWHEG-BOX generation stage is complete."""
        # Perform manual check to allow re-use of grids in multicore mode
        return False

    def validate_parameters(self):
        """! Validate any parameters which need it before preparing runcard."""
        for function_name in self.validation_functions:
            getattr(self, function_name)()

    def validate_integration_parameters(self):
        """! Validate integration keywords by forcing to integer values."""
        self.expose()  # convenience call to simplify syntax
        for name in ("foldcsi", "foldphi", "foldy", "itmx1", "itmx2", "ncall1", "ncall2", "nEvents"):
            for parameter in self.parameters_by_name(name):
                try:
                    parameter.value = int(parameter.value)
                except TypeError:
                    logger.fatal("Failed to validate {} with value {}".format(name, parameter.value))
                    raise

    def check_decay_mode(self, decay_mode, allowed_decay_modes=None):
        """! Check whether a decay mode is allowed an raise an exception if it is not."""
        if allowed_decay_modes is None:
            allowed_decay_modes = self.allowed_decay_modes
        if decay_mode not in allowed_decay_modes:
            logger.warning("Decay mode {} not recognised!".format(decay_mode))
            logger.info("Allowed decay modes are:")
            for allowed_decay_mode in allowed_decay_modes:
                logger.info("... {}".format(allowed_decay_mode))
            raise ValueError("Decay mode {} not recognised!".format(decay_mode))

    def check_using_integration_files(self):
        ###! Return true if pre-made integration grids will be used."""
        search_strings = [] # list of filename patters to be searched for
        found_files = [] # list of found files will be printed out for info
        missing_patterns = [] # list of filename patters which hasn't been found but would be needed for pre-made integration grids
        try:
            search_strings = self.mandatory_integration_file_names
        except AttributeError:
            logger.fatal("No integration grid file name patterns defined for this process.")
            raise
        for s in search_strings:
            found = glob.glob(s)
            if found != []:
                found_files += found
            else:
                missing_patterns.append(s)
        if missing_patterns == []:
            logger.info("Integration grid files found locally. Event generation shall continue, skipping the integration step.")
            logger.info("Integration grid files found locally: {}".format(found_files))
        else:
            logger.info("Integration grid files needed were not found locally. Event generation shall continue, starting by the integration step.")
            logger.info("Missing integration grid files with these patterns: {}".format(missing_patterns))
            logger.info("Integration grid files found locally (if any): {}".format(found_files))


    def modify_parameter(self, stage = 0):

        #skip modifying if dict is empty
        if not bool(self.parameterStageDict):
            return

        logger.info("Modifying parameters for the stages : {0}".format(self.parameterStageDict))

        ## The structure is: {"parameter" : [default_value, changed_value, parallelstage_to_changed_value]}
        for key in self.parameterStageDict:
            settingsList = self.parameterStageDict[key]
            if abs(settingsList[2] - stage) <=1e-09:
                self.set_parameter_in_config(key,settingsList[1])
            else: 
                self.set_parameter_in_config(key,settingsList[0])

    def set_parameter_in_config(self, key, value):
        FileParser("powheg.input").text_replace(key+".*", key+" {}".format(value))
