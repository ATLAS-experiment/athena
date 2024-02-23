/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
//***************************************************************************
//    gTower - Defines all properties and methods for the gFEX towers
//                              -------------------
//     begin                : 01 04 2021
//     email                : cecilia.tosciri@cern.ch
//***************************************************************************

#include "L1CaloFEXSim/gTower.h"
#include "L1CaloFEXSim/gFEXCompression.h"


namespace LVL1 {

  // default constructors
  gTower::gTower():
    m_eta(0),
    m_phi(0),
    m_tower_id(-9999999),
    m_posneg(0)
  {
    this->clear_scIDs();
    this->clearET();
  }

  /** constructs a tower and sets the coordinates and identifier */
  gTower::gTower(int ieta, int iphi, int nphi, int id_modifier, int posneg):
    m_eta(ieta),
    m_phi(iphi),
    m_tower_id(id_modifier + iphi + (nphi * ieta)),
    m_posneg(posneg)
  {
    this->clear_scIDs();
    this->clearET();
    getEtaPhi(m_eta_float, m_phi_float, iEta(), iPhi());
  }

  /** Clear and resize ET value vector */
  void gTower::clearET()
  {
    m_et = 0;
    m_et_float = 0.0;
    m_et_float_perlayer.assign(2, 0.0);
  }

  /** Clear and resize Identifier value vector */
  void gTower::clear_scIDs()
  {
    m_scID.clear();
  }

  void gTower::setPosNeg(int posneg){

    m_posneg = posneg;

    return;

  }


  /** Add ET */
  void gTower::addET(float et, int layer)
  {

    m_et_float_perlayer[layer] += et; // for monitoring
    m_et_float += et;

    return;

  }

  void gTower::setET()
  {

    // addET(et, layer);

    //multi linear digitisation encoding
    unsigned int gcode = gFEXCompression::compress(m_et_float_perlayer[0]);//Only decode EM energy
    int emET = gFEXCompression::expand(gcode);
    int outET = emET + m_et_float_perlayer[1];//Sum EM and HAD energy 

    outET = outET/200.;//Convert to gFEX digit scale (200 MeV tbc)
    
    //noise cut
    const bool SCpass = noiseCut(outET);
    if (SCpass){ m_et = outET; }
    else{ m_et = 0; }
  }

  void gTower::setTotalEt(int totEt) 
  {

    m_et = totEt;

    return;
  }

  /** Set supercell position ID **/
  void gTower::setSCID(Identifier ID)
  {

    m_scID.push_back(ID);

    return;

  }

  /** Apply noise cut per layer **/
  bool gTower::noiseCut(int et) const
  {

    bool pass = true;

    if(et < m_noisecut){ pass = false; }

    return pass;

  }

  /** Return unique identifier */
  int gTower::getID() const {
    return m_tower_id;
  }

  // Return global eta index.
  int gTower::iEta() const {
    int index = (m_eta * m_posneg);
    if (m_posneg < 0){
      index = index + 19;
    }
    else if ((m_posneg > 0)){
      index = index + 20;
    }
    
    return index;
  }

  // Return global phi index.
  int gTower::iPhi() const {
    return m_phi;
  }

  /** Return ET (total) */
  int gTower::getET() const {

    return m_et;

  }

  /** Return ET (total) FLOAT VERSION */
  float gTower::getET_float() const {

    // Return ET
    return m_et_float;

  }

  /** Return ET for EM */
  int gTower::getET_EM_float() const {

    return m_et_float_perlayer[0];

  }

  /** Return ET for HAD */
  int gTower::getET_HAD_float() const {

    return m_et_float_perlayer[1];

  }

  int gTower::getFWID() const {
    int iPhiFW, iEtaFW;
    return getFWID(iPhiFW, iEtaFW);
  }


  /** Return the firmware ID from the software ID */
  //This is about assigning a unique ID to the gTowers, that reflects as much as possible 
  //the tower identification in firmware.
  //Some descriptions of this can be found in https://its.cern.ch/jira/browse/ATLGFEX-95.
  //Since the indices used in firmware are the same for each FPGA (0-383), here we add a prefix 
  //for FPGA 1 (which corresponds to FPGA-B) and for FPGA 2 (FPGA-C) of 10000 and 20000, respectively, 
  //for differentiating the FPGAs. So we have 0-383 for FPGA 1 (FPGA-A), 10000-10383 FPGA 2 (FPGA-B), 
  //and 20000-20383 FPGA 3 (FPGA-C).  
  //iEta and iPhi are global indices, with iEta in 0-39 and iPhi in 0-32, and they uniquely determine 
  //one gTower object in the simulation. We assign here a unique ID to each gTower. 
  //The hardcoded numbers come from the definition of local FPGA IDs from global eta, phi indices.  

