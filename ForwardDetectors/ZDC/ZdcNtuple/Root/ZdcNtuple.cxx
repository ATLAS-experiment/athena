/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <TSystem.h>
#include <TFile.h>
#include "xAODRootAccess/tools/Message.h"
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODCore/ShallowCopy.h"
#include "AthContainers/ConstAccessor.h"

#include <ZdcNtuple/ZdcNtuple.h>
#include <ZdcUtils/ZdcEventInfo.h>
#include <ZdcConditions/ZdcInjPulserAmpMap.h>

// this is needed to distribute the algorithm to the workers
//ClassImp(ZdcNtuple)

ZdcNtuple :: ZdcNtuple (const std::string& name, ISvcLocator *pSvcLocator)
  : EL::AnaAlgorithm(name, pSvcLocator),
    m_grl ("GoodRunsListSelectionTool/grl", this),
    //m_zdcAnalysisTool("ZDC::ZdcAnalysisTool/ZdcAnalysisTool", this),
    m_selTool( "InDet::InDetTrackSelectionTool/TrackSelectionTool", this )
{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().
  m_setupTrigHist = false;
  m_eventCounter = 0;

  declareProperty("isMC",  m_isMC = false, "MC mode");
  declareProperty("enableOutputTree",  enableOutputTree = false, "Enable output tree");
  declareProperty("enableOutputSamples",  enableOutputSamples = false, "Output ZDC and RPD raw data");
  declareProperty("enableTrigger",  enableTrigger = true, "comment");
  declareProperty("enableTracks",  enableTracks = false, "comment");
  declareProperty("trackLimit",  trackLimit = 500, "comment");
  declareProperty("enableID",  enableID = false, "turn on to enable ID tracks & vertices for physics streams");
  declareProperty("enableCalo",  enableCalo = false, "turn on to enable calorimeter energy info for physics streams");
  declareProperty("enableClusters",  enableClusters = false, "turn on to enable calo topo cluster info for physics streams");
  declareProperty("writeOnlyTriggers",  writeOnlyTriggers = false, "comment");
  declareProperty("useGRL",  useGRL = true, "comment");
  declareProperty("grlFilename",  grlFilename = "$ROOTCOREBIN/data/ZdcNtuple/data16_hip8TeV.periodAllYear_DetStatus-v86-pro20-19_DQDefects-00-02-04_PHYS_HeavyIonP_All_Good.xml", "comment");
  declareProperty("slimmed",  slimmed = false, "comment");
  declareProperty("zdcCalib",  zdcCalib = false, "zdc/ZDCCalib file");
  declareProperty("zdcInj",  zdcInj = false, "ZDC injected pulse");
  declareProperty("zdcLaser",  zdcLaser = false, "Run 2 ZDC Laser");
  declareProperty("zdcOnly", zdcOnly = false, "comment");
  declareProperty("zdcLowGainMode",  zdcLowGainMode = 0, "comment");

  declareProperty("flipDelay",  flipDelay = 0, "comment");
  declareProperty("reprocZdc",  reprocZdc = 0, "comment");
  declareProperty("auxSuffix",  auxSuffix = "", "comment");
  declareProperty("nsamplesZdc",  nsamplesZdc = 24, "number of samples, 7 = most of Run 2, 24,32,40 = Run 3");
  declareProperty("lhcf2022", lhcf2022 = false,"LHCf2022 general config");
  declareProperty("lhcf2022afp", lhcf2022afp = false,"LHCf2022 AFP-specific config");
  declareProperty("lhcf2022zdc", lhcf2022zdc = false,"LHCf2022 ZDC-specific config");
  declareProperty("pbpb2023", pbpb2023 = true, "PbPb2023 config");
  declareProperty("zdcConfig", zdcConfig = "PbPb2018", "argument to configure ZdcAnalysisTool");
  declareProperty("doZdcCalib", doZdcCalib = false, "perform ZDC energy calibration");
  declareProperty("enableZDC", enableZDC = true);
  declareProperty("enableRPD",enableRPD = false,"enable reading any RPD decorations");
  declareProperty("enableRPDAmp",enableRPDAmp = false,"enable reading RPD amplitudes (also requires enableRPD)");
  declareProperty("enableCentroid",enableCentroid = false,"enable reading centroid decorations (also requires enableRPD)");

  declareProperty( "TrackSelectionTool", m_selTool );

  m_zdcAnalysisTool.declarePropertyFor (this, "zdcAnalysisTool");
  
}

/*
StatusCode ZdcNtuple :: setupJob (EL::Job& job)
{
  // Here you put code that sets up the job on the submission object
  // so that it is ready to work with your algorithm, e.g. you can
  // request the D3PDReader service or add output files.  Any code you
  // put here could instead also go into the submission script.  The
  // sole advantage of putting it here is that it gets automatically
  // activated/deactivated when you add/remove the algorithm from your
  // job, which may or may not be of value to you.

  job.useXAOD ();
  ANA_CHECK( "setupJob()", xAOD::Init() ); // call before opening first file

  return StatusCode::SUCCESS;
}
*/


