#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from sys import exit

from WorkflowTestRunner.ScriptUtils import setup_logger, setup_parser, get_test_setup, \
    run_tests, run_checks, run_summary
from WorkflowTestRunner.StandardTests import DerivationTest, GenerationTest, PileUpTest, QTest, SimulationTest
from WorkflowTestRunner.Test import WorkflowRun, WorkflowType
from AthenaConfiguration.TestDefaults import defaultGeometryTags, defaultConditionsTags

def main():
    name = "RunUpgradeTests"
    run = WorkflowRun.Run4

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
        tests_to_run.append(SimulationTest("s3761", run, WorkflowType.FullSim, ["EVNTtoHITS"], setup, f"{options.extra_args}  --geometryVersion {defaultGeometryTags.RUN4}") )
    elif options.overlay:
        log.error("Overlay not supported yet")
        exit(1)
    elif options.pileup:
        if setup.parallel_execution:
            log.error("Parallel execution not supported for pile-up workflow")
            exit(1)
        if not options.workflow or options.workflow is WorkflowType.PileUpPresampling:
            tests_to_run.append(PileUpTest("d1920", run, WorkflowType.PileUpPresampling, ["HITtoRDO"], setup, f"{options.extra_args} --digiSteeringConf StandardInTimeOnlyTruth --geometryVersion {defaultGeometryTags.RUN4} --conditionsTag default:{defaultConditionsTags.RUN4_MC}"))
        if not options.workflow or options.workflow is WorkflowType.MCPileUpReco:
            tests_to_run.append(QTest("q456", run, WorkflowType.MCPileUpReco, ["Overlay", "RAWtoALL"], setup, options.extra_args))
    elif options.reco:
        tests_to_run.append(QTest("q447", run, WorkflowType.MCReco, ["HITtoRDO", "RAWtoALL"], setup, f"{options.extra_args} --geometryVersion {defaultGeometryTags.RUN4}"))
    elif options.derivation:
        test_id = "MC_PHYS" if not options.ami_tag else options.ami_tag
        test_id = f"{test_id}_{run.value}"
        tests_to_run.append(DerivationTest(test_id, run, WorkflowType.Derivation, ["Derivation"], setup, options.extra_args))
    else:
        if setup.parallel_execution:
            log.error("Parallel execution not supported for the default Phase-II workflow")
            exit(1)
        tests_to_run.append(SimulationTest("s3761", run, WorkflowType.FullSim, ["EVNTtoHITS"], setup, f"{options.extra_args}  --geometryVersion {defaultGeometryTags.RUN4}"))
        tests_to_run.append(QTest("q447", run, WorkflowType.MCReco, ["HITtoRDO", "RAWtoALL"], setup, f"{options.extra_args} --geometryVersion {defaultGeometryTags.RUN4} --inputHITSFile ../run_s3761/myHITS.pool.root"))

    # Define which perfomance checks to run
    # TODO: standard performance checks do not work, disable for now
    # performance_checks = get_standard_performance_checks(setup)
    performance_checks = []

    # Define and run jobs
    run_tests(setup, tests_to_run)

    # Run post-processing checks
    all_passed = run_checks(setup, tests_to_run, performance_checks)

    # final report
    run_summary(setup, tests_to_run, all_passed)


if __name__ == "__main__":
    main()
