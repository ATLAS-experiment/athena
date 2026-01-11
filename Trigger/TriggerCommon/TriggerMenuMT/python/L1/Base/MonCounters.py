# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

from functools import total_ordering

from AthenaCommon.Logging import logging
log = logging.getLogger(__name__)

class MenuMonCountersCollection(object):

    def __init__(self):
        # list of monitoring counters
        self.counters = {
            'ctpmon': [],
            'ctpin': []
        }

    def addCounter(self, c):
        if c.montype not in self.counters:
            self.counters[c.montype] = []
        self.counters[c.montype] += [c]

    def json(self):
        confObj = { key: {c.name: c.json() for c in clist} for key,clist in self.counters.items() }
        return confObj


@total_ordering
class MonCounter(object):

    def __init__(self, threshold, multiplicity, montype):
        self.name = "%i%s" % (multiplicity, threshold)
        self.threshold = threshold
        self.multiplicity = int(multiplicity)
        self.montype = montype
        pass

    def __lt__(self, o):
        if(self.threshold!=o.threshold):
            return self.threshold < o.threshold
        return self.multiplicity < o.multiplicity

    def __eq__(self, o):
        return self.name == o.name

    def json(self):
        confObj = {
            "thr": self.threshold,
            "multiplicity": self.multiplicity
        }
        return confObj

    
class CtpinCounter(MonCounter):
    """
    These monitor the CTP Item counts
    """
    def __init__(self, threshold, multiplicity):
        super().__init__(threshold, multiplicity, 'ctpin')

class CtpmonCounter(MonCounter):
    """
    These monitor the CTPInput signal counts
    """
    def __init__(self, threshold, multiplicity):
        super().__init__(threshold, multiplicity, 'ctpmon')