StatusCode ZdcNtuple :: initialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.

  ANA_MSG_DEBUG("Howdy from Initialize!");

  ANA_CHECK(m_zdcModuleContainerName.initialize());
  ANA_CHECK(m_zdcSumContainerName.initialize());
  ATH_CHECK(m_mcEventCollectionName.initialize());

  if (enableOutputTree)
  {

    ANA_CHECK( book(TTree("zdcTree", "ZDC Tree")));
    m_outputTree = tree( "zdcTree" );

    m_outputTree->Branch("runNumber", &t_runNumber, "runNumber/i");
    m_outputTree->Branch("eventNumber", &t_eventNumber, "eventNumber/i");
    m_outputTree->Branch("lumiBlock", &t_lumiBlock, "lumiBlock/i");
    m_outputTree->Branch("bunchGroup", &t_bunchGroup, "bunchGroup/b");
    m_outputTree->Branch("bcid", &t_bcid, "bcid/i");
    if (zdcInj) m_outputTree->Branch("vInj",&t_vInj,"vInj/F");
    m_outputTree->Branch("avgIntPerCrossing", &t_avgIntPerCrossing, "avgIntPerCrossing/F");
    m_outputTree->Branch("actIntPerCrossing", &t_actIntPerCrossing, "actIntPerCrossing/F");
    m_outputTree->Branch("trigger", &t_trigger, "trigger/l");
    m_outputTree->Branch("trigger_TBP", &t_trigger_TBP, "trigger_TBP/i");
    m_outputTree->Branch("tbp", &t_tbp, "tbp[16]/i");
    m_outputTree->Branch("tav", &t_tav, "tav[16]/i");
    m_outputTree->Branch("passBits", &t_passBits, "passBits/i");
    m_outputTree->Branch("extendedLevel1ID",&t_extendedLevel1ID,"extendedLevel1ID/i");
    m_outputTree->Branch("timeStamp",&t_timeStamp,"timeStamp/i");
    m_outputTree->Branch("timeStampNSOffset",&t_timeStampNSOffset,"timeStampNSOffset/i");
    m_outputTree->Branch("zdcEventInfoError",&t_zdcEventInfoError,"zdcEventInfoError/b");
    m_outputTree->Branch("zdcEventInfoErrorWord",&t_zdcEventInfoErrorWord,"zdcEventInfoErrorWord/i");
    // ZDC and RPD decoding errors
    m_outputTree->Branch("zdcDecodingError",&t_zdcDecodingError,"zdcDecodingError/b");
    m_outputTree->Branch("rpdDecodingError",&t_rpdDecodingError,"rpdDecodingError/b");

    if (enableZDC)
      {
	if (enableOutputSamples)
	  {
	    if (nsamplesZdc == 7)
	      {
		ANA_MSG_INFO("Setting up for 7 samples");
		m_outputTree->Branch("zdc_raw", &t_raw7, "zdc_raw[2][4][2][2][7]/s"); // 7 samples
	      }
	    
	    if (nsamplesZdc == 15)
	      {
		ANA_MSG_INFO("Setting up for 15 samples");
		m_outputTree->Branch("zdc_raw", &t_raw15, "zdc_raw[2][4][2][2][15]/s"); // 15 samples
	      }
	    
	    if (nsamplesZdc == 24)
	      {
		ANA_MSG_INFO("Setting up for 24 samples");
		m_outputTree->Branch("zdc_raw", &t_raw24, "zdc_raw[2][4][2][2][24]/s"); // 24 samples
		m_outputTree->Branch("rpd_raw", &t_rpdRaw, "rpd_raw[2][16][24]/s"); // 24 samples
	      }
	    if (nsamplesZdc == 32)
	      {
		ANA_MSG_INFO("Setting up for 32 samples");
		m_outputTree->Branch("zdc_raw", &t_raw32, "zdc_raw[2][4][2][2][32]/s"); // 24 samples
		m_outputTree->Branch("rpd_raw", &t_rpdRaw32, "rpd_raw[2][16][32]/s"); // 24 samples
	      }
	    if (nsamplesZdc == 40)
	      {
		ANA_MSG_INFO("Setting up for 40 samples");
		m_outputTree->Branch("zdc_raw", &t_raw40, "zdc_raw[2][4][2][2][40]/s"); // 24 samples
		m_outputTree->Branch("rpd_raw", &t_rpdRaw40, "rpd_raw[2][16][40]/s"); // 24 samples
	      }
	  }
	
	m_outputTree->Branch("zdc_ZdcAmp", &t_ZdcAmp, "zdc_ZdcAmp[2]/F");
	m_outputTree->Branch("zdc_ZdcAmpErr", &t_ZdcAmpErr, "zdc_ZdcAmpErr[2]/F");
	m_outputTree->Branch("zdc_ZdcEnergy", &t_ZdcEnergy, "zdc_ZdcEnergy[2]/F");
	m_outputTree->Branch("zdc_ZdcEnergyErr", &t_ZdcEnergyErr, "zdc_ZdcEnergyErr[2]/F");
	m_outputTree->Branch("zdc_ZdcNLEnergy", &t_ZdcNLEnergy, "zdc_ZdcNLEnergy[2]/F");
	m_outputTree->Branch("zdc_ZdcNLEnergyErr", &t_ZdcNLEnergyErr, "zdc_ZdcNLEnergyErr[2]/F");
	m_outputTree->Branch("zdc_ZdcTime", &t_ZdcTime, "zdc_ZdcTime[2]/F");
	m_outputTree->Branch("zdc_ZdcStatus", &t_ZdcStatus, "zdc_ZdcStatus[2]/S");
	m_outputTree->Branch("zdc_ZdcTrigEff", &t_ZdcTrigEff, "zdc_ZdcTrigEff[2]/F");
	m_outputTree->Branch("zdc_ZdcModuleMask", &t_ZdcModuleMask, "zdc_ZdcModuleMask/i");
	m_outputTree->Branch("zdc_ZdcLucrodTriggerSideAmp",&t_ZdcLucrodTriggerSideAmp,"zdc_ZdcLucrodTriggerSideAmp[2]/S");
	m_outputTree->Branch("zdc_ZdcLucrodTriggerSideAmpLG",&t_ZdcLucrodTriggerSideAmpLG,"zdc_ZdcLucrodTriggerSideAmpLG[2]/S");
	
	m_outputTree->Branch("zdc_ZdcModuleAmp", &t_ZdcModuleAmp, "zdc_ZdcModuleAmp[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModuleTime", &t_ZdcModuleTime, "zdc_ZdcModuleTime[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModuleFitAmp", &t_ZdcModuleFitAmp, "zdc_ZdcModuleFitAmp[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModuleFitT0", &t_ZdcModuleFitT0, "zdc_ZdcModuleFitT0[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModuleStatus", &t_ZdcModuleStatus, "zdc_ZdcModuleStatus[2][4]/i");
	m_outputTree->Branch("zdc_ZdcModuleChisq", &t_ZdcModuleChisq, "zdc_ZdcModuleChisq[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModuleCalibAmp", &t_ZdcModuleCalibAmp, "zdc_ZdcModuleCalibAmp[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModuleCalibTime", &t_ZdcModuleCalibTime, "zdc_ZdcModuleCalibTime[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModuleBkgdMaxFraction", &t_ZdcModuleBkgdMaxFraction, "zdc_ZdcModuleBkgdMaxFraction[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModuleAmpError", &t_ZdcModuleAmpError, "zdc_ZdcModuleAmpError[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModuleMinDeriv2nd", &t_ZdcModuleMinDeriv2nd, "zdc_ZdcModuleMinDeriv2nd[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModulePresample", &t_ZdcModulePresample, "zdc_ZdcModulePresample[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModulePreSampleAmp", &t_ZdcModulePreSampleAmp, "zdc_ZdcModulePreSampleAmp[2][4]/F");
	m_outputTree->Branch("zdc_ZdcLucrodTriggerAmp",&t_ZdcLucrodTriggerAmp,"zdc_ZdcLucrodTriggerAmp[2][4]/S");
	m_outputTree->Branch("zdc_ZdcLucrodTriggerAmpLG",&t_ZdcLucrodTriggerAmpLG,"zdc_ZdcLucrodTriggerAmpLG[2][4]/S");
	m_outputTree->Branch("zdc_ZdcModuleMaxADC",&t_ZdcModuleMaxADC,"zdc_ZdcModuleMaxADC[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModuleMaxADCHG",&t_ZdcModuleMaxADCHG,"zdc_ZdcModuleMaxADCHG[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModuleMaxADCLG",&t_ZdcModuleMaxADCLG,"zdc_ZdcModuleMaxADCLG[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModulePeakADCHG",&t_ZdcModulePeakADCHG,"zdc_ZdcModulePeakADCHG[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModulePeakADCLG",&t_ZdcModulePeakADCLG,"zdc_ZdcModulePeakADCLG[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModuleAmpLGRefit", &t_ZdcModuleAmpLGRefit, "zdc_ZdcModuleAmpLGRefit[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModuleAmpCorrLGRefit", &t_ZdcModuleAmpCorrLGRefit, "zdc_ZdcModuleAmpCorrLGRefit[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModuleT0LGRefit", &t_ZdcModuleT0LGRefit, "zdc_ZdcModuleT0LGRefit[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModuleT0SubLGRefit", &t_ZdcModuleT0SubLGRefit, "zdc_ZdcModuleT0SubLGRefit[2][4]/F");
	m_outputTree->Branch("zdc_ZdcModuleChisqLGRefit", &t_ZdcModuleChisqLGRefit, "zdc_ZdcModuleChisqLGRefit[2][4]/F");
	
	if(m_isMC){
	  //Modules
	  m_outputTree->Branch("zdc_ZdcModuleTruthTotal",&t_ZdcModuleTruthTotal,"zdc_ZdcModuleTruthTotal[2][7]/F");
	  m_outputTree->Branch("zdc_ZdcModuleTruthInvisible",&t_ZdcModuleTruthInvis,"zdc_ZdcModuleTruthInvisible[2][7]/F");
	  m_outputTree->Branch("zdc_ZdcModuleTruthEM",&t_ZdcModuleTruthEM,"zdc_ZdcModuleTruthEM[2][7]/F");
	  m_outputTree->Branch("zdc_ZdcModuleTruthNonEM",&t_ZdcModuleTruthNonEM,"zdc_ZdcModuleTruthNonEM[2][7]/F");
	  m_outputTree->Branch("zdc_ZdcModuleTruthEscaped",&t_ZdcModuleTruthEscaped,"zdc_ZdcModuleTruthEscaped[2][7]/F");
	  m_outputTree->Branch("zdc_ZdcModuleTruthNphotons",&t_ZdcModuleTruthNphotons,"zdc_ZdcModuleTruthNphotons[2][7]/i");
	  m_outputTree->Branch("zdc_RpdModuleTruthNphotons",&t_RpdModuleTruthNphotons,"zdc_RpdModuleTruthNphotons[2][16]/i");
	  
	  //Sums
	  m_outputTree->Branch("zdc_ZdcTruthTotal",&t_ZdcTruthTotal,"zdc_ZdcTruthTotal[2]/F");
	  m_outputTree->Branch("zdc_ZdcTruthInvisible",&t_ZdcTruthInvis, "zdc_ZdcTruthInvisible[2]/F");
	  m_outputTree->Branch("zdc_ZdcTruthEM",&t_ZdcTruthEM,"zdc_ZdcTruthEM[2]/F");
	  m_outputTree->Branch("zdc_ZdcTruthNonEM",&t_ZdcTruthNonEM,"zdc_ZdcTruthNonEM[2]/F");
	  m_outputTree->Branch("zdc_ZdcTruthEscaped",&t_ZdcTruthEscaped,"zdc_ZdcTruthEscaped[2]/F");
	  
	  //Event gen particles
	  m_outputTree->Branch("zdc_ZdcTruthParticlePosx",&t_ZdcTruthParticlePosx);
	  m_outputTree->Branch("zdc_ZdcTruthParticlePosy",&t_ZdcTruthParticlePosy);
	  m_outputTree->Branch("zdc_ZdcTruthParticlePosz",&t_ZdcTruthParticlePosz);
	  m_outputTree->Branch("zdc_ZdcTruthParticleTime",&t_ZdcTruthParticleTime);
	  m_outputTree->Branch("zdc_ZdcTruthParticlePx",&t_ZdcTruthParticlePx);
	  m_outputTree->Branch("zdc_ZdcTruthParticlePy",&t_ZdcTruthParticlePy);
	  m_outputTree->Branch("zdc_ZdcTruthParticlePz",&t_ZdcTruthParticlePz);
	  m_outputTree->Branch("zdc_ZdcTruthParticleEnergy",&t_ZdcTruthParticleEnergy);
	  m_outputTree->Branch("zdc_ZdcTruthParticlePid",&t_ZdcTruthParticlePid);
	  m_outputTree->Branch("zdc_ZdcTruthParticleStatus",&t_ZdcTruthParticleStatus);
	}
      }
    if (enableRPD)
      {
      if (enableRPDAmp) {
	m_outputTree->Branch("zdc_RpdChannelBaseline",&t_RpdChannelBaseline,"zdc_RpdChannelBaseline[2][16]/F");
	m_outputTree->Branch("zdc_RpdChannelPileupExpFitParams",&t_RpdChannelPileupExpFitParams,"zdc_RpdChannelPileupExpFitParams[2][16][2]/F");
	m_outputTree->Branch("zdc_RpdChannelPileupStretchedExpFitParams",&t_RpdChannelPileupStretchedExpFitParams,"zdc_RpdChannelPileupStretchedExpFitParams[2][16][3]/F");
	m_outputTree->Branch("zdc_RpdChannelPileupExpFitParamErrs",&t_RpdChannelPileupExpFitParamErrs,"zdc_RpdChannelPileupExpFitParamErrs[2][16][2]/F");
	m_outputTree->Branch("zdc_RpdChannelPileupStretchedExpFitParamErrs",&t_RpdChannelPileupStretchedExpFitParamErrs,"zdc_RpdChannelPileupStretchedExpFitParamErrs[2][16][3]/F");
	m_outputTree->Branch("zdc_RpdChannelPileupExpFitMSE",&t_RpdChannelPileupExpFitMSE,"zdc_RpdChannelPileupExpFitMSE[2][16]/F");
	m_outputTree->Branch("zdc_RpdChannelPileupStretchedExpFitMSE",&t_RpdChannelPileupStretchedExpFitMSE,"zdc_RpdChannelPileupStretchedExpFitMSE[2][16]/F");
	m_outputTree->Branch("zdc_RpdChannelAmplitude",&t_RpdChannelAmplitude,"zdc_RpdChannelAmplitude[2][16]/F");
	m_outputTree->Branch("zdc_RpdChannelAmplitudeCalib",&t_RpdChannelAmplitudeCalib,"zdc_RpdChannelAmplitudeCalib[2][16]/F");
	m_outputTree->Branch("zdc_RpdChannelMaxADC",&t_RpdChannelMaxADC,"zdc_RpdChannelMaxADC[2][16]/F");
	m_outputTree->Branch("zdc_RpdChannelMaxADCCalib",&t_RpdChannelMaxADCCalib,"zdc_RpdChannelMaxADCCalib[2][16]/F");
	m_outputTree->Branch("zdc_RpdChannelMaxSample",&t_RpdChannelMaxSample,"zdc_RpdChannelMaxSample[2][16]/i");
	m_outputTree->Branch("zdc_RpdChannelStatus",&t_RpdChannelStatus,"zdc_RpdChannelStatus[2][16]/i");
	m_outputTree->Branch("zdc_RpdChannelPileupFrac",&t_RpdChannelPileupFrac,"zdc_RpdChannelPileupFrac[2][16]/F");
	m_outputTree->Branch("zdc_RpdSideStatus",&t_RpdSideStatus,"zdc_RpdSideStatus[2]/i");
      }
    if (enableCentroid)
      {
	m_outputTree->Branch("zdc_centroidAvailable", &t_centroidDecorationsAvailable);
	m_outputTree->Branch("zdc_centroidEventValid", &t_centroidEventValid, "zdc_centroidEventValid/B");
	m_outputTree->Branch("zdc_centroidStatus", &t_centroidStatus, "zdc_centroidStatus[2]/i");
	m_outputTree->Branch("zdc_RPDChannelSubtrAmp", &t_RPDChannelSubtrAmp, "zdc_RPDChannelSubtrAmp[2][16]/F");
	m_outputTree->Branch("zdc_RPDSubtrAmpSum", &t_RPDSubtrAmpSum, "zdc_RPDSubtrAmpSum[2]/F");
	m_outputTree->Branch("zdc_xCentroidPreGeomCorPreAvgSubtr", &t_xCentroidPreGeomCorPreAvgSubtr, "zdc_xCentroidPreGeomCorPreAvgSubtr[2]/F");
	m_outputTree->Branch("zdc_yCentroidPreGeomCorPreAvgSubtr", &t_yCentroidPreGeomCorPreAvgSubtr, "zdc_yCentroidPreGeomCorPreAvgSubtr[2]/F");
	m_outputTree->Branch("zdc_xCentroidPreAvgSubtr", &t_xCentroidPreAvgSubtr, "zdc_xCentroidPreAvgSubtr[2]/F");
	m_outputTree->Branch("zdc_yCentroidPreAvgSubtr", &t_yCentroidPreAvgSubtr, "zdc_yCentroidPreAvgSubtr[2]/F");
	m_outputTree->Branch("zdc_xCentroid", &t_xCentroid, "zdc_xCentroid[2]/F");
	m_outputTree->Branch("zdc_yCentroid", &t_yCentroid, "zdc_yCentroid[2]/F");
	m_outputTree->Branch("zdc_xRowCentroid", &t_xRowCentroid, "zdc_xRowCentroid[2][4]/F");
	m_outputTree->Branch("zdc_yColCentroid", &t_yColCentroid, "zdc_yColCentroid[2][4]/F");
	m_outputTree->Branch("zdc_reactionPlaneAngle", &t_reactionPlaneAngle, "zdc_reactionPlaneAngle[2]/F");
	m_outputTree->Branch("zdc_cosDeltaReactionPlaneAngle", &t_cosDeltaReactionPlaneAngle, "zdc_cosDeltaReactionPlaneAngle/F");
      }
    }
    
    if (!(zdcCalib || zdcLaser || zdcOnly || zdcInj))
    {
      m_outputTree->Branch("mbts_in_e", &t_mbts_in_e, "mbts_in_e[2][8]/F");
      m_outputTree->Branch("mbts_in_t", &t_mbts_in_t, "mbts_in_t[2][8]/F");
      m_outputTree->Branch("mbts_out_e", &t_mbts_out_e, "mbts_out_e[2][4]/F");
      m_outputTree->Branch("mbts_out_t", &t_mbts_out_t, "mbts_out_t[2][4]/F");

      m_outputTree->Branch("T2mbts_in_e", &t_T2mbts_in_e, "T2mbts_in_e[2][8]/F");
      m_outputTree->Branch("T2mbts_in_t", &t_T2mbts_in_t, "T2mbts_in_t[2][8]/F");
      m_outputTree->Branch("T2mbts_out_e", &t_T2mbts_out_e, "T2mbts_out_e[2][4]/F");
      m_outputTree->Branch("T2mbts_out_t", &t_T2mbts_out_t, "T2mbts_out_t[2][4]/F");

      m_outputTree->Branch("L1ET", &t_L1ET, "L1ET/F");
      m_outputTree->Branch("L1ET24", &t_L1ET24, "L1ET24/F");

      m_outputTree->Branch("totalEt", &t_totalEt, "totalEt/F");
      m_outputTree->Branch("totalEt_TTsum", &t_totalEt_TTsum, "totalEt_TTsum/F");

      m_outputTree->Branch("totalEt24", &t_totalEt24, "totalEt24/F");
      m_outputTree->Branch("totalEt24_TTsum", &t_totalEt24_TTsum, "totalEt24_TTsum/F");

      m_outputTree->Branch("fcalEt", &t_fcalEt, "fcalEt/F");
      m_outputTree->Branch("fcalEtA", &t_fcalEtA, "fcalEtA/F");
      m_outputTree->Branch("fcalEtC", &t_fcalEtC, "fcalEtC/F");
      m_outputTree->Branch("fcalEtA_TT", &t_fcalEtA_TT, "fcalEtA_TT/F");
      m_outputTree->Branch("fcalEtC_TT", &t_fcalEtC_TT, "fcalEtC_TT/F");

      m_outputTree->Branch("nvx", &t_nvx, "nvx/I");
      m_outputTree->Branch("vx", &t_vx, "vx[3]/F");
      m_outputTree->Branch("pvindex", &t_pvindex, "pvindex/I");
      m_outputTree->Branch("vxntrk", &t_vxntrk, "vxntrk/I");
      m_outputTree->Branch("vx_trk_index", "vector<int>", &t_vx_trk_index);
      m_outputTree->Branch("vxtype", &t_vxtype, "vxtype/I");
      m_outputTree->Branch("vxcov", &t_vxcov, "vxcov[6]/F");
      m_outputTree->Branch("vxsumpt2", &t_vxsumpt2, "vxsumpt2/F");
      m_outputTree->Branch("nstrong", &t_nstrong, "nstrong/I");
      m_outputTree->Branch("puvxntrk", &t_puvxntrk, "puvxntrk/I");
      m_outputTree->Branch("puvxsumpt", &t_puvxsumpt, "puvxsumpt/F");
      m_outputTree->Branch("puvxz", &t_puvxz, "puvxz/F");
      m_outputTree->Branch("vxnlooseprimary", &t_vxnlooseprimary, "vxnlooseprimary/I");
      m_outputTree->Branch("vxnminbias", &t_vxnminbias, "vxnminbias/I");
      m_outputTree->Branch("vxngoodmuon", &t_vxngoodmuon, "vxngoodmuon/I");

      m_outputTree->Branch("t_nvtx", &t_nvtx, "nvtx/I");
      m_outputTree->Branch("vtx_type", "vector<int8_t>", &t_vtx_type);
      m_outputTree->Branch("vtx_x", "vector<float>", &t_vtx_x);
      m_outputTree->Branch("vtx_y", "vector<float>", &t_vtx_y);
      m_outputTree->Branch("vtx_z", "vector<float>", &t_vtx_z);
      m_outputTree->Branch("vtx_ntrk_all", "vector<int16_t>", &t_vtx_ntrk_all);
      m_outputTree->Branch("vtx_sumpt2_all", "vector<float>", &t_vtx_sumpt2_all);
      m_outputTree->Branch("vtx_ntrk", "vector<int16_t>", &t_vtx_ntrk);
      m_outputTree->Branch("vtx_sumpt2", "vector<float>", &t_vtx_sumpt2);
      m_outputTree->Branch("vtx_trk_index", "vector< vector<int16_t> >", &t_vtx_trk_index);

      m_outputTree->Branch("mbts_countA", &t_mbts_countA, "mbts_countA/s");
      m_outputTree->Branch("mbts_countC", &t_mbts_countC, "mbts_countC/s");
      m_outputTree->Branch("T2mbts_countAin", &t_T2mbts_countAin, "T2mbts_countAin/s");
      m_outputTree->Branch("T2mbts_countCin", &t_T2mbts_countCin, "T2mbts_countCin/s");
      m_outputTree->Branch("mbts_timeA", &t_mbts_timeA, "mbts_timeA/F");
      m_outputTree->Branch("mbts_timeC", &t_mbts_timeC, "mbts_timeC/F");
      m_outputTree->Branch("mbts_timeDiff", &t_mbts_timeDiff, "mbts_timeDiff/F");

      t_nclus = 0;
      m_outputTree->Branch("nclus", &t_nclus, "nclus/i");
      m_outputTree->Branch("clusEt", &t_clusEt, "clusEt/F");
      m_outputTree->Branch("clusEtMax", &t_clusEtMax, "clusEtMax/F");
      m_outputTree->Branch("clusetaMax", &t_clusetaMax, "clusetaMax/F");
      m_outputTree->Branch("clusphiMax", &t_clusphiMax, "clusphiMax/F");

      m_outputTree->Branch("cc_pt", "vector<float>", &t_cc_pt);
      m_outputTree->Branch("cc_eta", "vector<float>", &t_cc_eta);
      m_outputTree->Branch("cc_phi", "vector<float>", &t_cc_phi);
      m_outputTree->Branch("cc_e", "vector<float>", &t_cc_e);
      m_outputTree->Branch("cc_raw_m", "vector<float>", &t_cc_raw_m);
      m_outputTree->Branch("cc_raw_eta", "vector<float>", &t_cc_raw_eta);
      m_outputTree->Branch("cc_raw_phi", "vector<float>", &t_cc_raw_phi);
      m_outputTree->Branch("cc_raw_e", "vector<float>", &t_cc_raw_e);
      m_outputTree->Branch("cc_raw_samp", "vector<vector<float>>", &t_cc_raw_samp);
      m_outputTree->Branch("cc_sig", "vector<float>", &t_cc_sig);
      m_outputTree->Branch("cc_layer", "vector<int>", &t_cc_layer);

      m_outputTree->Branch("edgeGapA", &t_edgeGapA, "edgeGapA/F");
      m_outputTree->Branch("edgeGapC", &t_edgeGapC, "edgeGapC/F");

      m_outputTree->Branch("ntrk", &t_ntrk, "ntrk/i");
      if (enableTracks)
        {
          m_outputTree->Branch("trk_pt", "vector<float>", &t_trk_pt);
          m_outputTree->Branch("trk_eta", "vector<float>", &t_trk_eta);
          m_outputTree->Branch("trk_theta", "vector<float>", &t_trk_theta);
          m_outputTree->Branch("trk_phi", "vector<float>", &t_trk_phi);
          m_outputTree->Branch("trk_e", "vector<float>", &t_trk_e);
          m_outputTree->Branch("trk_index", "vector<int>", &t_trk_index);
          m_outputTree->Branch("trk_d0", "vector<float>", &t_trk_d0);
          m_outputTree->Branch("trk_z0", "vector<float>", &t_trk_z0);
          m_outputTree->Branch("trk_vz", "vector<float>", &t_trk_vz);
          m_outputTree->Branch("trk_vtxz", "vector<float>", &t_trk_vtxz);
          m_outputTree->Branch("trk_pixeldEdx", "vector<float>", &t_trk_pixeldEdx);
          m_outputTree->Branch("trk_charge", "vector<int8_t>", &t_trk_charge);
          m_outputTree->Branch("trk_quality", "vector<int16_t>", &t_trk_quality);
          m_outputTree->Branch("trk_nPixHits", "vector<uint8_t>", &t_trk_nPixHits);
          m_outputTree->Branch("trk_nSctHits", "vector<uint8_t>", &t_trk_nSctHits);
          m_outputTree->Branch("trk_nPixDead", "vector<uint8_t>", &t_trk_nPixDead);
          m_outputTree->Branch("trk_nSctDead", "vector<uint8_t>", &t_trk_nSctDead);
          m_outputTree->Branch("trk_nPixHoles", "vector<uint8_t>", &t_trk_nPixHoles);
          m_outputTree->Branch("trk_nSctHoles", "vector<uint8_t>", &t_trk_nSctHoles);
          m_outputTree->Branch("trk_nTrtHits", "vector<uint8_t>", &t_trk_nTrtHits);
          m_outputTree->Branch("trk_nTrtOutliers", "vector<uint8_t>", &t_trk_nTrtOutliers);
          m_outputTree->Branch("trk_inPixHits", "vector<uint8_t>", &t_trk_inPixHits);
          m_outputTree->Branch("trk_exPixHits", "vector<uint8_t>", &t_trk_exPixHits);
          m_outputTree->Branch("trk_ninPixHits", "vector<uint8_t>", &t_trk_ninPixHits);
          m_outputTree->Branch("trk_nexPixHits", "vector<uint8_t>", &t_trk_nexPixHits);
        }

      if( lhcf2022 || lhcf2022zdc || lhcf2022afp){
	m_outputTree->Branch("nProtons", &nProtons, "nProtons/i");
	m_outputTree->Branch("proton_pt", "vector<double>", &proton_pt);
	m_outputTree->Branch("proton_eta", "vector<double>", &proton_eta);
	m_outputTree->Branch("proton_phi", "vector<double>", &proton_phi);
	m_outputTree->Branch("proton_e", "vector<double>", &proton_e);
	m_outputTree->Branch("proton_side", "vector<int>", &proton_side);
	m_outputTree->Branch("proton_eLoss", "vector<double>", &proton_eLoss);
	m_outputTree->Branch("proton_t", "vector<double>", &proton_t);
	m_outputTree->Branch("proton_track_stationID", "vector<vector<int>>", &proton_track_stationID);
	m_outputTree->Branch("proton_track_nClusters", "vector<vector<int>>", &proton_track_nClusters);
	m_outputTree->Branch("proton_track_xLocal", "vector<vector<float>>", &proton_track_xLocal);
	m_outputTree->Branch("proton_track_yLocal", "vector<vector<float>>", &proton_track_yLocal);
	m_outputTree->Branch("proton_track_zLocal", "vector<vector<float>>", &proton_track_zLocal);
	m_outputTree->Branch("proton_track_xSlope", "vector<vector<float>>", &proton_track_xSlope);
	m_outputTree->Branch("proton_track_ySlope", "vector<vector<float>>", &proton_track_ySlope);
      }

    }
  }
  
  ANA_MSG_DEBUG("Anti-howdy from Initialize!");
  
  
  // Here you do everything that you need to do after the first input
  // file has been connected and before the first event is processed,
  // e.g. create additional histograms based on which variables are
  // available in the input files.  You can also create all of your
  // histograms and trees in here, but be aware that this method
  // doesn't get called if no events are processed.  So any objects
  // you create here won't be available in the output if you have no
  // input events.

  ANA_MSG_INFO("enableOutputTree = " << enableOutputTree);
  ANA_MSG_INFO("writeOnlyTriggers = " << writeOnlyTriggers);
  ANA_MSG_INFO("enableOutputSamples = " << enableOutputSamples);
  ANA_MSG_INFO("enableTrigger = " << enableTrigger);
  ANA_MSG_INFO("enableRPD = " << enableRPD);
  ANA_MSG_INFO("enableCentroid = " << enableCentroid);
  ANA_MSG_INFO("zdcCalib = " << zdcCalib);
  ANA_MSG_INFO("zdcInj = " << zdcInj);
  ANA_MSG_INFO("zdcLaser = " << zdcLaser);
  ANA_MSG_INFO("zdcConfig = " << zdcConfig);
  ANA_MSG_INFO("reprocZdc = " << reprocZdc);
  ANA_MSG_INFO("auxSuffix = " << auxSuffix );
  ANA_MSG_INFO("zdcLowGainMode = " << zdcLowGainMode);
  ANA_MSG_INFO("enableID = " << enableID);
  ANA_MSG_INFO("enableCalo = " << enableCalo);
  ANA_MSG_INFO("enableClusters = " << enableClusters);
  ANA_MSG_INFO("trackLimit = " << trackLimit);
  ANA_MSG_INFO("trackLimitReject = " << trackLimitReject);
  ANA_MSG_INFO("isMC = " << m_isMC);
  ANA_MSG_INFO("useGRL = " <<  useGRL);
  ANA_MSG_INFO("grlFilename = " <<  grlFilename);
  ANA_MSG_INFO("slimmed = " <<  slimmed);
  ANA_MSG_INFO("zdcOnly = " << zdcOnly);
  ANA_MSG_INFO("flipDelay = " <<  flipDelay);
  ANA_MSG_INFO("nsamplesZdc = " <<  nsamplesZdc);
  ANA_MSG_INFO("lhcf2022 = " << lhcf2022);
  ANA_MSG_INFO("lhcf2022afp = " << lhcf2022afp);
  ANA_MSG_INFO("lhcf2022zdc = " << lhcf2022zdc);
  ANA_MSG_INFO("doZdcCalib = " << doZdcCalib);

  ANA_MSG_DEBUG("initialize: Initialize!");


  // GRL

  if (useGRL)
  {
    const char* fullGRLFilePath = gSystem->ExpandPathName (grlFilename.c_str());
    ANA_MSG_INFO("GRL: " << fullGRLFilePath);
    std::vector<std::string> vecStringGRL;
    vecStringGRL.push_back(fullGRLFilePath);
    ANA_CHECK(m_grl.setProperty( "GoodRunsListVec", vecStringGRL));
    ANA_CHECK(m_grl.setProperty("PassThrough", false)); // if true (default) will ignore result of GRL and will just pass all events
    ANA_CHECK(m_grl.initialize());

  }

  if (enableTracks)
    {
      ANA_CHECK(m_selTool.retrieve());
    }

  if (enableTrigger) // HLT related
  {
    ANA_MSG_INFO("Trying to initialize TDT");
    ANA_CHECK(m_trigDecisionTool.retrieve());
  }

  // ZDC re-reco tool
  if (reprocZdc)
  {
    m_zdcAnalysisTool.setTypeAndName("ZDC::ZdcAnalysisTool/ZdcAnalysisTool");

    ANA_MSG_INFO("Trying to configure ZDC Analysis Tool!");

    ANA_CHECK(m_zdcAnalysisTool.setProperty("FlipEMDelay", flipDelay));
    ANA_CHECK(m_zdcAnalysisTool.setProperty("LowGainMode", zdcLowGainMode));
    ANA_CHECK(m_zdcAnalysisTool.setProperty("DoCalib", doZdcCalib));
    ANA_CHECK(m_zdcAnalysisTool.setProperty("Configuration", zdcConfig));
    ANA_CHECK(m_zdcAnalysisTool.setProperty("AuxSuffix", auxSuffix));
    ANA_CHECK(m_zdcAnalysisTool.setProperty("ForceCalibRun", -1));
    
    ANA_MSG_INFO("Setting up zdcConfig=" << zdcConfig);
    if (zdcConfig=="LHCf2022")
      {
	ANA_CHECK(m_zdcAnalysisTool.setProperty("DoTrigEff", false)); // for now
	ANA_CHECK(m_zdcAnalysisTool.setProperty("DoTimeCalib", false)); // for now
	ANA_CHECK(m_zdcAnalysisTool.setProperty("Configuration", "LHCf2022"));
      }
    else if (zdcConfig == "PbPb2018")
      {
	ANA_CHECK(m_zdcAnalysisTool.setProperty("Configuration", "PbPb2018"));	
      }
    else if (zdcConfig == "pPb2016")
      {
	ANA_CHECK(m_zdcAnalysisTool.setProperty("Configuration", "pPb2016"));
      }
    else if (zdcConfig == "PbPb2015")
      {
	ANA_CHECK(m_zdcAnalysisTool.setProperty("Configuration", "PbPb2015"));
      }
    
    if (flipDelay)
      ANA_MSG_INFO("FLIP ZDC DELAY IN EM MODULES");
    else
      ANA_MSG_INFO("NO FLIP ZDC DELAY IN EM MODULES");


    ANA_MSG_INFO("Trying to initialize ZDC Analysis Tool!");
    ANA_CHECK(m_zdcAnalysisTool.initialize());
  }
  
  if (zdcInj)
    {
      m_zdcInjPulserAmpMap = std::make_shared<ZdcInjPulserAmpMap>();
      ATH_MSG_INFO( "Using JSON file for injector-pulse voltage at path " << m_zdcInjPulserAmpMap->getFilePath() );
    }
  
  return StatusCode::SUCCESS;
}

  
  
