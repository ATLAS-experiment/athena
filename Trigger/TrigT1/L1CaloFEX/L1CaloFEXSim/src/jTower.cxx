/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//***************************************************************************
//                           jTower.h  -  description
//                              -------------------
//     begin                : 19 02 2019
//     email                : Alan.Watson@cern.ch, jacob.julian.kempster@cern.ch
//  ***************************************************************************/

#include "L1CaloFEXSim/jTower.h"
#include "L1CaloFEXSim/jFEXCompression.h"
#include <cmath>


namespace LVL1 {

  const int s_nLayers = 2;
  const int s_cells[s_nLayers]   = {1,1};
  const int s_offsets[s_nLayers] = {0,1};
  
  // default constructor
  jTower::jTower():
    m_iEta(0),
    m_iPhi(0),
    m_tower_id(-9999999),
    m_tower_id_online(0xffff),
    m_posneg(0),
    m_centre_eta(0.),
    m_centre_phi(0.),
    m_fcal_layer(-1)
  {
    this->clearET();
  }
  
  /** constructs a tower and sets the coordinates and identifier */
  jTower::jTower(int ieta, int iphi, int towerid, int posneg, float centre_eta, float centre_phi, int fcal_layer):
    m_iEta(ieta),
    m_iPhi(iphi),
    m_tower_id(towerid),
    m_tower_id_online(-9999999),
    m_posneg(posneg),
    m_centre_eta(centre_eta),
    m_centre_phi(centre_phi),
    m_fcal_layer(fcal_layer)
    {   
        m_centre_phi_toPI = centre_phi;
        if(centre_phi>M_PI) m_centre_phi_toPI = centre_phi-2*M_PI;
        this->clearET();
    }
  
  
  /** Clear and resize ET value vector */
  void jTower::clearET()
  {
    m_et.clear();
    m_et.resize(2,0);
    m_et_float_raw.clear();
    m_et_float_raw.resize(2,0.0);
  }

  /** Clear and resize EM SC Identifier value vector */
  void jTower::clear_EM_scIDs()
  {
    m_EM_scID.clear();
  }

