# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
from AthenaConfiguration.ComponentFactory import CompFactory

def VarToolCfg(flags, var, Tname="VarTool", **kwargs):
    """ 
    Provides InputVariable 
    
    var: Either a string with the variable name or a dictionary with the VarTool config options
    """
    if isinstance(var, dict):
        kwargs.update(var)
        kwargs.setdefault("Name", "pt")
    elif isinstance(var, str):
        kwargs.setdefault("Name", var)
    else:
        raise TypeError('Unregonised type for VarTool block')
    # Guess if this is a jet variable if not explicitly set
    if var in ["e", "et", "pt", "eta", "abseta", "|eta|", "rapidity", "y", "|y|", "absy", "absrapidity", "|rapidity|", "DetectorEta", "absDetEta", "LOGmOe"]:
        kwargs.setdefault("isJetVar", True)
    else:
        kwargs.setdefault("isJetVar", False)
    kwargs.setdefault("Type","float")
    # Guess the scale if not explicitly set
    if kwargs.get("isJetVar") and var in ["pt", "e", "et"]:
        kwargs.setdefault("Scale", 1e-3)
    else:
        kwargs.setdefault("Scale",1.0)
    tname = Tname + kwargs["Name"]
    return CompFactory.JetHelper.VarTool(tname, **kwargs)

def HistoInputCfg(flags, Tname, inputFile, histName, varX, **kwargs):
    """ Provides Histogram reader """

    kwargs.setdefault("histName",histName)
    kwargs.setdefault("inputfile",inputFile)
    kwargs.setdefault("InterpType", "Full")
    
    varTool1 = VarToolCfg(flags, varX)
    kwargs.setdefault("varTool1",varTool1)
    tname = Tname+"_"+varTool1.Name

    # 2D histogram if varY provided, 3D histogram if varZ provided, else 1D histogram
    varY = kwargs.pop('varY', None)
    varZ = kwargs.pop('varZ', None)
    if varY:
        varTool2 = VarToolCfg(flags, varY)
        tname+="_"+varTool2.Name
        kwargs.setdefault("varTool2",varTool2)
        if varZ:
            varTool3 = VarToolCfg(flags, varZ)
            tname+="_"+varTool3.Name
            kwargs.setdefault("varTool3",varTool3)
            return CompFactory.JetHelper.HistoInput3D(tname, **kwargs)
        else:
            return CompFactory.JetHelper.HistoInput2D(tname, **kwargs)
    else:
        return CompFactory.JetHelper.HistoInput1D(tname, **kwargs)
 

def MCJESToolCfg(flags, Tname, inFile, corrKey, **kwargs):
    """Provides Text reader for MCJES type"""
    tname = Tname+"_"+corrKey
    kwargs.setdefault("inputfile",inFile)
    kwargs.setdefault("corrName",corrKey)
    return CompFactory.JetHelper.TextInputMCJES(tname,**kwargs)
