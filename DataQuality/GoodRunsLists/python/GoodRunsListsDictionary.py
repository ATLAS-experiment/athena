# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

def getGoodRunsLists():
    GRLDict={}
    ## RUN 3

    # 2024
    GRLDict['GRL2024'] = ['GoodRunsLists/data24_13p6TeV/20241118/physics_25ns_data24.xml']
    
    # 2023
    GRLDict['GRL2023'] = ['GoodRunsLists/data23_13p6TeV/20250321/data23_13p6TeV.periodAllYear_DetStatus-v133-pro31-11_MERGED_PHYS_StandardGRL_All_Good_25ns.xml']
    GRLDict['GRL2023_ignoreTRIG_JETCTPIN'] = ['GoodRunsLists/data23_13p6TeV/20250321/data23_13p6TeV.periodAllYear_DetStatus-v133-pro31-11_MERGED_PHYS_StandardGRL_All_Good_25ns_ignoreTRIG_JETCTPIN.xml']

    # 2022
    GRLDict['GRL2022'] = ['GoodRunsLists/data22_13p6TeV/20250321/data22_13p6TeV.periodAllYear_DetStatus-v134-pro28-09_MERGED_PHYS_StandardGRL_All_Good_25ns.xml']
    GRLDict['GRL2022_ignore_TRIGLAR'] = ['GoodRunsLists/data22_13p6TeV/20250321/data22_13p6TeV.periodAllYear_DetStatus-v134-pro28-10_MERGED_PHYS_StandardGRL_All_Good_25ns_ignore_TRIGLAR.xml']

    ## RUN 2

    # 2018
    GRLDict['GRL2018_Triggerno17e33prim'] = ['GoodRunsLists/data18_13TeV/20190318/data18_13TeV.periodAllYear_DetStatus-v102-pro22-04_Unknown_PHYS_StandardGRL_All_Good_25ns_Triggerno17e33prim.xml']      
    GRLDict['GRL2018_BjetHLT'] = ['GoodRunsLists/data18_13TeV/20200426/data18_13TeV.periodAllYear_DetStatus-v102-pro22-04_PHYS_StandardGRL_All_Good_25ns_BjetHLT.xml']
    
    # 2017
    GRLDict['GRL2017_Triggerno17e33prim'] = ['GoodRunsLists/data17_13TeV/20180619/data17_13TeV.periodAllYear_DetStatus-v99-pro22-01_Unknown_PHYS_StandardGRL_All_Good_25ns_Triggerno17e33prim.xml']
    GRLDict['GRL2017_BjetHLT_Normal2017'] = ['GoodRunsLists/data17_13TeV/20180619/data17_13TeV.periodAllYear_DetStatus-v99-pro22-01_Unknown_PHYS_StandardGRL_All_Good_25ns_BjetHLT_Normal2017.xml']
    GRLDict['GRL2017_JetHLT_Normal2017'] = ['GoodRunsLists/data17_13TeV/20180619/data17_13TeV.periodAllYear_DetStatus-v99-pro22-01_Unknown_PHYS_StandardGRL_All_Good_25ns_JetHLT_Normal2017.xml']

    # 2016
    GRLDict['GRL2016'] = ['GoodRunsLists/data16_13TeV/20180129/data16_13TeV.periodAllYear_DetStatus-v89-pro21-01_DQDefects-00-02-04_PHYS_StandardGRL_All_Good_25ns.xml']
    GRLDict['GRL2016_ignore_TOROID_STATUS'] = ['GoodRunsLists/data16_13TeV/20180129/data16_13TeV.periodAllYear_DetStatus-v89-pro21-01_DQDefects-00-02-04_PHYS_StandardGRL_All_Good_25ns_ignore_TOROID_STATUS.xml']
    GRLDict['GRL2016_BjetHLT'] = ['GoodRunsLists/data16_13TeV/20180129/data16_13TeV.periodAllYear_DetStatus-v89-pro21-01_DQDefects-00-02-04_PHYS_StandardGRL_All_Good_25ns_BjetHLT.xml']
    GRLDict['GRL2016_BjetHLT_Tight'] = ['GoodRunsLists/data16_13TeV/20180129/data16_13TeV.periodAllYear_DetStatus-v89-pro21-01_DQDefects-00-02-04_PHYS_StandardGRL_All_Good_25ns_BjetHLT_Tight.xml']


    # 2015
    GRLDict['GRL2015'] = ['GoodRunsLists/data15_13TeV/20170619/data15_13TeV.periodAllYear_DetStatus-v89-pro21-02_Unknown_PHYS_StandardGRL_All_Good_25ns.xml']

    return GRLDict
