#
#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#

from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaCommon.Logging import logging


@AccumulatorCache
def TrigDecisionToolCfg(flags):
    '''
    @brief Configures and returns the TrigDecisionTool (TDT) for use in Athena-MT. 
    The TDT uses a Tool interface such that it can be used in either Athena or AnalysisBase releases.
    The manages an MT-safe internal state, hence we should only have one instance of it configured in any job.
    It is thus one of the few places where it OK to use a PublicTool in Athena-MT.

    Obtain the configured public tool instance by calling getPrimary()
    on the ComponentAccumulator returned by this function, or by calling
    getPublicTool('TrigDecisionTool') or getPrimaryAndMerge()
    on any downstream merged ComponentAccumulator.


    When running in AnalysisBase, the tdt.TrigConfigTool='TrigConf::xAODConfigTool' should be used 
    in place of the TrigConf::xAODConfigSvc
    '''
    msg = logging.getLogger('TrigDecisionToolCfg')
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaConfiguration.Enums import Format, Project
    acc = ComponentAccumulator()

    from TrigConfxAOD.TrigConfxAODConfig import getxAODConfigSvc
    cfgsvc = acc.getPrimaryAndMerge(getxAODConfigSvc(flags))

    tdt = CompFactory.Trig.TrigDecisionTool('TrigDecisionTool')
    tdt.TrigConfigSvc = cfgsvc
    use_run3_format = flags.Trigger.EDMVersion >= 3 # or flags.Trigger.doEDMVersionConversion (to be added when this is working)
    tdt.NavigationFormat = 'TrigComposite' if use_run3_format else 'TriggerElement'
    tdt.HLTSummary = getRun3NavigationContainerFromInput(flags)

    if flags.Input.Format is Format.BS and flags.Trigger.EDMVersion in [1, 2]:
        tdt.UseAODDecision = True

    if flags.Common.Project is not Project.AthAnalysis:
        # Full Athena
        # This pre-loads libraries required to read the run 2 trigger navigation
        from TrigEDMConfig.TriggerEDM import EDMLibraries
        nav = CompFactory.HLT.Navigation('Navigation')
        nav.Dlls = [e for e in  EDMLibraries if 'TPCnv' not in e]
        tdt.Navigation = nav
        acc.addPublicTool(nav)

    acc.addPublicTool(tdt, primary=True)

    msg.info('Configuring the TrigDecisionTool and xAODConfigSvc to use ConfigSource: %s, Run3NavigationFormat: %s, Run3NavigationSummaryCollection: %s',
        'InFileMetadata' if flags.Trigger.triggerConfig == 'INFILE' else 'ConditionsAndDetStore',
        str(use_run3_format),
        tdt.HLTSummary)

    return acc

@AccumulatorCache
def getRun3NavigationContainerFromInput(flags):
    from TrigDecisionTool.TrigDecisionToolHelpers import (
        getRun3NavigationContainerFromInput_forAnalysisBase)
    return getRun3NavigationContainerFromInput_forAnalysisBase(flags)


if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.TestDefaults import defaultTestFiles
    import sys

    flags = initConfigFlags()
    if '--RAWRUN2' in sys.argv:
        flags.Input.Files = defaultTestFiles.RAW_RUN2
    else:
        flags.Input.Files = defaultTestFiles.AOD_RUN2_DATA
        #TODO expand the test scope Run3 AODs and RAWs

    flags.lock()
    acc = TrigDecisionToolCfg(flags)
    acc.printConfig(withDetails=True, summariseProps=True, prefix='UnitTest')
    acc.wasMerged()
    # TODO possibly add EDM printing alg using the TDT

