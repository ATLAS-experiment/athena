# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# Files for use in configuration unit tests

# These samples can be access via shell script through commands such as:
# inputAODFile=$(python -c "from AthenaConfiguration.TestDefaults import defaultTestFiles; print(defaultTestFiles.AOD_RUN3_MC23[0])")

class defaultTestFiles:
    import os
    d = os.environ.get ("ATLAS_REFERENCE_DATA",
                        "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art")
    EVNT_RUN3_1K = [f"{d}/CampaignInputs/mc23/EVNT/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.evgen.EVNT.e8514/EVNT.32288062._002040.pool.root.1"] # MC23 10k events - max 1k events used by ART jobs SPOT jobs use full 10k events
    EVNT_RUN3_50 =  EVNT_RUN3_1K # FIXME replace with a 50 event file

    HITS_RUN2_1K = [f"{d}/Tier0ChainTests/mc16_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.simul.HITS.e6337_s3681/HITS.25836812._004813.pool.root.1"] #MC20 10k events
    HITS_RUN2 = HITS_RUN2_1K # FIXME replace with a 25 event file # TODO Update DigitizationTests(MT) jobs to use this input
    HITS_RUN3_2022 = [f"{d}/CampaignInputs/mc23/HITS/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8514_s4162/100events.HITS.pool.root"] #MC23a 100 events # TODO Update DigitizationTests(MT) jobs to use this input
    HITS_RUN3_2023 = [f"{d}/CampaignInputs/mc23/HITS/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8514_s4159/100events.HITS.pool.root"] #MC23d 100 events # TODO Update DigitizationTests(MT) jobs to use this input
    HITS_RUN3_2024 = HITS_RUN3_2022 # Temporary back-compatibility [f"{d}/CampaignInputs/mc23/HITS/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.merge.HITS.e8514_e8528_s4369/100events.HITS.pool.root"] #MC23e # TODO Update DigitizationTests(MT) jobs to use this input
    HITS_RUN3_HI = HITS_RUN3_2022 # Temporary back-compatibility
    HITS_RUN3 = HITS_RUN3_2022 # Temporary back-compatibility
    HITS_RUN4 = [f"{d}/PhaseIIUpgrade/HITS/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.e8481_s4149/HITS.33605501._000106.pool.root.1"] # 1k events - most jobs use 25/50 events, some use 400 events
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
    HITS_DATA_OVERLAY_HI_RUN3_2023 = [f"{d}//OverlayTests/DataOverlaySimulation/24.0/v1/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.simul.HITS.pool.root"]

    RAW_RUN1 =          [f"{d}/Tier0ChainTests/data12_8TeV.00209109.physics_JetTauEtmiss.merge.RAW._lb0186._SFO-1._0001.1"] # 1.3GB
    RAW_RUN2 =          [f"{d}/TrigP1Test/data17_13TeV.00327265.physics_EnhancedBias.merge.RAW._lb0100._SFO-1._0001.1_50evt"]
    RAW_RUN3 =          [f"{d}/Tier0ChainTests/TCT_Run3/data22_13p6TeV.00431493.physics_Main.daq.RAW._lb0525._SFO-16._0001.data"] # 1.2GB

    RAW_RUN1_DATA11 =   [f"{d}/RecJobTransformTests/high_mu-data11_7TeV.00179725.physics_JetTauEtmiss.merge.RAW._lb0021.data"] # 198MB # see also RecJobTransformTests/data11_7TeV.00191920.physics_JetTauEtmiss.merge.RAW._lb0257._SFO-9._0001.1.10evts 198MB
    RAW_RUN1_DATA12 =   [f"{d}/RecJobTransformTests/data12_8TeV.00209109.physics_JetTauEtmiss.merge.RAW._lb0186._SFO-1._0001.1"] # 1.3GB
    RAW_RUN2_DATA15 =   [f"{d}/RecJobTransformTests/data15_13TeV.00283429.physics_Main.daq.RAW._lb0154._SFO-1._0001.data"] # 2.5GB
    RAW_RUN2_DATA16 =   [f"{d}/RecJobTransformTests/data16_13TeV.00310809.physics_Main.daq.RAW._lb1219._SFO-2._0001.data"] # 2.5GB
    RAW_RUN2_DATA17 =   [f"{d}/RecJobTransformTests/data17_13TeV/data17_13TeV.00336782.physics_Main.daq.RAW._lb0875._SFO-3._0001.data"] # 2.5GB
    RAW_RUN2_DATA18 =   [f"{d}/RecJobTransformTests/data18_13TeV/data18_13TeV.00348885.physics_Main.daq.RAW._lb0827._SFO-8._0002.data"] # 1.8GB
    RAW_RUN3_DATA22 =   [f"{d}/RecJobTransformTests/data22_13p6TeV/data22_13p6TeV.00430536.physics_Main.daq.RAW/data22_13p6TeV.00430536.physics_Main.daq.RAW._lb1015._SFO-20._0001.data"] # 2.5GB
    RAW_RUN3_DATA24 =   [f"{d}/CampaignInputs/data24/RAW/data24_13p6TeV.00484909.physics_Main.daq.RAW/1627events_data24_13p6TeV.00484909.physics_Main.daq.RAW._lb0098._SFO-16._0001.data"] # 2.7GB
    RAW_BKG =           [f"{d}/OverlayTests/mc15_valid.00200010.overlay_streamsAll_2016_pp_1.skim.DRAW.r8381/DRAW.09331084._000146.pool.root.1"] # 192MB

    RDO_RUN2 = [f"{d}/CampaignInputs/mc20/RDO/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.recon.AOD.e6337_s3681_r13145/100events.RDO.pool.root"]
    RDO_RUN3 = [f"{d}/CampaignInputs/mc23/RDO/mc23_13p6TeV.801451.Py8EG_A3_NNPDF23LO_minbias_ND.recon.RDO.e8486_e8528_s4232_s4114_r15112/300events_RDO.39752217._004082.pool.root.1"] #low-mu minbias sample
    RDO_RUN4 = [f"{d}/PhaseIIUpgrade/RDO/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.RDO.e8481_s4149_r14700/RDO.33629020._000047.pool.root.1"]
    RDO_BKG_RUN2 = [f"{d}/OverlayTests/PresampledPileUp/22.0/Run2/large/mc20_13TeV.900149.PG_single_nu_Pt50.digit.RDO.e8307_s3482_s3136_d1715/RDO.26811908._031801.pool.root.1"]
    RDO_BKG_RUN3_2022 = [f"{d}/CampaignInputs/mc23/RDO_BKG/mc23_13p6TeV.900149.PG_single_nu_Pt50.merge.RDO.e8514_e8528_s4112_d1865_d1858/100events.RDO.pool.root"] #mc23a
    RDO_BKG_RUN3_2023 = [f"{d}/CampaignInputs/mc23/RDO_BKG/mc23_13p6TeV.900149.PG_single_nu_Pt50.merge.RDO.e8514_e8528_s4153_d1907_d1908/100events.RDO.pool.root"] #mc23d
    RDO_BKG_RUN3_2024 = RDO_BKG_RUN3_2022 # Temporary Back-compatibility
    RDO_BKG_RUN3 = RDO_BKG_RUN3_2022 # Temporary Back-compatibility
    RDO_BKG_RUN4 = [f"{d}/PhaseIIUpgrade/RDO_BKG/ATLAS-P2-RUN4-03-00-00/RUN4_presampling.mu200.withSuperCell.50events.RDO.pool.root"]
    RDO_BKG_HI_RUN3_2023 = [f"{d}/OverlayTests/MinBiasOverlay/processed/24.0/v2/data23_hi.00463124.physics_MinBiasOverlay.daq.RDO_BKG._lb0305._SFO-16._0001.pool.root"]

    ESD_RUN2_MC = [f"{d}/RecExRecoTest/mc16_13TeV.361022.Pythia8EvtGen_A14NNPDF23LO_jetjet_JZ2W.recon.ESD.e3668_s3170_r10572_homeMade.pool.root"] # MC16 TODO Update to MC20
    ESD_RUN3_MC = [f"{d}/CampaignInputs/mc23/ESD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.ESD.e8514_e8528_s4257_s4114_r15221/100events_ESD.36313379._000013.pool.root.1"]
    ESD = ESD_RUN3_MC # Backwards compatibility
    ESD_RUN3_DATA22 = [f"{d}/CampaignInputs/data22/ESD/data22_13p6TeV.00440543.physics_Main.recon.ESD.r14623_r14633/ESD.33252657._001240.pool.root.1"]

    AOD_RUN2_DATA = [f"{d}/CampaignInputs/data18/AOD/data18_13TeV.00357772.physics_Main.merge.AOD.r13286_p4910/1000events.AOD.27655096._000455.pool.root.1"]
    AOD_RUN2_MC = [f"{d}/CampaignInputs/mc20/AOD/mc20_13TeV.410470.PhPy8EG_A14_ttbar_hdamp258p75_nonallhad.recon.AOD.e6337_s3681_r13145/1000events.AOD.27121237._002005.pool.root.1"]
    AOD_RUN3_DATA = [f"{d}/CampaignInputs/data22/AOD/data22_13p6TeV.00431906.physics_Main.merge.AOD.r13928_p5279/1000events.AOD.30220215._001367.pool.root.1"]
    AOD_RUN3_MC = [f"{d}/CampaignInputs/mc23/AOD/mc23_13p6TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.recon.AOD.e8514_s4159_r14799/1000events.AOD.34124794._001345.pool.root.1"] #MC23c
    AOD_RUN4_MC = [f"{d}/PhaseIIUpgrade/AOD/ATLAS-P2-RUN4-03-00-00/mc21_14TeV.601229.PhPy8EG_A14_ttbar_hdamp258p75_SingleLep.merge.AOD.e8481_s4149_r14697_r14702/AOD.33629011._000002.pool.root.1"]

    # Files for dedicated detector setups
    RAW_RUN1_DATA11_HI =[f"{d}/RecJobTransformTests/data11_hi.00193321.physics_HardProbes.merge.RAW._lb0050._SFO-9._0002.1"] # 57MB
    RAW_RUN2_DATA15_HI =[f"{d}/RecJobTransformTests/data15_hi.00286711.physics_MinBiasOverlay.daq.RAW._lb0217._SFO-2._0001.data"] # 343MB
    RAW_RUN2_DATA18_HI =[f"{d}/RecJobTransformTests/data18_hi.00367384.physics_HardProbes.daq.RAW._lb0145._SFO-8._0001.data"] # 2.5GB
    RAW_RUN3_DATA22_HI =[f"{d}/RecJobTransformTests/data22_hi/RAWFiles/data22_hi.00440101.physics_MinBias.daq.RAW/data22_hi.00440101.physics_MinBias.daq.RAW._lb0214._SFO-11._0001.data"] # 4.0 GB
    RAW_RUN3_DATA23_HI =[f"{d}/RecJobTransformTests/data23_hi/data23_hi.00462809.physics_EnhancedBias.merge.RAW._lb0422._SFO-11._0001.1"] # 843MB
    RAW_RUN3_DATA24_HI =[f"{d}/RecJobTransformTests/data24_hi/data24_hi.00490145.physics_MinBias.daq.RAW._lb0142._SFO-15._0001.data"] # 140MB


    # AFP tests should update? https://its.cern.ch/jira/browse/ATLASRECTS-8109
    HITS_RUN2_MC20_AFP = [f"{d}/RecJobTransformTests/user.ladamczy/user.ladamczy.mc15_13TeV.860102.SuperChicPy8_gg_jj_CEP_70_new.evgen.HITS.e8419.v1_EXT0/user.ladamczy.28711500.EXT0._000009.HITS.pool.root"] #Run mc20e MC setup, with APF, without pileup
    RAW_RUN3_DATA22_AFP= [f"{d}/RecJobTransformTests/data22_13p6TeV/data22_13p6TeV.00435229.physics_Main.daq.RAW/data22_13p6TeV.00435229.physics_Main.daq.RAW._lb1526._SFO-12._0001.data"]
    RAW_RUN3_DATA24_IDCosmic =   [f"{d}/RecJobTransformTests/data24_cos/data24_cos.00475431.physics_IDCosmic.merge.RAW/data24_cos.00475431.physics_IDCosmic.merge.RAW._lb0002._SFO-ALL._0001.1"]
    RAW_RUN3_DATA24_CosmicCalo = [f"{d}/RecJobTransformTests/data24_13p6TeV.00485881.physics_CosmicCalo.merge.RAW/351events_data24_13p6TeV.00485881.physics_CosmicCalo.merge.RAW._lb0080._SFO-ALL._0001.1"]

