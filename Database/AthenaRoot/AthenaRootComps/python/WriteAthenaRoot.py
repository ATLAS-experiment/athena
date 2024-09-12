# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# @file AthenaRootComps/python/WriteAthenaRoot.py
# @purpose make the Athena framework write a set of ROOT files
# @author Sebastien Binet <binet@cern.ch>
# 

## import the default class to create output streams
from AthenaRootComps.OutputStreamAthenaRoot import createNtupleOutputStream

## export the default class to create output streams
__all__ = [ 'createNtupleOutputStream' ]

def _configureWriteAthenaRoot():
    """ Helper method to configure Athena to write out ROOT files """

    from AthenaCommon.Logging import logging
    msg = logging.getLogger( 'configureWriteAthenaRoot' )
    msg.debug( "Configuring Athena for writing ROOT files..." )

    # Load the basic services
    import AthenaRootComps.AthenaRootBase  # noqa: F401

    msg.debug( "Configuring Athena for writing ROOT files... [DONE]" )
    return

## configuration at module import
_configureWriteAthenaRoot()

## clean-up: avoid running multiple times this method
del _configureWriteAthenaRoot
