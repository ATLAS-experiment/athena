/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "L1CaloFEXSim/FEXAlgoSpaceDefs.h"
#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jTowerBuilder.h"
#include "L1CaloFEXSim/jTowerContainer.h"

// TOWER IS A COLLECTION OF SUPER CELLS
// IT SHOULD HAVE A UNIQUE ID
// IT SHOULD BE ABLE TO RETURN LIST OF SUPER CELLS BELONGING TO IT

// THIS IS A CLASS DESIGNED TO BUILD AN JTOWER USING THE JTOWER CLASS AND THEN PRINT THE RELEVANT INFORMATION TO THE SCREEN USING FUNCTION CALLS FROM THE JTOWER CLASS

namespace LVL1 {

jTowerBuilder::jTowerBuilder(const std::string& type,const std::string& name,const IInterface* parent): AthAlgTool(type,name,parent) {
    declareInterface<IjTowerBuilder>(this);
}

StatusCode jTowerBuilder::initialize()
{
    ATH_CHECK( m_BDToolKey.initialize() );

    return StatusCode::SUCCESS;
}



void jTowerBuilder::init(std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const {

    execute(jTowerContainerRaw);
    
    jTowerContainerRaw->clearContainerMap();
    jTowerContainerRaw->fillContainerMap();

}


void jTowerBuilder::reset() const {

}


void jTowerBuilder::execute(std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const {
  BuildAllTowers(jTowerContainerRaw);
}
 
  // TOWER IDs FOR CLARITY
  // Left Barrel IETower = 100000 + X
  // Right Barrel IETower = 200000 + X
  // Left Transition ID Tower = 300000 + X;
  // Right Transition ID Tower = 400000 + X;
  // Left Endcap ID Tower = 500000 + X
  // Right Endcap ID Tower = 600000 + X
  // Left Hadronic Endcap ID Tower = 11100000 + X --> These are just Layer 5 of Endcap Towers.  They will never be generated as standalone jTowers.
  // Right Haronic Endcap ID Tower = 22200000 + X --> These are just Layer 5 of Endcap Towers.  They will never be generated as standalone jTowers.

void jTowerBuilder::BuildEMBjTowers(std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const {
    // Regions 0 only.  Region 1 is 'transition region'.
    for (int ieta = 0; ieta < 14; ++ieta) { // loop over 14 eta steps (ignoring last step as it is transition region)
        float centre_eta = (0.1*ieta) + (0.05) ;
        for (int iphi = 0; iphi < 64; ++iphi) { // loop over 64 phi steps
            int key_eta = ieta;
            float centre_phi = (m_TT_Size_phi*iphi) + (m_TT_Size_phi/2);
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 100000, -1, -1*centre_eta, centre_phi);
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 200000,  1,    centre_eta, centre_phi);
        }
    }

}

void jTowerBuilder::BuildTRANSjTowers(std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const {
    int TRANS_MODIFIER = 14;
    int tmpVal = TRANS_MODIFIER;

    for (int ieta = tmpVal; ieta < tmpVal + 1; ieta++) { // loop over eta steps
        float centre_eta = (0.1*ieta) + (0.05);
        for (int iphi = 0; iphi < 64; ++iphi) { // loop over 64 phi steps
            int key_eta = ieta;
            float centre_phi = (m_TT_Size_phi*iphi) + (m_TT_Size_phi/2);
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 300000, -1,-1*centre_eta, centre_phi);
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 400000,  1,   centre_eta, centre_phi);
        }
    }

}

void jTowerBuilder::BuildEMEjTowers(std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const {
    // Region 1
    int EME_MODIFIER = 15;
    int tmpVal = EME_MODIFIER;

    for (int ieta = tmpVal; ieta < tmpVal + 3; ++ieta) { // loop over eta steps
        float centre_eta =(0.1*ieta) + (0.05) ;
        for (int iphi = 0; iphi < 64; ++iphi) { // loop over 64 phi steps
            int key_eta = ieta;
            float centre_phi = (m_TT_Size_phi*iphi) + (m_TT_Size_phi/2);
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 500000, -1, -1*centre_eta, centre_phi);
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 600000,  1,    centre_eta, centre_phi);
        }
        EME_MODIFIER++;
    }

