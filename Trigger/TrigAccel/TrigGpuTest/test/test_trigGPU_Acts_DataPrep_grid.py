#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# art-description: Trigger GPU test on data
# art-type: grid
# art-include: main/Athena
# art-input-nfiles: 1
# art-athena-mt: 8
# art-architecture: '#&nvidia'
# art-output: *.txt
# art-output: *.log
# art-output: log.*
# art-output: *.out
# art-output: *.err
# art-output: *.log.tar.gz
# art-output: *.new
# art-output: *.json
# art-output: expert-monitoring.root
# art-output: rootcomp.root
# art-output: *.pmon.gz
# art-output: *perfmon*
# art-output: prmon*
# art-output: *.check*

import os
import re
#Some traccc files still in dev area
os.environ["PATHRESOLVER_DEVAREARESPONSE"] = "WARNING"

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps
from TrigValTools.TrigValSteering.Step import Step
from AthenaConfiguration.TestDefaults import defaultConditionsTags


class ClusterDiscrepancyStep(Step):
    '''
    Check the matching between GPU (traccc) and CPU (ACTS) clusters, based on
    the ValSum summary printed by ActsClusterComparisonAlg:
     - pixel: no unmatched clusters and no position differences
     - strip: no unmatched clusters and no position difference > 1 sigma
     - strip barrel: no position difference > 0.25 sigma
     - strip endcap: fraction of matched strip clusters with position
       difference > 0.5 (0.25) sigma below ec_max_frac_0p5sig (ec_max_frac_0p25sig)
    '''

    def __init__(self, name='ClusterDiscrepancy'):
        super(ClusterDiscrepancyStep, self).__init__(name)
        self.input_file = 'athena.log'
        self.alg_name = 'GPU_ActsClusterComparisonAlg'
        self.ec_max_frac_0p5sig = 0.02
        self.ec_max_frac_0p25sig = 0.25
        self.auto_report_result = True
        self.required = True
        self.depends_on_exec = True

    @staticmethod
    def _key(label):
        if 'unmatched' in label:
            return 'unmatched'
        if 'matched' in label:
            return 'matched'
        if 'barrel/EC' in label:
            return 'diff_0p25sig_barrel_ec'
        for sig, key in (('> 1 sigma', 'diff_1sig'), ('> 0.5 sigma', 'diff_0p5sig'), ('> 0.25 sigma', 'diff_0p25sig')):
            if sig in label:
                return key
        return None

    def parse(self):
        '''Returns {section: {key: [counts]}}, ignoring the "(x%)" fractions'''
        summary = {}
        section = None
        with open(self.input_file, encoding='utf-8') as f_in:
            for line in f_in:
                if self.alg_name not in line or 'ValSum' not in line:
                    continue
                text = line.split('ValSum', 1)[1].strip()
                if 'PIXEL CLUSTER' in text:
                    section = summary.setdefault('pixel', {})
                elif 'STRIP CLUSTER' in text:
                    section = summary.setdefault('strip', {})
                elif 'SPACEPOINT' in text:
                    section = None
                elif section is not None and ':' in text:
                    label, values = text.rsplit(':', 1)
                    key = self._key(label)
                    if key is not None:
                        values = re.sub(r'\(.*?\)', '', values)
                        section[key] = [int(v) for v in re.findall(r'\d+', values)]
                        self.log.info('%s', text)
        return summary

    def check(self, summary):
        failures = []

        def require(cond, msg):
            if not cond:
                failures.append(msg)

        def get(section, key, n=1):
            values = summary.get(section, {}).get(key)
            if values is None or len(values) != n:
                failures.append('{}: "{}" missing from summary'.format(section, key))
                return [0] * n
            return values

        for section in ('pixel', 'strip'):
            matched, = get(section, 'matched')
            require(matched > 0, '{}: no matched clusters'.format(section))
            unmatched = get(section, 'unmatched', 2)
            require(not any(unmatched), '{}: unmatched clusters mon/ref = {} / {}'.format(section, *unmatched))
            diff_1sig, = get(section, 'diff_1sig')
            require(diff_1sig == 0, '{}: {} clusters with pos diff > 1 sigma'.format(section, diff_1sig))

        for key in ('diff_0p5sig', 'diff_0p25sig'):
            diff, = get('pixel', key)
            require(diff == 0, 'pixel: {} clusters with {}'.format(diff, key))

        strip_matched, = get('strip', 'matched')
        barrel, ec = get('strip', 'diff_0p25sig_barrel_ec', 2)
        require(barrel == 0, 'strip barrel: {} clusters with pos diff > 0.25 sigma'.format(barrel))
        # Barrel has no clusters > 0.25 sigma, so the strip totals are all from the endcap
        for key, max_frac in (('diff_0p5sig', self.ec_max_frac_0p5sig), ('diff_0p25sig', self.ec_max_frac_0p25sig)):
            diff, = get('strip', key)
            frac = diff / strip_matched if strip_matched > 0 else 0.
            self.log.info('strip endcap: %s = %d (%.3f%% of matched, max %.3f%%)', key, diff, 100. * frac, 100. * max_frac)
            require(frac <= max_frac, 'strip endcap: {} = {} ({:.3f}% of matched) exceeds {:.3f}%'.format(
                key, diff, 100. * frac, 100. * max_frac))
        return failures

    def run(self, dry_run=False):
        self.log.info('Running %s step on %s', self.name, self.input_file)
        if dry_run:
            self.result = 0
            return self.result, '# (internal) {}'.format(self.name)

        if not os.path.isfile(self.input_file):
            failures = ['input file {} is missing'.format(self.input_file)]
        else:
            failures = self.check(self.parse())

        for failure in failures:
            self.log.error('%s: %s', self.name, failure)
        self.result = 1 if failures else 0
        if self.auto_report_result:
            self.report_result()
        return self.result, '# (internal) {} -> {}'.format(self.name, 'failed' if failures else 'ok')


ex = ExecStep.ExecStep()
ex.type = 'athena'
ex.input = 'ttbar_pu200_Run4'
ex.max_events = 1
ex.threads = 8
ex.job_options = 'ActsGPUDataPreparation/ActsDeviceClusterizationTest.py'
ex.flags = [f'IOVDb.GlobalTag="{defaultConditionsTags.RUN4_MC}"']
test = Test.Test()
test.art_type = 'grid'
test.exec_steps = [ex]
# Compare to reference
refcomp = CheckSteps.RegTestStep('RegTest')
refcomp.regex = 'GPU_ActsClusterComparisonAlg.*INFO.*ValSum'
refcomp.reference = 'TrigGpuTest/test_trigGPU_ActsClusterComparison.ref'
refcomp.required = False             # Informational, matching is enforced below

# GPU and CPU clusters must match, with tolerance only for endcap strip positions
clustercheck = ClusterDiscrepancyStep()  # Final exit code depends on this step

test.check_steps = CheckSteps.default_check_steps(test)
test.check_steps.append(refcomp)
test.check_steps.append(clustercheck)

import sys
sys.exit(test.run())
