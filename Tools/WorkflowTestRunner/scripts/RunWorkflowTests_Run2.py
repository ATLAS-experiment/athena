#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from sys import exit

from WorkflowTestRunner.ScriptUtils import setup_logger, setup_parser, get_test_setup, get_standard_performance_checks, \
    run_tests, run_checks, run_summary
from WorkflowTestRunner.StandardTests import QTest, GenerationTest, SimulationTest, OverlayTest, DataOverlayTest, PileUpTest, DerivationTest
from WorkflowTestRunner.Test import WorkflowRun, WorkflowType


def main():
    name = "Run2Tests"
    run = WorkflowRun.Run2

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
            tests_to_run.append(SimulationTest("s4005", run, WorkflowType.FullSim, ["EVNTtoHITS"], setup, options.extra_args))
        if not options.workflow or options.workflow is WorkflowType.AF3:
            log.error("AF3 not supported yet")
    elif options.overlay:
        if not options.workflow or options.workflow is WorkflowType.MCOverlay:
            tests_to_run.append(OverlayTest("d1726", run, WorkflowType.MCOverlay, ["Overlay"], setup, options.extra_args))
        if not options.workflow or options.workflow is WorkflowType.DataOverlay:
            tests_to_run.append(DataOverlayTest("d1912", run, WorkflowType.DataOverlay, ["Overlay"], setup, options.extra_args))
    elif options.pileup:
        if setup.parallel_execution:
            log.error("Parallel execution not supported for pile-up workflow")
            exit(1)
        if not options.workflow or options.workflow is WorkflowType.PileUpPresampling:
            tests_to_run.append(PileUpTest("d1918", run, WorkflowType.PileUpPresampling, ["HITtoRDO"], setup, options.extra_args))
        if not options.workflow or options.workflow is WorkflowType.MCPileUpReco:
            tests_to_run.append(QTest("q453", run, WorkflowType.MCPileUpReco, ["Overlay", "RAWtoALL"], setup, options.extra_args))
    elif options.derivation:
        test_id = "MC_PHYS" if not options.ami_tag else options.ami_tag
        test_id = f"{test_id}_{run.value}"
        tests_to_run.append(DerivationTest(test_id, run, WorkflowType.Derivation, ["Derivation"], setup, options.extra_args))
    else:
        if not options.workflow or options.workflow is WorkflowType.MCReco:
            tests_to_run.append(QTest("q452", run, WorkflowType.MCReco, ["HITtoRDO", "RDOtoRDOTrigger", "RAWtoALL"], setup, options.extra_args))
        if not options.workflow or options.workflow is WorkflowType.DataReco:
            tests_to_run.append(QTest("q442", run, WorkflowType.DataReco, ["RAWtoALL", "DQHistogramMerge"], setup, options.extra_args))

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
