#
#  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
#

'''@file TrigBjetMonitorAlgorithm.py
@author E. Nagy
@author T. Bold
@date 2020-05-27
@brief Example trigger python configuration for the Run III AthenaMonitoring package, based on the example by C Burton and P Onyisi
'''

def TrigBjetMonConfig(inputFlags):
    '''Function to configures some algorithms in the monitoring system.'''

    from AthenaCommon.Logging import logging
    log = logging.getLogger( 'TrigBjetMonitorAlgorithm.py' )

    ### STEP 1 ###
    # Define one top-level monitoring algorithm. The new configuration 
    # framework uses a component accumulator.
    # EN: Not needed for the moment
    # from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    # result = ComponentAccumulator()

    # The following class will make a sequence, configure algorithms, and link
    # them to GenericMonitoringTools
    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(inputFlags,'TrigBjetAthMonitorCfg')


    ### STEP 2 ###
    # Adding an algorithm to the helper. Here, we will use the example 
    # algorithm in the AthenaMonitoring package. Just pass the type to the 
    # helper. Then, the helper will instantiate an instance and set up the 
    # base class configuration following the inputFlags. The returned object 
    # is the algorithm.
    #The added algorithm must exist as a .h file 

    from AthenaConfiguration.ComponentFactory import CompFactory
    trigBjetMonAlg = helper.addAlgorithm(CompFactory.TrigBjetMonitorAlgorithm,'TrigBjetMonAlg')

    # You can actually make multiple instances of the same algorithm and give 
    # them different configurations
    #shifterTrigBjetMonAlg = helper.addAlgorithm(TrigBjetMonitorAlgorithm,'ShifterTrigBjetMonAlg')

    # # If for some really obscure reason you need to instantiate an algorithm
    # # yourself, the AddAlgorithm method will still configure the base 
    # # properties and add the algorithm to the monitoring sequence.
    # helper.AddAlgorithm(myExistingAlg)


    ### STEP 3 ###
    # Edit properties of a algorithm
    # some generic property
    # trigBjetMonAlg.RandomHist = True
    # to enable a trigger filter, for example:
    #trigBjetMonAlg.TriggerChain = 'HLT_mu26_ivarmedium'
    #trigBjetMonAlg.TriggerChain = 'HLT_e24_lhtight_nod0'
    trigBjetMonAlg.TriggerChain = ''

    ### STEP 4 ###
    # Read in the Bjet trigger chain names

    bjet_triglist = []
    # Trigger list from monitoring groups
    from TrigConfigSvc.TriggerConfigAccess import getHLTMonitoringAccess
    moniAccess=getHLTMonitoringAccess(inputFlags)
    BjetChainsE=moniAccess.monitoredChains(signatures="bJetMon",monLevels=["t0"])
    log.info (" ==> bjet_chainlist t0: %s", BjetChainsE)
    for chain in BjetChainsE:
        chain = "E_"+chain
        bjet_triglist.append(chain)
    BjetChainsS=moniAccess.monitoredChains(signatures="bJetMon",monLevels=["shifter"])
    log.info (" ==> bjet_chainlist shifter:  %s", BjetChainsS)
    for chain in BjetChainsS:
        chain = "S_"+chain
        bjet_triglist.append(chain)
    log.info (" ==> bjet_triglist:  %s", bjet_triglist)

    # Check if BeamType is 'collisions'
    # P.Onyisi's suggestion
    from AthenaConfiguration.Enums import BeamType
    CollisionRun = (inputFlags.Beam.Type == BeamType.Collisions)
    if CollisionRun:
        log.info (" ==> BeamType is collision: %s", inputFlags.Beam.Type)
    else:
        log.info (" ==> BeamType is not collision: %s", inputFlags.Beam.Type)
    trigBjetMonAlg.CollisionRun = CollisionRun


    # Set Express Stream Flag
    ExpressStreamFlag = inputFlags.Common.doExpressProcessing
    trigBjetMonAlg.ExpressStreamFlag = ExpressStreamFlag

    # Add some tools. N.B. Do not use your own trigger decion tool. Use the
    # standard one that is included with AthMonitorAlgorithm.

    # # Add a tool that doesn't have its own configuration function. In
    # # this example, no accumulator is returned, so no merge is necessary.
    # from MyDomainPackage.MyDomainPackageConf import MyDomainTool
    # trigBjetMonAlg.MyDomainTool = MyDomainTool()

    # Add a generic monitoring tool (a "group" in old language). The returned 
    # object here is the standard GenericMonitoringTool.
    BjetMonGroup = helper.addGroup(trigBjetMonAlg,'TrigBjetMonitor','HLT/BjetMon/')

    # Add a GMT for the other example monitor algorithm
    #shifterGroup = helper.addGroup(shifterTrigBjetMonAlg,'TrigBjetMonitor','HLT/BjetMon/Shifter/')


    ### STEP 5 ###
    # Configure histograms
    #NB! The histograms defined here must match the ones in the cxx file exactly


    # Offline PV histograms - common for all trigger chains

    BjetMonGroup.defineHistogram('Off_NVtx', title='Number of Offline Vertices;NVtx;Events',
                                 path='Shifter/Offline',xbins=100,xmin=0.0,xmax=100.)
    BjetMonGroup.defineHistogram('Off_xVtx', title='Offline xVtx;xVtx;Events',
                                 path='Shifter/Offline',xbins=200,xmin=-1.5,xmax=+1.5)
    BjetMonGroup.defineHistogram('Off_yVtx', title='Offline yVtx;yVtx;Events',
                                 path='Shifter/Offline',xbins=200,xmin=-1.5,xmax=+1.5)
    BjetMonGroup.defineHistogram('Off_zVtx', title='Offline zVtx;zVtx;Events',
                                 path='Shifter/Offline',xbins=500,xmin=-250.0,xmax=+250.0)

    # Histograms which depend on the trigger chain

    # mu-jet histograms

    AllChains = []
    for chain in bjet_triglist :
        AllChains.append(chain[2:])

        if "a10sd_cssk" in chain:  # GN2Xv01 chains

            HistName = 'LargeR_nJet_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Number of LargeR jets;nJet;Events',
                                             path='Expert/'+chain[2:],xbins=40,xmin=0.0,xmax=40.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Number of LargeR jets;nJet;Events',
                                             path='Shifter/'+chain[2:],xbins=40,xmin=0.0,xmax=40.0)

            HistName = 'GN2Xv01_phbb_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2Xv01_phbb probability;GN2Xv01_phbb;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)
            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2Xv01_phbb probability;GN2Xv01_phbb;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)

            HistName = 'GN2Xv01_ptop_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2Xv01_ptop probability;GN2Xv01_ptop;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)
            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2Xv01_ptop probability;GN2Xv01_ptop;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)

            HistName = 'GN2Xv01_pqcd_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2Xv01_pqcd probability;GN2Xv01_pqcd;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)
            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2Xv01_pqcd probability;GN2Xv01_pqcd;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)

            HistName = 'GN2Xv01_mv_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2Xv01_mv LLR;GN2Xv01_mv;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=-50.,xmax=50.)
            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2Xv01_mv LLR;GN2Xv01_mv;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=-50.,xmax=50.)

            HistName = 'GN2XTrig_phbb_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2XTrig_phbb probability;GN2XTrig_phbb;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)
            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2XTrig_phbb probability;GN2XTrig_phbb;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)

            HistName = 'GN2XTrig_ptop_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2XTrig_ptop probability;GN2XTrig_ptop;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)
            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2XTrig_ptop probability;GN2XTrig_ptop;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)

            HistName = 'GN2XTrig_pqcd_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2XTrig_pqcd probability;GN2XTrig_pqcd;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)
            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2XTrig_pqcd probability;GN2XTrig_pqcd;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)

            HistName = 'GN2XTrig_mv_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2XTrig_mv LLR;GN2XTrig_mv;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=-50.,xmax=50.)
            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2XTrig_mv LLR;GN2XTrig_mv;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=-50.,xmax=50.)

            HistName = 'LargeR_jetPt_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of LargeR jet Pt;Pt_jet;Events',
                                             path='Expert/'+chain[2:],xbins=100,xmin=0.0,xmax=750.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of LargeR jet Pt;Pt_jet;Events',
                                             path='Shifter/'+chain[2:],xbins=100,xmin=0.0,xmax=750.0)

            HistName = 'LargeR_jetMass_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of LargeR jet Mass;Mass;Events',
                                             path='Expert/'+chain[2:],xbins=100,xmin=0.0,xmax=750.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of LargeR jet Mass;Mass;Events',
                                             path='Shifter/'+chain[2:],xbins=100,xmin=0.0,xmax=750.0)

            HistName = 'LargeR_jetEta_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of LargeR jet Eta;Eta_jet;Events',
                                             path='Expert/'+chain[2:],xbins=100,xmin=-7.5,xmax=7.5)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of LargeR jet Eta;Eta_jet;Events',
                                             path='Shifter/'+chain[2:],xbins=100,xmin=-7.5,xmax=7.5)

            HistName = 'LargeR_jetEta_' + chain[2:] + ',LargeR_jetPhi_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName,type='TH2F',title='LargeR jet Phi vs LargeR jet Eta;Eta_jet;Phi_jet',
                                             path='Expert/'+chain[2:],xbins=20,xmin=-5.0,xmax=+5.0,ybins=20,ymin=-3.1416,ymax=+3.1416)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName,type='TH2F',title='LargeR jet Phi vs LargeR jet Eta;Eta_jet;Phi_jet',
                                             path='Shifter/'+chain[2:],xbins=20,xmin=-5.0,xmax=+5.0,ybins=20,ymin=-3.1416,ymax=+3.1416)


        elif chain[2:8] == 'HLT_mu' : # mu-jets

            HistName = 'nMuon_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of N_muon;N_muon;Events',
                                             path='Expert/'+chain[2:],xbins=10,xmin=0.,xmax=10.)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of N_muon;N_muon;Events',
                                             path='Shifter/'+chain[2:],xbins=10,xmin=0.,xmax=10.)

            HistName = 'muonPt_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of Pt_muon;Pt_muon;Events',
                                             path='Expert/'+chain[2:],xbins=100,xmin=0.0,xmax=750.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of Pt_muon;Pt_muon;Events',
                                             path='Shifter/'+chain[2:],xbins=100,xmin=0.0,xmax=750.0)

            HistName = 'muonEta_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of Eta_muon;Eta_muon;Events',
                                             path='Expert/'+chain[2:],xbins=100,xmin=-7.5,xmax=7.5)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of Eta_muon;Eta_muon;Events',
                                             path='Shifter/'+chain[2:],xbins=100,xmin=-7.5,xmax=7.5)

            HistName = 'nJet_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Number of jets;nJet;Events',
                                             path='Expert/'+chain[2:],xbins=40,xmin=0.0,xmax=40.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Number of jets;nJet;Events',
                                             path='Shifter/'+chain[2:],xbins=40,xmin=0.0,xmax=40.0)

            HistName = 'jetPt_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of Pt_jet;Pt_jet;Events',
                                             path='Expert/'+chain[2:],xbins=100,xmin=0.0,xmax=750.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of Pt_jet;Pt_jet;Events',
                                             path='Shifter/'+chain[2:],xbins=100,xmin=0.0,xmax=750.0)

            HistName = 'jetEta_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of Eta_jet;Eta_jet;Events',
                                             path='Expert/'+chain[2:],xbins=100,xmin=-7.5,xmax=7.5)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of Eta_jet;Eta_jet;Events',
                                             path='Shifter/'+chain[2:],xbins=100,xmin=-7.5,xmax=7.5)

            HistName = 'RatioPt_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of Pt_muon/Pt_jet;Pt_muon/Pt_jet;Events',
                                             path='Expert/'+chain[2:],xbins=100,xmin=0.,xmax=2.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of Pt_muon/Pt_jet;Pt_muon/Pt_jet;Events',
                                             path='Shifter/'+chain[2:],xbins=100,xmin=0.,xmax=2.0)

            HistName = 'RelPt_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of Pt_muon wrt jet direction;RelPt_muon;Events',
                                             path='Expert/'+chain[2:],xbins=100,xmin=0.,xmax=20.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of Pt_muon wrt jet direction;RelPt_muon;Events',
                                             path='Shifter/'+chain[2:],xbins=100,xmin=0.,xmax=20.0)


            HistName = 'wGN1_' + chain[2:] + ',RelPt_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName,type='TH2F',title='RelPt vs GN1 weight;GN1 weight;RelPt',
                                             path='Expert/'+chain[2:],xbins=20,xmin=-20.0,xmax=+20.0,ybins=20,ymin=0.,ymax=20.)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName,type='TH2F',title='RelPt vs GN1 weight;GN1 weight;RelPt',
                                             path='Shifter/'+chain[2:],xbins=20,xmin=-20.0,xmax=+20.0,ybins=20,ymin=0.,ymax=20.)

            HistName = 'wGN2_' + chain[2:] + ',RelPt_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName,type='TH2F',title='RelPt vs GN2 weight;GN2 weight;RelPt',
                                             path='Expert/'+chain[2:],xbins=20,xmin=-20.0,xmax=+20.0,ybins=20,ymin=0.,ymax=20.)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName,type='TH2F',title='RelPt vs GN2 weight;GN2 weight;RelPt',
                                             path='Shifter/'+chain[2:],xbins=20,xmin=-20.0,xmax=+20.0,ybins=20,ymin=0.,ymax=20.)


            HistName = 'DeltaR_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of DeltaR(muon,jet);Delta R;Events',
                                             path='Expert/'+chain[2:],xbins=100,xmin=0.,xmax=6.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of DeltaR(muon,jet);Delta R;Events',
                                             path='Shifter/'+chain[2:],xbins=100,xmin=0.,xmax=6.0)

            HistName = 'DeltaZ_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of DeltaZ(muon,jet);Delta Z;Events',
                                             path='Expert/'+chain[2:],xbins=100,xmin=0.,xmax=10.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of DeltaZ(muon,jet);Delta Z;Events',
                                             path='Shifter/'+chain[2:],xbins=100,xmin=0.,xmax=10.0)



            continue
        else :                      # b-jets

      # b-jet histograms

         # Primary vertex histograms

            # PV associated to jets

            HistName = 'PVz_jet_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of online zPV from jets;zPV from jets;Events',
                                             path='Expert/'+chain[2:],xbins=500,xmin=-250.0,xmax=250.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of online zPV from jets;zPV from jets;Events',
                                             path='Shifter/'+chain[2:],xbins=500,xmin=-250.0,xmax=250.0)

            HistName = 'PVx_jet_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of online xPV from jets;xPV from jets;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=-1.5,xmax=+1.5)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of online xPV from jets;xPV from jets;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=-1.5,xmax=+1.5)

            HistName = 'PVy_jet_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of online yPV from jets;yPV from jets;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=-1.5,xmax=+1.5)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of online yPV from jets;yPV from jets;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=-1.5,xmax=+1.5)

            # PV directly from SG

            HistName = 'nPV_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Number of online PV per event;nPV;Events',
                                             path='Expert/'+chain[2:],xbins=101,xmin=-1.0,xmax=100.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Number of online PV per event;nPV;Events',
                                             path='Shifter/'+chain[2:],xbins=101,xmin=-1.0,xmax=100.0)

            HistName = 'PVz_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of online zPV;zPV;Events',
                                             path='Expert/'+chain[2:],xbins=500,xmin=-250.0,xmax=250.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of online zPV;zPV;Events',
                                             path='Shifter/'+chain[2:],xbins=500,xmin=-250.0,xmax=250.0)

            HistName = 'PVx_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of online xPV;xPV;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=-1.5,xmax=+1.5)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of online xPV;xPV;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=-1.5,xmax=+1.5)

            HistName = 'PVy_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of online yPV;yPV;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=-1.5,xmax=+1.5)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of online yPV;yPV;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=-1.5,xmax=+1.5)



         # Difference of the online and offline PV coordinates

            HistName = 'DiffOnOffPVz_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Difference of online and offline zPV;zPVonline-zPVoffline;Events',
                                             path='Expert/'+chain[2:],xbins=400,xmin=-2.0,xmax=2.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Difference of online and offline zPV;zPVonline-zPVoffline;Events',
                                             path='Shifter/'+chain[2:],xbins=400,xmin=-2.0,xmax=2.0)

            HistName = 'DiffOnOffPVx_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Difference of online and offline xPV;xPVonline-xPVoffline;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=-0.1,xmax=0.1)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Difference of online and offline xPV;xPVonline-xPVoffline;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=-0.1,xmax=0.1)

            HistName = 'DiffOnOffPVy_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Difference of online and offline yPV;yPVonline-yPVoffline;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=-0.1,xmax=0.1)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Difference of online and offline yPV;yPVonline-yPVoffline;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=-0.1,xmax=0.1)



         # track histograms

            HistName = 'nTrack_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Number of tracks;nTrack;Events',
                                             path='Expert/'+chain[2:],xbins=40,xmin=0.0,xmax=40.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Number of tracks;nTrack;Events',
                                             path='Shifter/'+chain[2:],xbins=40,xmin=0.0,xmax=40.0)

            HistName = 'trkPt_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Pt of tracks;Pt;Events',
                                             path='Expert/'+chain[2:],xbins=100,xmin=0.0,xmax=50.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Pt of tracks;Pt;Events',
                                             path='Shifter/'+chain[2:],xbins=100,xmin=0.0,xmax=50.0)

            HistName = 'trkEta_' + chain[2:] + ',trkPhi_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName,type='TH2F',title='Phi vs Eta of tracks;Eta;Phi',
                                             path='Expert/'+chain[2:],xbins=20,xmin=-5.0,xmax=+5.0,ybins=20,ymin=-3.1416,ymax=+3.1416)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName,type='TH2F',title='Phi vs Eta of tracks;Eta;Phi',
                                             path='Shifter/'+chain[2:],xbins=20,xmin=-5.0,xmax=+5.0,ybins=20,ymin=-3.1416,ymax=+3.1416)

            HistName = 'd0_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of d0;d0;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=-2.0,xmax=2.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of d0;d0;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=-2.0,xmax=2.0)

            HistName = 'ed0_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of d0 uncertainty;sigma(d0);Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=0.,xmax=1.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of d0 uncertainty;sigma(d0);Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=0.,xmax=1.0)

            HistName = 'sd0_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of d0 significance;significance(d0);Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=0.,xmax=20.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of d0 significance;significance(d0);Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=0.,xmax=20.0)

            HistName = 'z0_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of z0;z0;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=-250.0,xmax=250.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of z0;z0;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=-250.0,xmax=250.0)

            HistName = 'ez0_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of z0 uncertainty;sigma(z0);Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=0.,xmax=5.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of z0 uncertainty;sigma(z0);Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=0.,xmax=5.0)

         # jet histograms

            HistName = 'nJet_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Number of jets;nJet;Events',
                                             path='Expert/'+chain[2:],xbins=40,xmin=0.0,xmax=40.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Number of jets;nJet;Events',
                                             path='Shifter/'+chain[2:],xbins=40,xmin=0.0,xmax=40.0)
            HistName = 'jetPt_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of Pt_jet;Pt_jet;Events',
                                             path='Expert/'+chain[2:],xbins=100,xmin=0.0,xmax=750.0)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of Pt_jet;Pt_jet;Events',
                                             path='Shifter/'+chain[2:],xbins=100,xmin=0.0,xmax=750.0)

            HistName = 'jetEta_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of Eta_jet;Eta_jet;Events',
                                             path='Expert/'+chain[2:],xbins=100,xmin=-7.5,xmax=7.5)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of Eta_jet;Eta_jet;Events',
                                             path='Shifter/'+chain[2:],xbins=100,xmin=-7.5,xmax=7.5)

            HistName = 'jetEta_' + chain[2:] + ',jetPhi_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName,type='TH2F',title='Phi vs Eta of jets;Eta_jet;Phi_jet',
                                             path='Expert/'+chain[2:],xbins=20,xmin=-5.0,xmax=+5.0,ybins=20,ymin=-3.1416,ymax=+3.1416)

            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName,type='TH2F',title='Phi vs Eta of jets;Eta_jet;Phi_jet',
                                             path='Shifter/'+chain[2:],xbins=20,xmin=-5.0,xmax=+5.0,ybins=20,ymin=-3.1416,ymax=+3.1416)

      # b-tagging quantities


            HistName = 'GN1_pu_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN1_pu probability;GN1_pu;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)
            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN1_pu probability;GN1_pu;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)

            HistName = 'GN1_pc_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN1_pc probability;GN1_pc;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)
            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN1_pc probability;GN1_pc;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)

            HistName = 'GN1_pb_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN1_pb probability;GN1_pb;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)
            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN1_pb probability;GN1_pb;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)

            HistName = 'GN1_mv_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN1_mv LLR;GN1_mv;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=-50.,xmax=50.)
            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN1_mv LLR;GN1_mv;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=-50.,xmax=50.)


            HistName = 'GN2_pu_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2_pu probability;GN2_pu;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)
            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2_pu probability;GN2_pu;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)

            HistName = 'GN2_pc_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2_pc probability;GN2_pc;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)
            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2_pc probability;GN2_pc;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)

            HistName = 'GN2_pb_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2_pb probability;GN2_pb;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)
            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2_pb probability;GN2_pb;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=0.0,xmax=1.0)

            HistName = 'GN2_mv_tr_' + chain[2:]
            if chain[0:1] == "E" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2_mv LLR;GN2_mv;Events',
                                             path='Expert/'+chain[2:],xbins=200,xmin=-50.,xmax=50.)
            if chain[0:1] == "S" :
                BjetMonGroup.defineHistogram(HistName, title='Distribution of GN2_mv LLR;GN2_mv;Events',
                                             path='Shifter/'+chain[2:],xbins=200,xmin=-50.,xmax=50.)



            continue


    log.info (" ==> In TrigBjetMonitorAlgorithm.py: AllChains list:  %s", AllChains)
    trigBjetMonAlg.AllChains = AllChains


    ### STEP 6 ###
    # Finalize. The return value should be a tuple of the ComponentAccumulator
    # and the sequence containing the created algorithms. If we haven't called
    # any configuration other than the AthMonitorCfgHelper here, then we can 
    # just return directly (and not create "result" above)
    return helper.result()
    
    # # Otherwise, merge with result object and return
    # acc = helper.result()
    # result.merge(acc)
    # return result

