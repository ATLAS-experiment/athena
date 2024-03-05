# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
from typing import List

from .Checks import AODContentCheck, AODDigestCheck, FrozenTier0PolicyCheck, MetadataCheck
from .Inputs import input_EVNT, input_HITS, \
    input_HITS_unfiltered, \
    input_RDO_BKG, \
    input_HITS_data_overlay, input_BS_SKIM, \
    input_HITS_minbias_low, input_HITS_minbias_high, input_HITS_neutrino, \
    input_AOD
from .Test import TestSetup, WorkflowRun, WorkflowTest, WorkflowType


class QTest(WorkflowTest):
    """General workflow q-test."""

    def __init__(self, ID: str, run: WorkflowRun, type: WorkflowType, steps: List[str], setup: TestSetup, extra_args: str = "") -> None:
        if "maxEvents" not in extra_args:
            if type == WorkflowType.MCPileUpReco or run == WorkflowRun.Run4:
                extra_args += " --maxEvents 5"
            else:
                extra_args += " --maxEvents 20"

        if type == WorkflowType.MCPileUpReco:
            if "inputHITSFile" not in extra_args:
                extra_args += f" --inputHITSFile {input_HITS[run]}"
            if "inputRDO_BKGFile" not in extra_args:
                extra_args += " --inputRDO_BKGFile ../run_d*/myRDO.pool.root"

        threads = 1
        threads_argument = '--multithreaded'
        if setup.custom_threads is not None:
            threads = setup.custom_threads
        if threads <= 0:
            threads_argument = ''

        self.command = \
            (f"ATHENA_CORE_NUMBER={threads} Reco_tf.py {threads_argument} --AMIConfig {ID}"
             f" --imf False {extra_args}")

        self.output_checks = []
        # TODO: disable RDO comparison for now
        # if type == WorkflowType.MCReco:
        #     self.output_checks.append(FrozenTier0PolicyCheck(setup, "RDO", 10))
        self.output_checks.append(FrozenTier0PolicyCheck(setup, "AOD", 60))
        self.output_checks.append(FrozenTier0PolicyCheck(setup, "ESD", 20))
        self.output_checks.append(MetadataCheck(setup, "AOD"))

        self.digest_checks = []
        if not setup.disable_output_checks:
            self.digest_checks.append(AODContentCheck(setup))
        self.digest_checks.append(AODDigestCheck(setup))

        super().__init__(ID, run, type, steps, setup)


class SimulationTest(WorkflowTest):
    """Simulation workflow test."""

    def __init__(self, ID: str, run: WorkflowRun, type: WorkflowType, steps: List[str], setup: TestSetup, extra_args: str = "") -> None:
        if "maxEvents" not in extra_args:
            extra_args += " --maxEvents 20"

        if "jobNumber" not in extra_args and run is WorkflowRun.Run3 and type is WorkflowType.FullSim:
            extra_args += " --jobNumber 5"

        input_argument = ""
        if "inputEVNTFile" not in extra_args and "inputHITSFile" not in extra_args:
            if type is WorkflowType.HitsFilter:
                input_argument = f"--inputHITSFile {input_HITS_unfiltered[run]}"
            elif type is WorkflowType.HitsMerge:
                input_argument = f"--inputHITSFile {input_HITS[run]}"
            else:
                input_argument = f"--inputEVNTFile {input_EVNT[run]}"

        threads = 0
        threads_argument = '--multithreaded'
        if setup.custom_threads is not None:
            threads = setup.custom_threads
        if threads <= 0:
            threads_argument = ''

        if type is WorkflowType.HitsMerge:
            self.command = \
                (f"ATHENA_CORE_NUMBER={threads} HITSMerge_tf.py {threads_argument} --AMIConfig {ID}"
                f" {input_argument} --outputHITS_MRGFile myHITS.pool.root"
                f" --imf False {extra_args}")
        elif type is WorkflowType.HitsFilter:
            self.command = \
                (f"ATHENA_CORE_NUMBER={threads} FilterHit_tf.py {threads_argument} --AMIConfig {ID}"
                f" {input_argument} --outputHITS_FILTFile myHITS.pool.root"
                f" --imf False {extra_args}")
        else:
            self.command = \
                (f"ATHENA_CORE_NUMBER={threads} Sim_tf.py {threads_argument} --AMIConfig {ID}"
                f" {input_argument} --outputHITSFile myHITS.pool.root"
                f" --imf False {extra_args}")

        self.output_checks = [
            FrozenTier0PolicyCheck(setup, "HITS", 10),
            MetadataCheck(setup, "HITS"),
        ]

        super().__init__(ID, run, type, steps, setup)


