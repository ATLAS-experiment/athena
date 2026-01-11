# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from abc import abstractmethod
from typing import Any
from AthenaCommon.Logging import logging
import re

from .ThresholdType import ThrType

log = logging.getLogger(__name__)

##
## These classes are base classes for the auto-generated algorithm python representations
## 
## C++ L1Topo algorithms are defined in Trigger/TrigT1/L1Topo/L1TopoAlgorithms
## During the build, from each class a python class is generated and put in the release
## Those generated python classes derive fro SortingAlgo and DecisionAlgo below.

class Variable:
    def __init__(self, name, selection, value):
        self.name = name
        self.selection = int(selection)
        self.value = int(value)
            
class Generic:
    def __init__(self, name, value):
        self.name = name
        from L1TopoHardware.L1TopoHardware import HardwareConstrainedParameter
        if isinstance(value,HardwareConstrainedParameter):
            self.value = ":%s:" % value.name
        else:
            self.value = value


class GlobalAlgo:

    # list of available variable names (will be overridden or extended in derived classes)
    _availableVars: list[str] = []

    def __init__(self, klass: str, name: str):
        self._klass: str = klass
        self._name: str = name
        self.generics = []
        self.variables = []

    def __str__(self):
        return f"{self._name}@{self._klass}"

    @property
    def name(self) -> str:
        return self._name

    def addvariable(self, name, value, selection = -1):
        if name in self._availableVars:
            self.variables += [ Variable(name, selection, value) ]
            return self
        log.fatal("Variable parameter '%s' does not exist for algorithm %s of type %s,\navailable parameters are %r", name,self._name, self._klass, self._availableVars)
        raise RuntimeError("Illegal variable parameter '%s'" % name)

    def addgeneric(self, name, value):
        if name in self._availableVars:
            self.generics += [ Generic(name, value) ]
        else:
            log.fatal("Generic parameter '%s' does not exist for algorithm %s of type %s,\navailable parameters are %r" % (name,self._name, self._klass, self._availableVars))
            raise RuntimeError("Illegal generic parameter '%s'" % name)
        return self

    @abstractmethod
    def json(self) -> dict[str, Any]: ...

class GlobalHypoAlgo(GlobalAlgo):
    def __init__(self, klass: str, name: str):
        super().__init__( klass=klass, name=name)

class GlobalMultiplicityAlgo(GlobalHypoAlgo):
    _availableVars: list[str] = ["input", "output"]

    def __init__(self, name: str, input: str, output: str):
        super().__init__( klass="GlobalMultiplicityAlgo", name=name)
        self.addgeneric('input', input)
        self.addgeneric('output', output)

    def output(self) -> str | list[str]:
        for gen in self.generics:
            if gen.name == "output":
                return gen.value
        raise RuntimeError("No output defined for GlobalMultiplicityAlgo %s" % self.name)

class GlobalDecisionAlgo(GlobalHypoAlgo):
    def __init__(self, name: str):
        super().__init__( klass="GlobalDecisionAlgo", name=name)


class TopoAlgo:

    _availableVars = []

    #__slots__ = ['_name', '_selection', '_value', '_generic']
    def __init__(self, classtype, name):
        self.classtype = classtype
        self.name = name
        self.generics = []
        self.variables = []
        
    def __str__(self):  
        return self.name

    def isSortingAlg(self) -> bool:
        return False

    def isDecisionAlg(self) -> bool:
        return False

    def isMultiplicityAlg(self) -> bool:
        return False

    def setThresholds(self, thresholds):
        # link to all thresholds in the menu need for configuration
        self.menuThr = thresholds

    def addvariable(self, name, value, selection = -1):
        if name in self._availableVars:
            self.variables += [ Variable(name, selection, value) ]
        else:
            raise RuntimeError("Variable parameter '%s' does not exist for algorithm %s of type %s,\navailable parameters are %r" % (name,self.name, self.classtype, self._availableVars))
        return self

    def addgeneric(self, name, value):
        if name in self._availableVars:
            self.generics += [ Generic(name, value) ]
        else:
            raise RuntimeError("Generic parameter '%s' does not exist for algorithm %s of type %s,\navailable parameters are %r" % (name,self.name, self.classtype, self._availableVars))
        return self

    def json(self):
        confObj = {
            "klass": self.classtype
        }
        return confObj

    def getScaleToCountsEM(self):  # legacy Et conversion!!
        tw = self.menuThr.typeWideThresholdConfig(ThrType["EM"])
        return 1000 // tw["resolutionMeV"]
        