    // Region 2
    tmpVal = EME_MODIFIER;
    for (int ieta = tmpVal; ieta < tmpVal + 2; ++ieta) { // loop over eta steps
        float centre_eta = (0.1*ieta) + (0.05);
        for (int iphi = 0; iphi < 64; ++iphi) { // loop over 64 phi steps
            int key_eta = ieta;
            float centre_phi = (m_TT_Size_phi*iphi) + (m_TT_Size_phi/2);
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 500000, -1,-1*centre_eta, centre_phi);
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 600000,  1,   centre_eta, centre_phi);
        }
        EME_MODIFIER++;
    }

    // Region 3
    tmpVal = EME_MODIFIER;
    for (int ieta = tmpVal; ieta < tmpVal + 4; ++ieta) { // loop over eta steps
        float centre_eta= (0.1*ieta) + (0.05) ;
        for (int iphi = 0; iphi < 64; ++iphi) { // loop over 64 phi steps
            int key_eta = ieta;
            float centre_phi = (m_TT_Size_phi*iphi) + (m_TT_Size_phi/2);
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 500000, -1,-1*centre_eta, centre_phi);
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 600000,  1,   centre_eta, centre_phi);
        }
        EME_MODIFIER++;
    }

    // Region 4
    tmpVal = EME_MODIFIER;
    for (int ieta = tmpVal; ieta < tmpVal + 1; ++ieta) { // loop over eta steps
        float centre_eta = (0.1*ieta) + (0.05);
        for (int iphi = 0; iphi < 64; ++iphi) { // loop over 64 phi steps
            int key_eta = ieta;
            //float centre_phi =(TT_Size*iphi) + (0.5*TT_Size) ;
            float centre_phi = (m_TT_Size_phi*iphi) + (m_TT_Size_phi/2);
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 500000, -1, -1*centre_eta, centre_phi);
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 600000,  1,    centre_eta, centre_phi);
        }
        EME_MODIFIER++;
    }


}

  // EMIE = Electromagnetic Inner ECAL - i.e. the forward ECAL region at high eta
void jTowerBuilder::BuildEMIEjTowers(std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const {
    int EMIE_MODIFIER = 25;
    int tmpVal = EMIE_MODIFIER;
    int cellCountEta = 0;

    for (int ieta = tmpVal; ieta < tmpVal + 3; ++ieta) { // loop over eta steps (there are 3 here, 2.5-2.7, 2.7-2.9, 2.9-3.1)
        cellCountEta++;
        float centre_eta =(0.1*ieta) + (0.1*cellCountEta) ;
        for (int iphi = 0; iphi < 32; ++iphi) { // loop over 32 phi steps
            int key_eta = ieta;
            float centre_phi = (2*m_TT_Size_phi*iphi) + m_TT_Size_phi;
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, /*7*/500000, -1, -1*centre_eta, centre_phi);
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, /*8*/600000,  1,    centre_eta, centre_phi);
        }
        EMIE_MODIFIER++;
    }

    tmpVal = EMIE_MODIFIER;
    for (int ieta = tmpVal; ieta < tmpVal + 1; ++ieta) { // loop over eta steps (there are 1 here, 3.1-3.2)
        float centre_eta = (0.1*ieta + 0.3) + (0.05);
        for (int iphi = 0; iphi < 32; ++iphi) { // loop over 32 phi steps
            int key_eta = ieta;
            float centre_phi = (2*m_TT_Size_phi*iphi) + m_TT_Size_phi;
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, /*7*/500000, -1, -1*centre_eta, centre_phi);
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, /*8*/600000,  1,    centre_eta, centre_phi);
        }
        EMIE_MODIFIER++;
    }

}

