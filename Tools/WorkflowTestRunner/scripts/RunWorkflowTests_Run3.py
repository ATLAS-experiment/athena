#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from sys import exit

from WorkflowTestRunner.ScriptUtils import setup_logger, setup_parser, get_test_setup, get_standard_performance_checks, \
    run_tests, run_checks, run_summary
from WorkflowTestRunner.StandardTests import DataOverlayPreparationTest, DerivationTest, GenerationTest, OverlayTest, DataOverlayTest, PileUpTest, QTest, SimulationTest
from WorkflowTestRunner.Test import WorkflowRun, WorkflowType


def main():
    name = "Run3Tests"
    run = WorkflowRun.Run3

    # Setup the environment
    log = setup_logger(name)
    parser = setup_parser()
    options = parser.parse_args()
    setup = get_test_setup(name, options, log)

    # Define which tests to run
    tests_to_run = []
    if options.generation:
        dsid = "421356" if not options.dsid else options.dsid
        tests_to_run.append(GenerationTest(f"gen{dsid}", run, WorkflowType.Generation, ["generate"], setup, options.extra_args))
    elif options.simulation:
        if not options.workflow or options.workflow is WorkflowType.FullSim:
            ami_tag = "s4006" if not options.ami_tag else options.ami_tag
            tests_to_run.append(SimulationTest(ami_tag, run, WorkflowType.FullSim, ["EVNTtoHITS"], setup, options.extra_args + " --conditionsTag 'default:OFLCOND-MC21-SDR-RUN3-07' --geometryVersion 'default:ATLAS-R3S-2021-03-02-00'"))
        if options.workflow is WorkflowType.AF3:
            ami_tag = "a913" if not options.ami_tag else options.ami_tag
            tests_to_run.append(SimulationTest(ami_tag, run, WorkflowType.AF3, ["EVNTtoHITS"], setup, options.extra_args))
        if options.workflow is WorkflowType.HitsMerge:
            ami_tag = "s4007" if not options.ami_tag else options.ami_tag
            tests_to_run.append(SimulationTest(ami_tag, run, WorkflowType.HitsMerge, ["HITSMerge"], setup, options.extra_args))
        if options.workflow is WorkflowType.HitsFilter:
            ami_tag = "s4008" if not options.ami_tag else options.ami_tag
            tests_to_run.append(SimulationTest(ami_tag, run, WorkflowType.HitsFilter, ["FilterHitTf"], setup, options.extra_args))
    elif options.overlay:
        if not options.workflow or options.workflow is WorkflowType.MCOverlay:
            ami_tag = "d1759" if not options.ami_tag else options.ami_tag
            tests_to_run.append(OverlayTest(ami_tag, run, WorkflowType.MCOverlay, ["Overlay"], setup, options.extra_args + " --runNumber 601229 --conditionsTag 'default:OFLCOND-MC23-SDR-RUN3-08'"))
        if not options.workflow or options.workflow is WorkflowType.DataOverlay:
            ami_tag = "d2029" if not options.ami_tag else options.ami_tag
            tests_to_run.append(DataOverlayTest(ami_tag, run, WorkflowType.DataOverlay, ["Overlay"], setup, options.extra_args + " --runNumber 601229 --postInclude \"default:PyJobTransforms.UseFrontier\""))
        if not options.workflow or options.workflow is WorkflowType.DataOverlayChain:
            ami_tag = "d2030" if not options.ami_tag else options.ami_tag
            tests_to_run.append(DataOverlayTest(ami_tag, run, WorkflowType.DataOverlayChain, ["EVNTtoRDO"], setup, options.extra_args + " --runNumber 603398 --postInclude \"default:PyJobTransforms.UseFrontier\""))
    elif options.pileup:
        if setup.parallel_execution:
            log.error("Parallel execution not supported for pile-up workflow")
            exit(1)
        if not options.workflow or options.workflow is WorkflowType.PileUpPresampling:
            ami_tag = "d1919" if not options.ami_tag else options.ami_tag
            tests_to_run.append(PileUpTest(ami_tag, run, WorkflowType.PileUpPresampling, ["HITtoRDO"], setup, options.extra_args))
        if not options.workflow or options.workflow is WorkflowType.MCPileUpReco:
            tests_to_run.append(QTest("q455", run, WorkflowType.MCPileUpReco, ["Overlay", "RDOtoRDOTrigger", "RAWtoALL"], setup, options.extra_args))
        if options.workflow is WorkflowType.MinbiasPreprocessing:
            ami_tag = "d2008" if not options.ami_tag else options.ami_tag
            tests_to_run.append(DataOverlayPreparationTest(ami_tag, run, WorkflowType.MinbiasPreprocessing, ["BStoRDO"], setup, options.extra_args))
    elif options.derivation:
        test_id = "MC_PHYS" if not options.ami_tag else options.ami_tag
        test_id = f"{test_id}_{run.value}"
        tests_to_run.append(DerivationTest(test_id, run, WorkflowType.Derivation, ["Derivation"], setup, options.extra_args))
    else:
        if not options.workflow or options.workflow is WorkflowType.MCReco:
            ami_tag = "q454" if not options.ami_tag else options.ami_tag
            tests_to_run.append(QTest(ami_tag, run, WorkflowType.MCReco, ["HITtoRDO", "RDOtoRDOTrigger", "RAWtoALL"], setup, options.extra_args))
        if not options.workflow or options.workflow is WorkflowType.DataReco:
            ami_tag = "q449" if not options.ami_tag else options.ami_tag
            tests_to_run.append(QTest(ami_tag, run, WorkflowType.DataReco, ["RAWtoALL", "DQHistogramMerge"], setup, options.extra_args))
        if options.workflow is WorkflowType.DataOverlayReco:
            ami_tag = "q458" if not options.ami_tag else options.ami_tag
            tests_to_run.append(QTest(ami_tag, run, WorkflowType.DataOverlayReco, ["RAWtoALL"], setup, options.extra_args))

    # Define which perfomance checks to run
    performance_checks = get_standard_performance_checks(setup)

    # Define and run jobs
    run_tests(setup, tests_to_run)

    # Run post-processing checks
    all_passed = run_checks(setup, tests_to_run, performance_checks)

    # final report
    run_summary(setup, tests_to_run, all_passed)


if __name__ == "__main__":
    main()