class SortingAlgo(TopoAlgo):
    
    def __init__(self, classtype, name, inputs, outputs):
        super(SortingAlgo, self).__init__(classtype=classtype, name=name)
        self.inputs = inputs
        self.outputs = outputs
        self.inputvalue=  self.inputs
        if self.inputs.find("Cluster")>=0: # to extract inputvalue (for FW) from output name
            if self.outputs.find("TAU")>=0:
                self.inputvalue= self.inputvalue.replace("Cluster","Tau")
            if self.outputs.find("EM")>=0:
                self.inputvalue= self.inputvalue.replace("Cluster","Em")

    def isSortingAlg(self) -> bool:
        return True
        
    def json(self):
        confObj = super(SortingAlgo, self).json()
        confObj["input"] = self.inputvalue
        confObj["output"] = self.outputs
        confObj["fixedParameters"] = {}
        confObj["fixedParameters"]["generics"] = {}
        for (pos, genParm) in enumerate(self.generics):
            confObj["fixedParameters"]["generics"][genParm.name] = {"value": genParm.value, "position": pos}

        confObj["variableParameters"] = list()
        _emscale_for_decision = self.getScaleToCountsEM() # for legacy algos
        _mu_for_decision= 10 # MU4->3GeV, MU6->5GeV, MU10->9GeV because selection is done by pt>X in 100 MeV units for Run3 muons
        if "MUCTP-" in self.name:
            _mu_for_decision= 1 
        for (pos, variable) in enumerate(self.variables): 
            if variable.name == "MinET" or variable.name == "MinEtTGC" or variable.name == "MinEtRPC":
                if "e" in self.outputs or "j" in self.outputs or "g" in self.outputs: 
                    variable.value *= 1 # no conversion needed in Run3 algo
                elif "TAU" in self.outputs or "EM" in self.outputs:
                    variable.value *= _emscale_for_decision
                if "MU" in self.outputs:
                    variable.value = ((variable.value - _mu_for_decision ) if variable.value>0 else variable.value)
            confObj["variableParameters"].append({"name": variable.name, "value": variable.value})

            if type(variable.value) is float:
                raise RuntimeError("In algorithm %s the variable %s with value %r is of type float but must be int" % (self.name,variable.name,variable.value))
        return confObj


class DecisionAlgo(TopoAlgo):

    def __init__(self, classtype, name, inputs, outputs):
        super(DecisionAlgo, self).__init__(classtype=classtype, name=name)
        self.inputs = inputs if type(inputs)==list else [inputs]
        self.outputs = outputs if type(outputs)==list else [outputs]

    def isDecisionAlg(self) -> bool:
        return True

    def json(self):
        confObj = super(DecisionAlgo, self).json()
        confObj["input"] = self.inputs # list of input names
        confObj["output"] = self.outputs # list of output names
        # fixed parameters
        confObj["fixedParameters"] = {}
        confObj["fixedParameters"]["generics"] = {}
        for (pos, genParm) in enumerate(self.generics):
            confObj["fixedParameters"]["generics"][genParm.name] = {"value": genParm.value, "position": pos}

        # variable parameters
        confObj["variableParameters"] = list()
        _emscale_for_decision = self.getScaleToCountsEM() # for legacy algos
        _mu_for_decision= 1 # MU4->3GeV, MU6->5GeV, MU10->9GeV because selection is done by pt>X in 100 MeV units for Run3 muons
        if "MUCTP-" in self.name:
            _mu_for_decision= 1 
        for (pos, variable) in enumerate(self.variables):
            # scale MinET if inputs match with EM or TAU
            for _minet in ["MinET"]:
                if variable.name==_minet+"1" or variable.name==_minet+"2" or variable.name==_minet+"3" or variable.name==_minet:
                    for (tobid, _input) in enumerate(self.inputs):
                        if (_input.find("e")>=0 or _input.find("j")>=0 or _input.find("g")>=0):
                            variable.value *= 1 # no conversion needed in Run3 algo
                        elif (_input.find("TAU")>=0 or _input.find("EM")>=0):
                            if (len(self.inputs)>1 and (variable.name==_minet+str(tobid+1) or (tobid==0 and variable.name==_minet))) or (len(self.inputs)==1 and (variable.name.find(_minet)>=0)):
                                variable.value *= _emscale_for_decision

                        if _input.find("MU")>=0:
                            if (len(self.inputs)>1 and (variable.name==_minet+str(tobid+1) or (tobid==0 and variable.name==_minet))) or (len(self.inputs)==1 and (variable.name.find(_minet)>=0)):
                                variable.value = ((variable.value - _mu_for_decision ) if variable.value>0 else variable.value)

            if type(variable.value) is float:
                raise RuntimeError("In algorithm %s the variable %s with value %r is of type float but must be int" % (self.name,variable.name,variable.value))

            if variable.selection >= 0:
                confObj["variableParameters"].append({"name": variable.name, "selection": variable.selection, "value": variable.value})
            else:
                confObj["variableParameters"].append({"name": variable.name, "value": variable.value})

        return confObj