StatusCode ZdcNtuple :: execute ()
{
  // Here you do everything that needs to be done on every single
  // events, e.g. read input variables, apply cuts, and fill
  // histograms and trees.  This is where most of your actual analysis
  // code will go.


  // prefilter with a track limit

  if (!evtStore())
  {
    ANA_MSG_INFO("*** No event found! ***");
    return StatusCode::SUCCESS;
  }

  ANA_CHECK(evtStore()->retrieve( m_eventInfo, "EventInfo"));
  processEventInfo();
  if (zdcInj){
  	processVInjInfo();
  }

  //tracks used to go here

  m_trackParticles = 0;

  if ((!(zdcCalib || zdcLaser || zdcOnly || zdcInj)) && enableID)
  {
    ANA_MSG_DEBUG("Trying to extract InDetTrackParticles from evtStore()=" << evtStore());
    ANA_CHECK(evtStore()->retrieve( m_trackParticles, "InDetTrackParticles") );
    size_t n = m_trackParticles->size();
    ANA_MSG_DEBUG("Done w/ extracting InDetTrackParticles with size = " << n);

    if (n > trackLimit && trackLimitReject)  return StatusCode::SUCCESS;
  }

  bool passTrigger = true;
  m_trigDecision = 0;
  if (enableTrigger)
  {
    ANA_CHECK(evtStore()->retrieve( m_trigDecision, "xTrigDecision"));
    if (!m_setupTrigHist) setupTriggerHistos();
    passTrigger = processTriggerDecision();
  }

  if (reprocZdc){
    ANA_MSG_INFO ("Reprocessing ZDC in ZdcNtuple");
    ANA_CHECK(m_zdcAnalysisTool->reprocessZdc());
  }else{
    ANA_MSG_DEBUG ("No ZDC reprocessing");
  }

  processZdcNtupleFromModules(); // same model in both cases -- processZdcNtuple() goes straight to the anlaysis tool, which is good for debugging

  if(m_isMC){
    processMCEventCollection();
  }

  if (!(zdcCalib || zdcLaser || zdcOnly || zdcInj))
  {

    // PLEASE NOTE: the commented sections here will be restored once we have a better sense of the Run 3 HI data

    // Global E_T quantities for centrality
    if (enableCalo){
	    ANA_CHECK(evtStore()->retrieve( m_caloSums, "CaloSums") );
	    ANA_CHECK(evtStore()->retrieve( m_eventShapes, "HIEventShape") );

	    m_lvl1EnergySumRoI = 0;
	    ANA_CHECK(evtStore()->retrieve( m_lvl1EnergySumRoI, "LVL1EnergySumRoI") );

    	processFCal();
    } 

    // MBTS quantities, but may require a derivation to be accessible (required STDM6 in pp)
    //ANA_CHECK(evtStore()->retrieve( m_mbtsInfo, "MBTSForwardEventInfo") );
    //ANA_CHECK(evtStore()->retrieve( m_mbtsModules, "MBTSModules") );
    //m_trigT2MbtsBits = 0;
    //ANA_CHECK(evtStore()->retrieve( m_trigT2MbtsBits, "HLT_xAOD__TrigT2MbtsBitsContainer_T2Mbts") );
    //processMBTS();

    if (enableID){
	    ANA_CHECK(evtStore()->retrieve( m_primaryVertices, "PrimaryVertices") );
	    processInDet();    	
    }


    if (enableClusters){
	    ANA_CHECK(evtStore()->retrieve( m_caloClusters, "CaloCalTopoClusters"));
	    processClusters();    	
    }

    // Gaps will require some evaluation of Run 3 performance of the clusters
    //processGaps();

    if( lhcf2022||lhcf2022zdc||lhcf2022afp ){
      ANA_CHECK(evtStore()->retrieve( m_afpProtons, "AFPProtonContainer"));
      processProtons();
    }
  }

  // if trigger enabled, only write out events which pass one of them, unless using MC

  if (enableTrigger && !passTrigger && !m_isMC && writeOnlyTriggers) return StatusCode::SUCCESS;


  if (enableOutputTree)
  {
    tree( "zdcTree" )->Fill();
  }

  return StatusCode::SUCCESS;
}

