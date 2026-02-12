# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from __future__ import annotations

from abc import abstractmethod
from typing import TYPE_CHECKING, Self
from AthenaCommon.Logging import logging

if TYPE_CHECKING:
    from TriggerMenuMT.L1.Base.Thresholds import MenuThresholdsCollection
    from TriggerMenuMT.L1.Base.Typing import JSONType

log = logging.getLogger(__name__)

##
## These classes are base classes for the auto-generated algorithm python representations
## 
## C++ L1Topo algorithms are defined in Trigger/TrigT1/L1Topo/L1TopoAlgorithms
## During the build, from each class a python class is generated and put in the release
## Those generated python classes derive fro SortingAlgo and DecisionAlgo below.

class VarParam:
    def __init__(self, name, selection, value: JSONType):
        self.name = name
        self.selection = int(selection)
        self.value: JSONType = value
            
class FixedParam:
    def __init__(self, name, value):
        self.name = name
        self.value: JSONType
        from L1TopoHardware.L1TopoHardware import HardwareConstrainedParameter
        if isinstance(value,HardwareConstrainedParameter):
            self.value = ":%s:" % value.name
        else:
            self.value = value


class GlobalAlgo:

    def __init__(self, klass: str, name: str):
        self._klass: str = klass
        self._name: str = name
        self._fixed_params: dict[str, FixedParam] = {}
        self._var_params: dict[str, VarParam] = {}

    def __str__(self):
        return f"{self._name}@{self._klass}"

    @property
    def name(self) -> str:
        return self._name

    @property
    def classtype(self) -> str:
        return self._klass

    @property
    def klass(self) -> str:
        return self._klass

    def vpar(self, name: str):
        return self._var_params[name]

    def add_variable_parameter(self, name, value, selection = -1) -> Self:
        self._var_params[name] = VarParam(name, selection, value)
        return self

    def add_fixed_parameter(self, name, value) -> Self:
        self._fixed_params[name] = FixedParam(name, value)
        return self

    @abstractmethod
    def json(self) -> JSONType: ...


class GlobalHypoAlgo(GlobalAlgo):

    def __init__(self, klass: str, name: str, wiredInputs: list[list[str]], inputs: list[str], thresholds: list[str]):
        super().__init__( klass=klass, name=name)
        self.wiredInputs: list[list[str]] = wiredInputs  # wired inputs of the algorithm, see derived classes
        self.add_fixed_parameter('wiredInputs', wiredInputs)
        self.add_variable_parameter('inputs', inputs)
        self.add_variable_parameter('thresholds', thresholds)

    @property
    def inputs(self) -> list[str]:
        return self.vpar('inputs').value  # type: ignore
    
    def json(self) -> JSONType:
        confObj: JSONType = {
            "klass": self.klass,
            "fixedParameters": {
                k: p.value for (k, p) in self._fixed_params.items()
            },
            "variableParameters": { 
                k: p.value for (k, p) in self._var_params.items()
            }
        }
        return confObj

class GlobalMultiplicityAlgo(GlobalHypoAlgo):

    def __init__(self, threshold: str, input: str, output: str, nbits: int):
        super().__init__( klass="GlobalMultiplicityAlgo", name=f'Mult_{threshold}', wiredInputs=[[input]], inputs=[input],  thresholds=[threshold])
        self.menuThr: MenuThresholdsCollection
        self.add_fixed_parameter('nbits', nbits)
        self._output: str = output

    def output(self) -> str | list[str]:
        return self._output

    def setThresholds(self, thresholds: MenuThresholdsCollection):
        # link to all thresholds in the menu need for configuration
        self.menuThr: MenuThresholdsCollection = thresholds

class GlobalDecisionAlgo(GlobalHypoAlgo):
    def __init__(self, name: str, wiredInputs: list[list[str]], 
                 inputs: str | list[str], thresholds: str | list[str]):
        super().__init__( klass="GlobalDecisionAlgo", name=name, wiredInputs=wiredInputs, 
                         inputs=[inputs] if isinstance(inputs, str) else inputs,
                         thresholds=[thresholds] if isinstance(thresholds, str) else thresholds )

