# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from operator import attrgetter
from enum import Enum

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

from .TopoAlgos import DecisionAlgo, MultiplicityAlgo, SortingAlgo, TopoAlgo
from .GlobalAlgos import GlobalAlgo, GlobalMultiplicityAlgo

class AlgType(Enum):
    SORT = ('sortingAlgorithms')
    DEC = ('decisionAlgorithms') 
    MULT = ('multiplicityAlgorithms')
    def __init__(self, key):
        self.key = key
    
class AlgCategory(Enum):
    TOPO = (1, 'TOPO', 'new topo', 'TopoAlgoDef')
    MUCTPI = (2, 'MUTOPO', 'muctpi topo', 'TopoAlgoDefMuctpi')
    LEGACY = (3, 'R2TOPO', 'legacy topo', 'TopoAlgoDefLegacy')
    MULTI = (4, 'MULTTOPO', 'multiplicity topo', 'TopoAlgoDefMultiplicity')
    GLOBHYPO = (5, 'hypoAlgorithm', 'L0 global hypo', 'GlobalHypoAlgoDef')

    def __init__(self, _, key, desc, defFile ):
        self.key: str = key  # key for json output
        self.prefix = key + '_' if key else ''
        self.desc = desc
        self.defFile = defFile

    def __str__(self):
        return self.desc

    @staticmethod
    def getAllCategories(run=3):
        assert(run in [2,3,4]), "Only run 3 and run 4 are supported, but got run %s" % run
        if run in (2,3):
            return [ AlgCategory.TOPO, AlgCategory.MUCTPI, AlgCategory.LEGACY, AlgCategory.MULTI ]
        else:
            return [ AlgCategory.GLOBHYPO ]

    @staticmethod
    def getCategoryFromBoardName(boardName):
        if 'muctpi' in boardName.lower():
            currentTopoCategory = AlgCategory.MUCTPI
        elif 'topo' in boardName.lower():
            if 'legacy' in boardName.lower():
                currentTopoCategory = AlgCategory.LEGACY
            else:
                currentTopoCategory = AlgCategory.TOPO
        else:
            raise RuntimeError("Board %s is not a topo board" % boardName )
        return currentTopoCategory


class MenuTopoAlgorithmsCollection:

    def __init__(self, run):
        # all algos that are in menu (new and legacy)
        self.topoAlgos: dict[AlgCategory, dict[AlgType, dict[str, TopoAlgo | GlobalAlgo]]] = {}
        for cat in AlgCategory.getAllCategories(run):
            self.topoAlgos[cat] = {}
            if cat in [AlgCategory.TOPO, AlgCategory.MUCTPI, AlgCategory.LEGACY]:
                self.topoAlgos[cat][AlgType.DEC] = {}
                self.topoAlgos[cat][AlgType.SORT] = {}
            elif cat in [AlgCategory.MULTI]:
                self.topoAlgos[cat][AlgType.MULT] = {}
            elif cat in [AlgCategory.GLOBHYPO]:
                self.topoAlgos[cat][AlgType.MULT] = {}
                self.topoAlgos[cat][AlgType.DEC] = {}

    def addAlgo(self, algo, category):
        if type(category) is not AlgCategory:
            raise RuntimeError( "No category is provided when adding topo algo %s to menu" % algo.name)

        if isinstance(algo,DecisionAlgo):
            algType = AlgType.DEC
        elif isinstance(algo, SortingAlgo):
            algType = AlgType.SORT
        elif isinstance(algo, MultiplicityAlgo):
            algType = AlgType.MULT
        elif isinstance(algo, GlobalMultiplicityAlgo):
            algType = AlgType.MULT
        else:
            raise RuntimeError("Trying to add topo algorithm %s of unknown type %s to the menu" % (algo.name, type(algo)))

        if algType not in self.topoAlgos[category]:
            self.topoAlgos[category][algType] = {}

        if algo.name in self.topoAlgos[category][algType]:
            raise RuntimeError("Trying to add topo algorithm %s a second time" % algo.name)

        self.topoAlgos[category][algType][algo.name] = algo


    def json(self):

        confObj = {}
        for cat in self.topoAlgos:
            confObj[cat.key] = {}
            for typ in self.topoAlgos[cat]:
                confObj[cat.key][typ.key] = {}
                for alg in sorted(self.topoAlgos[cat][typ].values(), key=attrgetter('name')):
                    confObj[cat.key][typ.key][alg.name] = alg.json()

        return confObj
