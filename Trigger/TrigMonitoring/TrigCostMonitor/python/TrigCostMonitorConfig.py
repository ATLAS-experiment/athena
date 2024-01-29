#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#


def TrigCostMonitorCfg(flags, seqName=""):
    """
    Component Accumulator based configuration of Trigger Cost Service and associated Auditor
    """
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from AthenaCommon.Logging import logging
    log = logging.getLogger('TrigCostMonitorSetup')

    acc = ComponentAccumulator(seqName) if seqName else ComponentAccumulator()

    if flags.Trigger.CostMonitoring.doCostMonitoring:
      costSvc = CompFactory.TrigCostSvc(
          MonitorAllEvents = flags.Trigger.CostMonitoring.monitorAllEvents,
          SaveHashes = True # This option will go away once the TrigConfigSvc is fully up & running
      )
      acc.addService(costSvc)
      acc.addAuditor(CompFactory.TrigCostAuditor())
      acc.setAppProperty("AuditAlgorithms", True)

      costSupervisorAlg = CompFactory.TrigCostSupervisorAlg()
      costSupervisorAlg.TrigCostSvc = costSvc
      acc.addEventAlgo(costSupervisorAlg)

      log.info('Enabling online trigger cost monitoring')
    else:
      log.info('Will NOT schedule online trigger cost monitoring')
      return ComponentAccumulator()

    return acc


def TrigCostMonitorFinalizeCfg(flags, seqName=""):
    """
    Component Accumulator based configuration of Trigger Cost Finalize Alg
    """
    from TrigEDMConfig.TriggerEDM import recordable
    from AthenaConfiguration.ComponentFactory import CompFactory
    from AthenaCommon.Logging import logging
    log = logging.getLogger('TrigCostMonitorSetup')

    if flags.Trigger.CostMonitoring.doCostMonitoring:
      costFinalizeAlg = CompFactory.TrigCostFinalizeAlg()
      costFinalizeAlg.CostWriteHandleKey = recordable(flags.Trigger.CostMonitoring.outputCollection)

      log.debug('Enabling finalize of online trigger cost monitoring')
      return costFinalizeAlg
    else:
      log.debug('Cost monitoring is not enabled - TrigCostFinalizeAlg will not be included')

    return None