void ZdcNtuple::processZdcNtupleFromModules()
{
  
  SG::ReadHandle<xAOD::ZdcModuleContainer> zdcModules (m_zdcModuleContainerName);
  SG::ReadHandle<xAOD::ZdcModuleContainer> zdcSums (m_zdcSumContainerName);
  
  ANA_MSG_DEBUG ("copying already processed info!");
  
  //Reset the truth separately since it has a different range
  for(int iside : {0,1}){
    for(int imod = 0; imod < 7; ++imod){
      t_ZdcModuleTruthTotal[iside][imod] = 0; 
      t_ZdcModuleTruthInvis[iside][imod] = 0; 
      t_ZdcModuleTruthEM[iside][imod] = 0; 
      t_ZdcModuleTruthNonEM[iside][imod] = 0; 
      t_ZdcModuleTruthEscaped[iside][imod] = 0;
      t_ZdcModuleTruthNphotons[iside][imod] = 0;
    }
    for(int ch = 0; ch < 16; ++ch){
      t_RpdModuleTruthNphotons[iside][ch] = 0;
    }
  }
  
  for (size_t iside = 0; iside < 2; iside++)
    {
      t_ZdcAmp[iside] = 0; t_ZdcEnergy[iside] = 0; t_ZdcEnergyErr[iside] = 0;t_ZdcTime[iside] = 0; t_ZdcStatus[iside] = 0;
      t_ZdcNLEnergy[iside] = 0;t_ZdcNLEnergyErr[iside] = 0;
      t_ZdcTrigEff[iside] = 0;t_ZdcLucrodTriggerSideAmp[iside] = 0; t_ZdcLucrodTriggerSideAmpLG[iside] = 0; t_ZdcTruthTotal[iside] = 0;
      t_ZdcTruthInvis[iside] = 0; t_ZdcTruthEM[iside] = 0; t_ZdcTruthNonEM[iside] = 0;
      t_ZdcTruthEscaped[iside] = 0;
      for (int imod = 0; imod < 4; imod++)
	{
	  t_ZdcModuleAmp[iside][imod] = 0; t_ZdcModuleTime[iside][imod] = 0; t_ZdcModuleStatus[iside][imod] = 0;
	  
	  t_ZdcModuleCalibAmp[iside][imod] = 0; t_ZdcModuleCalibTime[iside][imod] = 0; t_ZdcModuleChisq[iside][imod] = 0; t_ZdcModuleFitAmp[iside][imod] = 0;
	  t_ZdcModuleFitT0[iside][imod] = 0; t_ZdcModuleBkgdMaxFraction[iside][imod] = 0; t_ZdcModuleAmpError[iside][imod] = 0;
	  t_ZdcModuleMinDeriv2nd[iside][imod] = 0; t_ZdcModulePresample[iside][imod] = 0; t_ZdcModulePreSampleAmp[iside][imod] = 0;
	  t_ZdcLucrodTriggerAmp[iside][imod] = 0;t_ZdcLucrodTriggerAmpLG[iside][imod] = 0;
	  t_ZdcModuleMaxADC[iside][imod] = 0; t_ZdcModuleMaxADCHG[iside][imod] = 0; t_ZdcModuleMaxADCLG[iside][imod] = 0;
	  t_ZdcModulePeakADCHG[iside][imod] = 0; t_ZdcModulePeakADCLG[iside][imod] = 0;
	  t_ZdcModuleAmpLGRefit[iside][imod] = 0; t_ZdcModuleAmpCorrLGRefit[iside][imod] = 0; 
	  t_ZdcModuleT0LGRefit[iside][imod] = 0; t_ZdcModuleT0SubLGRefit[iside][imod] = 0; t_ZdcModuleChisqLGRefit[iside][imod] = 0;
	  
	  if (enableOutputSamples)
	    {
	      for (int ig=0;ig<2;ig++)
		{
		  for (int id=0;id<2;id++)
		    {
		      for (unsigned int isamp=0;isamp<nsamplesZdc;isamp++)
			{
			  if (nsamplesZdc==7) t_raw7[iside][imod][ig][id][isamp]=0;
			  if (nsamplesZdc==15) t_raw15[iside][imod][ig][id][isamp]=0;
			  if (nsamplesZdc==24) t_raw24[iside][imod][ig][id][isamp]=0;
			}
		    }
		}
	      if (nsamplesZdc==24)
		{
		  for (int ch=0;ch<16;ch++)
		    {
		      for (unsigned int isamp=0;isamp<nsamplesZdc;isamp++)
			{
			  t_rpdRaw[iside][ch][isamp]=0;
			}
		    }
		}
	    }
	  if (enableRPD)
	    {
	      for (int ch = 0; ch < 16; ch++) {
		t_RpdChannelBaseline[iside][ch] = 0;
		std::fill(t_RpdChannelPileupExpFitParams[iside][ch], t_RpdChannelPileupExpFitParams[iside][ch] + 2, 0);
		std::fill(t_RpdChannelPileupStretchedExpFitParams[iside][ch], t_RpdChannelPileupStretchedExpFitParams[iside][ch] + 3, 0);
		std::fill(t_RpdChannelPileupExpFitParamErrs[iside][ch], t_RpdChannelPileupExpFitParamErrs[iside][ch] + 2, 0);
		std::fill(t_RpdChannelPileupStretchedExpFitParamErrs[iside][ch], t_RpdChannelPileupStretchedExpFitParamErrs[iside][ch] + 3, 0);
		t_RpdChannelPileupExpFitMSE[iside][ch] = 0;
		t_RpdChannelPileupStretchedExpFitMSE[iside][ch] = 0;
		t_RpdChannelAmplitude[iside][ch] = 0;
		t_RpdChannelAmplitudeCalib[iside][ch] = 0;
		t_RpdChannelMaxADC[iside][ch] = 0;
		t_RpdChannelMaxADCCalib[iside][ch] = 0;
		t_RpdChannelMaxSample[iside][ch] = 0;
		t_RpdChannelStatus[iside][ch] = 0;
		t_RpdChannelPileupFrac[iside][ch] = 0;
	      }
	      t_RpdSideStatus[iside] = 0;
	    }
	  if (enableCentroid)
	    {
	      t_centroidStatus[iside] = 0;
	      std::fill(t_RPDChannelSubtrAmp[iside], t_RPDChannelSubtrAmp[iside] + 16, 0);
	      t_RPDSubtrAmpSum[iside] = 0;
	      t_xCentroidPreGeomCorPreAvgSubtr[iside] = 0;
	      t_yCentroidPreGeomCorPreAvgSubtr[iside] = 0;
	      t_xCentroidPreAvgSubtr[iside] = 0;
	      t_yCentroidPreAvgSubtr[iside] = 0;
	      t_xCentroid[iside] = 0;
	      t_yCentroid[iside] = 0;
	      std::fill(t_xRowCentroid[iside], t_xRowCentroid[iside] + 4, 0);
	      std::fill(t_yColCentroid[iside], t_yColCentroid[iside] + 4, 0);
	      t_reactionPlaneAngle[iside] = 0;
	    }
	}
    }
  
  t_ZdcModuleMask = 0;
  if (enableCentroid) {
    t_centroidDecorationsAvailable = false;
    t_centroidEventValid = false;
    t_cosDeltaReactionPlaneAngle = 0;
  }

  /*
  if (t_zdcEventInfoError == xAOD::EventInfo::Error)
    {
      ANA_MSG_INFO("ZDC event failed EventInfo error check - aborting!");
      return;
    }
  */

  static const SG::ConstAccessor<char> centroidEventValidAcc("centroidEventValid" + auxSuffix);
  static const SG::ConstAccessor<float> cosDeltaReactionPlaneAngleAcc("cosDeltaReactionPlaneAngle" + auxSuffix);
  static const SG::ConstAccessor<float> CalibEnergyAcc("CalibEnergy"+auxSuffix);
  static const SG::ConstAccessor<float> CalibEnergyErrAcc("CalibEnergyErr"+auxSuffix);
  static const SG::ConstAccessor<float> NLCalibEnergyAcc("NLCalibEnergy"+auxSuffix);
  static const SG::ConstAccessor<float> NLCalibEnergyErrAcc("NLCalibEnergyErr"+auxSuffix);
  static const SG::ConstAccessor<float> UncalibSumAcc("UncalibSum"+auxSuffix);
  static const SG::ConstAccessor<float> UncalibSumErrAcc("UncalibSumErr"+auxSuffix);
  static const SG::ConstAccessor<float> AverageTimeAcc("AverageTime"+auxSuffix);
  static const SG::ConstAccessor<unsigned int> StatusAcc("Status"+auxSuffix);
  static const SG::ConstAccessor<unsigned int> ModuleMaskAcc("ModuleMask"+auxSuffix);
  static const SG::ConstAccessor<float> TruthTotalEnergyAcc("TruthTotalEnergy" + auxSuffix);
  static const SG::ConstAccessor<float> TruthInvisibleEnergyAcc("TruthInvisibleEnergy" + auxSuffix);
  static const SG::ConstAccessor<float> TruthEMEnergyAcc("TruthEMEnergy" + auxSuffix);
  static const SG::ConstAccessor<float> TruthNonEMEnergyAcc("TruthNonEMEnergy" + auxSuffix);
  static const SG::ConstAccessor<float> TruthEscapedEnergyAcc("TruthEscapedEnergy" + auxSuffix);
  static const SG::ConstAccessor<unsigned int> centroidStatusAcc("centroidStatus" + auxSuffix);
  static const SG::ConstAccessor<std::vector<float> > RPDChannelSubtrAmpAcc("RPDChannelSubtrAmp" + auxSuffix);
  static const SG::ConstAccessor<float> RPDSubtrAmpSumAcc("RPDSubtrAmpSum" + auxSuffix);
  static const SG::ConstAccessor<float> xCentroidPreGeomCorPreAvgSubtrAcc("xCentroidPreGeomCorPreAvgSubtr" + auxSuffix);
  static const SG::ConstAccessor<float> yCentroidPreGeomCorPreAvgSubtrAcc("yCentroidPreGeomCorPreAvgSubtr" + auxSuffix);
  static const SG::ConstAccessor<float> xCentroidPreAvgSubtrAcc("xCentroidPreAvgSubtr" + auxSuffix);
  static const SG::ConstAccessor<float> yCentroidPreAvgSubtrAcc("yCentroidPreAvgSubtr" + auxSuffix);
  static const SG::ConstAccessor<float> xCentroidAcc("xCentroid" + auxSuffix);
  static const SG::ConstAccessor<float> yCentroidAcc("yCentroid" + auxSuffix);
  static const SG::ConstAccessor<std::vector<float> > xRowCentroidAcc("xRowCentroid" + auxSuffix);
  static const SG::ConstAccessor<std::vector<float> > yColCentroidAcc("yColCentroid" + auxSuffix);
  static const SG::ConstAccessor<float> reactionPlaneAngleAcc("reactionPlaneAngle" + auxSuffix);
  static const SG::ConstAccessor<unsigned int> RPDStatusAcc("RPDStatus" + auxSuffix);
  static const SG::ConstAccessor<unsigned int> nPhotonsAcc("nPhotons" + auxSuffix);
  static const SG::ConstAccessor<float> CalibTimeAcc("CalibTime" + auxSuffix);
  static const SG::ConstAccessor<float> AmplitudeAcc("Amplitude" + auxSuffix);
  static const SG::ConstAccessor<float> TimeAcc("Time" + auxSuffix);
  static const SG::ConstAccessor<float> ChisqAcc("Chisq" + auxSuffix);
  static const SG::ConstAccessor<float> FitAmpAcc("FitAmp" + auxSuffix);
  static const SG::ConstAccessor<float> FitAmpErrorAcc("FitAmpError" + auxSuffix);
  static const SG::ConstAccessor<float> FitT0Acc("FitT0" + auxSuffix);
  static const SG::ConstAccessor<float> BkgdMaxFractionAcc("BkgdMaxFraction" + auxSuffix);
  static const SG::ConstAccessor<float> MinDeriv2ndAcc("MinDeriv2nd" + auxSuffix);
  static const SG::ConstAccessor<float> PresampleAcc("Presample" + auxSuffix);
  static const SG::ConstAccessor<float> PreSampleAmpAcc("PreSampleAmp" + auxSuffix);
  static const SG::ConstAccessor<float> RPDChannelBaselineAcc("RPDChannelBaseline" + auxSuffix);
  static const SG::ConstAccessor<std::vector<float> > RPDChannelPileupExpFitParamsAcc("RPDChannelPileupExpFitParams" + auxSuffix);
  static const SG::ConstAccessor<std::vector<float> > RPDChannelPileupExpFitParamErrsAcc("RPDChannelPileupExpFitParamErrs" + auxSuffix);
  static const SG::ConstAccessor<std::vector<float> > RPDChannelPileupStretchedExpFitParamsAcc("RPDChannelPileupStretchedExpFitParams" + auxSuffix);
  static const SG::ConstAccessor<std::vector<float> > RPDChannelPileupStretchedExpFitParamErrsAcc("RPDChannelPileupStretchedExpFitParamErrs" + auxSuffix);
  static const SG::ConstAccessor<float> RPDChannelPileupExpFitMSEAcc("RPDChannelPileupExpFitMSE" + auxSuffix);
  static const SG::ConstAccessor<float> RPDChannelPileupStretchedExpFitMSEAcc("RPDChannelPileupStretchedExpFitMSE" + auxSuffix);
  static const SG::ConstAccessor<float> RPDChannelAmplitudeAcc("RPDChannelAmplitude" + auxSuffix);
  static const SG::ConstAccessor<float> RPDChannelAmplitudeCalibAcc("RPDChannelAmplitudeCalib" + auxSuffix);
  static const SG::ConstAccessor<float> RPDChannelMaxADCAcc("RPDChannelMaxADC" + auxSuffix);
  static const SG::ConstAccessor<float> RPDChannelMaxADCCalibAcc("RPDChannelMaxADCCalib" + auxSuffix);
  static const SG::ConstAccessor<unsigned int> RPDChannelMaxSampleAcc("RPDChannelMaxSample" + auxSuffix);
  static const SG::ConstAccessor<unsigned int> RPDChannelStatusAcc("RPDChannelStatus" + auxSuffix);
  static const SG::ConstAccessor<float> RPDChannelPileupFracAcc("RPDChannelPileupFrac" + auxSuffix);
  static const SG::ConstAccessor<float> AmpLGRefitAcc("AmpLGRefit" + auxSuffix);
  static const SG::ConstAccessor<float> T0LGRefitAcc("T0LGRefit" + auxSuffix);
  static const SG::ConstAccessor<float> T0SubLGRefitAcc("T0SubLGRefit" + auxSuffix);
  static const SG::ConstAccessor<float> ChisqLGRefitAcc("ChisqLGRefit" + auxSuffix);

  static const SG::ConstAccessor<uint16_t> LucrodTriggerAmpAcc("LucrodTriggerAmp");
  static const SG::ConstAccessor<uint16_t> LucrodTriggerAmpLGAcc("LucrodTriggerAmpLG");
  static const SG::ConstAccessor<uint16_t> LucrodTriggerSideAmpAcc("LucrodTriggerSideAmp");
  static const SG::ConstAccessor<uint16_t> LucrodTriggerSideAmpLGAcc("LucrodTriggerSideAmpLG");
  static const SG::ConstAccessor<float> Amplitude0Acc("Amplitude");
  static const SG::ConstAccessor<float> MaxADCAcc("MaxADC");
  static const SG::ConstAccessor<float> MaxADCHGAcc("MaxADCHG");
  static const SG::ConstAccessor<float> MaxADCLGAcc("MaxADCLG");
  static const SG::ConstAccessor<float> PeakADCHGAcc("PeakADCHG");
  static const SG::ConstAccessor<float> PeakADCLGAcc("PeakADCLG");
  static const SG::ConstAccessor<std::vector<uint16_t> > g0d0DataAcc("g0d0Data");
  static const SG::ConstAccessor<std::vector<uint16_t> > g0d1DataAcc("g0d1Data");
  static const SG::ConstAccessor<std::vector<uint16_t> > g1d0DataAcc("g1d0Data");
  static const SG::ConstAccessor<std::vector<uint16_t> > g1d1DataAcc("g1d1Data");
  static const SG::ConstAccessor<std::vector<uint16_t> > g0dataAcc("g0data");
  static const SG::ConstAccessor<std::vector<uint16_t> > g1dataAcc("g1data");
  
  bool rpdErr = m_eventInfo->isEventFlagBitSet(xAOD::EventInfo::ForwardDet, ZdcEventInfo::RPDDECODINGERROR );
  bool zdcErr = m_eventInfo->isEventFlagBitSet(xAOD::EventInfo::ForwardDet, ZdcEventInfo::ZDCDECODINGERROR );
  t_rpdDecodingError = rpdErr;
  t_zdcDecodingError = zdcErr;
  if (enableCentroid) {
    // locate global sum (only if centroid is needed)
    xAOD::ZdcModule const * globalSum = nullptr;
    for (auto const * zdcSum : *zdcSums) {
      if (zdcSum->zdcSide() == 0) {
        globalSum = zdcSum;
        break;
      }
    }
    if (!globalSum) {
      ANA_MSG_ERROR("unable to locate global ZdcSum (side = 0)");
      t_centroidDecorationsAvailable = false;
    }
    else {
      t_centroidDecorationsAvailable = centroidStatusAcc.isAvailable(*globalSum);
    }
  }

  if (rpdErr||zdcErr) ANA_MSG_WARNING( "Decoding errors ZDC=" << zdcErr << " RPD=" << rpdErr );

  if (zdcSums.ptr())
    {
      ANA_MSG_DEBUG( "accessing ZdcSums" );
      for (const auto zdcSum : *zdcSums)
	{
	  if (zdcSum->zdcSide()==0) {
	      // trap new global sum
	      if (enableCentroid && t_centroidDecorationsAvailable) {
	        t_centroidEventValid = centroidEventValidAcc(*zdcSum);
	        t_cosDeltaReactionPlaneAngle = cosDeltaReactionPlaneAngleAcc(*zdcSum);
	      }
	      // no other branches are filled from global sum - skip to the real sides (C=-1 and A=1)
	      continue;
	    }
	  int iside = 0;
	  if (zdcSum->zdcSide() > 0) iside = 1;
	  
	  //static SG::AuxElement::ConstAccessor< float > acc( "CalibEnergy" );
	  //t_ZdcEnergy[iside] = acc(*zdcSum);
	  
	  if (enableZDC && !zdcErr)
	    {
	      t_ZdcEnergy[iside] = CalibEnergyAcc(*zdcSum);
	      t_ZdcEnergyErr[iside] = CalibEnergyErrAcc(*zdcSum);
	      t_ZdcNLEnergy[iside] = NLCalibEnergyAcc(*zdcSum);
	      t_ZdcNLEnergyErr[iside] = NLCalibEnergyErrAcc(*zdcSum);
	      
	      t_ZdcAmp[iside] = UncalibSumAcc(*zdcSum);
	      t_ZdcAmpErr[iside] = UncalibSumErrAcc(*zdcSum);
	      if (LucrodTriggerSideAmpAcc.isAvailable(*zdcSum))
		t_ZdcLucrodTriggerSideAmp[iside] = LucrodTriggerSideAmpAcc(*zdcSum);
	      if (LucrodTriggerSideAmpLGAcc.isAvailable(*zdcSum))
		t_ZdcLucrodTriggerSideAmpLG[iside] = LucrodTriggerSideAmpLGAcc(*zdcSum);
	      
	      ANA_MSG_VERBOSE("processZdcNtupleFromModules: ZdcSum energy = " << t_ZdcEnergy[iside]);
	      
	      t_ZdcTime[iside] = AverageTimeAcc(*zdcSum);
	      t_ZdcStatus[iside] = StatusAcc(*zdcSum);
	      t_ZdcModuleMask += ( ModuleMaskAcc(*zdcSum) << 4 * iside);
	      
	      if(m_isMC){
		ANA_MSG_DEBUG("Filling sum truth");
		t_ZdcTruthTotal  [iside] = TruthTotalEnergyAcc(*zdcSum);
		t_ZdcTruthInvis  [iside] = TruthInvisibleEnergyAcc(*zdcSum);
		t_ZdcTruthEM     [iside] = TruthEMEnergyAcc(*zdcSum);
		t_ZdcTruthNonEM  [iside] = TruthNonEMEnergyAcc(*zdcSum);
		t_ZdcTruthEscaped[iside] = TruthEscapedEnergyAcc(*zdcSum);
	      }
	      
	    }
	  
	  if (enableRPD)
	    {
	      if (enableRPDAmp && !rpdErr)
		{
		  t_RpdSideStatus[iside] = RPDStatusAcc(*zdcSum);
		}
	      if (enableCentroid && t_centroidDecorationsAvailable)
		{
		  t_centroidStatus[iside] = centroidStatusAcc(*zdcSum);
		  std::vector<float> const& rpdChannelSubtrAmp = RPDChannelSubtrAmpAcc(*zdcSum);
		  std::copy(rpdChannelSubtrAmp.begin(), rpdChannelSubtrAmp.end(), t_RPDChannelSubtrAmp[iside]);
		  t_RPDSubtrAmpSum[iside] = RPDSubtrAmpSumAcc(*zdcSum);
		  t_xCentroidPreGeomCorPreAvgSubtr[iside] = xCentroidPreGeomCorPreAvgSubtrAcc(*zdcSum);
		  t_yCentroidPreGeomCorPreAvgSubtr[iside] = yCentroidPreGeomCorPreAvgSubtrAcc(*zdcSum);
		  t_xCentroidPreAvgSubtr[iside] = xCentroidPreAvgSubtrAcc(*zdcSum);
		  t_yCentroidPreAvgSubtr[iside] = yCentroidPreAvgSubtrAcc(*zdcSum);
		  t_xCentroid[iside] = xCentroidAcc(*zdcSum);
		  t_yCentroid[iside] = yCentroidAcc(*zdcSum);
		  std::vector<float> const& xRowCentroid = xRowCentroidAcc(*zdcSum);
		  std::copy(xRowCentroid.begin(), xRowCentroid.end(), t_xRowCentroid[iside]);
		  std::vector<float> const& yColCentroid = yColCentroidAcc(*zdcSum);
		  std::copy(yColCentroid.begin(), yColCentroid.end(), t_yColCentroid[iside]);
		  t_reactionPlaneAngle[iside] = reactionPlaneAngleAcc(*zdcSum);
		}
	    }
	}
    }
  
  ANA_MSG_DEBUG(  "accessing ZdcModules" );
  
  if (zdcModules.ptr())
    {
      for (const auto zdcMod : *zdcModules)
	{
	  int iside = 0;
	  if (zdcMod->zdcSide() > 0) iside = 1;
	  int imod = zdcMod->zdcModule();
	  
	  if (m_isMC){
	    //Calib hits are only stored in channel 0 of the RPD
	    if(!(imod == 4 && zdcMod->zdcChannel() != 0)){
	      t_ZdcModuleTruthTotal  [iside][imod] = TruthTotalEnergyAcc(*zdcMod);
	      t_ZdcModuleTruthInvis  [iside][imod] = TruthInvisibleEnergyAcc(*zdcMod);
	      t_ZdcModuleTruthEM     [iside][imod] = TruthEMEnergyAcc(*zdcMod);
	      t_ZdcModuleTruthNonEM  [iside][imod] = TruthNonEMEnergyAcc(*zdcMod);
	      t_ZdcModuleTruthEscaped[iside][imod] = TruthEscapedEnergyAcc(*zdcMod);
	      t_ZdcModuleTruthNphotons[iside][imod] = nPhotonsAcc(*zdcMod);
	    }
	    //Calib hits are stored for all modules
	    //Other data is only valid for module 1-4
	    if(imod > 4) continue;
	  }
	  
	  ANA_MSG_VERBOSE ("Module " << zdcMod->zdcSide() << " " << zdcMod->zdcModule() << " amp:" << AmplitudeAcc(*zdcMod));
	  
	  if (zdcMod->zdcType() == 0 && !zdcErr)
	    {
	      // ZDC energy type modules
	      t_ZdcModuleCalibAmp[iside][imod] = CalibEnergyAcc(*zdcMod);
	      t_ZdcModuleCalibTime[iside][imod] = CalibTimeAcc(*zdcMod);
	      t_ZdcModuleStatus[iside][imod] = StatusAcc(*zdcMod);
	      if (t_ZdcModuleAmp[iside][imod] != 0.)
		Warning("processZdcNtupleFromModules", "overwriting side %d module %d!", iside, imod);
	      t_ZdcModuleAmp[iside][imod] = AmplitudeAcc(*zdcMod);
	      t_ZdcModuleTime[iside][imod] = TimeAcc(*zdcMod);
	      
	      t_ZdcModuleChisq[iside][imod] = ChisqAcc(*zdcMod);
	      t_ZdcModuleFitAmp[iside][imod] = FitAmpAcc(*zdcMod);
	      t_ZdcModuleAmpError[iside][imod] = FitAmpErrorAcc(*zdcMod);
	      t_ZdcModuleFitT0[iside][imod] = FitT0Acc(*zdcMod);
	      t_ZdcModuleBkgdMaxFraction[iside][imod] = BkgdMaxFractionAcc(*zdcMod);
	      t_ZdcModuleMinDeriv2nd[iside][imod] = MinDeriv2ndAcc(*zdcMod);
	      t_ZdcModulePresample[iside][imod] = PresampleAcc(*zdcMod);
	      t_ZdcModulePreSampleAmp[iside][imod] = PreSampleAmpAcc(*zdcMod);
	      
	      if (AmpLGRefitAcc.isAvailable(*zdcMod)) {
		t_ZdcModuleAmpLGRefit[iside][imod] = AmpLGRefitAcc(*zdcMod);
		t_ZdcModuleT0LGRefit[iside][imod] = T0LGRefitAcc(*zdcMod);
		t_ZdcModuleT0SubLGRefit[iside][imod] = T0SubLGRefitAcc(*zdcMod);
		t_ZdcModuleChisqLGRefit[iside][imod] = ChisqLGRefitAcc(*zdcMod);
	      }
	      
	      if (LucrodTriggerAmpAcc.isAvailable(*zdcMod))
		t_ZdcLucrodTriggerAmp[iside][imod] = LucrodTriggerAmpAcc(*zdcMod);
	      if (LucrodTriggerAmpLGAcc.isAvailable(*zdcMod))
		t_ZdcLucrodTriggerAmpLG[iside][imod] = LucrodTriggerAmpLGAcc(*zdcMod);

	      if (MaxADCAcc.isAvailable(*zdcMod))
		t_ZdcModuleMaxADC[iside][imod] = MaxADCAcc(*zdcMod);
	      if (MaxADCHGAcc.isAvailable(*zdcMod))
		t_ZdcModuleMaxADCHG[iside][imod] = MaxADCHGAcc(*zdcMod);
	      if (MaxADCLGAcc.isAvailable(*zdcMod))
		t_ZdcModuleMaxADCLG[iside][imod] = MaxADCLGAcc(*zdcMod);
	      if (PeakADCHGAcc.isAvailable(*zdcMod))
		t_ZdcModulePeakADCHG[iside][imod] = PeakADCHGAcc(*zdcMod);
	      if (PeakADCLGAcc.isAvailable(*zdcMod))
		t_ZdcModulePeakADCLG[iside][imod] = PeakADCLGAcc(*zdcMod);

	      if (enableOutputSamples && !zdcErr)
		{
		  for (unsigned int isamp = 0; isamp < nsamplesZdc; isamp++) // 7 samples
		    {
		      if (nsamplesZdc == 7)
			{
			  t_raw7[iside][imod][0][0][isamp] = g0d0DataAcc(*zdcMod).at(isamp);
			  t_raw7[iside][imod][0][1][isamp] = g0d1DataAcc(*zdcMod).at(isamp);
			  t_raw7[iside][imod][1][0][isamp] = g1d0DataAcc(*zdcMod).at(isamp);
			  t_raw7[iside][imod][1][1][isamp] = g1d1DataAcc(*zdcMod).at(isamp);
			}
		      
		      if (nsamplesZdc == 15)
			{
			  t_raw15[iside][imod][0][0][isamp] = g0d0DataAcc(*zdcMod).at(isamp);
			  t_raw15[iside][imod][0][1][isamp] = g0d1DataAcc(*zdcMod).at(isamp);
			  t_raw15[iside][imod][1][0][isamp] = g1d0DataAcc(*zdcMod).at(isamp);
			  t_raw15[iside][imod][1][1][isamp] = g1d1DataAcc(*zdcMod).at(isamp);
			}
		      
		      if (nsamplesZdc == 24)
			{
			  t_raw24[iside][imod][0][0][isamp] = g0dataAcc(*zdcMod).at(isamp);
			  t_raw24[iside][imod][1][0][isamp] = g1dataAcc(*zdcMod).at(isamp);
			}
		    }
		}
	    }
	  else if (zdcMod->zdcType() == 1 && nsamplesZdc == 24)
	    {
	      // this is the RPD
	      if (enableRPD)
		{
		  if (enableRPDAmp && !rpdErr)
		    {
		      t_RpdChannelBaseline[iside][zdcMod->zdcChannel()] = RPDChannelBaselineAcc(*zdcMod);
		      std::vector<float> const &rpdChannelPileupExpFitParams = RPDChannelPileupExpFitParamsAcc(*zdcMod);
		      std::copy(rpdChannelPileupExpFitParams.begin(), rpdChannelPileupExpFitParams.end(), t_RpdChannelPileupExpFitParams[iside][zdcMod->zdcChannel()]);
		      std::vector<float> const &rpdChannelPileupExpFitParamErrs = RPDChannelPileupExpFitParamErrsAcc(*zdcMod);
		      std::copy(rpdChannelPileupExpFitParamErrs.begin(), rpdChannelPileupExpFitParamErrs.end(), t_RpdChannelPileupExpFitParamErrs[iside][zdcMod->zdcChannel()]);
		      std::vector<float> const &rpdChannelPileupStretchedExpFitParams = RPDChannelPileupStretchedExpFitParamsAcc(*zdcMod);
		      std::copy(rpdChannelPileupStretchedExpFitParams.begin(), rpdChannelPileupStretchedExpFitParams.end(), t_RpdChannelPileupStretchedExpFitParams[iside][zdcMod->zdcChannel()]);
		      std::vector<float> const &rpdChannelPileupStretchedExpFitParamErrs = RPDChannelPileupStretchedExpFitParamErrsAcc(*zdcMod);
		      std::copy(rpdChannelPileupStretchedExpFitParamErrs.begin(), rpdChannelPileupStretchedExpFitParamErrs.end(), t_RpdChannelPileupStretchedExpFitParamErrs[iside][zdcMod->zdcChannel()]);
		      t_RpdChannelPileupExpFitMSE[iside][zdcMod->zdcChannel()] = RPDChannelPileupExpFitMSEAcc(*zdcMod);
		      t_RpdChannelPileupStretchedExpFitMSE[iside][zdcMod->zdcChannel()] = RPDChannelPileupStretchedExpFitMSEAcc(*zdcMod);
		      t_RpdChannelAmplitude[iside][zdcMod->zdcChannel()] = RPDChannelAmplitudeAcc(*zdcMod);
		      t_RpdChannelAmplitudeCalib[iside][zdcMod->zdcChannel()] = RPDChannelAmplitudeCalibAcc(*zdcMod);
		      t_RpdChannelMaxADC[iside][zdcMod->zdcChannel()] = RPDChannelMaxADCAcc(*zdcMod);
		      t_RpdChannelMaxADCCalib[iside][zdcMod->zdcChannel()] = RPDChannelMaxADCCalibAcc(*zdcMod);
		      t_RpdChannelMaxSample[iside][zdcMod->zdcChannel()] = RPDChannelMaxSampleAcc(*zdcMod);
		      t_RpdChannelStatus[iside][zdcMod->zdcChannel()] = RPDChannelStatusAcc(*zdcMod);
		      t_RpdChannelPileupFrac[iside][zdcMod->zdcChannel()] = RPDChannelPileupFracAcc(*zdcMod);
		      if(m_isMC){
			t_RpdModuleTruthNphotons[iside][zdcMod->zdcChannel()] = nPhotonsAcc(*zdcMod);
		      }
		    }
		  if (enableOutputSamples)
		    {
		      std::vector<uint16_t> const &rpdChannelRaw = g0dataAcc(*zdcMod);
		      std::copy(rpdChannelRaw.begin(), rpdChannelRaw.end(), t_rpdRaw[iside][zdcMod->zdcChannel()]);
		    }
		}
	    }
	}
    }
  else
    {
      ANA_MSG_INFO("No ZdcModules" << auxSuffix << " when expected!");
    }
  
  if (msgLvl (MSG::VERBOSE))
    {
      std::ostringstream message;
      message << "Dump zdc_ZdcModuleAmp: ";
      for (int iside = 0; iside < 2; iside++)
	{
	  for (int imod = 0; imod < 4; imod++)
	    {
	      message << t_ZdcModuleAmp[iside][imod] << " ";
	    }
	}
    }
}

