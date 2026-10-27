# Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration

from __future__ import annotations

__all__ = ['Logic', 'Not']

from copy import copy
from enum import Enum

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)


class LogicType(Enum):
    NONE = ('')
    AND = ('&')
    OR = ('|')
    NOT = ('!')
    THRESHOLD = ('T')
    INTERNAL = ('I')

    def __init__(self, opSymbol: str):
        self.opSymbol: str = opSymbol

    def __str__(self):
        return self.opSymbol

    def __repr__(self):
        return self.name


class Logic:

    __slots__ = ['logicType', 'subConditions']

    @staticmethod
    def Not(x: Logic) -> Logic:
        a = Logic()
        a.logicType = LogicType.NOT
        a.subConditions.append(x)
        return a

    @staticmethod
    def stripBunchGroups(theLogic: Logic) -> tuple[Logic, list[str]]:
        subConds: list['Logic'] = []
        bunchGroups: list[str] = []
        for sc in theLogic.subConditions:
            if sc.logicType is LogicType.INTERNAL and sc.name().startswith('BGRP'):
                bunchGroups.append(sc.name())
            else:
                subConds.append(sc)

        if len(subConds) > 1:
            theLogic.subConditions = subConds
            return (theLogic, bunchGroups)

        if len(subConds) == 1:
            return (subConds[0], bunchGroups)

        raise RuntimeError("Item with logic '%s' has only internal triggers defined" % theLogic)

    def __init__(self, logicType: LogicType = LogicType.NONE):
        self.logicType: LogicType = logicType
        self.subConditions: list[Logic] = []  # holds Logic instances

    def name(self) -> str:
        """Jut the symbol for the logic type, e.g. '&' for AND, '|' for OR, '!' for NOT
        Will be overridden in derived classes to return the name of the threshold or internal trigger
        """
        return self.logicType.opSymbol

    def __or__(self, x: Logic) -> Logic:
        newLogic = Logic(logicType=LogicType.OR)

        if self.logicType is LogicType.OR:
            newLogic.subConditions += copy(self.subConditions)
        else:
            newLogic.subConditions += [copy(self)]

        if x.logicType is LogicType.OR:
            newLogic.subConditions += copy(x.subConditions)
        else:
            newLogic.subConditions += [copy(x)]

        return newLogic

    def __and__(self, x: Logic) -> Logic:
        newLogic = Logic(logicType=LogicType.AND)

        if self.logicType is LogicType.AND:
            newLogic.subConditions += copy(self.subConditions)
        else:
            newLogic.subConditions += [copy(self)]

        if x.logicType is LogicType.AND:
            newLogic.subConditions += copy(x.subConditions)
        else:
            newLogic.subConditions += [copy(x)]

        return newLogic

    def __not__(self, x: Logic) -> Logic:
        if self.logicType is LogicType.NONE:
            self.logicType = LogicType.NOT
        if self.logicType is LogicType.NOT:
            if len(self.subConditions) == 1:
                log.debug('not is a unary operator, ignore it')
            else:
                self.subConditions.append(x)
        return self

    def __str__(self):
        if self.logicType is LogicType.NONE:
            log.error('LogicType NONE should not have an instance')
            return ''

        if self.logicType is LogicType.NOT:
            if len(self.subConditions) == 1:
                if self.subConditions[0].logicType is LogicType.NONE:
                    return '!' + str(self.subConditions[0])
                else:
                    return '!(' + str(self.subConditions[0]) + ')'
            log.error('Logic NOT must have exactly one element but has %i', len(self.subConditions))
            return ''

        if self.logicType in (LogicType.AND, LogicType.OR):
            s = ''
            if len(self.subConditions) <= 1:
                log.error('Logic AND/OR must have more than one sub element but has %i: %r', len(self.subConditions), self.subConditions)
                return ''
            else:
                for (i, a) in enumerate(self.subConditions):
                    if i > 0:
                        s += ' ' + str(self.logicType) + ' '
                    if a.logicType in (LogicType.THRESHOLD, LogicType.INTERNAL, LogicType.NOT):
                        s += str(a)
                    else:
                        s += '(' + str(a) + ')'
            return s
        return ''

    def __lt__(self, other):
        if not isinstance(other, Logic):
            return NotImplemented
        return self.name() < other.name()

    def thresholdNames(self, include_bgrp: bool = False) -> list[str]:
        """List of names of thresholds in this logic

        Args:
            include_bgrp (bool, optional): if True,include bunch group names in the list. Defaults to False.

        Returns:
            list[str]: list of threshold names
        """
        names: set[str] = set()
        for sc in self.subConditions:
            names.update(sc.thresholdNames(include_bgrp))
        return sorted(list(names))

    def conditions(self, include_internal: bool = False) -> list[Logic]:
        """Sorted list of names of conditions in this logic"""
        cond: set[Logic] = set([])
        for sc in self.subConditions:
            cond.update(sc.conditions(include_internal))
        return sorted(list(cond))

    def normalize(self):
        if self.logicType in (LogicType.AND, LogicType.OR):
            mylogic = self.logicType
            newconditions = []
            for c in self.subConditions:
                if c.logicType == mylogic:  # X&(A&B) or X|(A|B)
                    # expand it to X&A&B or X|A|B
                    c.normalize()
                    newconditions.extend(c.subConditions)
                else:
                    newconditions.append(c)
            self.subConditions = newconditions


def Not(x: Logic) -> Logic:
    return Logic.Not(x)
