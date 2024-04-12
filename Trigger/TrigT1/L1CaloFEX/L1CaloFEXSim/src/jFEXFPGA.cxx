/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           jFEXFPGA  -  description
//                              -------------------
//     begin                : 19 10 2020
//     email                : jacob.julian.kempster@cern.ch
//  ***************************************************************************/

#include "L1CaloFEXSim/jFEXFPGA.h"
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerContainer.h"
#include "L1CaloFEXSim/jFEXSmallRJetAlgo.h" 
#include "L1CaloFEXSim/jFEXLargeRJetAlgo.h" 
#include "L1CaloFEXSim/jFEXOutputCollection.h" 
#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"
#include "L1CaloFEXSim/jFEXtauAlgo.h" 
#include "L1CaloFEXSim/jFEXsumETAlgo.h" 
#include "L1CaloFEXSim/jFEXmetAlgo.h" 
#include "L1CaloFEXSim/jFEXForwardJetsAlgo.h"
#include "L1CaloFEXSim/jFEXForwardJetsInfo.h"
#include "L1CaloFEXSim/jFEXForwardElecAlgo.h"
#include "L1CaloFEXSim/jFEXForwardElecInfo.h"
#include "L1CaloFEXSim/jFEXPileupAndNoise.h"
#include "L1CaloFEXSim/jFEXFormTOBs.h"
#include "CaloEvent/CaloCellContainer.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/CaloCell_SuperCell_ID.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include <vector>
#include "TrigConfData/L1Menu.h"
#include "TH1F.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/IClassIDSvc.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"
#include "SGTools/TestStore.h"
#include "StoreGate/StoreGateSvc.h"


namespace LVL1 {

