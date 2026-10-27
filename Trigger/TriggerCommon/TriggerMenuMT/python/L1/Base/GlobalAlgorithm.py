# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from __future__ import annotations

from abc import abstractmethod
from enum import Enum
from typing import TYPE_CHECKING, Self
from AthenaCommon.Logging import logging

if TYPE_CHECKING:
    from .Thresholds import MenuThresholdsCollection
    from .Typing import JSONType

log = logging.getLogger(__name__)

# There are different types of global algorithms which run on the Global Event Processor (GEP) in the L1Global Trigger. These algorithms can be categorized into three main types:
# 1. Reconstruction Algorithms: 
# These algorithms are responsible for reconstructing trigger objects using the raw detector data to
# refine objects arriving from the FEX-inputs. Examples are cluster makers, jet, met, electron and tau 
# reconstruction algorithms. In the simulation these are implemented as AthAlgorithms
# 2. Multiplicity Algorithms:
# These algorithms count the number of trigger objects of a certain type that pass a given threshold.
# 3. Topological Algorithms:
# These algorithms perform calculations on the trigger objects (like invariant mass, delta phi, etc.) 
# to determine if they satisfy certain topological conditions.
# Type 2. and 3. algorithms run inside the GEP Hypothesis block, together they form the L1Global decision. In the simulation these are implemented as AthAlgoTools, all being called by a single AthAlgorithm which simulates the Hypothesis block.

class GlobalAlgType(Enum):
    RECONSTRUCTION = ('reconstructionAlgorithms')
    MULTIPLICITY = ('multiplicityAlgorithms') 
    TOPOLOGICAL = ('topologicalAlgorithms')
    def __init__(self, key: str):
        self.key: str = key

class GlobalAlgorithm:
    """Base class for all global compute algorithms
    
    These are
    - the preprocessing algorithms
    - the multiplicity algorithms (inside the GEP Hypo algorithm) 
    - the topological algorithms (inside the GEP Hypo algorithm)
    """

    def __init__(self, alg_type: GlobalAlgType, klass: str, name: str):
        self._alg_type: GlobalAlgType = alg_type  # the particular type of global algorithm, see GlobalAlgType
        self._klass: str = klass  # the C++ class name of the algorithm
        self._name: str = name  # the instance name of the algorithm as it appears in the python configuration

    def __str__(self) -> str:
        return f"{self._name}@{self._klass}"

    @property
    def name(self) -> str:
        """The instance name of the algorithm as it appears in the python configuration."""
        return self._name

    @property
    def alg_type(self) -> GlobalAlgType:
        """The particular type of global algorithm, see GlobalAlgType."""
        return self._alg_type

    @property
    def klass(self) -> str:
        """The C++ class name of the algorithm."""
        return self._klass

    @abstractmethod
    def json(self) -> JSONType: ...


class VarParam:
    """Concept of a variable parameter of a global algorithm was carried over from the old L1Topo configuration. This type of parameters can change between configurations without the need to rebuild the firmware. This is because these parameters
    are loaded into registers in the firmware through the IPBUS.
    If the Global algorithms will be described this way needs to be decided, but for now we keep this concept.
    """
    def __init__(self, name: str, selection, value: JSONType):
        self.name = name
        self.selection = int(selection)
        self.value: JSONType = value
            
class FixedParam:
    """Concept of a fixed parameter of a global algorithm was carried over from the old L1Topo configuration. Changing this type of parameters requires a rebuild of the firmware. This is because these parameters determine the firmware layout.
    If the Global algorithms will be described this way needs to be decided, but for now we keep this concept.
    """
    def __init__(self, name: str, value):
        self.name = name
        self.value: JSONType
        from L1TopoHardware.L1TopoHardware import HardwareConstrainedParameter
        if isinstance(value,HardwareConstrainedParameter):
            self.value = ":%s:" % value.name
        else:
            self.value = value

class GlobalHypoAlgo(GlobalAlgorithm):
    """
    Base class for selection algorithms that run inside the GEP Hypothesis block
    """
    def __init__(self, alg_type: GlobalAlgType, klass: str, name: str, wiredInputs: list[list[str]], inputs: list[str], thresholds: list[str]):
        super().__init__(alg_type=alg_type, klass=klass, name=name)
        self._fixed_params: dict[str, FixedParam] = {}
        self._var_params: dict[str, VarParam] = {}

        self.wiredInputs: list[list[str]] = wiredInputs  # wired inputs of the algorithm, see derived classes
        self.add_fixed_parameter('wiredInputs', wiredInputs)
        self.add_variable_parameter('inputs', inputs)
        self.add_variable_parameter('thresholds', thresholds)

    def vpar(self, name: str):
        return self._var_params[name]

    def add_variable_parameter(self, name, value, selection = -1) -> Self:
        self._var_params[name] = VarParam(name, selection, value)
        return self

    def add_fixed_parameter(self, name, value) -> Self:
        self._fixed_params[name] = FixedParam(name, value)
        return self

    @property
    def inputs(self) -> list[str]:
        """The input signals for this algorithm."""
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
    """
    Class for describing multiplicity algorithms that run inside the GEP Hypothesis block
    """
    def __init__(self, threshold: str, input: str, output: str, nbits: int):
        super().__init__(alg_type=GlobalAlgType.MULTIPLICITY, klass="GlobalMultiplicityAlgo",
                         name=f'Mult_{threshold}', wiredInputs=[[input]], inputs=[input],  thresholds=[threshold])
        self.menuThr: MenuThresholdsCollection  # The collection of thresholds in the menu.
        self.add_fixed_parameter('nbits', nbits)
        self._output: str = output

    def output(self) -> str | list[str]:
        """The output signal for this algorithm."""
        return self._output

    def setThresholds(self, thresholds: MenuThresholdsCollection):
        """Sets the link to all thresholds in the menu needed for configuration."""
        self.menuThr: MenuThresholdsCollection = thresholds

class GlobalTopologicalAlgo(GlobalHypoAlgo):
    """
    Class for describing topological selection algorithms that run inside the GEP Hypothesis block
    """
    def __init__(self, name: str, wiredInputs: list[list[str]], 
                 inputs: str | list[str], thresholds: str | list[str]):
        super().__init__(alg_type=GlobalAlgType.TOPOLOGICAL, klass="GlobalDecisionAlgo", name=name, wiredInputs=wiredInputs, 
                         inputs=[inputs] if isinstance(inputs, str) else inputs,
                         thresholds=[thresholds] if isinstance(thresholds, str) else thresholds )

