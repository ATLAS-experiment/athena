# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags
from AthenaCommon.Logging import logging

msg = logging.getLogger('BFieldConfigFlags')


#So far no attempt to auto-config field for MC or online-running
#
#The old-style config did auto-config the field for online based on IS
#(see https://gitlab.cern.ch/atlas/athena/-/blob/1802605a4ab69cab7ee3e53d75f162c7da99a944/Reconstruction/RecExample/RecExOnline/python/OnlineISConfiguration.py#L52)
#
#The old-sytle config tried to auto-config the field based on in-file metadata, falling back to Geometry and Conditions tags. See  
#(see https://gitlab.cern.ch/atlas/athena/-/blob/1802605a4ab69cab7ee3e53d75f162c7da99a944/Reconstruction/RecExample/RecExConfig/python/AutoConfiguration.py#L181

def _fieldAutoCfg(prevFlags):
    if prevFlags.Input.isMC or prevFlags.Common.isOnline:
        return [True,True]
    
    from CoolConvUtilities.MagFieldUtils import getFieldForRun
    lbs=prevFlags.Input.LumiBlockNumbers
    fieldStat=getFieldForRun(run=prevFlags.Input.RunNumbers[0],lumiblock=0 if len(lbs)==0 else lbs[0],quiet=True)
    if fieldStat is None:
        msg.error("Unable to get field status from DCS, assume both magnets ON")
        return [True,True]
    return [fieldStat.solenoidCurrent()>1, fieldStat.toroidCurrent()>1]

    
                              
def createBFieldConfigFlags(): 
    bcf=AthConfigFlags()
    # True when solenoid is on
    bcf.addFlag("BField.solenoidOn", lambda prevFlags : _fieldAutoCfg(prevFlags)[0])
    # True when barrel toroid is on
    bcf.addFlag("BField.barrelToroidOn", lambda prevFlags : _fieldAutoCfg(prevFlags)[1])
    # True when endcap toroid is on
    bcf.addFlag("BField.endcapToroidOn", lambda prevFlags : _fieldAutoCfg(prevFlags)[1])
    return bcf