  int gTower::getFWID(int & iPhiFW, int & iEtaFW) const {

    int gFEXtowerID; // the firmware ID to be calculated

    int iEta = this->iEta();
    int iPhi = this->iPhi();
    float Eta = this->eta();

    iPhiFW = iPhi;
    iEtaFW = iEta;

    bool is_central = true;
    if (iEta <= 7 || iEta >= 32) is_central = false;

    if (is_central)
    {

      if (iEta < 20)
      {
        // FPGA 0
        gFEXtowerID = (iEta - 8) + (iPhi * 12);
      } else
      {
        // FPGA 1
        gFEXtowerID = 10000 + (iEta - 20) + (iPhi * 12);

      }

    } else
    {
      gFEXtowerID = 20000;

      if ( Eta < 0 ){

        if ( iEta == 0 ){
          gFEXtowerID = gFEXtowerID + (iPhi*24);
          iPhiFW = iPhi*2;
          iEtaFW = 2;
        }
        else if ( iEta == 1 ){
          gFEXtowerID = gFEXtowerID + ((iPhi*24) + 12);
          iPhiFW = (iPhi*2)+1;
          iEtaFW = 2;
        }   
        else if ( iEta == 2 ){
          gFEXtowerID = gFEXtowerID + ((iPhi*24) + 1);
          iPhiFW = iPhi*2;
          iEtaFW = 3;
        } 
        else if ( iEta == 3 ){
          gFEXtowerID = gFEXtowerID + ((iPhi*24) + 13);
          iPhiFW = (iPhi*2)+1;
          iEtaFW = 3;
        }
        else if ( iEta >= 4 and iEta <= 7 ){
          gFEXtowerID = gFEXtowerID + ((iPhi*12) + (iEta -2));
        } 

      }

      else if ( Eta > 0 ){

        if ( iEta >= 32 and iEta <= 35){
          gFEXtowerID = gFEXtowerID + (iPhi*12) + (iEta -32 +6);
        }
        else if ( iEta == 36 ){
          gFEXtowerID = gFEXtowerID + ((iPhi*24) + 22);
          iPhiFW = (iPhi*2)+1;
          iEtaFW = 36;
        }   
        else if ( iEta == 37 ){
          gFEXtowerID = gFEXtowerID + ((iPhi*24) + 10);
          iPhiFW = iPhi*2;
          iEtaFW = 36;
        } 
        else if ( iEta == 38 ){
          gFEXtowerID = gFEXtowerID + ((iPhi*24) + 23);
          iPhiFW = (iPhi*2)+1;
          iEtaFW = 37;
        }
        else if ( iEta == 39 ){
          gFEXtowerID = gFEXtowerID + ((iPhi*24) + 11);
          iPhiFW = iPhi*2;
          iEtaFW = 37;
        }  
      }

    }

    return gFEXtowerID;

  }

  void gTower::getEtaPhi ( float &Eta, float &Phi, int iEta, int iPhi) const{
    
    float s_centralPhiWidth = (2*M_PI)/32; //In central region, gFex has 32 bins in phi
    float s_forwardPhiWidth = (2*M_PI)/16; //In forward region, gFex has 16 bins in phi (before rearranging bins)

    constexpr std::array<float, 40> s_EtaCenter = { -4.7, -4.2, -3.7, -3.4, -3.2, -3, 
                                                    -2.8, -2.6, -2.35, -2.1, -1.9, -1.7, -1.5, -1.3, -1.1, -0.9,  
                                                    -0.7, -0.5, -0.3, -0.1, 0.1, 0.3, 0.5, 0.7, 0.9, 1.1,                                                 
                                                    1.3, 1.5, 1.7, 1.9, 2.1, 2.35, 2.6, 2.8, 3.0,
                                                    3.2, 3.4, 3.7, 4.2, 4.7};

    Eta = s_EtaCenter.at(iEta); 

    float Phi_gFex = -99;
    if (( iEta <= 3 ) || ( (iEta >= 36) )){
      Phi_gFex = ( (iPhi * s_forwardPhiWidth) + s_forwardPhiWidth/2);
    }  
    else {
      Phi_gFex = ( (iPhi * s_centralPhiWidth) + s_centralPhiWidth/2);
    }
   
    if (Phi_gFex < M_PI) {
      Phi = Phi_gFex;
    }
    else {
      Phi = (Phi_gFex - 2*M_PI);
    }
  }


} // end of namespace bracket