void jTowerBuilder::BuildFCALjTowers(std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const {
    int FCAL_MODIFIER = 29; // there's 0.1 overlap with EMIE here in eta, but in counting we pretend it's the next one along.
    int tmpVal = FCAL_MODIFIER;

    //These jTowers split between all of the layers as well (FCAL0,1,2) but we treat them as though they are a single flat layer of 24 supercells and also pretend that they do not overlap - when they definitely do...
    //This means that from a tower numbering perspective we start with FCAL0 and work upwards (in numbers?), but real ordering in eta is different and this has to be built into the jTower internal properties
    //Right now we are unfortunately using hard-coded eta and phi positions to do this, but maybe these should be drawn from a database someday

    // THIS REGION DEFINES 1 jTOWER PER SUPERCELL! AS SUCH THE jTOWER AND SUPERCELL POSITIONS IN ETA-PHI SPACE WILL MATCH EXACTY
    // (That's good right? Supposed to make life easier?)

    // 21/01/21 IN THE MC:
    // FCAL 0 Region [NOT SPECIFIED IN MC] has 12 supercells in 3.2 < eta < 4.88, and 16 supercells in phi.  Supercells are 0.14 x 0.4. posneg +-2
    // FCAL 1 Region [NOT SPECIFIED IN MC] has 8 supercells in 3.2 < eta < 4.48, and 16 supercells in phi.  Supercells are 0.16 x 0.4. posneg +-2
    // FCAL 2 Region [NOT SPECIFIED IN MC] has 4 supercells in 3.2 < eta < 4.48, and 16 supercells in phi.  Supercells are 0.32 x 0.4. posneg +-2

    //FCAL0
    float eta_width = 1.4;
    int cellCountEta = 0;
    int FCAL0_INITIAL = FCAL_MODIFIER;
    std::vector<int> TT_etapos{31,33,34,36,37,39,40,42,43,45,46,48}; // Eta position of each supercell, need to be change for the real coords. Future MR
    for (int ieta = tmpVal; ieta < tmpVal + 12; ++ieta) { // loop over eta steps (there are 12 here in varying positions for FCAL0)
        int key_eta = ieta - FCAL0_INITIAL;
        float centre_eta = (TT_etapos[cellCountEta]+eta_width/2)/10.0;
        cellCountEta++;

        for (int iphi = 0; iphi < 16; ++iphi) { // loop over 16 phi steps
            float centre_phi = (2*m_TT_Size_phi_FCAL*iphi) + m_TT_Size_phi_FCAL;
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 700000, -1, -1*centre_eta, centre_phi, 0);
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 800000,  1,    centre_eta, centre_phi, 0);
        }
        FCAL_MODIFIER++;
    }

    //FCAL1
    eta_width = 1.6;
    cellCountEta = 0;
    tmpVal = FCAL_MODIFIER;
    TT_etapos = {31,33,35,37,39,41,43,44};// Eta position of each supercell, need to be change for the real coords. Future MR
    int FCAL1_INITIAL = FCAL_MODIFIER;
    for (int ieta = tmpVal; ieta < tmpVal + 8; ++ieta) { // loop over eta steps (there are 8 here in varying positions for FCAL1)
        int key_eta = ieta - FCAL1_INITIAL;
        float centre_eta = (TT_etapos[cellCountEta]+eta_width/2)/10.0;
        cellCountEta++;
        for (int iphi = 0; iphi < 16; ++iphi) { // loop over 16 phi steps
            float centre_phi = (2*m_TT_Size_phi_FCAL*iphi) + m_TT_Size_phi_FCAL;
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta,  900000, -1, -1*centre_eta, centre_phi, 1);
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 1000000,  1,    centre_eta, centre_phi, 1);
        }
        FCAL_MODIFIER++;
    }


    //FCAL2
    eta_width = 3.2;
    cellCountEta = 0;
    tmpVal = FCAL_MODIFIER;
    TT_etapos = {31,34,37,41};// Eta position of each supercell, need to be change for the real coords. Future MR
    int FCAL2_INITIAL = FCAL_MODIFIER;
    for (int ieta = tmpVal; ieta < tmpVal + 4; ++ieta) { // loop over eta steps (there are 4 here in varying positions for FCAL2)
        int key_eta = ieta - FCAL2_INITIAL;
        float centre_eta = (TT_etapos[cellCountEta]+eta_width/2)/10.0;
        cellCountEta++;
        for (int iphi = 0; iphi < 16; ++iphi) { // loop over 16 phi steps
            float centre_phi = (2*m_TT_Size_phi_FCAL*iphi) + m_TT_Size_phi_FCAL;
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 1100000, -1, -1*centre_eta, centre_phi, 2);
            BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 1200000,  1,    centre_eta, centre_phi, 2);
        }
        FCAL_MODIFIER++;
    }


}



  void jTowerBuilder::BuildHECjTowers(std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const {
  // Region 0
  int HEC_MODIFIER = 29;
  int tmpVal = HEC_MODIFIER;
  for (int ieta = tmpVal; ieta < tmpVal + 10; ++ieta){ // loop over eta steps
    for (int iphi = 0; iphi < 64; ++iphi){ // loop over 64 phi steps
      int key_eta = ieta;
      BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 11100000, -1, ieta, iphi);
      BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 22200000, 1, ieta, iphi);
    }
    HEC_MODIFIER++;
  }

  // Region 1
  tmpVal = HEC_MODIFIER;
  for (int ieta = tmpVal; ieta < tmpVal + 4; ++ieta){ // loop over eta steps
    for (int iphi = 0; iphi < 32; ++iphi){ // loop over 64 phi steps
      int key_eta = ieta;
      BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 11100000, -1, ieta, iphi);
      BuildSingleTower(jTowerContainerRaw, ieta, iphi, key_eta, 22200000, 1, ieta, iphi);
    }
    HEC_MODIFIER++;
  }

}
//=================================================================================================================================================================

