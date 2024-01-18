# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# Files for use in configuration unit tests

class defaultTestFiles:
    import os
    d = os.environ.get ("ATLAS_REFERENCE_DATA",
                        "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art")
    EVNT = [f"{d}/CampaignInputs/mc23/EVNT/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8514/EVNT.32288062._002040.pool.root.1"]
    HITS_RUN2 = [f"{d}/Tier0ChainTests/mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.simul.HITS.e6337_s3681/HITS.25836812._004813.pool.root.1"]
    HITS_RUN3 = [f"{d}/CampaignInputs/mc21/HITS/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8453_s3873/50events.HITS.pool.root"]
    HITS_RUN4 = [f"{d}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8481_s4149/HITS.33605501._000106.pool.root.1"]
    HITS_RUN2_MINBIAS_HIGH = [
        f"{d}/Tier0ChainTests/mc16_13TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.simul.HITS_FILT.e8341_s3687_s3704/HITS_FILT.26106512._000149.pool.root.1",
        f"{d}/Tier0ChainTests/mc16_13TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.simul.HITS_FILT.e8341_s3687_s3704/HITS_FILT.26106512._000581.pool.root.1",
        f"{d}/Tier0ChainTests/mc16_13TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.simul.HITS_FILT.e8341_s3687_s3704/HITS_FILT.26106512._000717.pool.root.1",
    ]
    HITS_RUN2_MINBIAS_LOW = [
        f"{d}/Tier0ChainTests/mc16_13TeV.900311.Epos_minbias_inelastic_lowjetphoton.simul.HITS_FILT.e8341_s3687_s3704/HITS_FILT.26106626._000068.pool.root.1",
        f"{d}/Tier0ChainTests/mc16_13TeV.900311.Epos_minbias_inelastic_lowjetphoton.simul.HITS_FILT.e8341_s3687_s3704/HITS_FILT.26106626._000480.pool.root.1",
        f"{d}/Tier0ChainTests/mc16_13TeV.900311.Epos_minbias_inelastic_lowjetphoton.simul.HITS_FILT.e8341_s3687_s3704/HITS_FILT.26106626._000574.pool.root.1",
    ]
    HITS_DATA_OVERLAY = [f"{d}/OverlayTests/DataOverlaySimulation/22.0/v1/mc16_13TeV.361107.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zmumu.HITS.pool.root"]
    RAW_RUN1 = [f"{d}/Tier0ChainTests/data12_8TeV.00209109.physics_JetTauEtmiss.merge.RAW._lb0186._SFO-1._0001.1"]
    RAW_RUN2 = [f"{d}/TrigP1Test/data17_13TeV.00327265.physics_EnhancedBias.merge.RAW._lb0100._SFO-1._0001.1"]
    RAW_RUN3 = [f"{d}/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00431493.physics_Main.daq.RAW._lb0525._SFO-16._0001.data"]
    RAW_BKG = [f"{d}/OverlayTests/mc15_valid.00200010.overlay_streamsAll_2016_pp_1.skim.DRAW.r8381/DRAW.09331084._000146.pool.root.1"]
    RDO_RUN2 = [f"{d}/CampaignInputs/mc20/RDO/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.recon.AOD.e6337_s3681_r13145/100events.RDO.pool.root"]
    RDO_RUN4 = [f"{d}/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"]
    RDO_BKG_RUN2 = [f"{d}/OverlayTests/PresampledPileUp/22.0/Run2/large/mc20_13TeV.900149.PG_single_nu_Pt50.digit.RDO.e8307_s3482_s3136_d1715/RDO.26811908._031801.pool.root.1"]
    RDO_BKG_RUN3 = [f"{d}/CampaignInputs/mc21/RDO_BKG/mc21_13p6TeV.900149.PG_single_nu_Pt50.digit.RDO.e8453_e8455_s3864_d1761/50events.RDO.pool.root"]
    RDO_BKG_RUN4 = [f"{d}/PhaseIIUpgrade/RDO_BKG/ATLAS-P2-RUN4-03-00-00/RUN4_presampling.mu200.50events.RDO.pool.root"]
    ESD = [f"{d}/RecExRecoTest/mc16_13TeV.361022.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ2W.recon.ESD.e3668_s3170_r10572_homeMade.pool.root"]
    AOD_RUN2_DATA = [f"{d}/CampaignInputs/data18/AOD/data18_13TeV.00357772.physics_Main.merge.AOD.r13286_p4910/1000events.AOD.27655096._000455.pool.root.1"]
    AOD_RUN2_MC = [f"{d}/CampaignInputs/mc20/AOD/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.recon.AOD.e6337_s3681_r13145/1000events.AOD.27121237._002005.pool.root.1"]
    AOD_RUN3_DATA = [f"{d}/CampaignInputs/data22/AOD/data22_13p6TeV.00431906.physics_Main.merge.AOD.r13928_p5279/1000events.AOD.30220215._001367.pool.root.1"]
    AOD_RUN3_MC = [f"{d}/CampaignInputs/mc21/AOD/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8453_s3873_r13829/1000events.AOD.29787656._000153.pool.root.1"]
    AOD_RUN4_MC = [f"{d}/PhaseIIUpgrade/AOD/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.merge.AOD.e8481_s4149_r14697_r14702/AOD.33629011._000002.pool.root.1"]



class defaultGeometryTags:
    RUN1_2010 = "ATLAS-R1-2010-02-00-00"
    RUN1_2011 = "ATLAS-R1-2011-02-00-00"
    RUN1_2012 = "ATLAS-R1-2012-03-02-00"
    RUN2 = "ATLAS-R2-2016-01-00-01"
    RUN2_BEST_KNOWLEDGE = "ATLAS-R2-2016-01-02-01"
    RUN3 = "ATLAS-R3S-2021-03-02-00"
    RUN4 = "ATLAS-P2-RUN4-03-00-00"