void ZdcNtuple::processMCEventCollection(){
  /******************************************
   * Get the McEventCollection (input)
   ******************************************/
  SG::ReadHandle<McEventCollection> mcEventCollection (m_mcEventCollectionName, getContext());
  if (!mcEventCollection.isValid()){
    ANA_MSG_ERROR("Could not retrieve HepMC with key:" << m_mcEventCollectionName.key());
    return;
  }else{
    ANA_MSG_DEBUG("Retrieved HepMC with key: " << m_mcEventCollectionName.key());
  }

  /******************************************
   * Clear and resize the output vectors
  ******************************************/
  t_ZdcTruthParticlePosx.clear();
  t_ZdcTruthParticlePosy.clear();
  t_ZdcTruthParticlePosz.clear();
  t_ZdcTruthParticleTime.clear();
  t_ZdcTruthParticlePx.clear();
  t_ZdcTruthParticlePy.clear();
  t_ZdcTruthParticlePz.clear();
  t_ZdcTruthParticleEnergy.clear();
  t_ZdcTruthParticlePid.clear();
  t_ZdcTruthParticleStatus.clear();

  /******************************************
   * Sort the particles into sides and add
   * them to the output vectors
  ******************************************/  
  for (unsigned int cntr = 0; cntr < mcEventCollection->size(); ++cntr){
    const HepMC::GenEvent *genEvt = (*mcEventCollection)[cntr];
    for (const auto &vertex : genEvt->vertices()){
      for (const auto &particle : vertex->particles_in()){
        t_ZdcTruthParticlePosx.push_back(vertex->position().x());
        t_ZdcTruthParticlePosy.push_back(vertex->position().y());
        t_ZdcTruthParticlePosz.push_back(vertex->position().z());
        t_ZdcTruthParticleTime.push_back(vertex->position().t());
        t_ZdcTruthParticlePx.push_back(particle->momentum().x());
        t_ZdcTruthParticlePy.push_back(particle->momentum().y());
        t_ZdcTruthParticlePz.push_back(particle->momentum().z());
        t_ZdcTruthParticleEnergy.push_back(particle->momentum().e());
        t_ZdcTruthParticlePid.push_back(particle->pid());
        t_ZdcTruthParticleStatus.push_back(particle->status());
      } // end loop over particles
    }// end loop over vertices
  }// end loop over HepMC events
}