void jTowerBuilder::BuildSingleTower(std::unique_ptr<jTowerContainer> & jTowerContainerRaw,float eta, float phi, int key_eta, float keybase, int posneg, float centre_eta, float centre_phi, int fcal_layer) const {
    int towerID = keybase + phi + (64 * key_eta);
    jTowerContainerRaw->push_back(eta, phi, towerID, posneg, centre_eta, centre_phi, fcal_layer);

}



StatusCode jTowerBuilder::AssignPileupAndNoiseValues(std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const{
    
    SG::ReadCondHandle<jFEXDBCondData> myDBTool = SG::ReadCondHandle<jFEXDBCondData>( m_BDToolKey/*, ctx*/ );
    if (!myDBTool.isValid()){
        ATH_MSG_ERROR("Not able to read " << m_BDToolKey );
        return StatusCode::FAILURE;
    }

    for(const auto & jtower : *jTowerContainerRaw) {
        
        auto [CutJetEM, CutJetHad, CutMetEM, CutMetHad] = myDBTool->get_NoiseCuts( jtower->OnlineID() );
        auto [PileUpWeightEM, PileUpWeightHad, InverseWeightEM, InverseWeightHad] = myDBTool->get_PileUpValues( jtower->OnlineID() );
        
         //Simulation used MeV not counts, those 
        int LSBscale_EM  = 25; // cf LATOME
        int LSBscale_HAD = 25; // cf LATOME
        
         //TREX uses another conversion factor
        if(std::abs(jtower->centreEta()) < 1.5){
            LSBscale_HAD = 500;// cf TREX
        }
        
        //Since the COOL DB for FCAL individual towers are sharing the same OnlideID ( to save space)
        //but in reality they are different towers. we need to set the parameters to 0
        if(jtower->OfflineID() >= FEXAlgoSpaceDefs::jFEX_FCAL2_start){
            PileUpWeightEM  = 0;
            InverseWeightEM = 0;
        }
        else if(jtower->OfflineID() >= FEXAlgoSpaceDefs::jFEX_FCAL1_start){
            PileUpWeightHad  = 0;
            InverseWeightHad = 0;
        }
        
        jtower->setTTowerArea(PileUpWeightEM,0);
        jtower->setTTowerArea(PileUpWeightHad,1);
        
        jtower->setTTowerAreaInv(InverseWeightEM,0);
        jtower->setTTowerAreaInv(InverseWeightHad,1);

        jtower->setNoiseForMet(CutMetEM*LSBscale_EM,0);
        jtower->setNoiseForMet(CutMetHad*LSBscale_HAD,1);
        
        jtower->setNoiseForJet(CutJetEM*LSBscale_EM,0);
        jtower->setNoiseForJet(CutJetHad*LSBscale_HAD,1);
        
    }
    return StatusCode::SUCCESS;
}


void jTowerBuilder::BuildAllTowers(std::unique_ptr<jTowerContainer> & jTowerContainerRaw) const {
    BuildEMBjTowers(jTowerContainerRaw);
    BuildTRANSjTowers(jTowerContainerRaw);
    BuildEMEjTowers(jTowerContainerRaw);
    BuildEMIEjTowers(jTowerContainerRaw);
    BuildFCALjTowers(jTowerContainerRaw);
}

} // end of LVL1 namespace

