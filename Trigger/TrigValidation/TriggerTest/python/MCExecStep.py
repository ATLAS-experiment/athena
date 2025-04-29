#
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#

'''
Specialisation of ExecStep for MC production tests
'''


from TrigValTools.TrigValSteering.ExecStep import ExecStep
from TrigValTools.TrigMCCommonParams import mcDefaults

class MCExecStep(ExecStep):
    '''
    Provides standard default settings for the MC test execution
    '''

    def __init__(
            self, name=None, menu=None, signatures=None,
            global_tag=mcDefaults.global_tag, mc_campaign=mcDefaults.mc_campaign
        ):
        super(MCExecStep, self).__init__(name)

        assert menu is not None, "Menu must be supplied to MCExecStep"

        self.type = 'athena'
        self.job_options = 'TriggerJobOpts/runHLT.py'


        self.flags = [
            f'Trigger.triggerMenuSetup="{menu}"',
            f'IOVDb.GlobalTag="{global_tag}"'
            ]
        self.args += f'--preInclude "{mc_campaign}"'

        if signatures is not None:
            enabled_signatures_str = ','.join([f'\\\"{sig}\\\"' for sig in signatures])
            self.flags.append(f'Trigger.enabledSignatures=[{enabled_signatures_str}]')

class MCBuildStep(MCExecStep):
    '''
    Provides standard default settings for the MC test execution
    '''

    def __init__(
            self, name=None, menu=None, signatures=None,
            global_tag=mcDefaults.global_tag, mc_campaign=mcDefaults.mc_campaign
        ):
        super(MCBuildStep, self).__init__(name, menu, signatures, global_tag, mc_campaign)
        self.threads = 1

class MCGridStep(MCExecStep):
    '''
    Provides standard default settings for the MC test execution
    '''

    def __init__(
            self, name=None, menu=None, signatures=None,
            global_tag=mcDefaults.global_tag, mc_campaign=mcDefaults.mc_campaign,
        ):
        super(MCGridStep, self).__init__(name, menu, signatures, global_tag, mc_campaign)
        self.threads = 8
        self.concurrent_events = 8