bool ZdcNtuple::processTriggerDecision()
{
  ANA_MSG_DEBUG ("Processing trigger");

  bool passTrigger = false;

  t_trigger = 0;
  t_trigger_TBP = 0;

  for (int i = 0; i < 16; i++)
  {
    t_tav[i] = 0;
    t_tbp[i] = 0;
  }

  if (m_trigDecision)
    {
      for (int i = 0; i < 16; i++)
	{
	  t_tbp[i] = m_trigDecision->tbp().at(i);
	  t_tav[i] = m_trigDecision->tav().at(i);	  
	  ANA_MSG_DEBUG( "TD: " << i << " tbp: " << std::hex << t_tbp[i] << "\t" << t_tav[i] );
	}
      t_bunchGroup = m_trigDecision->bgCode();
    }

  if (enableTrigger)
  {

    int ic = 0;
    for (auto cg : m_chainGroups)
    {
      
      if (zdcCalib)
	{
	  std::string name = cg->getListOfTriggers().at(0);
	  const unsigned int triggerbits = m_trigDecisionTool->isPassedBits(name);
	  // deferred functionality
	  //bool tbp = triggerbits&TrigDefs::L1_isPassedBeforePrescale;
	  //bool tap = triggerbits&TrigDefs::L1_isPassedAfterPrescale;
	  bool tav = triggerbits&TrigDefs::L1_isPassedAfterVeto;
	  ANA_MSG_DEBUG("TD: checking trigger name=" << name<< " tav=" << tav);
	  if (tav)
	    {
	      t_trigger += (1 << ic);
	      t_decisions[ic] = true;
	      t_prescales[ic] = cg->getPrescale();
	      passTrigger = true;	      
	    }
	  else
	    {
	      t_decisions[ic] = 0;
	      t_prescales[ic] = 0;
	    }
	}
      else
	{
	  if (cg->isPassed())
	    {
	      t_trigger += (1 << ic);
	      t_decisions[ic] = true;
	      t_prescales[ic] = cg->getPrescale();
	      passTrigger = true;
	    }
	  else
	    {
	      t_decisions[ic] = 0;
	      t_prescales[ic] = 0;
	    }
	}

      if (cg->isPassedBits()&TrigDefs::EF_passedRaw)
      {
        t_trigger_TBP += (1 << ic);
      }


      ic++;
    }

    int irc = 0;
    for (auto cg : m_rerunChainGroups)
    {
      t_rerunDecisions[irc] = false;
      if (cg->isPassedBits()&TrigDefs::EF_passedRaw)
      {
        t_rerunDecisions[irc] = true;
      }
      irc++;
    }

  }

  return passTrigger;
}

void ZdcNtuple::processEventInfo()
{
  ANA_MSG_DEBUG( "processing event info");

  t_bcid = m_eventInfo->bcid();
  t_runNumber = m_eventInfo->runNumber();
  t_eventNumber = m_eventInfo->eventNumber();
  t_lumiBlock = m_eventInfo->lumiBlock();
  t_bunchGroup = -1;
  t_extendedLevel1ID = m_eventInfo->extendedLevel1ID();
  t_timeStamp = m_eventInfo->timeStamp();
  t_timeStampNSOffset = m_eventInfo->timeStampNSOffset();
  t_passBits = acceptEvent();
  t_avgIntPerCrossing = m_eventInfo->averageInteractionsPerCrossing();
  t_actIntPerCrossing = m_eventInfo->actualInteractionsPerCrossing();
  t_zdcEventInfoError = m_eventInfo->errorState(xAOD::EventInfo::ForwardDet);
  t_zdcEventInfoErrorWord = m_eventInfo->eventFlags(xAOD::EventInfo::ForwardDet);
  
  if ( !(m_eventCounter++ % 1000) || msgLvl(MSG::DEBUG))
  {
    ANA_MSG_INFO("Event# " << m_eventCounter << " Run " << m_eventInfo->runNumber() << " Event " << m_eventInfo->eventNumber() << " LB " << m_eventInfo->lumiBlock() );
  }

}

void ZdcNtuple::processVInjInfo(){
	// Check for new run number
  //
  if (t_runNumber != m_lastRunNumber) {
    //
    // Get access to the injector pulse steps for this run
    //
    m_injMapRunToken = m_zdcInjPulserAmpMap->lookupRun(t_runNumber, true);
    if (!m_injMapRunToken.isValid()) {
      ANA_MSG_ERROR("Unable to obtain injector pulse steps for run " << t_runNumber);
    }
    else {
      unsigned int startLB = m_zdcInjPulserAmpMap->getFirstLumiBlock(m_injMapRunToken);
      unsigned int nsteps = m_zdcInjPulserAmpMap->getNumSteps(m_injMapRunToken);
      ANA_MSG_DEBUG("Successfully obtained injector pulse steps for run " << t_runNumber
		    << ", first LB = " << startLB << ", number of steps = " << nsteps);
    }

    // update the last run number to be the current run number
    m_lastRunNumber = t_runNumber;
  }
  
  t_vInj = m_zdcInjPulserAmpMap->getPulserAmplitude(m_injMapRunToken, t_lumiBlock);
}

