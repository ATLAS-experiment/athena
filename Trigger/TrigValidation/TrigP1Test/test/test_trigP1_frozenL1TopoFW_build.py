#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# art-description: Frozen L1Topo Firmware test -- generates the L1 menu and then checks for changes implying L1Topo FW changes
# art-type: build
# art-include: main/Athena
# art-include: 24.0/Athena

from TrigValTools.TrigValSteering import Test, ExecStep, CheckSteps

#====================================================================================================
# Run Physics menu and produce files to create SMK

menu = ExecStep.ExecStep('GenL1Menu')
menu.type = 'other'
menu.executable = 'generateL1MenuRun3.py PhysicsP1_pp_run3_v1'
menu.input = ''

#====================================================================================================
# Git clone topoconverterph1 package

topocnvclone = ExecStep.ExecStep('CloneTopoConverter')
topocnvclone.type = 'other'
# On updates to the topoconverterph1 package, update the branch here
topocnvclone.executable = 'git clone --depth 1 --branch art-2024-00-00 https://:@gitlab.cern.ch:8443/atlas-l1calo/l1topo/topoconverterph1.git'
topocnvclone.input = ''

#====================================================================================================
# Run topoconverter L1 menu

class DiffL1MenuStep(CheckSteps.RefComparisonStep):
    def __init__(self,name):
        super(DiffL1MenuStep, self).__init__(name)
        self.executable = 'topoconverterph1/diffL1Menu.py'
        self.input_file = None

    def configure(self, test):
        if self.reference is None:
            self.log.error('Missing reference for %s', self.name)
        if self.input_file is None:
            self.log.error('Missing input for %s', self.name)

        super(DiffL1MenuStep,self).configure(test)
        self.args += ' {} {}'.format(self.input_file, self.reference)

    def run(self, dry_run=False):
        retcode, cmd = super(DiffL1MenuStep, self).run(dry_run)
        # Permit return code 1 here, as this indicates parameter changes
        # which are transparent to the firmware.
        retcode = retcode if retcode >=2 else 0
        return retcode, cmd

# Hosted on CVMFS mirrored from as the ref file is sizeable.
# Replacements should be placed in /eos/atlas/atlascerngroupdisk/data-art/grid-input/TrigP1Test
# See https://twiki.cern.ch/twiki/bin/view/AtlasComputing/ART#Inputs_read_from_CVMFS
refdir = '/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/TrigP1Test'

diffl1 = DiffL1MenuStep('DiffL1Menu')
diffl1.reference = f'{refdir}/L1Menu_PhysicsP1_pp_run3_v1.json.ref'
diffl1.input_file = 'L1Menu_PhysicsP1_pp_run3_v1_*.json'
diffl1.required = True


#====================================================================================================
# The full test

test = Test.Test()
test.art_type = 'build'
test.exec_steps = [menu,topocnvclone]
test.check_steps = [diffl1]

import sys
sys.exit(test.run())
