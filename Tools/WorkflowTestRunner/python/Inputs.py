# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
import os

# corresponding EOS path: /eos/atlas/atlascerngroupdisk/data-art/grid-input/WorkflowReferences
references_CVMFS_path = os.environ.get("ATLAS_WORKFLOW_REFERENCES_PATH", "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/WorkflowReferences")
references_override_url = os.environ.get("ATLAS_WORKFLOW_REFERENCES_OVERRIDE_URL")

#####
# CI special input files
#####
from .Test import WorkflowRun
# common
input_HITS = {
    WorkflowRun.Run2: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc20/HITS/mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.simul.HITS.e6337_s3681/100events.HITS.pool.root",
    WorkflowRun.Run3: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/HITS/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8514_s4162/100events.HITS.pool.root",
}
input_HITS_minbias_low = {
    WorkflowRun.Run2: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.900311.Epos_minbias_inelastic_lowjetphoton.simul.HITS_FILT.e8341_s3687_s3704/*",
    WorkflowRun.Run3: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/HITS/mc23_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8514_e8528_s4157_s4120/5000events.HITS.pool.root.1",
    WorkflowRun.Run4: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8481_s4149_s4150/*"
}
input_HITS_minbias_high = {
    WorkflowRun.Run2: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/Tier0ChainTests/mc16_13TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.simul.HITS_FILT.e8341_s3687_s3704/*",
    WorkflowRun.Run3: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/HITS/mc23_13p6TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8514_e8528_s4157_s4120/500events.HITS.pool.root.1",
    WorkflowRun.Run4: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8481_s4149_s4150/*"
}
input_HITS_minbias_low_fulltruth = {
    WorkflowRun.Run4: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900311.Epos_minbias_inelastic_lowjetphoton.merge.HITS.e8481_s4149_s4151/*"
}
input_HITS_minbias_high_fulltruth = {
   WorkflowRun.Run4: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.800831.Py8EG_minbias_inelastic_highjetphotonlepton.merge.HITS.e8481_s4149_s4151/*"
}
input_HITS_unfiltered = {
    WorkflowRun.Run3: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/HITS/mc21_13p6TeV.900311.Epos_minbias_inelastic_lowjetphoton.simul.HITS.e8453_s3876/100events.HITS.pool.root",
}
input_HITS_neutrino = {
    WorkflowRun.Run2: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/mc16_13TeV.900149.PG_single_nu_Pt50.simul.HITS.e8307_s3482/HITS.24078104._234467.pool.root.1",
    WorkflowRun.Run3: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/HITS/mc23_13p6TeV.900149.PG_single_nu_Pt50.simul.HITS.e8514_e8528_s4153/10000events.HITS.pool.root",
    WorkflowRun.Run4: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.900149.PG_single_nu_Pt50.simul.HITS.e8481_s4149/HITS.33990267._000025.pool.root.1",
}

# simulation
input_EVNT = {
    WorkflowRun.Run2: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/SimCoreTests/valid1.410000.PowhegPythiaEvtGen_P2012_ttbar_hdamp172p5_nonallhad.evgen.EVNT.e4993.EVNT.08166201._000012.pool.root.1",
    WorkflowRun.Run3: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/EVNT/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8514/EVNT.32288062._002040.pool.root.1",
    WorkflowRun.Run4: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/PhaseIIUpgrade/EVNT/mc21_14TeV.601190.PhPy8EG_AZNLO_Zmumu.evgen.EVNT.e8481/EVNT.30802236._000313.pool.root.1",
}

# overlay
input_RDO_BKG = {
    WorkflowRun.Run2: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc20/RDO_BKG/mc20_13TeV.900149.PG_single_nu_Pt50.digit.RDO.e8307_s3482_s3136_d1715/100events.RDO.pool.root",
    WorkflowRun.Run3: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/RDO_BKG/mc23_13p6TeV.900149.PG_single_nu_Pt50.merge.RDO.e8514_e8528_s4153_d1907_d1908/100events.RDO.pool.root",
}
input_HITS_data_overlay = {
    WorkflowRun.Run2: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/DataOverlaySimulation/22.0/v1/mc16_13TeV.361107.PowhegPythia8EvtGen_AZNLOCTEQ6L1_Zmumu.HITS.pool.root",
}
input_BS_SKIM = {
    WorkflowRun.Run2: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/mc15_valid.00200010.overlay_streamsAll_2016_pp_1.skim.DRAW.r8381/DRAW.09331084._000146.pool.root.1",
}
input_BS_minimum_bias_overlay = {
    WorkflowRun.Run3: "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/OverlayTests/MinBiasOverlay/data23_hi.00463124.physics_MinBiasOverlay.daq.RAW._lb0305._SFO-16._0001.data",
}

# derivations
input_AOD = {
    WorkflowRun.Run2: {
        "data": "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data18/AOD/data18_13TeV.00357772.physics_Main.merge.AOD.r13286_p4910/1000events.AOD.27655096._000455.pool.root.1",
        "mc": "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc20/AOD/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.recon.AOD.e6337_s3681_r13145/1000events.AOD.27121237._002005.pool.root.1",
    },
    WorkflowRun.Run3: {
        "data": "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/data22/AOD/data22_13p6TeV.00431906.physics_Main.merge.AOD.r13928_p5279/1000events.AOD.30220215._001367.pool.root.1",
        "mc": "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc21/AOD/mc21_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8453_s3873_r13829/1000events.AOD.29787656._000153.pool.root.1",
        "af3": "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_a911_s4114_r15224/1000events.AOD.36216231._001858.pool.root.1",
    },
}
