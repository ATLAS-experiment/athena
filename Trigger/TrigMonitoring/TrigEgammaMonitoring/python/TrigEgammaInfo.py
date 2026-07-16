#
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
#

from ROOT.ChainNameParser import HLTChainInfo


class TrigEgammaInfo(object):

    EM = {"e", "electron"}
    GAMMA = {"g", "photon"}


    def __init__(self, trigger):
        self.__chain = trigger
        self.__legs = HLTChainInfo(trigger)
        self.__sigs = {leg.signature.lower() for leg in self.__legs}

    def chain(self):
        return self.__chain

    def legs(self):
        return self.__legs

    def signatures(self):
        return self.__sigs

    def isElectron(self):
        sigs = self.__sigs
        return bool(sigs & self.EM) and not (sigs & self.GAMMA)

    def isPhoton(self):
        sigs = self.__sigs
        return bool(sigs & self.GAMMA) and not (sigs & self.EM)

    def isTagAndProbeZeeg(self):
        sigs = self.__sigs
        return (
            "probe" in self.__chain.lower()
            or (bool(sigs & self.EM) and bool(sigs & self.GAMMA))
        )

    def threshold(self):
        if self.isElectron() or self.isPhoton():
            thresholdValue  = self.__legs.threshold
            thresholdString = str(thresholdValue)

        elif self.isTagAndProbeZeeg():
            if self.__sigs & self.GAMMA:  # photon/gamma-like signature
                thresholdValue  = self.__legs.threshold
                thresholdString = str(thresholdValue)

        return thresholdString

    def pidname(self):
        if not self.__legs:
            return None
        pid = None

        if self.isElectron() or self.isPhoton():
            pid = self.__legs.legParts[0]

        elif self.isTagAndProbeZeeg():
            if self.__sigs & self.GAMMA:  # photon/gamma-like signature
                pid = self.__legs.legParts[0]

        return pid

    def isIsolated(self):
        for part_name in ['iloose', 'ivarloose', 'icaloloose', 'icalovloose', 'icalotight']:
          if part_name in self.chain():
            return True
        return False




