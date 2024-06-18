#!/usr/bin/env python
# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import unittest
from TriggerMenuMT.TriggerAPI.TriggerAPI import TriggerAPI
from TriggerMenuMT.TriggerAPI.TriggerEnums import TriggerPeriod, TriggerType

class TriggerAPITest(unittest.TestCase):

    def setUp(self):
        TriggerAPI.reset()
        pass

    def test_unprescaled(self):
        from collections import defaultdict
        nTriggers = defaultdict(dict)
        nTriggers[1][1]=17
        nTriggers[1][2]=13
        nTriggers[1][4]=14
        nTriggers[1][8]=7
        nTriggers[1][16]=7
        nTriggers[1][32]=6
        nTriggers[1][64]=7
        nTriggers[1][128]=7
        nTriggers[1][256]=5
        nTriggers[1][512]=5
        nTriggers[1][1024]=5
        nTriggers[1][2048]=5
        nTriggers[1][4096]=5
        nTriggers[1][8192]=2
        nTriggers[1][16384]=4
        nTriggers[1][32768]=5
        nTriggers[1][65536]=2
        nTriggers[1][131072]=5
        nTriggers[2][0]=0
        nTriggers[2][1]=14
        nTriggers[2][2]=5
        nTriggers[2][4]=3
        nTriggers[2][8]=2
        nTriggers[2][16]=4
        nTriggers[2][32]=6
        nTriggers[2][64]=3
        nTriggers[2][128]=3
        nTriggers[2][256]=4
        nTriggers[2][512]=4
        nTriggers[2][1024]=4
        nTriggers[2][2048]=4
        nTriggers[2][4096]=4
        nTriggers[2][8192]=1
        nTriggers[2][16384]=4
        nTriggers[2][32768]=4
        nTriggers[2][65536]=1
        nTriggers[2][131072]=4
        nTriggers[4][0]=0
        nTriggers[4][1]=4
        nTriggers[4][2]=4
        nTriggers[4][4]=4
        nTriggers[4][8]=3
        nTriggers[4][16]=3
        nTriggers[4][32]=3
        nTriggers[4][64]=3
        nTriggers[4][128]=4
        nTriggers[4][256]=3
        nTriggers[4][512]=4
        nTriggers[4][1024]=4
        nTriggers[4][2048]=3
        nTriggers[4][4096]=3
        nTriggers[4][8192]=4
        nTriggers[4][16384]=4
        nTriggers[4][32768]=4
        nTriggers[4][65536]=3
        nTriggers[4][131072]=4
        nTriggers[8][0]=0
        nTriggers[8][1]=12
        nTriggers[8][2]=16
        nTriggers[8][4]=13
        nTriggers[8][8]=10
        nTriggers[8][16]=22
        nTriggers[8][32]=28
        nTriggers[8][64]=20
        nTriggers[8][128]=22
        nTriggers[8][256]=13
        nTriggers[8][512]=11
        nTriggers[8][1024]=13
        nTriggers[8][2048]=14
        nTriggers[8][4096]=12
        nTriggers[8][8192]=2
        nTriggers[8][16384]=9
        nTriggers[8][32768]=12
        nTriggers[8][65536]=2
        nTriggers[8][131072]=12
        nTriggers[16][0]=0
        nTriggers[16][1]=13
        nTriggers[16][2]=13
        nTriggers[16][4]=13
        nTriggers[16][8]=14
        nTriggers[16][16]=15
        nTriggers[16][32]=15
        nTriggers[16][64]=16
        nTriggers[16][128]=15
        nTriggers[16][256]=16
        nTriggers[16][512]=16
        nTriggers[16][1024]=19
        nTriggers[16][2048]=16
        nTriggers[16][4096]=15
        nTriggers[16][8192]=5
        nTriggers[16][16384]=12
        nTriggers[16][32768]=14
        nTriggers[16][65536]=4
        nTriggers[16][131072]=13
        nTriggers[32][0]=0
        nTriggers[32][1]=17
        nTriggers[32][2]=15
        nTriggers[32][4]=16
        nTriggers[32][8]=15
        nTriggers[32][16]=32
        nTriggers[32][32]=33
        nTriggers[32][64]=33
        nTriggers[32][128]=32
        nTriggers[32][256]=32
        nTriggers[32][512]=32
        nTriggers[32][1024]=32
        nTriggers[32][2048]=32
        nTriggers[32][4096]=32
        nTriggers[32][8192]=1
        nTriggers[32][16384]=34
        nTriggers[32][32768]=34
        nTriggers[32][65536]=1
        nTriggers[32][131072]=35
        nTriggers[64][0]=0
        nTriggers[64][1]=2
        nTriggers[64][2]=2
        nTriggers[64][4]=5
        nTriggers[64][8]=0
        nTriggers[64][16]=4
        nTriggers[64][32]=4
        nTriggers[64][64]=4
        nTriggers[64][128]=4
        nTriggers[64][256]=4
        nTriggers[64][512]=4
        nTriggers[64][1024]=4
        nTriggers[64][2048]=4
        nTriggers[64][4096]=4
        nTriggers[64][8192]=0
        nTriggers[64][16384]=4
        nTriggers[64][32768]=4
        nTriggers[64][65536]=0
        nTriggers[64][131072]=7
        nTriggers[128][0]=0
        nTriggers[128][1]=2
        nTriggers[128][2]=1
        nTriggers[128][4]=1
        nTriggers[128][8]=0
        nTriggers[128][16]=3
        nTriggers[128][32]=4
        nTriggers[128][64]=4
        nTriggers[128][128]=4
        nTriggers[128][256]=4
        nTriggers[128][512]=4
        nTriggers[128][1024]=4
        nTriggers[128][2048]=4
        nTriggers[128][4096]=0
        nTriggers[128][8192]=0
        nTriggers[128][16384]=4
        nTriggers[128][32768]=4
        nTriggers[128][65536]=0
        nTriggers[128][131072]=4
        nTriggers[256][0]=0
        nTriggers[256][1]=2
        nTriggers[256][2]=2
        nTriggers[256][4]=1
        nTriggers[256][8]=1
        nTriggers[256][16]=1
        nTriggers[256][32]=1
        nTriggers[256][64]=1
        nTriggers[256][128]=1
        nTriggers[256][256]=1
        nTriggers[256][512]=1
        nTriggers[256][1024]=1
        nTriggers[256][2048]=1
        nTriggers[256][4096]=1
        nTriggers[256][8192]=0
        nTriggers[256][16384]=2
        nTriggers[256][32768]=2
        nTriggers[256][65536]=0
        nTriggers[256][131072]=3
        nTriggers[512][0]=0
        nTriggers[512][1]=3
        nTriggers[512][2]=3
        nTriggers[512][4]=2
        nTriggers[512][8]=2
        nTriggers[512][16]=3
        nTriggers[512][32]=5
        nTriggers[512][64]=3
        nTriggers[512][128]=3
        nTriggers[512][256]=6
        nTriggers[512][512]=6
        nTriggers[512][1024]=7
        nTriggers[512][2048]=7
        nTriggers[512][4096]=3
        nTriggers[512][8192]=0
        nTriggers[512][16384]=13
        nTriggers[512][32768]=13
        nTriggers[512][65536]=0
        nTriggers[512][131072]=19
        nTriggers[1024][0]=0
        nTriggers[1024][1]=3
        nTriggers[1024][2]=2
        nTriggers[1024][4]=2
        nTriggers[1024][8]=2
        nTriggers[1024][16]=2
        nTriggers[1024][32]=2
        nTriggers[1024][64]=2
        nTriggers[1024][128]=2
        nTriggers[1024][256]=2
        nTriggers[1024][512]=2
        nTriggers[1024][1024]=2
        nTriggers[1024][2048]=2
        nTriggers[1024][4096]=2
        nTriggers[1024][8192]=1
        nTriggers[1024][16384]=2
        nTriggers[1024][32768]=2
        nTriggers[1024][65536]=1
        nTriggers[1024][131072]=2
        nTriggers[2048][0]=0
        nTriggers[2048][1]=4
        nTriggers[2048][2]=5
        nTriggers[2048][4]=3
        nTriggers[2048][8]=4
        nTriggers[2048][16]=6
        nTriggers[2048][32]=6
        nTriggers[2048][64]=7
        nTriggers[2048][128]=6
        nTriggers[2048][256]=8
        nTriggers[2048][512]=6
        nTriggers[2048][1024]=10
        nTriggers[2048][2048]=10
        nTriggers[2048][4096]=8
        nTriggers[2048][8192]=1
        nTriggers[2048][16384]=6
        nTriggers[2048][32768]=6
        nTriggers[2048][65536]=1
        nTriggers[2048][131072]=6
        nTriggers[4096][0]=0
        nTriggers[4096][1]=12
        nTriggers[4096][2]=12
        nTriggers[4096][4]=12
        nTriggers[4096][8]=8
        nTriggers[4096][16]=5
        nTriggers[4096][32]=10
        nTriggers[4096][64]=5
        nTriggers[4096][128]=5
        nTriggers[4096][256]=5
        nTriggers[4096][512]=5
        nTriggers[4096][1024]=5
        nTriggers[4096][2048]=5
        nTriggers[4096][4096]=5
        nTriggers[4096][8192]=0
        nTriggers[4096][16384]=5
        nTriggers[4096][32768]=4
        nTriggers[4096][65536]=2
        nTriggers[4096][131072]=1
        nTriggers[8192][0]=0
        nTriggers[8192][1]=1
        nTriggers[8192][2]=1
        nTriggers[8192][4]=1
        nTriggers[8192][8]=1
        nTriggers[8192][16]=1
        nTriggers[8192][32]=1
        nTriggers[8192][64]=1
        nTriggers[8192][128]=1
        nTriggers[8192][256]=1
        nTriggers[8192][512]=1
        nTriggers[8192][1024]=1
        nTriggers[8192][2048]=1
        nTriggers[8192][4096]=1
        nTriggers[8192][8192]=0
        nTriggers[8192][16384]=1
        nTriggers[8192][32768]=1
        nTriggers[8192][65536]=0
        nTriggers[8192][131072]=1
        nTriggers[16384][0]=0
        nTriggers[16384][1]=17
        nTriggers[16384][2]=28
        nTriggers[16384][4]=8
        nTriggers[16384][8]=5
        nTriggers[16384][16]=26
        nTriggers[16384][32]=32
        nTriggers[16384][64]=31
        nTriggers[16384][128]=30
        nTriggers[16384][256]=35
        nTriggers[16384][512]=36
        nTriggers[16384][1024]=36
        nTriggers[16384][2048]=38
        nTriggers[16384][4096]=30
        nTriggers[16384][8192]=1
        nTriggers[16384][16384]=47
        nTriggers[16384][32768]=50
        nTriggers[16384][65536]=1
        nTriggers[16384][131072]=51
        nTriggers[32768][0]=0
        nTriggers[32768][1]=5
        nTriggers[32768][2]=7
        nTriggers[32768][4]=11
        nTriggers[32768][8]=11
        nTriggers[32768][16]=13
        nTriggers[32768][32]=17
        nTriggers[32768][64]=15
        nTriggers[32768][128]=13
        nTriggers[32768][256]=17
        nTriggers[32768][512]=15
        nTriggers[32768][1024]=17
        nTriggers[32768][2048]=21
        nTriggers[32768][4096]=9
        nTriggers[32768][8192]=0
        nTriggers[32768][16384]=15
        nTriggers[32768][32768]=17
        nTriggers[32768][65536]=0
        nTriggers[32768][131072]=17
        nTriggers[65536][0]=0
        nTriggers[65536][1]=0
        nTriggers[65536][2]=0
        nTriggers[65536][4]=0
        nTriggers[65536][8]=0
        nTriggers[65536][16]=0
        nTriggers[65536][32]=0
        nTriggers[65536][64]=0
        nTriggers[65536][128]=0
        nTriggers[65536][256]=0
        nTriggers[65536][512]=0
        nTriggers[65536][1024]=0
        nTriggers[65536][2048]=0
        nTriggers[65536][4096]=0
        nTriggers[65536][8192]=0
        nTriggers[65536][16384]=0
        nTriggers[65536][32768]=0
        nTriggers[65536][65536]=4
        nTriggers[65536][131072]=0
        nTriggers[3][0]=0
        nTriggers[3][1]=31
        nTriggers[3][2]=18
        nTriggers[3][4]=17
        nTriggers[3][8]=9
        nTriggers[3][16]=11
        nTriggers[3][32]=12
        nTriggers[3][64]=10
        nTriggers[3][128]=10
        nTriggers[3][256]=9
        nTriggers[3][512]=9
        nTriggers[3][1024]=9
        nTriggers[3][2048]=9
        nTriggers[3][4096]=9
        nTriggers[3][8192]=3
        nTriggers[3][16384]=8
        nTriggers[3][32768]=9
        nTriggers[3][65536]=3
        nTriggers[3][131072]=9
        nTriggers[12][0]=0
        nTriggers[12][1]=16
        nTriggers[12][2]=20
        nTriggers[12][4]=17
        nTriggers[12][8]=13
        nTriggers[12][16]=25
        nTriggers[12][32]=31
        nTriggers[12][64]=23
        nTriggers[12][128]=26
        nTriggers[12][256]=16
        nTriggers[12][512]=15
        nTriggers[12][1024]=17
        nTriggers[12][2048]=17
        nTriggers[12][4096]=15
        nTriggers[12][8192]=6
        nTriggers[12][16384]=13
        nTriggers[12][32768]=16
        nTriggers[12][65536]=5
        nTriggers[12][131072]=16
        nTriggers[48][0]=0
        nTriggers[48][1]=30
        nTriggers[48][2]=28
        nTriggers[48][4]=29
        nTriggers[48][8]=29
        nTriggers[48][16]=47
        nTriggers[48][32]=48
        nTriggers[48][64]=49
        nTriggers[48][128]=47
        nTriggers[48][256]=48
        nTriggers[48][512]=48
        nTriggers[48][1024]=51
        nTriggers[48][2048]=48
        nTriggers[48][4096]=47
        nTriggers[48][8192]=6
        nTriggers[48][16384]=46
        nTriggers[48][32768]=48
        nTriggers[48][65536]=5
        nTriggers[48][131072]=48
        nTriggers[192][0]=0
        nTriggers[192][1]=4
        nTriggers[192][2]=3
        nTriggers[192][4]=6
        nTriggers[192][8]=0
        nTriggers[192][16]=7
        nTriggers[192][32]=8
        nTriggers[192][64]=8
        nTriggers[192][128]=8
        nTriggers[192][256]=8
        nTriggers[192][512]=8
        nTriggers[192][1024]=8
        nTriggers[192][2048]=8
        nTriggers[192][4096]=4
        nTriggers[192][8192]=0
        nTriggers[192][16384]=8
        nTriggers[192][32768]=8
        nTriggers[192][65536]=0
        nTriggers[192][131072]=11
        nTriggers[768][0]=0
        nTriggers[768][1]=5
        nTriggers[768][2]=5
        nTriggers[768][4]=3
        nTriggers[768][8]=3
        nTriggers[768][16]=4
        nTriggers[768][32]=6
        nTriggers[768][64]=4
        nTriggers[768][128]=4
        nTriggers[768][256]=7
        nTriggers[768][512]=7
        nTriggers[768][1024]=8
        nTriggers[768][2048]=8
        nTriggers[768][4096]=4
        nTriggers[768][8192]=0
        nTriggers[768][16384]=15
        nTriggers[768][32768]=15
        nTriggers[768][65536]=0
        nTriggers[768][131072]=22
        nTriggers[3072][0]=0
        nTriggers[3072][1]=7
        nTriggers[3072][2]=7
        nTriggers[3072][4]=5
        nTriggers[3072][8]=6
        nTriggers[3072][16]=8
        nTriggers[3072][32]=8
        nTriggers[3072][64]=9
        nTriggers[3072][128]=8
        nTriggers[3072][256]=10
        nTriggers[3072][512]=8
        nTriggers[3072][1024]=12
        nTriggers[3072][2048]=12
        nTriggers[3072][4096]=10
        nTriggers[3072][8192]=2
        nTriggers[3072][16384]=8
        nTriggers[3072][32768]=8
        nTriggers[3072][65536]=2
        nTriggers[3072][131072]=8
        nTriggers[131071][0]=0
        nTriggers[131071][1]=235
        nTriggers[131071][2]=216
        nTriggers[131071][4]=188
        nTriggers[131071][8]=116
        nTriggers[131071][16]=239
        nTriggers[131071][32]=283
        nTriggers[131071][64]=258
        nTriggers[131071][128]=250
        nTriggers[131071][256]=259
        nTriggers[131071][512]=253
        nTriggers[131071][1024]=269
        nTriggers[131071][2048]=276
        nTriggers[131071][4096]=210
        nTriggers[131071][8192]=18
        nTriggers[131071][16384]=277
        nTriggers[131071][32768]=288
        nTriggers[131071][65536]=25
        nTriggers[131071][131072]=350
        nTriggers[0][0]=0
        nTriggers[0][1]=0
        nTriggers[0][2]=0
        nTriggers[0][4]=0
        nTriggers[0][8]=0
        nTriggers[0][16]=0
        nTriggers[0][32]=0
        nTriggers[0][64]=0
        nTriggers[0][128]=0
        nTriggers[0][256]=0
        nTriggers[0][512]=0
        nTriggers[0][1024]=0
        nTriggers[0][2048]=0
        nTriggers[0][4096]=0
        nTriggers[0][8192]=0
        nTriggers[0][16384]=0
        nTriggers[0][32768]=0
        nTriggers[0][65536]=0
        nTriggers[0][131072]=0

        # in above map (generated from commented code below):
        #   first key is triggerType ENUM value
        #   second key is triggerPeriod ENUM value
        #   value is number of unprescaled triggers of that type in that period

        for triggerType,periods in nTriggers.items():
            for triggerPeriod,triggerCount in periods.items():
                assert len(TriggerAPI.getLowestUnprescaled(triggerPeriod,triggerType))==triggerCount

        # for triggerType in TriggerType:
        #     #print("\n------------- TriggerType:",triggerType)
        #     for triggerPeriod in TriggerPeriod:
        #         if triggerPeriod == TriggerPeriod.runNumber: break
        #         unprescaled = TriggerAPI.getLowestUnprescaled(triggerPeriod, triggerType)
        #         #print("- TriggerPeriod:",triggerPeriod)
        #         #print(unprescaled)
        #         print(f"nTriggers[{triggerType}][{triggerPeriod}]={len(unprescaled)}")

    def test_customGRL(self):
        from PathResolver import PathResolver
        grlPath = PathResolver.FindCalibFile("GoodRunsLists/data23_13p6TeV/20230828/data23_13p6TeV.periodAllYear_DetStatus-v110-pro31-06_MERGED_PHYS_StandardGRL_All_Good_25ns.xml")
        #grlPath = "/tmp/data23_13p6TeV.periodAllYear_DetStatus-v110-pro31-06_MERGED_PHYS_StandardGRL_All_Good_25ns.xml" # this was a local copy I modified to remove dodgy LBs
        self.assertNotEqual(grlPath,"")
        TriggerAPI.setCustomGRL(grlPath)


        for triggerType in TriggerType:
            if bin(triggerType.value).count('1')==1:continue # skip the 'single-type' types for this printout
            print("\n------------- TriggerType:",triggerType)
            unprescaled = TriggerAPI.getLowestUnprescaled(TriggerPeriod.customGRL, triggerType, livefraction=0.98)

            # do some more detailed analysis of the chains it picked
            ti = TriggerAPI.dbQueries[list(TriggerAPI.dbQueries.keys())[0]]
            for chain in unprescaled:
                for c in ti.triggerChains:
                    if c.name != chain: continue
                    print(TriggerType.toStr(c.triggerType),":",c.name,"liveFraction=",c.livefraction,"(activeLB=",c.activeLB,")")
                    # also print all the sufficiently unprescaled chains that were 'beaten' by this trigger
                    for c2 in ti.triggerChains:
                        if c2.triggerType != c.triggerType: continue
                        if not c2.isUnprescaled(0.99): continue
                        if c.isLowerThan(c2,ti.period)==1:
                            print("   ",c2.name,"liveFraction=",c2.livefraction,"(activeLB=",c2.activeLB,")")
                    break
            if len(unprescaled)==0:
                for c in ti.triggerChains:
                    if not c.passType(triggerType,TriggerType.UNDEFINED): continue
                    print("FAILED:",c.name,"liveFraction=",c.livefraction)


    def test_future2e34(self):
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        #flags.Input.Files = []
        flags.Trigger.triggerMenuSetup = "Physics_pp_run3_v1"
        flags.lock()
        TriggerAPI.setConfigFlags(flags)
        for triggerType in TriggerType:
            print("\n------------- TriggerType:",triggerType)
            triggers = TriggerAPI.getLowestUnprescaled(TriggerPeriod.future2e34, triggerType)
            #print(triggers)
            print(len(triggers))
        ti = TriggerAPI.dbQueries[(TriggerPeriod.future2e34,None)]
        nPrescaled=0;nUnprescaled=0;
        for c in ti.triggerChains:
            if c.isUnprescaled():
                nUnprescaled+=1 # these are the primary triggers
            else:
                nPrescaled+=1 # non-primary triggers
            if c.livefraction != 0 and c.livefraction != 1:
                print(c.name,c.livefraction)
        print("nPrescaled=",nPrescaled,"nUnprescaled=",nUnprescaled)

    def test_derivationList(self):
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        flags = initConfigFlags()
        #flags.Input.Files = []
        flags.Trigger.triggerMenuSetup = "Physics_pp_run3_v1"
        flags.lock()
        TriggerAPI.setConfigFlags(flags)
        allperiods = TriggerPeriod.future2e34
        trig_el  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el,  livefraction=0.8)
        trig_mu  = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu,  livefraction=0.8)
        trig_g   = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.g,   livefraction=0.8)
        trig_tau = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.tau, livefraction=0.8)
        ## Add cross-triggers for some sets
        trig_em = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el, additionalTriggerType=TriggerType.mu,  livefraction=0.8)
        trig_et = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.el, additionalTriggerType=TriggerType.tau, livefraction=0.8)
        trig_mt = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.mu, additionalTriggerType=TriggerType.tau, livefraction=0.8)
        # Note that this seems to pick up both isolated and non-isolated triggers already, so no need for extra grabs
        trig_txe = TriggerAPI.getLowestUnprescaledAnyPeriod(allperiods, triggerType=TriggerType.tau, additionalTriggerType=TriggerType.xe, livefraction=0.8)

        all = list(set(trig_el+trig_mu+trig_g+trig_em+trig_et+trig_mt+trig_em+trig_et+trig_mt+trig_txe+trig_tau))
        print(len(all))

    def test_derivationList2(self):
        # this config matches what is done in the derivation config:
        from AthenaConfiguration.AllConfigFlags import initConfigFlags
        from AthenaConfiguration.TestDefaults import defaultTestFiles
        flags = initConfigFlags()
        flags.Input.Files = defaultTestFiles.AOD_RUN3_DATA
        from DerivationFrameworkPhys.TriggerListsHelper import TriggerListsHelper
        triggerListsHelper = TriggerListsHelper(flags)
        chains = triggerListsHelper.Run3TriggerNamesNoTau + triggerListsHelper.Run3TriggerNamesTau
        print(len(chains))


if __name__ == "__main__":
    unittest.main(verbosity=2)