void ZdcNtuple::processInDet()
{
  ANA_MSG_DEBUG("processInDet(): processing tracks & vertices!");
  t_ntrk = 0;
  t_nvx = 0;
  t_vxntrk = 0;
  t_vx_trk_index.clear();
  t_vxsumpt2 = 0;
  t_vxtype = 0;
  t_pvindex = -1;
  t_puvxntrk = 0;
  t_puvxsumpt = 0;
  t_vxnlooseprimary = 0;
  t_vxnminbias = 0;

  int i;
  for (i = 0; i < 3; i++) t_vx[i] = 0;
  for (i = 0; i < 6; i++) t_vxcov[i] = 0;

  const xAOD::Vertex* primary_vertex = nullptr;
  size_t pv_index = -1;
  size_t vx_index = 0;
  float max_pileup_sumpT = 0.;
  int max_pileup_nTrack = 0;
  float max_pileup_z = 0;
  int nStrongPileup = 0;

  t_nvtx = 0;
  t_vtx_type.clear();
  t_vtx_x.clear();
  t_vtx_y.clear();
  t_vtx_z.clear();
  t_vtx_ntrk_all.clear();
  t_vtx_sumpt2_all.clear();
  t_vtx_ntrk.clear();
  t_vtx_sumpt2.clear();
  t_vtx_trk_index.clear();

  if (m_primaryVertices)
    {
      ANA_MSG_DEBUG("processInDet: processing vertices");

      t_nvx = m_primaryVertices->size();

      static const SG::ConstAccessor<float> sumPt2Acc("sumPt2");
      
      // start of new vertex representation
      t_nvtx = m_primaryVertices->size();
      for (const auto vertex : *m_primaryVertices)
	{
	  float vtx_sumpt2 = 0;
	  int vtx_ntrk = 0;

	  t_vtx_type.push_back(vertex->vertexType());
	  t_vtx_x.push_back(0);
	  t_vtx_y.push_back(0);
	  t_vtx_z.push_back(vertex->z());

	  t_vtx_ntrk.push_back(vtx_ntrk);
	  t_vtx_sumpt2.push_back(vtx_sumpt2 / 1e6);
	  t_vtx_ntrk_all.push_back(vertex->nTrackParticles());

	  if (sumPt2Acc.isAvailable(*vertex))
	    t_vtx_sumpt2_all.push_back(sumPt2Acc(*vertex));
	  else
	    t_vtx_sumpt2_all.push_back(-1);

	  std::vector<int16_t> trk_index;
	  if ( m_trackParticles && vertex->nTrackParticles() <= trackLimit )
	    {
	      const std::vector< ElementLink< xAOD::TrackParticleContainer > >& vxTrackParticles = vertex->trackParticleLinks();
	      for (size_t itrk = 0; itrk < vxTrackParticles.size(); itrk++)
		{
		  ElementLink< xAOD::TrackParticleContainer > trkLink = vxTrackParticles.at(itrk);
		  trk_index.push_back(trkLink.index());
		}
	    }
	  t_vtx_trk_index.push_back(trk_index);

	  // end of new vertex representation

	  if (vertex->vertexType() == xAOD::VxType::PriVtx)
	    {
	      primary_vertex = vertex;
	      pv_index = vx_index;
	    }
	  if (vertex->vertexType() == xAOD::VxType::PileUp)
	    {
	      float pileup_sumpT = 0;
	      int pileup_nTrack = 0;
	      for (size_t itr = 0; itr < vertex->nTrackParticles(); itr++)
		{
		  int track_quality = trackQuality(vertex->trackParticle(itr), vertex);
		  if (track_quality != -1 && (track_quality & 128) != 0)
		    {
		      pileup_nTrack++;
		      pileup_sumpT += vertex->trackParticle(itr)->pt();
		    }
		}
	      if (pileup_sumpT > max_pileup_sumpT)
		{
		  max_pileup_sumpT = pileup_sumpT;
		  max_pileup_nTrack = pileup_nTrack;
		  max_pileup_z = vertex->z();
		}
	      if (pileup_sumpT > 5e3 || pileup_nTrack > 5) nStrongPileup++;
	    }
	  vx_index++;
	}
    }

  t_nstrong = nStrongPileup;

  if (primary_vertex != nullptr)
    {
      t_vx[0] = primary_vertex->x();
      t_vx[1] = primary_vertex->y();
      t_vx[2] = primary_vertex->z();
      /*
    const std::vector<float>& cov = primary_vertex->covariance();
      */
      //for (size_t i=0;i<cov.size();i++)
      for (size_t i = 0; i < 6; i++)
	{
	  //t_vxcov[i] = cov.at(i);
	  t_vxcov[i] = 0;
	}
      t_vxntrk = primary_vertex->nTrackParticles();
      static const SG::ConstAccessor<float> sumPt2Acc("sumPt2");
      if (sumPt2Acc.isAvailable(*primary_vertex))
	t_vxsumpt2 = sumPt2Acc(*primary_vertex);
      else
	t_vxsumpt2 = 0;
      
      t_vxtype = primary_vertex->vertexType();
      t_pvindex = pv_index;
      t_puvxz = max_pileup_z;
      t_puvxsumpt = max_pileup_sumpT;
      t_puvxntrk = max_pileup_nTrack;

      const std::vector< ElementLink< xAOD::TrackParticleContainer > >& vxTrackParticles = primary_vertex->trackParticleLinks();

      for (size_t itrk = 0; itrk < vxTrackParticles.size(); itrk++)
	{
	  ElementLink< xAOD::TrackParticleContainer > trkLink = vxTrackParticles.at(itrk);
	  if (!trkLink.isValid()) continue;
	  if (m_trackParticles)
	    {
	      if (m_trackParticles->size() <= trackLimit)
		t_vx_trk_index.push_back(trkLink.index());
	    }
	}

    }


  if (m_trackParticles)
    {
      ANA_MSG_DEBUG("processInDet: processing trackss");

      t_trk_pt.clear();
      t_trk_eta.clear();
      t_trk_phi.clear();
      t_trk_e.clear();
      t_trk_index.clear();
      t_trk_theta.clear();
      t_trk_charge.clear();
      t_trk_d0.clear();
      t_trk_z0.clear();
      t_trk_vz.clear();
      t_trk_vtxz.clear();
      t_trk_quality.clear();
      t_trk_nPixHits.clear();
      t_trk_nSctHits.clear();
      t_trk_nPixDead.clear();
      t_trk_nSctDead.clear();
      t_trk_nPixHoles.clear();
      t_trk_nSctHoles.clear();
      t_trk_nTrtHits.clear();
      t_trk_nTrtOutliers.clear();
      t_trk_inPixHits.clear();
      t_trk_exPixHits.clear();
      t_trk_ninPixHits.clear();
      t_trk_nexPixHits.clear();
      t_trk_pixeldEdx.clear();

      t_ntrk = m_trackParticles->size();

      if ( !enableTracks ) return;

      if ( t_ntrk <= trackLimit ) // dump all tracks
	{
	  int trk_index = 0;
	  for (const auto track : *m_trackParticles)
	    {
	      writeTrack(track, primary_vertex, trk_index++);
	    }
	}
      else // write small vertices
	{
	  for (const auto vertex : *m_primaryVertices)
	    {
	      if (vertex->nTrackParticles() <= trackLimit )
		{
		  const std::vector< ElementLink< xAOD::TrackParticleContainer > >& vxTrackParticles = vertex->trackParticleLinks();
		  for (size_t itrk = 0; itrk < vxTrackParticles.size(); itrk++)
		    {
		      ElementLink< xAOD::TrackParticleContainer > trkLink = vxTrackParticles.at(itrk);
		      writeTrack(*trkLink, vertex, trkLink.index());
		    }
		}
	    }
	}
    }

  return;
}


int ZdcNtuple::trackQuality(const xAOD::TrackParticle* track, const xAOD::Vertex* vertex)
{

  if (!track) return -1;

  bool pass_looseprimary = false;
  if (m_selTool->accept(*track,vertex))
    {
      pass_looseprimary = true;      
    }

  int quality = 0;
  if (pass_looseprimary) quality += 1;

  return quality;

}



void ZdcNtuple::writeTrack(const xAOD::TrackParticle* track, const xAOD::Vertex* vertex, int trk_index)
{
  t_trk_pt.push_back(track->pt());
  t_trk_eta.push_back(track->eta());
  t_trk_phi.push_back(track->phi());
  t_trk_e.push_back(track->e());
  t_trk_index.push_back(trk_index);
  t_trk_theta.push_back(track->theta());
  t_trk_charge.push_back(track->charge());
  t_trk_d0.push_back(track->d0());
  t_trk_z0.push_back(track->z0());
  t_trk_vz.push_back(track->vz());

  float vtxz = -999.;
  t_trk_vtxz.push_back(vtxz);

  static const SG::ConstAccessor<uint8_t> numberOfInnermostPixelLayerHitsAcc("numberOfInnermostPixelLayerHits");
  static const SG::ConstAccessor<uint8_t> expectInnermostPixelLayerHitAcc("expectInnermostPixelLayerHit");
  static const SG::ConstAccessor<uint8_t> numberOfNextToInnermostPixelLayerHitsAcc("numberOfNextToInnermostPixelLayerHits");
  static const SG::ConstAccessor<uint8_t> expectNextToInnermostPixelLayerHitAcc("expectNextToInnermostPixelLayerHit");
  static const SG::ConstAccessor<uint8_t> numberOfSCTHitsAcc("numberOfSCTHits");
  static const SG::ConstAccessor<uint8_t> numberOfPixelHitsAcc("numberOfPixelHits");
  static const SG::ConstAccessor<uint8_t> numberOfSCTDeadSensorsAcc("numberOfSCTDeadSensors");
  static const SG::ConstAccessor<uint8_t> numberOfPixelDeadSensorsAcc("numberOfPixelDeadSensors");
  static const SG::ConstAccessor<uint8_t> numberOfSCTHolesAcc("numberOfSCTHoles");
  static const SG::ConstAccessor<uint8_t> numberOfPixelHolesAcc("numberOfPixelHoles");
  static const SG::ConstAccessor<uint8_t> numberOfTRTHitsAcc("numberOfTRTHits");
  static const SG::ConstAccessor<uint8_t> numberOfTRTOutliersAcc("numberOfTRTOutliers");

  t_trk_quality.push_back(trackQuality(track, vertex));
  t_trk_inPixHits.push_back(numberOfInnermostPixelLayerHitsAcc(*track));
  t_trk_exPixHits.push_back(expectInnermostPixelLayerHitAcc(*track));
  t_trk_ninPixHits.push_back(numberOfNextToInnermostPixelLayerHitsAcc(*track));
  t_trk_nexPixHits.push_back(expectNextToInnermostPixelLayerHitAcc(*track));
  t_trk_nSctHits.push_back(numberOfSCTHitsAcc(*track));
  t_trk_nPixHits.push_back(numberOfPixelHitsAcc(*track));
  t_trk_nSctDead.push_back(numberOfSCTDeadSensorsAcc(*track));
  t_trk_nPixDead.push_back(numberOfPixelDeadSensorsAcc(*track));
  t_trk_nSctHoles.push_back(numberOfSCTHolesAcc(*track));
  t_trk_nPixHoles.push_back(numberOfPixelHolesAcc(*track));
  t_trk_nTrtHits.push_back(numberOfTRTHitsAcc(*track));
  t_trk_nTrtOutliers.push_back(numberOfTRTOutliersAcc(*track));

  float pixeldEdx = 0;
  track->summaryValue(pixeldEdx, xAOD::SummaryType::pixeldEdx);
  t_trk_pixeldEdx.push_back(pixeldEdx);
}


void ZdcNtuple::processFCal()
{
  ANA_MSG_DEBUG("processFCal: processing FCal");

  t_fcalEt = 0.;
  t_fcalEtA = 0.;
  t_fcalEtC = 0.;
  t_fcalEtA_TT = 0.;
  t_fcalEtC_TT = 0.;

  if (m_caloSums)
    {
      static const SG::ConstAccessor<std::string> SummaryAcc("Summary");
      for (const auto calosum : *m_caloSums)
	{
	  const std::string name = SummaryAcc(*calosum);
	  if (name == "FCal")
	    {
	      t_fcalEt = calosum->et();
	      ANA_MSG_DEBUG("processFCal: fcalEt = " << t_fcalEt);
	    }
	  
	  if (name == "All")
	    {
	      t_totalEt = calosum->et();
	      ANA_MSG_DEBUG("processFCal: totalEt = " << t_totalEt);
	    }
	}
    }

  t_fcalEtA = 0;
  t_fcalEtC = 0;
  t_totalEt24 = 0;

  if (m_eventShapes)
    {
      for (const auto eventShape : *m_eventShapes)
	{
	  int layer = eventShape->layer();
	  float eta = eventShape->etaMin();
	  float et = eventShape->et();
	  if (layer == 21 || layer == 22 || layer == 23)
	    {
	      if (eta > 0) t_fcalEtA += et;
	      if (eta < 0) t_fcalEtC += et;
	    }
	  
	  if (TMath::Abs(eta) < 2.4)
	    {
	      t_totalEt24 += et;
	    }
	}
    }

  t_L1ET = 0;
  t_L1ET24 = 0;

  if (m_lvl1EnergySumRoI)
  {
    t_L1ET = m_lvl1EnergySumRoI->energyT();
    //t_L1ET24 = m_lvl1EnergySumRoI->energyTRestricted(); // TBD when limited eta ET available
  }

  return;
}

void ZdcNtuple::processGaps()
{

  float eta_min = 5;
  float eta_max = -5;

  if (!m_caloClusters) return;
  for (const auto cl : *m_caloClusters)
  {

    if (cl->pt() < m_gapPtMin) continue;

    int etabin = h_TCSigCut->GetXaxis()->FindBin(cl->eta());
    if (etabin < 1 || etabin > h_TCSigCut->GetNbinsX()) continue;
    float sig_cut = h_TCSigCut->GetBinContent(etabin);
    float sig = cl->getMomentValue(xAOD::CaloCluster::CELL_SIGNIFICANCE);
    int cl_cell_sig_samp = static_cast<int>(cl->getMomentValue(xAOD::CaloCluster::CELL_SIG_SAMPLING));

    ANA_MSG_VERBOSE ("gapclus: etabin " << etabin << " sig_cut=" << sig_cut << " sig=" << sig << " samp=" << cl_cell_sig_samp);

    if (sig < sig_cut) continue;

    if (cl_cell_sig_samp >= CaloSampling::TileBar0 && cl_cell_sig_samp <= CaloSampling::TileExt2) continue;

    if (cl->eta() < eta_min) eta_min = cl->eta();
    if (cl->eta() > eta_max) eta_max = cl->eta();

  }

  t_edgeGapA = 4.9 - eta_max;
  t_edgeGapC = eta_min + 4.9;
  ANA_MSG_DEBUG("processGaps(): egA " << t_edgeGapA << " , egC " << t_edgeGapC);

}

void ZdcNtuple::processMBTS()
{
  ANA_MSG_DEBUG("processMBTS: trying to process!");
  t_mbts_countA = 0;
  t_mbts_countC = 0;
  t_T2mbts_countAin = 0;
  t_T2mbts_countCin = 0;
  t_mbts_timeA = 0.;
  t_mbts_timeC = 0.;
  t_mbts_timeDiff = 0.;

  if (m_mbtsInfo->size() > 0)
  {
    t_mbts_countA = m_mbtsInfo->at(0)->countA();
    t_mbts_countC = m_mbtsInfo->at(0)->countC();
    t_mbts_timeA = m_mbtsInfo->at(0)->timeA();
    t_mbts_timeC = m_mbtsInfo->at(0)->timeC();
    t_mbts_timeDiff = m_mbtsInfo->at(0)->timeDiff();
  }
  else
  {
    ANA_MSG_INFO("processMBTS: Warning: MBTS info empty!");
  }

  for (int iside = 0; iside < 2; iside++)
  {
    for (int iin = 0; iin < 8; iin++)
    {
      t_mbts_in_e[iside][iin] = 0.;
      t_mbts_in_t[iside][iin] = 0.;
      t_T2mbts_in_e[iside][iin] = 0.;
      t_T2mbts_in_t[iside][iin] = 0.;
    }
    for (int iout = 0; iout < 4; iout++)
    {
      t_mbts_out_e[iside][iout] = 0.;
      t_mbts_out_t[iside][iout] = 0.;
      t_T2mbts_out_e[iside][iout] = 0.;
      t_T2mbts_out_t[iside][iout] = 0.;
    }
  }

  ANA_MSG_DEBUG ("filling MBTS");

  if (m_mbtsModules == 0)
  {
    ANA_MSG_INFO("processMBTS: no MBTS container?");
    return;
  }

  for (const auto mbtsMod : *m_mbtsModules)
  {
    int iside = 1;
    if (mbtsMod->type() < 0) iside = 0.;
    float phibin = 0.;
    int iphibin = -1;
    if (mbtsMod->eta() > 3)
    {
      phibin = mbtsMod->phi() / (2 * TMath::Pi() / 8.) - 0.4;
      iphibin = static_cast<int>(phibin);
      if (iphibin < 0 || iphibin > 7)
      {
        ANA_MSG_INFO("processMBTS: MBTS has bad phi bin");
        continue;
      }
      t_mbts_in_e[iside][iphibin] = mbtsMod->e();
      t_mbts_in_t[iside][iphibin] = mbtsMod->time();
    }
    else
    {
      phibin = mbtsMod->phi() / (2 * TMath::Pi() / 4.) - 0.24;
      iphibin = static_cast<int>(phibin);
      if (iphibin < 0 || iphibin > 3)
      {
        ANA_MSG_INFO("processMBTS: MBTS has bad phi bin");
        continue;
      }
      t_mbts_out_e[iside][iphibin] = mbtsMod->e();
      t_mbts_out_t[iside][iphibin] = mbtsMod->time();
    }
  }

  if (!m_trigT2MbtsBits) return;

  for (const auto mbtsBits : *m_trigT2MbtsBits)
  {
    const std::vector<float>& energies = mbtsBits->triggerEnergies();
    const std::vector<float>& times = mbtsBits->triggerTimes();
    for (int imbts = 0; imbts < 32; imbts++)
    {
      int side = imbts / 16;
      int ring = (imbts - 16 * side) / 8;
      bool isInner = (ring == 0);
      int index = (imbts - 16 * side - ring * 8);
      if (!isInner)
      {
        if ((index % 2) != 0) continue; // skip odd out ring
        index /= 2;
      }
      int iside = (side == 0) ? 1 : 0; // code maps side 1 into first 16 bits and side -1 into second set

      ANA_MSG_VERBOSE ("imbts=" << imbts << " isInner=" << isInner << " iside=" << iside << " index=" << index << " e=" << energies.at(imbts) << " t=" << times.at(imbts));
      if (isInner)
      {
        t_T2mbts_in_e[iside][index] = energies.at(imbts);
        t_T2mbts_in_t[iside][index] = times.at(imbts);
        if (TMath::Abs(times.at(imbts)) < 12.0 && energies.at(imbts) > 40 / 222.)
        {
          if (iside == 0) t_T2mbts_countCin++;
          if (iside == 1) t_T2mbts_countAin++;
        }
      }
      else
      {
        t_T2mbts_out_e[iside][index] = energies.at(imbts);
        t_T2mbts_out_t[iside][index] = times.at(imbts);
      }
    }
  }

  return;
}