if __name__=='__main__':
    # Setup logs
    from AthenaCommon.Logging import log
    from AthenaCommon.Constants import DEBUG
    log.setLevel(DEBUG)

    # Set the Athena configuration flags
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    # Input files
    # AOD file to be run w/ MT access and Mon Groups implemented
    file = '/eos/user/e/enagy/ARTfiles/MCtest271022.AOD.pool.root'

    flags.Input.Files = [file]
    flags.Input.isMC = True

    # Output file (root)

    flags.Output.HISTFileName = 'TrigBjetMonitorOutput.root'

    # flags.Trigger.triggerMenuSetup="Physics_pp_v7_primaries"
    
    flags.lock()

    # Initialize configuration object, add accumulator, merge, and run.
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg 
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    cfg = MainServicesCfg(flags)
    cfg.merge(PoolReadCfg(flags))

    trigBjetMonitorAcc = TrigBjetMonConfig(flags)
    cfg.merge(trigBjetMonitorAcc)

    # If you want to turn on more detailed messages ...
    #trigBjetMonitorAcc.getEventAlgo('TrigBjetMonAlg').OutputLevel = 2 # DEBUG
    cfg.printConfig(withDetails=True) # set True for exhaustive info

    Nevents = 200
    cfg.run(Nevents)
    #cfg.run() #for all events. Use cfg.run(20) to only run on first 20 events


