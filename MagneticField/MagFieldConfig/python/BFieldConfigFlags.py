# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.AthConfigFlags import AthConfigFlags



#So far no attempt to auto-config field for MC or online-running
#
#The old-style config did auto-config the field for online based on IS
#(see https://gitlab.cern.ch/atlas/athena/-/blame/1802605a4ab69cab7ee3e53d75f162c7da99a944/Reconstruction/RecExample/RecExOnline/python/OnlineISConfiguration.py#L52)
#
#The old-sytle config tried to auto-config the field based on in-file metadata, falling back to Geometry and Conditions tags. See  
#(see https://gitlab.cern.ch/atlas/athena/-/blame/1802605a4ab69cab7ee3e53d75f162c7da99a944/Reconstruction/RecExample/RecExConfig/python/AutoConfiguration.py#L181

def _toroidFieldAutoCfg(prevFlags):
    if prevFlags.Input.isMC or prevFlags.Common.isOnline:
        return True
    
    from CoolConvUtilities.MagFieldUtils import getFieldForRun
    lbs=prevFlags.Input.LumiBlockNumbers
    fieldStat=getFieldForRun(run=prevFlags.Input.RunNumbers[0],lumiblock=0 if len(lbs)==0 else lbs[0],quiet=True)
    return (fieldStat.toroidCurrent()>1)

def _solenoidFieldAutoCfg(prevFlags):
    if prevFlags.Input.isMC or prevFlags.Common.isOnline:
        return True

    from CoolConvUtilities.MagFieldUtils import getFieldForRun
    lbs=prevFlags.Input.LumiBlockNumbers
    fieldStat=getFieldForRun(run=prevFlags.Input.RunNumbers[0],lumiblock=0 if len(lbs)==0 else lbs[0],quiet=True)
    return (fieldStat.solenoidCurrent()>1)
    
                              
def createBFieldConfigFlags(): 
    bcf=AthConfigFlags()
    # True when solenoid is on
    bcf.addFlag("BField.solenoidOn", _solenoidFieldAutoCfg)
    # True when barrel toroid is on
    bcf.addFlag("BField.barrelToroidOn", _toroidFieldAutoCfg)
    # True when endcap toroid is on
    bcf.addFlag("BField.endcapToroidOn", _toroidFieldAutoCfg)
    return bcf
