# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
from enum import Enum
from logging import Logger
from os import environ
from pathlib import Path
from typing import List, Optional
from uuid import uuid4
import subprocess

from .Helpers import get_release_setup, list_changed_packages
from .Inputs import references_CVMFS_path, references_override_url
from .References import references_map


class TestSetup:
    """Test setup."""

    def __init__(self, logger: Logger) -> None:
        self.logger = logger
        self.validation_run_path = Path.cwd()
        self.reference_run_path = Path("/tmp")
        self.diff_rules_path = None
        self.unique_ID = str(uuid4())
        self.disable_release_setup = False
        self.validation_only = False
        self.run_only = False
        self.checks_only = False
        self.release_reference = ""
        self.release_validation = ""
        self.release_ID = "24.0"  # The following is not flexible enough, can probably be hardcoded: environ["AtlasVersion"][0:4]
        self.parallel_execution = False
        self.disable_output_checks = False
        self.custom_threads = None
        self.detailed_comparison = False

    def setup_release(self, reference=None, validation=None) -> None:
        if reference and validation:
            self.release_reference = reference
            self.release_validation = validation
            self.logger.info(f"WARNING: You have specified a dedicated release as reference {reference} and as validation {validation} release.")
            self.logger.info("Your local setup area will not be considered!!!")
            self.logger.info("this option is mainly designed for comparing release versions!!")
        elif reference:
            self.release_reference = reference
            self.release_validation = ''
            self.logger.info(f"You have specified a dedicated release as reference {reference}.")
        else:
            self.release_reference = get_release_setup(self.logger, self.disable_release_setup)
            self.release_validation = self.release_reference
            try:
                list_changed_packages(self.logger, self.disable_release_setup)
            except Exception:
                self.logger.warning("Cannot list changed packages...\n")


class WorkflowRun(Enum):
    Run2 = "Run2"
    Run3 = "Run3"
    Run4 = "Run4"

    def __str__(self):
        return self.name


class WorkflowType(Enum):
    Generation = "Generation"
    FullSim = "FullSim"
    AF3 = "AF3"
    HitsMerge = "HitsMerge"
    HitsFilter = "HitsFilter"
    MCOverlay = "MCOverlay"
    DataOverlay = "DataOverlay"
    DataOverlayChain = "DataOverlayChain"
    DataOverlayReco = "DataOverlayReco"
    MCReco = "MCReco"
    MCPileUpReco = "MCPileUpReco"
    DataReco = "DataReco"
    PileUpPresampling = "PileUpPresampling"
    MinbiasPreprocessing = "MinbiasPreprocessing"
    Derivation = "Derivation"

    def __str__(self):
        return self.name


class WorkflowCheck:
    """Workflow check base class."""

    def __init__(self, setup: TestSetup) -> None:
        self.setup = setup
        self.logger = setup.logger

    def reference_file(self, test: "WorkflowTest", file_name: str) -> Optional[Path]:
        reference_path: Path = test.reference_path
        reference_file = reference_path / file_name

        # Read references from CVMFS
        if self.setup.validation_only:
            # Resolve the subfolder first. Results are stored like: main_folder/branch/test/version/.
            reference_revision = references_map[f"{test.ID}"]
            cvmfs_path = Path(references_CVMFS_path)
            rel_path = Path(self.setup.release_ID) / test.ID / reference_revision
            reference_path = cvmfs_path / rel_path
            reference_file = reference_path / file_name

            if not reference_path.exists():
                self.logger.error(f"CVMFS reference location {reference_path} does not exist!")
                return None

            if references_override_url is not None:
                import requests

                url = references_override_url
                if not url.endswith("/"): url += "/"
                url += str(rel_path / file_name)
                self.logger.info("Checking for reference override at %s", url)
                if requests.head(url).ok: # file exists at url
                    reference_file = Path.cwd() / f"reference_{file_name}"
                    self.logger.info("Downloading reference from %s to %s", url, reference_file)
                    r = requests.get(url, stream=True)
                    with reference_file.open('wb') as f:
                        for chunk in r.iter_content(chunk_size=1024):
                            if chunk: # filter out keep-alive new chunks
                                f.write(chunk)
                else:
                    self.logger.info("No reference override found")

        return reference_file


class WorkflowTest:
    """Workflow test base class."""

    def __init__(self, ID: str, run: WorkflowRun, type: WorkflowType, steps: List[str], setup: TestSetup) -> None:
        if not hasattr(self, "ID"):
            self.ID = ID

        if not hasattr(self, "tag"):
            self.tag = ID

        if not hasattr(self, "steps"):
            self.steps = steps

        if not self.command:
            raise NotImplementedError("Command needs to be defined")

        if not hasattr(self, "output_checks"):
            self.output_checks = []

        if not hasattr(self, "digest_checks"):
            self.digest_checks = []

        if not hasattr(self, "skip_performance_checks"):
            self.skip_performance_checks = False

        self.run = run
        self.type = type
        self.setup = setup
        self.logger = setup.logger
        self.validation_path: Path = self.setup.validation_run_path / f"run_{self.ID}"
        self.reference_path: Path = self.setup.reference_run_path / f"run_{self.ID}"

    def run_reference(self) -> None:
        self.logger.info(f"Running reference in rel {self.setup.release_reference}")
        self.logger.info(f"\"{self.command}\"")

        self.reference_path.mkdir(parents=True, exist_ok=True)

        cmd = (f"cd {self.reference_path};"
               f"source $AtlasSetup/scripts/asetup.sh {self.setup.release_reference} >& /dev/null;")
        cmd += f"TRF_NOECHO=1 {self.command} > {self.ID}.log 2>&1"

        subprocess.call(cmd, shell=True)

        self.logger.info(f"Finished clean in rel {self.setup.release_reference}")
        self.logger.info(f"\"{self.command}\"")

    def run_validation(self) -> None:
        self.logger.info(f"Running validation in rel {self.setup.release_validation}")
        self.logger.info(f"\"{self.command}\"")

        self.validation_path.mkdir(parents=True, exist_ok=True)

        cmd = f"cd {self.validation_path};"
        if self.setup.disable_release_setup or not self.setup.release_validation:
            pass
        elif "WorkDir_DIR" in environ:
            cmake_build_dir = environ["WorkDir_DIR"]
            cmd += (f"source $AtlasSetup/scripts/asetup.sh {self.setup.release_validation} >& /dev/null;"
                    f"source {cmake_build_dir}/setup.sh;")
        else:
            cmd += f"source $AtlasSetup/scripts/asetup.sh {self.setup.release_validation} >& /dev/null;"
        cmd += f"TRF_NOECHO=1 {self.command} > {self.ID}.log 2>&1"

        subprocess.call(cmd, shell=True)

        self.logger.info(f"Finished validation in rel {self.setup.release_validation}")
        self.logger.info(f"\"{self.command}\"")

    def run_checks(self, performance_checks: List[WorkflowCheck]) -> bool:
        self.logger.info("-----------------------------------------------------")
        self.logger.info(f"----------- Post-processing of {self.ID} Test -----------")
        result = True

        # digest checks
        for check in self.digest_checks:
            result = check.run(self) and result

        # output checks
        if not self.setup.disable_output_checks:
            for check in self.output_checks:
                result = check.run(self) and result

        if self.setup.validation_only or self.skip_performance_checks:
            return result  # Performance checks against static references not possible

        # performance checks
        for check in performance_checks:
            result = check.run(self) and result

        return result
