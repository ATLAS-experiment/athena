
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
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
    kwargs.setdefault("isJetVar", True)
    kwargs.setdefault("Type","float")
    if kwargs.get("isJetVar") and kwargs.get("Name") in ["pt", "e", "et"]:
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
    tname = Tname+"_"+varTool1.name

    # 2D histogram if varY provided, else 1D histogram
    varY = kwargs.pop('varY', None)
    if varY:
        varTool2 = VarToolCfg(flags, varY)
        tname+="_"+varTool2.name
        kwargs.setdefault("varTool2",varTool2)
        return CompFactory.JetHelper.HistoInput2D(tname, **kwargs)
    else:
        return CompFactory.JetHelper.HistoInput1D(tname, **kwargs)
 

def MCJESToolCfg(flags, Tname, inFile, corrKey, **kwargs):
    """Provides Text reader for MCJES type"""
    tname = Tname+"_"+corrKey
    kwargs.setdefault("inputfile",inFile)
    kwargs.setdefault("corrName",corrKey)
    return CompFactory.JetHelper.TextInputMCJES(tname,**kwargs)