  // default constructor for persistency

jFEXFPGA::jFEXFPGA(const std::string& type,const std::string& name,const IInterface* parent): AthAlgTool(type,name,parent) {
    declareInterface<IjFEXFPGA>(this);
}

/** Destructor */
jFEXFPGA::~jFEXFPGA()
{
}

//================ Initialisation =================================================
  
StatusCode jFEXFPGA::initialize() {

    ATH_CHECK(m_jTowerContainerKey.initialize());
    ATH_CHECK(m_l1MenuKey.initialize());
    ATH_CHECK(m_jFEXtauAlgoTool.retrieve());
    return StatusCode::SUCCESS;
}

StatusCode jFEXFPGA::init(int id, int jfexid) {
    m_id = id;
    m_jfexid = jfexid;

    return StatusCode::SUCCESS;

}

void jFEXFPGA::reset() {

    m_id = -1;
    m_jfexid = -1;
    m_tau_tobwords.clear();
    m_SRJet_tobwords.clear();
    m_LRJet_tobwords.clear();
    m_sumET_tobwords.clear();
    m_Met_tobwords.clear();
    m_map_Etvalues_FPGA.clear();
    m_map_EM_Etvalues_FPGA.clear();
    m_map_HAD_Etvalues_FPGA.clear();
    m_FwdEl_tobwords.clear();
}

StatusCode jFEXFPGA::execute(jFEXOutputCollection* inputOutputCollection, const std::pair<unsigned int, const std::vector<int>&> & jetCalibrationParameters) {
    
    // Retrieve the L1 menu configuration
    SG::ReadHandle<TrigConf::L1Menu> l1Menu (m_l1MenuKey/*, ctx*/);
    
    const TrigConf::L1ThrExtraInfo_jTAU & thr_jTAU = l1Menu->thrExtraInfo().jTAU(); 
    const TrigConf::L1ThrExtraInfo_jJ   & thr_jJ   = l1Menu->thrExtraInfo().jJ();  
    const TrigConf::L1ThrExtraInfo_jLJ  & thr_jLJ  = l1Menu->thrExtraInfo().jLJ(); 
    const TrigConf::L1ThrExtraInfo_jTE  & thr_jTE  = l1Menu->thrExtraInfo().jTE(); 
    const TrigConf::L1ThrExtraInfo_jXE  & thr_jXE  = l1Menu->thrExtraInfo().jXE();
    
    SG::ReadHandle<jTowerContainer> jTowerContainer(m_jTowerContainerKey/*,ctx*/);
    if(!jTowerContainer.isValid()) {
        ATH_MSG_ERROR("Could not retrieve container " << m_jTowerContainerKey.key() );
        return StatusCode::FAILURE;
    }
    
    ATH_CHECK( m_jFEXPileupAndNoiseTool->safetyTest());
    ATH_CHECK( m_jFEXPileupAndNoiseTool->reset());
    if(m_jfexid == 0 || m_jfexid == 5) {
        m_jFEXPileupAndNoiseTool->setup(m_jTowersIDs_Wide);
    }
    else {
        m_jFEXPileupAndNoiseTool->setup(m_jTowersIDs_Thin);
    }

    //Calculating and sustracting pileup
    const std::vector<int> pileup_rho = m_jFEXPileupAndNoiseTool->CalculatePileup();

    //From the DB
    ATH_CHECK(m_jFEXPileupAndNoiseTool->ApplyPileup());
    
    //Noise should be always applied
    m_jFEXPileupAndNoiseTool->ApplyNoise2Jets(true);
    m_jFEXPileupAndNoiseTool->ApplyNoise2Met(true);
    //Getting the values
    m_map_HAD_Etvalues_FPGA = m_jFEXPileupAndNoiseTool->Get_HAD_Et_values();
    m_map_EM_Etvalues_FPGA  = m_jFEXPileupAndNoiseTool->Get_EM_Et_values();
    m_map_Etvalues_FPGA     = m_jFEXPileupAndNoiseTool->GetEt_values();
    std::vector<int> pileup_ID;
    std::vector<int> pileup_HAD_jet;
    std::vector<int> pileup_EM_jet;
    std::vector<int> pileup_Total_jet;
    std::vector<int> pileup_HAD_met;
    std::vector<int> pileup_EM_met;
    std::vector<int> pileup_Total_met;
    for (auto const& [key, val] : m_map_HAD_Etvalues_FPGA)
    {
        pileup_ID.push_back(key);
        pileup_HAD_jet.push_back(val[0]);
        pileup_EM_jet.push_back(m_map_EM_Etvalues_FPGA[key][0]);
        pileup_Total_jet.push_back(m_map_Etvalues_FPGA[key][0]);
        pileup_HAD_met.push_back(val[1]);
        pileup_EM_met.push_back(m_map_EM_Etvalues_FPGA[key][1]);
        pileup_Total_met.push_back(m_map_Etvalues_FPGA[key][1]);
    }    
    
    //saving pileup information
    inputOutputCollection->addValue_pileup("pileup_FPGAid", m_id);
    inputOutputCollection->addValue_pileup("pileup_jFEXid", m_jfexid);
    inputOutputCollection->addValue_pileup("pileup_rho_EM", pileup_rho[0]);
    inputOutputCollection->addValue_pileup("pileup_rho_HAD1", pileup_rho[1]);
    inputOutputCollection->addValue_pileup("pileup_rho_HAD2", pileup_rho[2]);
    inputOutputCollection->addValue_pileup("pileup_rho_HAD3", pileup_rho[3]);
    inputOutputCollection->addValue_pileup("pileup_rho_FCAL", pileup_rho[4]);
    inputOutputCollection->addValue_pileup("pileup_map_ID"  , pileup_ID);
    inputOutputCollection->addValue_pileup("pileup_map_Et_values_HAD_jet"  , pileup_HAD_jet);
    inputOutputCollection->addValue_pileup("pileup_map_Et_values_EM_jet"   , pileup_EM_jet);
    inputOutputCollection->addValue_pileup("pileup_map_Et_values_Total_jet", pileup_Total_jet);
    inputOutputCollection->addValue_pileup("pileup_map_Et_values_HAD_met"  , pileup_HAD_met);
    inputOutputCollection->addValue_pileup("pileup_map_Et_values_EM_met"   , pileup_EM_met);
    inputOutputCollection->addValue_pileup("pileup_map_Et_values_Total_met", pileup_Total_met);
    inputOutputCollection->fill_pileup();    
    
    if(m_id==0 || m_id==3) {
        ATH_CHECK( m_jFEXsumETAlgoTool->safetyTest());
        ATH_CHECK( m_jFEXsumETAlgoTool->reset());
        ATH_CHECK( m_jFEXmetAlgoTool->safetyTest());
        ATH_CHECK( m_jFEXmetAlgoTool->reset());
        
        m_jFEXsumETAlgoTool->setFPGAEnergy(m_map_Etvalues_FPGA);
        m_jFEXmetAlgoTool->setFPGAEnergy(m_map_Etvalues_FPGA);
        
        unsigned int bin_pos = thr_jTE.etaBoundary_fw(m_jfex_string[m_jfexid]);
        
        std::unique_ptr<jFEXTOB> jXE_tob = std::make_unique<jFEXTOB>(); 
        uint32_t jXE_tobword = 0;
        
        std::unique_ptr<jFEXTOB> jTE_tob = std::make_unique<jFEXTOB>(); 
        uint32_t jTE_tobword = 0;
                      
                         
        int hemisphere = m_id == 0 ? 1 : -1;
        
        if(m_jfexid > 0 && m_jfexid < 5) {
            
            //-----------------jFEXsumETAlgo-----------------
            m_jFEXsumETAlgoTool->setup(m_jTowersIDs_Thin);
            m_jFEXsumETAlgoTool->buildBarrelSumET();

            //-----------------jFEXmetAlgo-----------------
            m_jFEXmetAlgoTool->setup(m_jTowersIDs_Thin, hemisphere);
            m_jFEXmetAlgoTool->buildBarrelmet();
        }
        else if(m_jfexid == 0 ) {
            
            int flipped_jTowersIDs      [FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width] = {{0}};
            int max_phi_it = FEXAlgoSpaceDefs::jFEX_algoSpace_height-1;
            int max_eta_it = FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width-1;
            for(int mphi = 0; mphi <= max_phi_it; mphi++) {
                for(int meta = 0; meta <= max_eta_it; meta++) {
                    flipped_jTowersIDs[mphi][meta]=m_jTowersIDs_Wide[mphi][max_eta_it-meta];
                }
            }
            //-----------------jFEXsumETAlgo-----------------
            m_jFEXsumETAlgoTool->setup(flipped_jTowersIDs);
            m_jFEXsumETAlgoTool->buildFWDSumET();

            //-----------------jFEXmetAlgo-----------------
            m_jFEXmetAlgoTool->setup(flipped_jTowersIDs, hemisphere);
            m_jFEXmetAlgoTool->buildFWDmet();
        }
        else if(m_jfexid == 5) {
            
            //-----------------jFEXsumETAlgo-----------------
            m_jFEXsumETAlgoTool->setup(m_jTowersIDs_Wide);
            m_jFEXsumETAlgoTool->buildFWDSumET();

            //-----------------jFEXmetAlgo-----------------
            m_jFEXmetAlgoTool->setup(m_jTowersIDs_Wide, hemisphere);
            m_jFEXmetAlgoTool->buildFWDmet();
        }
        
        jXE_tobword = m_IjFEXFormTOBsTool->formMetTOB(m_jFEXmetAlgoTool->GetMetXComponent(), m_jFEXmetAlgoTool->GetMetYComponent(),m_jFEXmetAlgoTool->getjXESat(),thr_jXE.resolutionMeV());
        jXE_tob->initialize(m_id,m_jfexid,jXE_tobword,thr_jXE.resolutionMeV(),0);
        m_Met_tobwords.push_back(std::move(jXE_tob));
        
        std::tuple<int,bool> jTElow;
        std::tuple<int,bool> jTEhigh;
        
        // NOTE: Foward FPGA in the C-side is already flipped, however we still need to flip the jFEX module 1 and 2 
        if(m_jfexid == 1 || m_jfexid == 2){
            jTElow  = m_jFEXsumETAlgoTool->getETupperEta(bin_pos);
            jTEhigh = m_jFEXsumETAlgoTool->getETlowerEta(bin_pos);
        }
        else{
            jTElow  = m_jFEXsumETAlgoTool->getETlowerEta(bin_pos);
            jTEhigh = m_jFEXsumETAlgoTool->getETupperEta(bin_pos);            
        }
        
        jTE_tobword = m_IjFEXFormTOBsTool->formSumETTOB(jTElow,jTEhigh,thr_jTE.resolutionMeV());
        jTE_tob->initialize(m_id,m_jfexid,jTE_tobword,thr_jTE.resolutionMeV(),0);
        m_sumET_tobwords.push_back(std::move(jTE_tob));
    }

    //-----------jFEXSmallRJet & Large R Jet Algo-----------------
    ATH_MSG_DEBUG("================ Central Algorithms ================");

    //Central region algorithms
    if(m_jfexid > 0 && m_jfexid < 5) {
        m_jFEXSmallRJetAlgoTool->setFPGAEnergy(m_map_Etvalues_FPGA);
        m_jFEXLargeRJetAlgoTool->setFPGAEnergy(m_map_Etvalues_FPGA);
        m_jFEXtauAlgoTool->setFPGAEnergy(m_map_Etvalues_FPGA);
        
        for(int mphi = 8; mphi < FEXAlgoSpaceDefs::jFEX_algoSpace_height-8; mphi++) {
            for(int meta = 8; meta < FEXAlgoSpaceDefs::jFEX_thin_algoSpace_width-8; meta++) {
                
                //definition of arrays
                int TT_seed_ID[3][3]= {{0}};
                int TT_First_ETring[36]= {0};
                int First_ETring_it = 0; 
                               
                int Jet_SearchWindow[7][7] = {{0}};
                int Jet_SearchWindowDisplaced[7][7] = {{0}};
                int largeRCluster_IDs[15][15]= {{0}};

                //filling up array to send them to the algorithms
                for(int i = -7; i< 8; i++ ) {
                    for(int j = -7; j< 8; j++) {

                        if(std::abs(i)<4 && std::abs(j)<4) {
                            Jet_SearchWindow[3 + i][3 + j] = m_jTowersIDs_Thin[mphi + i][meta +j];
                            Jet_SearchWindowDisplaced[3 + i][3 + j] = m_jTowersIDs_Thin[mphi+i+1][meta+j+1];
                        }
                        
                        uint deltaR = std::sqrt(std::pow(i,2)+std::pow(j,2));

                        if(deltaR<2) {
                            TT_seed_ID[i+1][j+1] = m_jTowersIDs_Thin[mphi +i][meta +j]; // Seed 0.3x0.3 in phi-eta plane
                        }
                        else if(deltaR<4) {
                            TT_First_ETring[First_ETring_it]= m_jTowersIDs_Thin[mphi +i][meta +j]; // First energy ring, will be used as tau ISO
                            ++First_ETring_it;

                        }                       
                        else if(deltaR<8) {
                            largeRCluster_IDs[7 +i][7 +j] = m_jTowersIDs_Thin[mphi + i][meta +j];
                        }
                    }
                }
                
                // ********  jJ and jLJ algorithms  ********
                ATH_CHECK( m_jFEXSmallRJetAlgoTool->safetyTest());
                ATH_CHECK( m_jFEXLargeRJetAlgoTool->safetyTest());
                m_jFEXSmallRJetAlgoTool->setup(Jet_SearchWindow, Jet_SearchWindowDisplaced);
                m_jFEXLargeRJetAlgoTool->setupCluster(largeRCluster_IDs);
                m_jFEXSmallRJetAlgoTool->buildSeeds();
                
                bool is_Jet_LM = m_jFEXSmallRJetAlgoTool->isSeedLocalMaxima();
                
                if(is_Jet_LM) {
                    
                    //getting the energies
                    int SRj_Et = m_jFEXSmallRJetAlgoTool->getSmallClusterET();
                    int LRj_Et = m_jFEXLargeRJetAlgoTool->getLargeClusterET(SRj_Et,m_jFEXLargeRJetAlgoTool->getRingET());
                    
                    bool SRj_Sat = m_jFEXSmallRJetAlgoTool->getSRjetSat();
                    bool LRj_Sat = SRj_Sat || m_jFEXLargeRJetAlgoTool->getLRjetSat();
                    
                    int meta_LM = meta;
                    int mphi_LM = mphi;

                    //Creating SR TOB
                    uint32_t SRJet_tobword = m_IjFEXFormTOBsTool->formSRJetTOB(m_jfexid, mphi_LM, meta_LM, SRj_Et, SRj_Sat, thr_jJ.resolutionMeV(), thr_jJ.ptMinToTopoMeV(m_jfex_string[m_jfexid]), jetCalibrationParameters);
                    
                    std::unique_ptr<jFEXTOB> jJ_tob = std::make_unique<jFEXTOB>(); 
                    jJ_tob->initialize(m_id,m_jfexid,SRJet_tobword,thr_jJ.resolutionMeV(),m_jTowersIDs_Thin[mphi_LM][meta_LM]);              
                    if ( SRJet_tobword != 0 ){
                        m_SRJet_tobwords.push_back(std::move(jJ_tob));
                    } 
                    
                    //Creating LR TOB
                    uint32_t LRJet_tobword = m_IjFEXFormTOBsTool->formLRJetTOB(m_jfexid, mphi_LM, meta_LM, LRj_Et, LRj_Sat, thr_jLJ.resolutionMeV(), thr_jLJ.ptMinToTopoMeV(m_jfex_string[m_jfexid]));

                    std::unique_ptr<jFEXTOB> jLJ_tob = std::make_unique<jFEXTOB>(); 
                    jLJ_tob->initialize(m_id,m_jfexid,LRJet_tobword,thr_jLJ.resolutionMeV(),m_jTowersIDs_Thin[mphi_LM][meta_LM]);              
                    if ( LRJet_tobword != 0 ) m_LRJet_tobwords.push_back(std::move(jLJ_tob));                    
                    
                }
                // ********  jTau algorithm  ********
                
                ATH_CHECK( m_jFEXtauAlgoTool->safetyTest());
                m_jFEXtauAlgoTool->setup(TT_seed_ID);
                bool is_tau_LocalMax = m_jFEXtauAlgoTool->isSeedLocalMaxima();
        
                // Save TOB is tau is a local maxima
                if ( is_tau_LocalMax ) { 
                    
                    //calculates the 1st energy ring
                    m_jFEXtauAlgoTool->setFirstEtRing(TT_First_ETring);
                    
                    
                    uint32_t jTau_tobword = m_IjFEXFormTOBsTool->formTauTOB(m_jfexid,mphi,meta,m_jFEXtauAlgoTool->getClusterEt(),m_jFEXtauAlgoTool->getFirstEtRing(),m_jFEXtauAlgoTool->getTauSat(),thr_jTAU.resolutionMeV(),thr_jTAU.ptMinToTopoMeV(m_jfex_string[m_jfexid]));

                    std::unique_ptr<jFEXTOB> jTau_tob = std::make_unique<jFEXTOB>();
                    jTau_tob->initialize(m_id,m_jfexid,jTau_tobword,thr_jTAU.resolutionMeV(),m_jTowersIDs_Thin[mphi][meta]);
                    
                    if ( jTau_tobword != 0 ){
                        m_tau_tobwords.push_back(std::move(jTau_tob)); 
                    }
                }                
            }
        }
    } //end of if statement for checking if in central jfex modules
    
    //FCAL region algorithm
    if(m_jfexid ==0 || m_jfexid ==5) {
        
        //**********Forward Jets***********************
        ATH_CHECK(m_jFEXForwardJetsAlgoTool->safetyTest());
        ATH_CHECK(m_jFEXForwardJetsAlgoTool->reset());
        m_jFEXForwardJetsAlgoTool->setFPGAEnergy(m_map_Etvalues_FPGA);
        m_jFEXForwardJetsAlgoTool->setup(m_jTowersIDs_Wide,m_jfexid);

        m_FCALJets =  m_jFEXForwardJetsAlgoTool->calculateJetETs();
        for(std::unordered_map<int, jFEXForwardJetsInfo>::iterator it = m_FCALJets.begin(); it!=(m_FCALJets.end()); ++it) {

            uint32_t TTID = it->first;
            jFEXForwardJetsInfo FCALJets = it->second;

            int iphi = FCALJets.getCentreLocalTTPhi();
            int ieta = FCALJets.getCentreLocalTTEta();
            m_SRJetET = FCALJets.getSeedET() + FCALJets.getFirstEnergyRingET();
            m_LRJetET = m_SRJetET + FCALJets.getSecondEnergyRingET();
            
            bool SRJ_sat = FCALJets.getSRjetSat();
            
            uint32_t SRFCAL_Jet_tobword = m_IjFEXFormTOBsTool->formSRJetTOB(m_jfexid, iphi, ieta, m_SRJetET, SRJ_sat, thr_jJ.resolutionMeV(), thr_jJ.ptMinToTopoMeV(m_jfex_string[m_jfexid]), jetCalibrationParameters);
            
            std::unique_ptr<jFEXTOB> jJ_tob = std::make_unique<jFEXTOB>(); 
            jJ_tob->initialize(m_id,m_jfexid,SRFCAL_Jet_tobword,thr_jJ.resolutionMeV(),TTID);   
            
            if ( SRFCAL_Jet_tobword != 0 ){
                m_SRJet_tobwords.push_back(std::move(jJ_tob));
            } 
            
            if(std::fabs(FCALJets.getCentreTTEta())<2.51){
                bool LRJ_sat = FCALJets.getLRjetSat();
                uint32_t LRFCAL_Jet_tobword = m_IjFEXFormTOBsTool->formLRJetTOB(m_jfexid, iphi, ieta, m_LRJetET, LRJ_sat, thr_jLJ.resolutionMeV(),thr_jLJ.ptMinToTopoMeV(m_jfex_string[m_jfexid]));

                std::unique_ptr<jFEXTOB> jLJ_tob = std::make_unique<jFEXTOB>(); 
                jLJ_tob->initialize(m_id,m_jfexid,LRFCAL_Jet_tobword,thr_jLJ.resolutionMeV(),TTID);              
                if ( LRFCAL_Jet_tobword != 0 ) m_LRJet_tobwords.push_back(std::move(jLJ_tob));                   
            }
            
        }
        //********** Forward Electrons ***********************
        ATH_CHECK(m_jFEXForwardElecAlgoTool->safetyTest());
        ATH_CHECK(m_jFEXForwardElecAlgoTool->reset());
        m_jFEXForwardElecAlgoTool->setFPGAEnergy(m_map_EM_Etvalues_FPGA,m_map_HAD_Etvalues_FPGA);        
        m_jFEXForwardElecAlgoTool->setup(m_jTowersIDs_Wide,m_jfexid,m_id);
        m_ForwardElecs = m_jFEXForwardElecAlgoTool->calculateEDM();

        /// Retrieve the L1 menu configuration 
	SG::ReadHandle<TrigConf::L1Menu> l1Menu (m_l1MenuKey/*, ctx*/);
	const TrigConf::L1ThrExtraInfo_jEM & thr_jEM = l1Menu->thrExtraInfo().jEM();
        const uint jFEXETResolution = thr_jEM.resolutionMeV();//200 
	std::string str_jfexname = m_jfex_string[m_jfexid];
	uint minEtThreshold = thr_jEM.ptMinToTopoMeV(str_jfexname)/jFEXETResolution;
	//uint Cval[9] = {1,2,3,20,30,40,20,30,40};//C values for iso, emfr1 and emfr2    
	std::vector<int> Ciso;
	std::vector<int> Chad1;
	std::vector<int> Chad2;

	for(std::unordered_map<uint, jFEXForwardElecInfo>::iterator itel = m_ForwardElecs.begin(); itel!=(m_ForwardElecs.end()); ++itel) {
	  uint32_t TTID = itel->first;
	  jFEXForwardElecInfo elCluster = itel->second;
	  uint meta = elCluster.getCoreIeta();//check whether this is the one used by the Trigger conf
    
	  //retrieve jet rejection thresholds from trigger configuration
	  auto wp_loose  = thr_jEM.isolation(TrigConf::Selection::WP::LOOSE, meta);
	  auto wp_medium = thr_jEM.isolation(TrigConf::Selection::WP::MEDIUM, meta);
	  auto wp_tight  = thr_jEM.isolation(TrigConf::Selection::WP::TIGHT, meta);
	  Ciso.clear();
	  Chad1.clear();
	  Chad2.clear();
	  Ciso.push_back(wp_loose.iso_fw());
	  Ciso.push_back(wp_medium.iso_fw());
	  Ciso.push_back(wp_tight.iso_fw());
	  Chad1.push_back(wp_loose.frac_fw());
	  Chad1.push_back(wp_medium.frac_fw());
	  Chad1.push_back(wp_tight.frac_fw());
	  Chad2.push_back(wp_loose.frac2_fw());
	  Chad2.push_back(wp_medium.frac2_fw());
	  Chad2.push_back(wp_tight.frac2_fw());
	  int Cval[9] = {Ciso[0], Ciso[1], Ciso[2], Chad1[0], Chad1[1], Chad1[2], Chad2[0], Chad2[1], Chad2[2]};

	  elCluster.setup(Cval,jFEXETResolution);
    elCluster.calcFwdElEDM();
          
	  uint etEM = elCluster.getEt();
	  uint32_t FwdEl_tobword = elCluster.getTobWord();
    
          
	  std::vector<uint32_t> FwdEltob_aux{FwdEl_tobword,TTID};
	  if ( FwdEl_tobword != 0  && etEM>minEtThreshold) m_FwdEl_tobwords.push_back(FwdEltob_aux);
	}
    
        //******************************** TAU **********************************************
        int jTowersIDs      [FEXAlgoSpaceDefs::jFEX_algoSpace_height][FEXAlgoSpaceDefs::jFEX_thin_algoSpace_width] = {{0}};
        int max_meta=17;
        
        if(m_jfexid ==0) {
            for(int i=0; i<FEXAlgoSpaceDefs::jFEX_algoSpace_height; i++) {
                for(int j=28; j<(FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width-6); j++) { //lower values of j (j<28) are the Fcals not entering in the jFEX tau range
                    jTowersIDs[i][j-28+8]=m_jTowersIDs_Wide[i][j]; // second argument in m_jTowersIDs is to center the FPGA core area in te same region as the central FPGAs
                }
            }         
            
        }
        else if(m_jfexid ==5 ) {

            // Filling m_jTowersIDs with the m_jTowersIDs_Wide ID values up to 2.5 eta
            for(int i=0; i<FEXAlgoSpaceDefs::jFEX_algoSpace_height; i++) {
                for(int j=4; j<17; j++) { //higher values of j (j>16) are the Fcals not entering in the jFEX tau range
                    jTowersIDs[i][j]=m_jTowersIDs_Wide[i][j];
                }
            }
        }
        ATH_MSG_DEBUG("============================ jFEXtauAlgo ============================");
        ATH_CHECK( m_jFEXtauAlgoTool->safetyTest());
        m_jFEXtauAlgoTool->setFPGAEnergy(m_map_Etvalues_FPGA);
        for(int mphi = 8; mphi < 24; mphi++) {
            for(int meta = 8; meta < max_meta; meta++) {

                bool is_tau_LocalMax = m_jFEXtauAlgoTool->isSeedLocalMaxima_fwd(jTowersIDs[mphi][meta]);
        
                // Save TOB is tau is a local maxima
                if ( is_tau_LocalMax ) { 
                    
                    uint32_t jTau_tobword = m_IjFEXFormTOBsTool->formTauTOB(m_jfexid,mphi,meta,m_jFEXtauAlgoTool->getClusterEt(),m_jFEXtauAlgoTool->getFirstEtRing(),m_jFEXtauAlgoTool->getTauSat(),thr_jTAU.resolutionMeV(),thr_jTAU.ptMinToTopoMeV(m_jfex_string[m_jfexid]));
                    
                    std::unique_ptr<jFEXTOB> jTau_tob = std::make_unique<jFEXTOB>();
                    jTau_tob->initialize(m_id,m_jfexid,jTau_tobword,thr_jTAU.resolutionMeV(),jTowersIDs[mphi][meta]);
                    if ( jTau_tobword != 0 ){
                        m_tau_tobwords.push_back(std::move(jTau_tob));  
                    } 
                }
            }
        }
    } //end of if statement for checking if in central jfex modules
    return StatusCode::SUCCESS;
} //end of the execute function

void jFEXFPGA::SetTowersAndCells_SG(int tmp_jTowersIDs_subset[][FEXAlgoSpaceDefs::jFEX_wide_algoSpace_width]){
    
  const int rows = FEXAlgoSpaceDefs::jFEX_algoSpace_height;
  const int cols = sizeof tmp_jTowersIDs_subset[0] / sizeof tmp_jTowersIDs_subset[0][0];
  
  std::copy(&tmp_jTowersIDs_subset[0][0], &tmp_jTowersIDs_subset[0][0]+(rows*cols),&m_jTowersIDs_Wide[0][0]);

  ATH_MSG_DEBUG("\n==== jFEXFPGA ========= FPGA (" << m_id << ") [on jFEX " << m_jfexid << "] IS RESPONSIBLE FOR jTOWERS :");

  for (int thisRow=rows-1; thisRow>=0; thisRow--){
    for (int thisCol=0; thisCol<cols; thisCol++){
      if(thisCol != cols-1){ ATH_MSG_DEBUG("|  " << m_jTowersIDs_Wide[thisRow][thisCol] << "  "); }
      else { ATH_MSG_DEBUG("|  " << m_jTowersIDs_Wide[thisRow][thisCol] << "  |"); }
    }
  }
  
}

void jFEXFPGA::SetTowersAndCells_SG(int tmp_jTowersIDs_subset[][FEXAlgoSpaceDefs::jFEX_thin_algoSpace_width]) {

    const int rows = FEXAlgoSpaceDefs::jFEX_algoSpace_height;
    const int cols = sizeof tmp_jTowersIDs_subset[0] / sizeof tmp_jTowersIDs_subset[0][0];

    std::copy(&tmp_jTowersIDs_subset[0][0], &tmp_jTowersIDs_subset[0][0]+(rows*cols),&m_jTowersIDs_Thin[0][0]);

    //this prints out the jTower IDs that each FPGA is responsible for
    ATH_MSG_DEBUG("\n==== jFEXFPGA ========= FPGA (" << m_id << ") [on jFEX " << m_jfexid << "] IS RESPONSIBLE FOR jTOWERS :");

    for (int thisRow=rows-1; thisRow>=0; thisRow--) {
        for (int thisCol=0; thisCol<cols; thisCol++) {
            if(thisCol != cols-1) {
                ATH_MSG_DEBUG("|  " << m_jTowersIDs_Thin[thisRow][thisCol] << "  ");
            }
            else {
                ATH_MSG_DEBUG("|  " << m_jTowersIDs_Thin[thisRow][thisCol] << "  |");
            }
        }
    }

}

std::vector <std::unique_ptr<jFEXTOB>> jFEXFPGA::getSmallRJetTOBs()
{
        
    std::vector<std::unique_ptr<jFEXTOB>> tobsSort;
    tobsSort.clear();
    
    // We need the copy since we cannot move a member of the class, since it will not be part of it anymore
    for(auto &j : m_SRJet_tobwords) {
        tobsSort.push_back(std::move(j));
    }
    std::sort (tobsSort.begin(), tobsSort.end(), std::bind(TOBetSort<std::unique_ptr<jFEXTOB>>, std::placeholders::_1, std::placeholders::_2, FEXAlgoSpaceDefs::jJ_etBit, 0x7ff));
    
    return tobsSort;    

}

std::vector <std::unique_ptr<jFEXTOB>> jFEXFPGA::getLargeRJetTOBs()
{
    
        
    std::vector<std::unique_ptr<jFEXTOB>> tobsSort;
    tobsSort.clear();
    
    // We need the copy since we cannot move a member of the class, since it will not be part of it anymore
    for(auto &j : m_LRJet_tobwords) {
        tobsSort.push_back(std::move(j));
    }
    std::sort (tobsSort.begin(), tobsSort.end(), std::bind(TOBetSort<std::unique_ptr<jFEXTOB>>, std::placeholders::_1, std::placeholders::_2, FEXAlgoSpaceDefs::jLJ_etBit, 0x1fff));
    
    return tobsSort;    

}