class defaultGeometryTags:
    RUN1_2010 = "ATLAS-R1-2010-02-00-00"
    RUN1_2011 = "ATLAS-R1-2011-02-00-00"
    RUN1_2012 = "ATLAS-R1-2012-03-02-00"
    RUN2_2015 = "ATLAS-R2-2015-03-01-00"
    RUN2 = "ATLAS-R2-2016-01-00-01"
    RUN2_BEST_KNOWLEDGE = "ATLAS-R2-2016-01-02-01"
    RUN3 = "ATLAS-R3S-2021-03-02-00"
    RUN4 = "ATLAS-P2-RUN4-04-00-00"

    RUN2_ZDC = "ATLAS-R2-2016-01-03-00" # default ZDC geometry tag for Run2
    RUN3_ZDC23 = "ATLAS-R3S-2021-03-03-00" # default ZDC geometry tag for Run3 Year 2023
    RUN3_ZDC24 = "ATLAS-R3S-2021-03-04-00" # default ZDC geometry tag for Run3 Year 2024

    @staticmethod
    def autoconfigure(flags):
        if flags.GeoModel.AtlasVersion:
            return flags.GeoModel.AtlasVersion

        from AthenaConfiguration.Enums import LHCPeriod
        if flags.GeoModel.Run is LHCPeriod.Run1:
            return defaultGeometryTags.RUN1_2012
        if flags.GeoModel.Run is LHCPeriod.Run2:
            return defaultGeometryTags.RUN2
        if flags.GeoModel.Run is LHCPeriod.Run3:
            return defaultGeometryTags.RUN3
        if flags.GeoModel.Run is LHCPeriod.Run4:
            return defaultGeometryTags.RUN4