  /** Clear and resize HAD SC Identifier value vector */
  void jTower::clear_HAD_scIDs()
  {
    m_HAD_scID.clear();
  }


void jTower::setPosNeg(int posneg) {

    m_posneg = posneg;

    return;

}

/** Add ET to a specified cell */
void jTower::addET(float et, int layer)
{
    if (layer < 0  || layer >= s_nLayers) {
        std::stringstream errMsg;
        errMsg << "addET: Attempt to set an invalid JTower layer with value: " << layer << ". Must be 0 (EM) or 1 (HAD) ";
        throw std::runtime_error(errMsg.str().c_str());
        return; //need to throw an error really...
    }

    m_et_float_raw[layer] += et;

    return;

}

void jTower::set_Et(int layer, int et){
    m_et[layer] = et;
}


void jTower::set_TileCal_Et(int layer, int et) {

    if (layer < 0  || layer >= s_nLayers) {
        std::stringstream errMsg;
        errMsg << "set_TileCal_Et: Attempt to set an invalid JTower layer with value: " << layer << ". Must be 0 (EM) or 1 (HAD) ";
        throw std::runtime_error(errMsg.str().c_str());
        return; //need to throw an error really...
    }

    addET(et, layer);
    
    set_Et(layer, et);
    return;

}

/** Set supercell position ID and ET**/
void jTower::set_LAr_Et(Identifier ID, int cell, float et, int layer)
{

    if((layer < 0) || (layer >= s_nLayers)) {
        std::stringstream errMsg;
        errMsg << "Attempt to set jTower SCID in invalid layer (" << layer << ")";
        throw std::runtime_error(errMsg.str().c_str());
        return; //need to throw an error really...
    }

    /// Check cell index in range for layer
    if (cell < 0  || cell > 2) {
        std::stringstream errMsg;
        errMsg << "Attempt to set jTower SCID in invalid cell slot (" << cell << ")";
        throw std::runtime_error(errMsg.str().c_str());
        return;
    }
    
    addET(et, cell);

    if(layer == 0) {
        m_EM_scID.push_back(ID);
    }
    else if(layer == 1) {
        m_HAD_scID.push_back(ID);
    }

    return;

}

//** Used to Compress and Expand the LATOME energies accordingly to the scheme
void jTower::Do_LAr_encoding(){
    
    //multi linear digitisation encoding
    for(uint layer=0; layer<m_et_float_raw.size(); layer++){
        unsigned int ecode = jFEXCompression::Compress( m_et_float_raw[layer] ); 
        int outET = jFEXCompression::Expand(ecode); 
        set_Et(layer,outET);
    }
}


/** Return global eta index. */
int jTower::iEta() const {
    return m_iEta;
}

/** Return global phi index.*/
int jTower::iPhi() const {
    return m_iPhi;
}

/** Return ET of specified supercell */
int jTower::getET(unsigned int layer,  int cell) const {

    /// Check cell index in range for layer
    if (layer >= s_nLayers || cell < 0 || cell >= s_cells[layer]) return 0;

    // Return ET
    return m_et[s_offsets[layer] + cell];

}

/** Return ET of specified supercell FLOAT VERSION */
float jTower::getET_float(unsigned int layer, int cell) const {

    /// Check cell index in range for layer
    if (layer >= s_nLayers || cell < 0 || cell >= s_cells[layer]) return 0;

    // Return ET
    return m_et_float_raw[s_offsets[layer] + cell];

}

/** Return ET of all supercells together*/
int jTower::getTotalET() const {

    int tmp = 0;
    for (unsigned int i=0; i<m_et.size(); i++) {
        tmp += m_et[i];
    }

    return tmp;

}

/** Return ET of all supercells together FLOAT VERSION */
float jTower::getTotalET_float() const {

    float tmp = 0;
    for (unsigned int i=0; i<m_et_float_raw.size(); i++) {
        tmp += m_et_float_raw[i];
    }

    return tmp;

}


/** Return supercell ET values for specified layer */
std::vector<int> jTower::getLayerETvec(unsigned int layer) const {

    /// Create empty vector of data
    std::vector<int> cells;

    /// Check cell index in range for layer
    if (layer >= s_nLayers) return cells;

    /// Fill output vector
    for (int cell = 0; cell < s_cells[layer]; ++cell) cells.push_back(m_et[s_offsets[layer] + cell]);

    return cells;
}


/** Return supercell ET values for specified layer FLOAT VERSION */
std::vector<float> jTower::getLayerETvec_float(unsigned int layer) const {

    /// Create empty vector of data
    std::vector<float> cells;

    /// Check cell index in range for layer
    if (layer >= s_nLayers) return cells;

    /// Fill output vector
    for (int cell = 0; cell < s_cells[layer]; ++cell) cells.push_back(m_et_float_raw[s_offsets[layer] + cell]);

    return cells;
}


/** Return supercell ET values for specified layer */
int jTower::getLayerTotalET(unsigned int layer) const {

    if (layer == 0) {
        return m_et[0];
    }
    else if (layer == 1) {
        return (m_et[1]);
    }

    return 0;

}

/** Return supercell ET values for specified layer FLOAT VERSION */
float jTower::getLayerTotalET_float(unsigned int layer) const {

    if (layer == 0) {
        return m_et_float_raw[0];
    }
    else if (layer == 1) {
        return (m_et_float_raw[1]);
    }

    return 0;

}

std::vector<Identifier> jTower::getLayerSCIDs(unsigned int layer) const {

    if (layer == 0) {
        return m_EM_scID;
    }
    else if (layer == 1) {
        return m_HAD_scID;
    }

    return std::vector<Identifier>();

}

void jTower::setCentreEta(float eta){
    m_centre_eta = eta;
}

void jTower::setiEta(int eta){
    m_iEta = eta;
}

void jTower::setCentrePhi(float phi){
    m_centre_phi_toPI = phi;
    m_centre_phi = phi;
    if(m_centre_phi<0) m_centre_phi = 2*M_PI+phi;  
} 
     
void jTower::setiPhi(int phi){
    m_iPhi = phi;
}

void jTower::setOnlineID(int tower_id_online){
    m_tower_id_online = tower_id_online;
}

void jTower::setTTowerArea(int area,int layer){
    m_TTowerArea.at(layer)=area;
}

int jTower::getTTowerArea(int layer)const{
    return m_TTowerArea.at(layer);
}

void jTower::setTTowerAreaInv(int area,int layer){
    m_TTowerAreaInv.at(layer)=area;
}

int jTower::getTTowerAreaInv(int layer)const{
    return m_TTowerAreaInv.at(layer);
}

void jTower::setNoiseForMet(int noiseVal,int layer){
    m_NoiseForMet[layer]=noiseVal;
}


int jTower::getNoiseForMet(int layer) const{
    return m_NoiseForMet[layer];
}

void jTower::setNoiseForJet(int noiseVal,int layer){
    m_NoiseForJet[layer]=noiseVal;
}


int jTower::getNoiseForJet(int layer) const{
    return m_NoiseForJet[layer];
}

} // end of namespace bracket