class MultiplicityAlgo(TopoAlgo):

    def __init__(self, classtype, name, threshold, input, output, nbits):
        super().__init__(classtype=classtype, name=name)
        self.threshold = threshold
        self.input = input
        self.outputs = output
        self.nbits = nbits

    def isMultiplicityAlg(self):
        return True            

    def configureFromThreshold(self, thr):
        pass

    def json(self):
        confObj = super(MultiplicityAlgo, self).json()
        confObj["threshold"] = self.threshold
        confObj["input"] = self.input
        confObj["output"] = self.outputs
        confObj["nbits"] = self.nbits
        return confObj

# eEM and jEM
class EMMultiplicityAlgo(MultiplicityAlgo):
    def __init__(self, name, threshold, nbits, classtype ):
        super().__init__(classtype=classtype, name=name, 
                                                 threshold = threshold, 
                                                 input=None, output="%s" % threshold,
                                                 nbits=nbits)
        if (m := re.match("(?P<type>[A-z]*)[0-9]*(?P<suffix>[VHILMT]*)",threshold)) is not None:
            mres = m.groupdict()
            self.input = mres["type"].replace('SPARE','')
        else:
            log.error("EMMultiplicityAlgo: could not parse threshold name %s, expected it to start with a valid type", threshold)

# eTAU, jTAU, cTAU
class TauMultiplicityAlgo(MultiplicityAlgo):
    def __init__(self, name, threshold, nbits, classtype ):
        super().__init__(classtype=classtype, name=name, 
                                                  threshold = threshold, 
                                                  input=None, output="%s" % threshold,
                                                  nbits=nbits)
        if (m := re.match("(?P<type>[A-z]*)[0-9]*(?P<suffix>[HLMT]*)",threshold)) is not None:
            mres = m.groupdict()
            self.input = mres["type"].replace('SPARE','')
        else:
            log.error("TauMultiplicityAlgo: could not parse threshold name %s, expected it to start with a valid type", threshold)

# jJ, gJ and gLJ
class JetMultiplicityAlgo(MultiplicityAlgo):
    def __init__(self, name, threshold, nbits, classtype ):
        super().__init__(classtype=classtype, name=name, 
                                                  threshold = threshold, 
                                                  input=None, output="%s" % threshold,
                                                  nbits=nbits)
        if(m := re.match("(?P<type>[A-z]*)[0-9]*(?P<suffix>[A-z]*)", threshold)) is not None:
            mres = m.groupdict()
            self.input = mres["type"].replace('SPARE','')
        else:
            log.error("JetMultiplicityAlgo: could not parse threshold name %s, expected it to start with a valid type", threshold)

# all XE and TE flavours
class XEMultiplicityAlgo(MultiplicityAlgo):
    def __init__(self, name, threshold, nbits, classtype = "EnergyThreshold"):
        super().__init__( classtype = classtype, name=name,  
                                                  threshold = threshold,
                                                  input=None, output="%s" % threshold,
                                                  nbits=nbits)
        if(m := re.match("(?P<type>[A-z]*)[0-9]*(?P<suffix>[A-z]*)",threshold)) is not None:        
            mres = m.groupdict()
            self.input = mres["type"].replace('SPARE','')
            self.flavour = self.input
        else:
            log.error("XEMultiplicityAlgo: could not parse threshold name %s, expected it to start with a valid type", threshold)

    def json(self):
        confObj = super().json()
        confObj["threshold"] = self.threshold
        confObj["input"] = self.input
        confObj["output"] = self.outputs
        confObj["flavour"] = self.flavour
        confObj["nbits"] = self.nbits
        return confObj

class MuMultiplicityAlgo(MultiplicityAlgo):
    def __init__(self, classtype, name, input, output, nbits):
        super().__init__(classtype=classtype, name=name, input=input, threshold="", output=output, nbits=nbits)

    def configureFromThreshold(self, thr):
        pass

class LArSaturationAlgo(MultiplicityAlgo):
    def __init__(self):
        name = 'LArSaturation'
        super().__init__(name=name, classtype=name, input='jTE', output=name, threshold=name, nbits=1)

class ZeroBiasAlgo(MultiplicityAlgo):
    def __init__(self, name):
        super().__init__(name=name, classtype='ZeroBias', input=name, threshold=name, output=name, nbits=1)

