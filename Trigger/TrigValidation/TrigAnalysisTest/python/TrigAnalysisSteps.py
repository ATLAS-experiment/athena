#
# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''
Definitions of additional validation steps in Trigger ART tests relevant only for TrigAnalysisTest.
The main common check steps are defined in the TrigValSteering.CheckSteps module.
'''

from TrigValTools.TrigValSteering.CheckSteps import CheckFileStep, InputDependentStep, LogMergeStep

##################################################
# Additional exec steps
##################################################

class TrigDecChecker(InputDependentStep):
    def __init__(self, name='TrigDecChecker', in_file='AOD.pool.root'):
        super().__init__(name)
        self.input_file = in_file
        self.executable = 'dumpTriggerInfo.py'

    def configure(self, test):
        self.args = f' --filesInput {self.input_file}'
        super().configure(test)

class TrigEDMChecker(InputDependentStep):
    def __init__(self, name='TrigEDMChecker', in_file='AOD.pool.root'):
        super().__init__(name)
        self.input_file = in_file
        self.executable = 'trigEDMChecker.py'

    def configure(self, test):
        self.args = f' --filesInput {self.input_file}'
        super().configure(test)


##################################################
# Additional check steps
##################################################

class CheckFileTrigSizeStep(CheckFileStep):
    '''
    Execute checkFileTrigSize.py for POOL files.
    '''
    # TODO: Avoid rerunning checkFile.py even when *checkFile output already produced.
    def __init__(self, name='CheckFileTrigSize'):
        super(CheckFileTrigSizeStep, self).__init__(name)
        self.input_file = 'AOD.pool.root,ESD.pool.root,RDO_TRIG.pool.root,DAOD_PHYS.DAOD.pool.root,DAOD_TLA.pool.root,DAOD_TLAFTAGPEB.pool.root'
        self.executable = 'checkFileTrigSize.py'


##################################################
# Getter functions
##################################################

def trig_analysis_exec_steps(input_file='AOD.pool.root'):
    # TODO: add TrigNavSlimming test
    return [
        TrigDecChecker(in_file=input_file),
        TrigEDMChecker(in_file=input_file)
    ]

def trig_analysis_check_steps():
    return [CheckFileTrigSizeStep()]

def add_analysis_steps(test, input_file='AOD.pool.root'):
    analysis_exec_steps = trig_analysis_exec_steps(input_file)
    test.exec_steps.extend(analysis_exec_steps)
    test.check_steps.extend(trig_analysis_check_steps())

    # Add the analysis exec step logs for merging
    logmerge = test.get_step_by_type(LogMergeStep)
    if not logmerge:
        test.log.warning('LogMerge step not found, cannot add TrigAnalysisSteps exec step log files for merging')
    else:
        for step in analysis_exec_steps:
            logmerge.log_files.append(step.get_log_file_name())