void ZdcNtuple::processClusters()
{
  //t_nclus = 0;

  t_cc_pt.clear();
  t_cc_eta.clear();
  t_cc_phi.clear();
  t_cc_e.clear();
  t_cc_raw_m.clear();
  t_cc_raw_eta.clear();
  t_cc_raw_phi.clear();
  t_cc_raw_e.clear();
  t_cc_raw_samp.clear();
  t_cc_layer.clear();
  t_cc_sig.clear();

  t_nclus = m_caloClusters->size();

  t_clusEt = 0;
  t_clusEtMax = -999;
  t_clusetaMax = 0;
  t_clusphiMax = 0;

  for (const auto cluster : *m_caloClusters)
  {
    t_cc_pt.push_back(cluster->pt());
    t_cc_eta.push_back(cluster->eta());
    t_cc_phi.push_back(cluster->phi());
    t_cc_e.push_back(cluster->e());
    t_cc_raw_m.push_back(cluster->rawM());
    t_cc_raw_eta.push_back(cluster->rawEta());
    t_cc_raw_phi.push_back(cluster->rawPhi());
    t_cc_raw_e.push_back(cluster->rawE());

    std::vector<float> energies;

    for (size_t s = CaloSampling::PreSamplerB; s < CaloSampling::Unknown; s++ )
    {
      bool hasSample = cluster->hasSampling( (xAOD::CaloCluster::CaloSample) s );
      float e = 0;
      if (hasSample)
      {
        e = cluster->eSample( (xAOD::CaloCluster::CaloSample) s);
      }
      energies.push_back(e);
    }
    t_cc_raw_samp.push_back(energies);

    float et = cluster->e() / TMath::CosH(cluster->eta());
    t_clusEt += et;
    if (et > t_clusEtMax)
    {
      t_clusEtMax = et;
      t_clusetaMax = cluster->eta();
      t_clusphiMax = cluster->phi();
    }

    double cell_sig = 0;
    if (!cluster->retrieveMoment(xAOD::CaloCluster::CELL_SIGNIFICANCE, cell_sig)) {ANA_MSG_DEBUG("processClusters() : No CELL_SIGNIFICANCE!");}
    t_cc_sig.push_back(cell_sig);
    double cell_layer = 0;
    if (!cluster->retrieveMoment(xAOD::CaloCluster::CELL_SIG_SAMPLING, cell_layer)) {ANA_MSG_DEBUG("processClusters() : No CELL_SIG_SAMPLING!");}
    t_cc_layer.push_back(static_cast<int>(cell_layer));
    //t_nclus++;
  }

  if ( (!enableClusters) || (t_ntrk >= trackLimit) ) // if disabled or if too many tracks
  {
    t_cc_pt.clear();
    t_cc_eta.clear();
    t_cc_phi.clear();
    t_cc_e.clear();
    t_cc_layer.clear();
    t_cc_sig.clear();
  }
  else
  {
    ANA_MSG_DEBUG("processClusters(): keeping clusters");
  }
  return;
}

void ZdcNtuple::processProtons(){

  proton_pt.clear();
  proton_eta.clear();
  proton_phi.clear();
  proton_e.clear();
  proton_side.clear();
  proton_eLoss.clear();
  proton_t.clear();

  proton_track_stationID.clear();
  proton_track_nClusters.clear();
  proton_track_xLocal.clear();
  proton_track_yLocal.clear();
  proton_track_zLocal.clear();
  proton_track_xSlope.clear();
  proton_track_ySlope.clear();

  proton_track_stationID.resize(m_afpProtons->size(), std::vector<int>(2));
  proton_track_nClusters.resize(m_afpProtons->size(), std::vector<int>(2));
  proton_track_xLocal.resize(m_afpProtons->size(), std::vector<float>(2));
  proton_track_yLocal.resize(m_afpProtons->size(), std::vector<float>(2));
  proton_track_zLocal.resize(m_afpProtons->size(), std::vector<float>(2));
  proton_track_xSlope.resize(m_afpProtons->size(), std::vector<float>(2));
  proton_track_ySlope.resize(m_afpProtons->size(), std::vector<float>(2));

  nProtons = 0;

  for(const auto * proton: *m_afpProtons){

    proton_pt.push_back(proton->pt());
    proton_eta.push_back(proton->eta());
    proton_phi.push_back(proton->phi());
    proton_e.push_back(proton->e());
    proton_side.push_back(proton->side());

    proton_eLoss.push_back((6800.-proton->e())/6800.);
    p_scat.SetPtEtaPhiE(proton->pt(), proton->eta(), proton->phi(), proton->e());
    (signbit(proton->eta())) ? p_beam.SetPxPyPzE(0.0, 0.0, -6800.0, 6800.0) : p_beam.SetPxPyPzE(0.0, 0.0, 6800.0,\
												6800.0);

    proton_t.push_back( (p_beam - p_scat)*(p_beam - p_scat));

    for(int i=0; i< int(proton->nTracks()); i++){

      proton_track_stationID.at(nProtons).at(i) = proton->track(i)->stationID();
      proton_track_nClusters.at(nProtons).at(i) = proton->track(i)->nClusters();
      proton_track_xLocal.at(nProtons).at(i) = proton->track(i)->xLocal();
      proton_track_yLocal.at(nProtons).at(i) = proton->track(i)->yLocal();
      proton_track_zLocal.at(nProtons).at(i) = proton->track(i)->zLocal();
      proton_track_xSlope.at(nProtons).at(i) = proton->track(i)->xSlope();
      proton_track_ySlope.at(nProtons).at(i) = proton->track(i)->ySlope();

    }

    nProtons++;
  }

  return;
}

uint32_t ZdcNtuple::acceptEvent()
{
  uint32_t passbits = 0;

  if (!m_isMC)
  {
    if (useGRL)
    {
      if (!m_grl->passRunLB(*m_eventInfo)) {
        passbits += 1; // UPC GRL
      }
    }

    /*
      if(!m_grl_mb->passRunLB(*m_eventInfo)){
      passbits += 4; // MB GRL
      }
    */

    if (   (m_eventInfo->errorState(xAOD::EventInfo::LAr) == xAOD::EventInfo::Error )
           || (m_eventInfo->errorState(xAOD::EventInfo::Tile) == xAOD::EventInfo::Error )
           || (m_eventInfo->errorState(xAOD::EventInfo::SCT) == xAOD::EventInfo::Error )
           || (m_eventInfo->isEventFlagBitSet(xAOD::EventInfo::Core, 18) ) )
    {
      passbits += 2;
    } // end if event flags check
  } // end if the event is data

  return passbits;
}

void ZdcNtuple::setupTriggerHistos()
{
  if (!enableTrigger) return;

  m_outputTree = tree( "zdcTree" );
  ANA_MSG_INFO("setupTriggerHistos(): Setting up trigger histos and ntuple = " << m_outputTree);

  std::vector<std::string> triggers;
  std::vector<std::string> rerunTriggers;
  bool zdc_triggers = true;

  // ZDC triggers
  if (zdc_triggers)
  {
    if (zdcCalib) // lists for calibration data
      {
	if (lhcf2022)
	  {
	    triggers.push_back("HLT_noalg_ZDCPEB_L1LHCF");
	    triggers.push_back("HLT_noalg_ZDCPEB_L1ZDC_OR");
	  }

	if (pbpb2023)
	  {
	    triggers.push_back("HLT_noalg_ZDCPEB_L1ZDC_OR");
	    triggers.push_back("HLT_noalg_ZDCPEB_L1ZDC_OR_EMPTY");
	    triggers.push_back("HLT_noalg_ZDCPEB_L1ZDC_OR_UNPAIRED_NONISO");
	    triggers.push_back("HLT_noalg_ZDCPEB_L1ZDC_A_C");
	    triggers.push_back("HLT_noalg_ZDCPEB_L1ZDC_A_C_EMPTY");
	    triggers.push_back("HLT_noalg_ZDCPEB_L1ZDC_A_C_UNPAIRED_NONISO");
	    triggers.push_back("HLT_noalg_ZDCPEB_L1ZDC_A");
	    triggers.push_back("HLT_noalg_ZDCPEB_L1ZDC_A_EMPTY");
	    triggers.push_back("HLT_noalg_ZDCPEB_L1ZDC_A_UNPAIRED_NONISO");
	    triggers.push_back("HLT_noalg_ZDCPEB_L1ZDC_C");
	    triggers.push_back("HLT_noalg_ZDCPEB_L1ZDC_C_EMPTY");
	    triggers.push_back("HLT_noalg_ZDCPEB_L1ZDC_C_UNPAIRED_NONISO");
	  }
      }
    else // lists for physics data
      {
	if (lhcf2022)
          {
            triggers.push_back("HLT_noalg_L1LHCF");
          }
	if (lhcf2022afp)
          {
            triggers.push_back("HLT_noalg_AFPPEB_L1AFP_A");
            triggers.push_back("HLT_noalg_AFPPEB_L1AFP_C");
          }
	if (lhcf2022zdc)
          {
            triggers.push_back("HLT_noalg_ZDCPEB_L1ZDC_OR");
            triggers.push_back("HLT_noalg_ZDCPEB_L1LHCF");
            triggers.push_back("HLT_noalg_L1ZDC_OR");
            triggers.push_back("HLT_noalg_L1ZDC_XOR_E2");
            triggers.push_back("HLT_noalg_L1ZDC_XOR_E1_E3");
            triggers.push_back("HLT_noalg_L1ZDC_A_AND_C");
            triggers.push_back("HLT_mb_sptrk_L1ZDC_OR");
            triggers.push_back("HLT_mb_sptrk_L1ZDC_XOR_E2");
            triggers.push_back("HLT_mb_sptrk_L1ZDC_XOR_E1_E3");
            triggers.push_back("HLT_mb_sptrk_L1ZDC_A_AND_C");
            triggers.push_back("HLT_mb_sp100_trk30_hmt_L1ZDC_XOR_E2");
            triggers.push_back("HLT_mb_sp100_trk30_hmt_L1ZDC_XOR_E1_E3");
            triggers.push_back("HLT_mb_sp100_trk30_hmt_L1ZDC_A_AND_C");
          }
      }
  }

  //char name[50];
  ANA_MSG_INFO("Adding trigger branches!");
  
  int ic = 0;
  for (auto &trig : triggers)
    {
      const Trig::ChainGroup* cg = m_trigDecisionTool->getChainGroup(trig);
      if (cg->getListOfTriggers().size())
	{
	  ANA_MSG_INFO("setupTriggerHistos(): Trigger found = " << trig.c_str() << " bit " << ic);
	}
      else
	{
	  ANA_MSG_INFO("setupTriggerHistos(): Trigger NOT found = " << trig.c_str()  << " bit " << ic);
	}
      m_chainGroups.push_back(cg);
      // force all triggers to show up in tree
      TString bname(trig.c_str());
      m_outputTree->Branch(bname, &(t_decisions[ic]), bname + "/O");
      m_outputTree->Branch("ps_" + bname, &(t_prescales[ic]), "ps_" + bname + "/F");
      ic++;
    }

  ANA_MSG_INFO( "triggers = " << triggers.size() << " chains = " << m_chainGroups.size() );

  int irc = 0;
  ANA_MSG_INFO("Adding rerun trigger branches!");
  for (auto &trig : rerunTriggers)
    {
      const Trig::ChainGroup* cg = m_trigDecisionTool->getChainGroup(trig);
      m_rerunChainGroups.push_back(cg);
      if (cg->getListOfTriggers().size())
	{
	  ANA_MSG_INFO("setupTriggerHistos(): Rerun trigger found = " << trig.c_str() << " bit " << irc);
	}
      else
	{
	  ANA_MSG_INFO("setupTriggerHistos(): Rerun trigger NOT found = " << trig.c_str()  << " bit " << irc);
	}
      // force all rerun triggers to show up in tree
      TString bname(trig.c_str());
      m_outputTree->Branch(bname, &(t_rerunDecisions[irc]), bname + "/O");
      irc++;
    }
  
  // trigger matching flags for electrons and muons
  
  m_setupTrigHist = true;

  ANA_MSG_INFO("setupTriggerHistos(): Finished setting up trigger");

}

// from TrigMuonMatching
double ZdcNtuple::dR(const double eta1, const double phi1, const double eta2, const double phi2)
{
  double deta = std::abs(eta1 - eta2);
  double dphi = std::abs(phi1 - phi2) < TMath::Pi() ? std::abs(phi1 - phi2) : 2 * TMath::Pi() - std::abs(phi1 - phi2);
  return std::sqrt(deta * deta + dphi * dphi);
}


StatusCode ZdcNtuple :: finalize ()
{
  // This method is the mirror image of initialize(), meaning it gets
  // called after the last event has been processed on the worker node
  // and allows you to finish up any objects you created in
  // initialize() before they are written to disk.  This is actually
  // fairly rare, since this happens separately for each worker node.
  // Most of the time you want to do your post-processing on the
  // submission node after all your histogram outputs have been
  // merged.  This is different from histFinalize() in that it only
  // gets called on worker nodes that processed input events.


  return StatusCode::SUCCESS;
}
