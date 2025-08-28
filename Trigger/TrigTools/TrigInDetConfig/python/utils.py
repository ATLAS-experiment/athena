#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#
#           Helper methods for configuration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
import logging


def getFlagsForActiveConfig(
    flags: AthConfigFlags, config_name: str, log: logging.Logger) -> AthConfigFlags:
    
    """Get the flags for the named config, ensure that they are set to be active

    Parameters
    ----------
    flags : AthConfigFlags
        The instance of the flags to check
    config_name : str
        The name of the desired tracking config
    log : logging.Logger
        Logger to print related messages

    Returns
    -------
    Either the current flags instance if all the ActiveConfig is correct or a new
    version with cloned flags
    
    the flags correspond to InDet/ITk format
    """

    if flags.hasFlag("Tracking.ActiveConfig.input_name"):
        if flags.Tracking.ActiveConfig.input_name == config_name:
            log.debug(
                "flags.Tracking.ActiveConfig is for %s",
                flags.Tracking.ActiveConfig.input_name,
            )
            return flags
        else:
            log.info(
                "flags.Tracking.ActiveConfig is not for %s but %s",
                config_name,
                flags.Tracking.ActiveConfig.input_name,
            )
    else:

        log.info(
            "Menu code invoked ID config without flags.Tracking.ActiveConfig for %s",
            config_name,
        )

    return _cloneFlagsToActiveConfig(flags, config_name)


def cloneFlagsToActiveConfig(
        flags: AthConfigFlags, config_name: str, log: logging.Logger) -> AthConfigFlags:

    """
    InDet/ITk specific clone and replace of ActiveConfig without checking flags vs config_name
    
    this function should be used only high up in the menu creation where a context of tracking flags 
    does not exist yet and is created for the first time in generateChainConfigs function
    or there are multiple contexts for ActiveConfig like in LRT

    in other cases getFlagsForActiveConfig should be used instead
    
    """
    
    log.info(f"Cloning tracking config for {config_name} to flags.Tracking.ActiveConfig")
    return _cloneFlagsToActiveConfig(flags, config_name)

def _cloneFlagsToActiveConfig(flags: AthConfigFlags, config_name: str) -> AthConfigFlags:

    prefix = "Trigger.InDetTracking."
    if flags.Detector.GeometryITk:
      prefix = "Trigger.ITkTracking."
      if flags.Trigger.useActsTracking: 
        prefix = "Trigger.ActsTracking."
      
    return flags.cloneAndReplace(
        "Tracking.ActiveConfig",
        prefix + config_name,
        keepOriginal = True
    )