  std::vector <std::vector <uint32_t>> jFEXFPGA::getFwdElTOBs()
  {
    auto tobsSort = m_FwdEl_tobwords;

    ATH_MSG_DEBUG("number of Forward Elec tobs: " << tobsSort.size() << " in FPGA: " << m_id<< " before truncation");
    //sort tobs by their et 
    std::sort (tobsSort.begin(), tobsSort.end(), etFwdElSort);
  
    return tobsSort;

  }


std::vector <std::unique_ptr<jFEXTOB>> jFEXFPGA::getTauTOBs() {
    
    std::vector<std::unique_ptr<jFEXTOB>> tobsSort;
    tobsSort.clear();
    
    // We need the copy since we cannot move a member of the class, since it will not be part of it anymore
    for(auto &j : m_tau_tobwords) {
        tobsSort.push_back(std::move(j));
    }
    std::sort (tobsSort.begin(), tobsSort.end(), std::bind(TOBetSort<std::unique_ptr<jFEXTOB>>, std::placeholders::_1, std::placeholders::_2, FEXAlgoSpaceDefs::jTau_etBit, 0x7ff));
    
    return tobsSort;
}

std::vector<std::unique_ptr<jFEXTOB>> jFEXFPGA::getSumEtTOBs() {
    
    std::vector<std::unique_ptr<jFEXTOB>> tobsSort;
    tobsSort.clear();
    
    // We need the copy since we cannot move a member of the class, since it will not be part of it anymore
    for(auto &j : m_sumET_tobwords) {
        tobsSort.push_back(std::move(j));
    }
    
    return tobsSort;    

}



std::vector<std::unique_ptr<jFEXTOB>> jFEXFPGA::getMetTOBs() {
    
    std::vector<std::unique_ptr<jFEXTOB>> tobsSort;
    tobsSort.clear();
    
    // We need the copy since we cannot move a member of the class, since it will not be part of it anymore
    for(auto &j : m_Met_tobwords) {
        tobsSort.push_back(std::move(j));
    }
    
    return tobsSort;    

}


//Returns the Electromagnetic energy for Jet Algos (NOT MET/SumET)
int jFEXFPGA::getTTowerET_EM(unsigned int TTID) {
    
    if(m_map_EM_Etvalues_FPGA.find(TTID) != m_map_EM_Etvalues_FPGA.end()){
        return m_map_EM_Etvalues_FPGA[TTID][0];
    }
    
    ATH_MSG_DEBUG("In jFEXFPGA::getTTowerET_EM, TTower ID not found in map: " << TTID );
    return -99999;
    
}


//Returns the Hadronic energy for Jet Algos (NOT MET/SumET)
int jFEXFPGA::getTTowerET_HAD(unsigned int TTID) {
    
    if(m_map_HAD_Etvalues_FPGA.find(TTID) != m_map_HAD_Etvalues_FPGA.end()){
        return m_map_HAD_Etvalues_FPGA[TTID][0];
    }
    
    ATH_MSG_DEBUG("In jFEXFPGA::getTTowerET_HAD, TTower ID not found in map: " << TTID );
    return -99999;
    
}


//Returns the Total TT energy for Jet Algos (NOT MET/SumET)
int jFEXFPGA::getTTowerET(unsigned int TTID) {

    return getTTowerET_EM(TTID)+getTTowerET_HAD(TTID);

}  


//Returns the Total TT energy for MET/SumÉT Algos
int jFEXFPGA::getTTowerET_forMET(unsigned int TTID) {

    int tmp_EM = 0;
    if(m_map_EM_Etvalues_FPGA.find(TTID) != m_map_EM_Etvalues_FPGA.end()){
        tmp_EM = m_map_EM_Etvalues_FPGA[TTID][1];
    }
    else{
        ATH_MSG_DEBUG("In jFEXFPGA::getTTowerET_forMET (EM energy), TTower ID not found in map: " << TTID );
        tmp_EM = -99999;
    }


    int tmp_HAD = 0;
    if(m_map_HAD_Etvalues_FPGA.find(TTID) != m_map_HAD_Etvalues_FPGA.end()){
        tmp_HAD = m_map_HAD_Etvalues_FPGA[TTID][1];
    }
    else{
        ATH_MSG_DEBUG("In jFEXFPGA::getTTowerET_forMET (HAD energy), TTower ID not found in map: " << TTID );
        tmp_HAD = -99999;
    }
    
    
    return tmp_EM + tmp_HAD;

}  


//Returns de ET of a given TT ID for Algorithm
int jFEXFPGA::getTTowerET_SG(unsigned int TTID) {
    
    if(TTID == 0){
        return -999;
    }
    SG::ReadHandle<jTowerContainer> jTowerContainer(m_jTowerContainerKey);
    const LVL1::jTower * tmpTower = jTowerContainer->findTower(TTID);
    return tmpTower->getTotalET();
}






} // end of namespace bracket