class OverlayTest(WorkflowTest):
    """MC overlay workflow test."""

    def __init__(self, ID: str, run: WorkflowRun, type: WorkflowType, steps: List[str], setup: TestSetup, extra_args: str = "") -> None:
        if "maxEvents" not in extra_args:
            extra_args += " --maxEvents 10"

        self.command = \
            (f"Overlay_tf.py --AMIConfig {ID}"
             f" --inputHITSFile {input_HITS[run]} --inputRDO_BKGFile {input_RDO_BKG[run]} --outputRDOFile myRDO.pool.root"
             f" --imf False {extra_args}")

        # skip performance checks for now
        self.skip_performance_checks = True

        self.output_checks = [
            FrozenTier0PolicyCheck(setup, "RDO", 10),
            MetadataCheck(setup, "RDO"),
        ]

        super().__init__(ID, run, type, steps, setup)


class DataOverlayTest(WorkflowTest):
    """Data overlay workflow test."""

    def __init__(self, ID: str, run: WorkflowRun, type: WorkflowType, steps: List[str], setup: TestSetup, extra_args: str = "") -> None:
        if "maxEvents" not in extra_args:
            extra_args += " --maxEvents 10"

        self.command = \
            (f"Overlay_tf.py --AMIConfig {ID}"
             f" --inputHITSFile {input_HITS_data_overlay[run]} --inputBS_SKIMFile {input_BS_SKIM[run]} --outputRDOFile myRDO.pool.root"
             " --triggerConfig 'Overlay=NONE'"  # disable trigger for now
             f" --imf False {extra_args}")

        self.output_checks = [
            FrozenTier0PolicyCheck(setup, "RDO", 10),
            MetadataCheck(setup, "RDO"),
        ]

        super().__init__(ID, run, type, steps, setup)


class PileUpTest(WorkflowTest):
    """Digitization with pile-up workflow test."""

    def __init__(self, ID: str, run: WorkflowRun, type: WorkflowType, steps: List[str], setup: TestSetup, extra_args: str = "") -> None:
        if "maxEvents" not in extra_args:
            extra_args += " --maxEvents 5"

        self.command = \
            (f"Digi_tf.py --AMIConfig {ID} --jobNumber 1 --digiSeedOffset1 1 --digiSeedOffset2 1"
             f" --inputHITSFile {input_HITS_neutrino[run]} --inputHighPtMinbiasHitsFile {input_HITS_minbias_high[run]} --inputLowPtMinbiasHitsFile {input_HITS_minbias_low[run]} --outputRDOFile myRDO.pool.root"
             f" --imf False {extra_args}")

        self.output_checks = [
            FrozenTier0PolicyCheck(setup, "RDO", 5),
            MetadataCheck(setup, "RDO"),
        ]

        super().__init__(ID, run, type, steps, setup)


class DerivationTest(WorkflowTest):
    """Derivations test."""

    def __init__(self, ID: str, run: WorkflowRun, type: WorkflowType, steps: List[str], setup: TestSetup, extra_args: str = "") -> None:
        test_def = ID.split("_")
        data_type = test_def[0].lower()
        formats = [format.upper() for format in test_def[1:-1]]

        threads = 0
        if setup.custom_threads is not None:
            threads = setup.custom_threads

        if "maxEvents" not in extra_args:
            base_events = 100
            events = threads * base_events + 1
            flush = 80

            extra_args += f" --maxEvents {events}"
            format_flush = ", ".join([f"\"DAOD_{format}\": {flush}" for format in formats])
            extra_args += f" --preExec 'flags.Output.TreeAutoFlush={{{format_flush}}}'"
        if "inputAODFile" not in extra_args:
            extra_args += f" --inputAODFile {input_AOD[run][data_type]}"

        # could also use p5503
        self.command = \
            (f"ATHENA_CORE_NUMBER={threads} Derivation_tf.py --CA"
             f" --formats {' '.join(formats)}"
             " --multiprocess --multithreadedFileValidation True"
             " --athenaMPMergeTargetSize 'DAOD_*:0'"
             " --sharedWriter True"
             " --outputDAODFile myOutput.pool.root"
             f" --imf False {extra_args}")

        # skip performance checks for now
        self.skip_performance_checks = True

        self.output_checks = []
        for format in formats:
            if format == "PHYS":
                self.output_checks.append(FrozenTier0PolicyCheck(setup, f"DAOD_{format}", 10))
                self.output_checks.append(MetadataCheck(setup, f"DAOD_{format}"))

        super().__init__(ID, run, type, steps, setup)


class GenerationTest(WorkflowTest):
    """Generation test."""

    def __init__(self, ID: str, run: WorkflowRun, type: WorkflowType, steps: List[str], setup: TestSetup, extra_args: str = "") -> None:
        if "maxEvents" not in extra_args:
            extra_args += " --maxEvents 10"

        if "ecmEnergy" not in extra_args:
            if run is WorkflowRun.Run2:
                extra_args += " --ecmEnergy 13000"
            elif run is WorkflowRun.Run3:
                extra_args += " --ecmEnergy 13600"
            else:
                extra_args += " --ecmEnergy 14000"

        dsid = ID.replace("gen", "")

        self.command = \
            (f"Gen_tf.py --jobConfig {dsid}"
             " --outputEVNTFile myEVNT.pool.root"
             f" --imf False {extra_args}")

        super().__init__(ID, run, type, steps, setup)
