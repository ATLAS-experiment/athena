#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import unittest

from TriggerMenuMT.TriggerAPI import TriggerAPISession, TriggerType

class TriggerAPISessionTest(unittest.TestCase):


    def test_run2(self):
        # TODO: this test will
        # confirm that the new session API can produce the same results as the singleton API
        # at the moment the old cache info is grouped by old TriggerPeriod enums

        # use the cache file in the CalibArea
        s = TriggerAPISession(json="TriggerMenu/TriggerInfo_20210302.json")
        for triggerType in TriggerType:
            print(TriggerType.toStr(triggerType),":")
            print("",*s.getLowestUnprescaled(triggerType=triggerType, livefraction=0.98),sep='\n ')

    def test_grl2023(self):
        # tests using a 2023 GRL on the new API
        s = TriggerAPISession(grl="GoodRunsLists/data23_13p6TeV/20230828/data23_13p6TeV.periodAllYear_DetStatus-v110-pro31-06_MERGED_PHYS_StandardGRL_All_Good_25ns.xml")

        # here are results of triggers by type for this GRL, for a livefraction of 0.98 (b.c there were a few lb where things got prescaled):
        # results were generated with code at bottom of this test
        triggersByType = {}
        triggersByType[TriggerType.el_single] = 6
        triggersByType[TriggerType.el_multi] = 6
        triggersByType[TriggerType.mu_single] = 5
        triggersByType[TriggerType.mu_multi] = 23
        triggersByType[TriggerType.j_single] = 22
        triggersByType[TriggerType.j_multi] = 20
        triggersByType[TriggerType.bj_single] = 6
        triggersByType[TriggerType.bj_multi] = 8
        triggersByType[TriggerType.tau_single] = 1
        triggersByType[TriggerType.tau_multi] = 5
        triggersByType[TriggerType.g_single] = 2
        triggersByType[TriggerType.g_multi] = 9
        triggersByType[TriggerType.xe] = 3
        triggersByType[TriggerType.ht] = 0
        triggersByType[TriggerType.mu_bphys] = 40
        triggersByType[TriggerType.exotics] = 2
        triggersByType[TriggerType.afp] = 0
        triggersByType[TriggerType.el] = 12
        triggersByType[TriggerType.mu] = 29
        triggersByType[TriggerType.j] = 42
        triggersByType[TriggerType.bj] = 14
        triggersByType[TriggerType.tau] = 6
        triggersByType[TriggerType.g] = 11
        triggersByType[TriggerType.ALL] = 301
        triggersByType[TriggerType.UNDEFINED] = 0


        for triggerType in TriggerType:
            print(TriggerType.toStr(triggerType),":")
            print("",*s.getLowestUnprescaled(triggerType=triggerType, livefraction=0.98),sep='\n ')
            assert(len(s.getLowestUnprescaled(triggerType=triggerType, livefraction=0.98))==triggersByType[triggerType])
        s.save("apiSessionDump.json")

        # confirm can recover the results from the dump
        s2 = TriggerAPISession(json="apiSessionDump.json")
        for triggerType in TriggerType:
            assert( s.getLowestUnprescaled(triggerType=triggerType, livefraction=0.98) == s2.getLowestUnprescaled(triggerType=triggerType, livefraction=0.98) )

        # this is the code that generates the results map above
        #for triggerType in TriggerType:
        #    print(f"triggersByType[TriggerType.{triggerType.name}] = {len(s.getLowestUnprescaled(triggerType=triggerType, livefraction=0.98))}")

    def test_menu(self):
        # tests using a menu on the new API
        s = TriggerAPISession(menu="Physics_pp_run3_v1")
        for triggerType in TriggerType:
            print(TriggerType.toStr(triggerType),":")
            print("",*s.getLowestUnprescaled(triggerType=triggerType, livefraction=0.98),sep='\n ')


if __name__ == "__main__":
    unittest.main(verbosity=2)
