# Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration

## @file AthenaPool_jobOptions.py
## @brief Core job options file for AthenaPOOL to setup the basic sercives.
## @author Peter van Gemmeren <gemmeren@anl.gov>
###############################################################
#
# The core jobOptions for AthenaPOOL
# Setup the basic sercives
#
#==============================================================
#
# Required libs:

def _loadBasicAthenaPool():
    """Loads the basic services for AthenaPool"""

    from AthenaCommon import CfgMgr
    ## make sure we have been -at least- minimally correctly configured
    import AthenaCommon.AtlasUnixStandardJob  # noqa: F401

    from AthenaCommon.Logging import logging
    from AthenaCommon.AppMgr  import ServiceMgr as svcMgr

    msg = logging.getLogger( 'loadBasicAthenaPool' )
    msg.debug( "Loading basic services for AthenaPool..." )

    if not hasattr (svcMgr, 'PoolSvc'):
        svcMgr += CfgMgr.PoolSvc()
    svcMgr.PoolSvc.MaxFilesOpen = 0

    svcMgr += CfgMgr.AthenaPoolCnvSvc()

    if not hasattr (svcMgr, 'EventPersistencySvc'):
        svcMgr += CfgMgr.EvtPersistencySvc( "EventPersistencySvc" )
    svcMgr.EventPersistencySvc.CnvServices += [ "AthenaPoolCnvSvc" ]
    if not hasattr (svcMgr, 'ProxyProviderSvc'):
        svcMgr += CfgMgr.ProxyProviderSvc()

    # Add in MetaDataSvc
    svcMgr += CfgMgr.MetaDataSvc( "MetaDataSvc" )

    # Make MetaDataSvc an AddressProvider
    svcMgr.ProxyProviderSvc.ProviderNames += [ "MetaDataSvc" ]
    if not hasattr (svcMgr, 'MetaDataStore'):
        svcMgr += CfgMgr.StoreGateSvc( "MetaDataStore" )
 
    msg.debug( "Loading basic services for AthenaPool... [DONE]" )
    return

## load basic services configuration at module import
_loadBasicAthenaPool()

## clean-up: avoid running multiple times this method
del _loadBasicAthenaPool

