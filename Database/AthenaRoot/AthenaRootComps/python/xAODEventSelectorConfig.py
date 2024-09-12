# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# @file xAODHybridSelectorConfig
# @purpose make the Athena framework read a set of xAOD files to emulate the
#          usual TEvent event loop ... BUT READ METADATA WITH POOL!
#          Converted from ReadAthenaxAODHybrid.py
# @author Teng Jian Khoo
#

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon import Logging
from AthenaCommon import Constants
from AthenaServices.MetaDataSvcConfig import MetaDataSvcCfg
from enum import IntEnum

# Duplicate here because it's probably more efficient than importing from ROOT
class xAODAccessMode(IntEnum):
    BRANCH_ACCESS = 0
    CLASS_ACCESS = 1
    ATHENA_ACCESS = 2

# Default to the most efficient access method
def xAODReadCfg(flags, AccessMode=xAODAccessMode.CLASS_ACCESS):
    """
    Creates a ComponentAccumulator instance containing the 
    athena services required for xAOD file reading
    """

    msg = Logging.logging.getLogger( 'ReadAthenaxAODHybrid' )
    msg.debug("Configuring Athena for reading xAOD files (via TEvent, with POOL for Metadata)...")


    result=ComponentAccumulator()
    
    result.addService(CompFactory.EvtPersistencySvc("EventPersistencySvc",)) #No service handle yet???
    result.merge(MetaDataSvcCfg(flags))

    result.addService(CompFactory.StoreGateSvc("MetaDataStore"))
    # Suppress some uninformative messages
    result.addService(CompFactory.PoolSvc("PoolSvc",OutputLevel=Constants.WARNING))

    # result.addService(CompFactory.THistSvc())
    result.addService(CompFactory.Athena.xAODCnvSvc())

    result.addService(CompFactory.ProxyProviderSvc("ProxyProviderSvc",ProviderNames=[ "MetaDataSvc"]))
    result.addService(
        CompFactory.Athena.xAODEventSelector(
            name='EventSelector',
            InputCollections=flags.Input.Files,
            SkipEvents=flags.Exec.SkipEvents,
            AccessMode=AccessMode,
            ReadMetaDataWithPool=True,
            printEventProxyWarnings=False,
            ))
    evSel = result.getService("EventSelector")

    result.setAppProperty("EvtSel",evSel.getFullJobOptName())

    msg.debug("Configuring Athena for reading ROOT files (via TEvent, with POOL for Metadata)... [OK]")

    return result



if __name__=="__main__":
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    import os
    asg_test_file_mc = os.environ['ASG_TEST_FILE_MC']
    flags = initConfigFlags()
    flags.Input.Files=[asg_test_file_mc]

    import sys
    if len(sys.argv)>1:
        flags.Input.Files=[sys.argv[1]]
    flags.lock()

    AccessMode = xAODAccessMode.CLASS_ACCESS
    if len(sys.argv)>2:
        AccessMode = xAODAccessMode(int(sys.argv[2]))

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(xAODReadCfg(flags,AccessMode))

    cfg.run(10)