class defaultConditionsTags:
    RUN1_DATA = "COMCOND-BLKPA-RUN1-07"
    RUN2_DATA = "CONDBR2-BLKPA-RUN2-11"
    RUN2_MC = "OFLCOND-MC16-SDR-RUN2-12"
    RUN3_DATA = "CONDBR2-BLKPA-2024-05"
    RUN3_DATA22 = "CONDBR2-BLKPA-2022-17"
    RUN3_DATA23 = "CONDBR2-BLKPA-2023-07"
    RUN3_DATA24 = "CONDBR2-BLKPA-2024-05"
    RUN3_MC = "OFLCOND-MC23-SDR-RUN3-08"
    RUN4_MC = "OFLCOND-MC21-SDR-RUN4-03"

    @staticmethod
    def autoconfigure(flags):
        # TODO: uncomment when defaults are removed
        # if flags.IOVDb.GlobalTag:
        #     return flags.IOVDb.GlobalTag

        from AthenaConfiguration.Enums import LHCPeriod
        if flags.GeoModel.Run is LHCPeriod.Run1:
            raise ValueError("No default conditions tags for Run 1")
        if flags.GeoModel.Run is LHCPeriod.Run2:
            return defaultConditionsTags.RUN2_MC if flags.Input.isMC else defaultConditionsTags.RUN2_DATA
        if flags.GeoModel.Run is LHCPeriod.Run3:
            if flags.Input.isMC:
                return defaultConditionsTags.RUN3_MC
            raise ValueError("No default data conditions tag for Run 3")
        if flags.GeoModel.Run is LHCPeriod.Run4:
            return defaultConditionsTags.RUN4_MC
