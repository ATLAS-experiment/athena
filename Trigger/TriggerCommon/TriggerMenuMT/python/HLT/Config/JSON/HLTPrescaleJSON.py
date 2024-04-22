# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import json
from TrigConfigSvc.TrigConfigSvcCfg import getHLTPrescalesSetFileName
from AthenaCommon.Logging import logging
__log = logging.getLogger( __name__ )

def generatePrescaleJSON(flags, chainDicts):
    """ Generates JSON given the ChainProps and sequences
    """
    # Prescale dictionary that is used to create the JSON content
    prescaleDict = {
        "filetype": "hltprescale",
        "name": flags.Trigger.triggerMenuSetup,
        "prescales": {}
    }
    # Sort chains by name not counter for easier post processing
    for chain in sorted(chainDicts, key=lambda d: d['chainName']):
        # Enabled flag is what determines disabling (here is based on prescale list from MenuPrescaleConfig)
        chainEnabled = True
        if (int(chain["prescale"]) <= 0):
            chainEnabled = False

        # Now have all information to write the chain to the prescale dictionary
        chainName = chain["chainName"]
        prescaleDict["prescales"][chainName] = {
            "hash": chain["chainNameHash"],
            "prescale": chain["prescale"],
            "enabled": chainEnabled
        }
        # Add express stream information
        if "express" in chain["stream"]:
            prescaleDict["prescales"][chainName]['prescale_express'] = chain["prescale"]
            prescaleDict["prescales"][chainName]['enabled_express'] = chainEnabled

    # Prescale dictionary now completed, write to JSON
    fileName = getHLTPrescalesSetFileName(flags)
    __log.info( "Writing HLT Prescale JSON to %s", fileName )
    with open( fileName, 'w' ) as fp:
        json.dump( prescaleDict, fp, indent=4, sort_keys=False )

