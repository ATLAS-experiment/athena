#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def getMenu(flags):
    """getMenu returns a L1Menu accessor object"""

    from TrigConfigSvc.TriggerConfigAccess import getL1MenuAccess
    return getL1MenuAccess(flags)


if __name__ == '__main__':
    # print details for selected L1Topo Alforithms

    from pprint import pprint
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
        
    flags.Input.Files = ['/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data23/RAW/data23_13p6TeV.00452463.physics_Main.daq.RAW/540events.data23_13p6TeV.00452463.physics_Main.daq.RAW._lb0514._SFO-16._0004.data']

    flags.lock()

    menu = getMenu(flags)
    menu.printSummary()

    d_algs = menu.topoAlgorithms('TOPO')['decisionAlgorithms']
    print("====== Decision Algs =======")
    alg_klasses = ('DeltaRSqrIncl2',)

    for alg_name in d_algs:
        if d_algs[alg_name]['klass'] in alg_klasses:
            print(alg_name)
            pprint(d_algs[alg_name])
        
            print ('-------')

    s_algs = menu.topoAlgorithms('TOPO')['sortingAlgorithms']
    print("====== SORT Algs =======")
    alg_klasses = ('jJetSelect',)
    for alg_name in s_algs:
        if s_algs[alg_name]['klass'] in alg_klasses:
            print(alg_name)
            pprint(s_algs[alg_name])
        
        print ('-------')

    c_algs = menu.topoAlgorithms('MULTTOPO')['multiplicityAlgorithms']

    print("====== COUNT Algs =======")
    alg_klasses = ('cTauMultiplicity',)
    for alg_name in c_algs:
        if c_algs[alg_name]['klass'] in alg_klasses:
            print(alg_name)
            pprint(c_algs[alg_name])
            algname = c_algs[alg_name]
            threshold =  c_algs[alg_name]['threshold']
            pprint(menu.thresholds()[threshold])
        
        print ('-------')


