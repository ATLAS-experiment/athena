#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''
@file CaloMonitoringConfig.py
@brief Python configuration of Calo Monitoring for the Run III
'''

def CaloMonitoringCfg(flags):
    ''' Function to configure Calo Monitoring in the monitoring system for Run III.'''

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    acc = ComponentAccumulator()

    from AthenaCommon.Logging import logging
    msg = logging.getLogger( 'CaloMonitoringCfg' )

    environment = flags.DQ.Environment

    if environment in ('online', 'tier0', 'tier0ESD'):
        msg.info('Setup Calo Monitoring for ESD data due to environment: %s', environment)

        from CaloMonitoring.TileCalCellMonAlg import TileCalCellMonAlgConfig
        acc.merge( TileCalCellMonAlgConfig(flags) )

        from CaloMonitoring.LArCellMonAlg import LArCellMonConfig
        acc.merge( LArCellMonConfig(flags) )

        from CaloMonitoring.CaloClusterCellMonAlg import CaloClusterCellMonConfig
        acc.merge( CaloClusterCellMonConfig(flags) )

        # FIXME could not be included yet, some trigger configurations are missing
        #from CaloMonitoring.CaloBaselineMonAlg import CaloBaselineMonConfig
        #acc.merge( CaloBaselineMonConfig(flags,False) )

    return acc



if __name__=='__main__':

   from AthenaConfiguration.AllConfigFlags import initConfigFlags
   from AthenaConfiguration.TestDefaults import defaultTestFiles
   flags = initConfigFlags()
   flags.Input.Files = defaultTestFiles.ESD

   flags.Output.HISTFileName = 'CaloMonitoringOutput.root'
   flags.DQ.enableLumiAccess = True
   flags.DQ.useTrigger = False
   flags.DQ.Environment = 'tier0'

   flags.lock()

   # Initialize configuration object, add accumulator, merge, and run.
   from AthenaConfiguration.MainServicesConfig import MainServicesCfg
   from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
   acc = MainServicesCfg(flags)
   acc.merge(PoolReadCfg(flags))

   acc.merge( CaloMonitoringCfg(flags) )

   acc.printConfig(withDetails = True, summariseProps = True)
   flags.dump()
   acc.store(open("CaloMonitoring.pkl","wb"))

   sc = acc.run(maxEvents = 3)
   import sys
   # Success should be 0
   sys.exit(not sc.isSuccess())
