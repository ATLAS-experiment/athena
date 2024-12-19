// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#include "LArClusterCellDumper/EventReaderBaseAlg.h"

#include "xAODTracking/TrackParticlexAODHelpers.h"
#include "CLHEP/Units/PhysicalConstants.h"
#include "CLHEP/Units/SystemOfUnits.h"

#include <TTree.h>


using CLHEP::GeV;
using CLHEP::pi;
using CLHEP::twopi;

EventReaderBaseAlg::EventReaderBaseAlg( const std::string& name, ISvcLocator* pSvcLocator ) : 
    AthAlgorithm(name, pSvcLocator) {}

EventReaderBaseAlg::~EventReaderBaseAlg() {}

StatusCode EventReaderBaseAlg::initialize(){
    return StatusCode::SUCCESS;
}
StatusCode EventReaderBaseAlg::execute(){
    return StatusCode::SUCCESS;
}
StatusCode EventReaderBaseAlg::finalize(){
    return StatusCode::SUCCESS;
}

bool EventReaderBaseAlg::eOverPElectron(const xAOD::Electron* electron){
  // the E/p is calculated here based on Electron caloCluster and its associated trackParticle
  // E_cluster/p_track
  float eoverp = 0.;
  float track_p = (electron->trackParticle())->pt()*std::cosh((electron->trackParticle())->eta());
  if (track_p != 0.) eoverp = (electron->caloCluster())->e()/track_p;
  else{
    ATH_MSG_WARNING ("(eOverP) Track_p == 0");
    return false;
  }
  if ((fabs(eoverp) < 0.7) || (fabs(eoverp) > 1.5) ){
    ATH_MSG_DEBUG ("(eOverP) E/p is out of range! (eoverp="<<eoverp<<").");
    return false;
  }
  else{
    ATH_MSG_DEBUG ("(eOverP) Passed: E/p="<<eoverp);
    return true;
  }
}

// create a new electrons container with the electrons that fullfill the list of criteria below
bool EventReaderBaseAlg::trackSelectionElectrons(const xAOD::Electron *electron, SG::ReadHandle<xAOD::VertexContainer> &primVertexCnt, SG::ReadHandle<xAOD::EventInfo> &ei){
  
  const xAOD::TrackParticle *trackElectron =  electron->trackParticle();
  const xAOD::CaloCluster *elClus = electron->caloCluster();

  if (!trackElectron) { //is there track?
    ATH_MSG_DEBUG ("(trackSelectionElectrons) No track particle for Tag check."); 
    return false;
  }
  // caloCluster  
  if (!elClus){ //is there a calo cluster associated to this electron?
    ATH_MSG_DEBUG ("(trackSelectionElectrons) No caloCluster associated for Tag check.");
    return false;
  }

  // track_pt > 7 GeV
  if (! ( (trackElectron->pt() * GeV ) > 7) ){
    ATH_MSG_DEBUG ("(trackSelectionElectrons) Pt_Track of the electron is below 7 GeV. Electron rejected.");
    return false;
  }
  // |eta| < 2.47
  if (! (fabs(trackElectron->eta()) < 2.47 )){
    ATH_MSG_DEBUG ("(trackSelectionElectrons) Electron track_eta is above 2.47. Electron rejected.");
    return false;
  }

  // check for primary vertex in the event
  // *************** Primary vertex check ***************
  //loop over vertices and look for good primary vertex
  bool  isPrimVtx      = false;
  bool  passDeltaZ0sin = false;
  // for (xAOD::VertexContainer::const_iterator vxIter = primVertexCnt->begin(); vxIter != primVertexCnt->end(); ++vxIter) {
  for ( const xAOD::Vertex *vertex : *primVertexCnt){
    // Select good primary vertex          
    float delta_z0      = fabs(trackElectron->z0() + trackElectron->vz() - vertex->z()); //where trk.vz() represents the point of reference for the z0 calculation (in this case, the beamspot position along the z axis).
    float delta_z0_sin  = delta_z0 * sin(trackElectron->theta()); //where sin(trk.theta()) parameterises the uncertainty of the z0 measurement. 

    if ((vertex->vertexType() == xAOD::VxType::PriVtx) && ( delta_z0_sin < m_z0Tag)){
      isPrimVtx       = true; //check for primary vertex in each vertex of the event.
      passDeltaZ0sin  = true; // and check if the longitudinal impact parameter difference for electron-to-vertex association.

      ATH_MSG_DEBUG ("(trackSelectionElectrons) delta_z0_sin < 0.5 mm ("<< delta_z0_sin << ")");
      ATH_MSG_DEBUG ("(trackSelectionElectrons) There is a primary vertex in the event.");
      break;
    }
  }

  // delta_z0_sin <= 0.5 mm
  if (!( isPrimVtx && passDeltaZ0sin)){
    ATH_MSG_DEBUG ("(trackSelectionElectrons) For this Tag, delta_z0_sin > 0.5mm, and there is NO primary vertices in the event. Rejecting electron.");
    return false;
  }  
  
  // d0_sig - transverse impact parameter significance
  double d0sig = fabs (xAOD::TrackingHelpers::d0significance( trackElectron, ei->beamPosSigmaX(), ei->beamPosSigmaY(), ei->beamPosSigmaXY() )); 
  if ( !( d0sig < m_d0TagSig)){
    ATH_MSG_DEBUG("(trackSelectionElectrons) Electron rejected. |d0sig| > " << m_d0TagSig << " ("<< d0sig <<")");
    return false;
  }

  ATH_MSG_DEBUG("(trackSelectionElectrons) Electron accepted.");
  return true; // if true, add electron to electronSelectionCnt container.
}

bool EventReaderBaseAlg::isTagElectron(const xAOD::Electron *el){
  // |eta| < 1.475
  if ( fabs(el->eta()) > 1.475 ){ // EM Barrel
    ATH_MSG_DEBUG ("(isTagElectron) Electron |eta| > 1.475 (" << fabs(el->eta()) << ").");
    return false;
  }

  // LAr crack region
  // ATH_MSG_DEBUG ("(isTagElectron) Selecting Tag Electron Eta outside crack region...");
  float absEta = fabs(el->caloCluster()->etaBE(2));
  if ( !isEtaOutsideLArCrack(absEta) ){
    ATH_MSG_DEBUG ("(isTagElectron) Selecting Tag Electron Eta is inside crack region.");
    return false;
  }

  // Tag electron PID verify if is tight
  bool isGood;
  if (! el->passSelection(isGood, m_offTagTightness) ) {
    ATH_MSG_DEBUG("(isTagElectron) Misconfiguration: " << m_offTagTightness << " is not a valid working point for electrons");
    return false; // no point in continuing
    }
  ATH_MSG_DEBUG("(isTagElectron) Trigger " << m_offTagTightness << " is OK");

  // Et > 25 (or 15) GeV && Et < 180
  float elTagEt = el->e()/(cosh(el->trackParticle()->eta()));
  float elTagPt = el->pt();
  if ( elTagPt < (m_etMinTag * GeV) ){
    ATH_MSG_DEBUG ("(isTagElectron) Tag Electron Et/pT: "<< elTagEt << "/"<< elTagPt << ", threshold =" << m_etMinTag * GeV << ".");
    return false;
  }

  // electron object quality
  ATH_MSG_DEBUG ("(isTagElectron) Checking electron object quality...");
  if ( !el->isGoodOQ(xAOD::EgammaParameters::BADCLUSELECTRON)){
    ATH_MSG_DEBUG ("(isTagElectron) \tTag Electron is a BADCLUSELECTRON.");
    return false;
  }

  return true;
}

bool EventReaderBaseAlg::isGoodProbeElectron(const xAOD::Electron *el){
  // |eta| < 1.475
  if ( fabs(el->eta()) > 1.475 ){ // EM Barrel
    ATH_MSG_DEBUG ("(isGoodProbeElectron) Electron |eta| > 1.475 (" << fabs(el->eta()) << ").");
    return false;
  }

  // electron object quality
  ATH_MSG_DEBUG (" (isGoodProbeElectron) Checking Probe electron object quality...");
  if ( !el->isGoodOQ(xAOD::EgammaParameters::BADCLUSELECTRON)){
    ATH_MSG_DEBUG (" (isGoodProbeElectron) \tProbe Electron is a BADCLUSELECTRON.");
    return false;
  }

  // Et > 15 GeV
  float electronPt = el->pt();
  if (electronPt * GeV < m_etMinProbe ){
    ATH_MSG_DEBUG ("(isGoodProbeElectron) Electron Et/pT < " << m_etMinProbe << " GeV (pT="<< electronPt * GeV<<")");
    return false;
  }

  // is outside the crack
  float absEta = fabs(el->caloCluster()->etaBE(2));
  if ( !isEtaOutsideLArCrack(absEta) ){
    ATH_MSG_DEBUG ("(isGoodProbeElectron) Electron Eta inside LAr crack region...");
    return false;
  }

  // loose ID Cut
  bool isGood;
  if (! el->passSelection(isGood, m_offProbeTightness)){
    ATH_MSG_DEBUG("(isGoodProbeElectron) Misconfiguration: " << m_offProbeTightness << " is not a valid working point for electrons");
    return false;
  }

  ATH_MSG_DEBUG ("(isGoodProbeElectron) Electron is a good probe!");
  return true;
}

bool EventReaderBaseAlg::isEtaOutsideLArCrack(float absEta){
  if ( (absEta > 1.37 && absEta < 1.52) || (absEta > 2.47) ){
    return false;
  }
  else{
    return true;
  }  
}

int EventReaderBaseAlg::getCaloRegionIndex(const CaloCell* cell){
  if (cell->caloDDE()->is_tile()) return 0; //belongs to Tile
  else if (cell->caloDDE()->is_lar_em_barrel()) return 1; //belongs to EM barrel
  else if (cell->caloDDE()->is_lar_em_endcap_inner()) return 2; //belongs to the inner wheel of EM end cap
  else if (cell->caloDDE()->is_lar_em_endcap_outer()) return 3; //belongs to the outer wheel of EM end cap
  else if (cell->caloDDE()->is_lar_hec()) return 4; //belongs to HEC
  else if (cell->caloDDE()->is_lar_fcal()) return 5; //belongs to FCAL
    
  ATH_MSG_ERROR (" #### Region not found for cell offline ID "<< cell->ID() <<" ! Returning -999.");
  return -999; //region not found
}

double EventReaderBaseAlg::fixPhi(double phi){
  // Verify the phi value, if its in -pi,pi interval, then shifts 2pi in the correct direction.
  if (phi < -1*pi) return (phi + 2*pi);
  if (phi >  1*pi) return (phi - 2*pi);
  return phi;
}

double EventReaderBaseAlg::deltaPhi(double phi1, double phi2){
  // Fix phi value for delta_phi calculation, to -pi,+pi interval.
  double deltaPhi = fixPhi(phi1) - fixPhi(phi2);
  return fixPhi(deltaPhi);
}

double EventReaderBaseAlg::deltaR( double deta, double dphi){
  return sqrt( deta*deta + fixPhi(dphi)*fixPhi(dphi) );
}




void EventReaderBaseAlg::bookDatabaseBranches(TTree *tree){
  // ## BCID/Luminosity vector
  tree->Branch ("lb_lumiblock",&m_lb_lumiblock);
  if (!m_isMC){
    tree->Branch ("lb_bcidLuminosity",&m_lb_bcidLuminosity);
  }
}

void EventReaderBaseAlg::clearLBData(){
  m_lb_lumiblock->clear();
  if (!m_isMC){
    m_lb_bcidLuminosity->clear();
  }  
}


void EventReaderBaseAlg::bookBranches(TTree *tree){
  // ## Event info
  tree->Branch ("event_RunNumber", &m_e_runNumber);
  tree->Branch ("event_EventNumber", &m_e_eventNumber);
  tree->Branch ("event_BCID", &m_e_bcid);
  tree->Branch ("event_Lumiblock",&m_e_lumiBlock);
  tree->Branch ("event_avg_mu_inTimePU", &m_e_inTimePileup);
  tree->Branch ("event_avg_mu_OOTimePU", &m_e_outOfTimePileUp);
  // #############

  // ## Cluster 
  tree->Branch ("cluster_index",&m_c_clusterIndex);
  tree->Branch ("c_electronIndex_clusterLvl",&m_c_electronIndex_clusterLvl);
  tree->Branch ("cluster_et",&m_c_clusterEnergy);
  tree->Branch ("cluster_time",&m_c_clusterTime);
  tree->Branch ("cluster_pt",&m_c_clusterPt);
  tree->Branch ("cluster_eta",&m_c_clusterEta);
  tree->Branch ("cluster_phi",&m_c_clusterPhi);
  // Cluster cell
  tree->Branch ("cluster_index_cellLvl",&m_c_clusterIndex_cellLvl);
  tree->Branch ("cluster_cell_index",&m_c_clusterCellIndex);
  tree->Branch ("cluster_cell_caloGain",&m_c_cellGain);
  tree->Branch ("cluster_cell_layer",&m_c_cellLayer);
  tree->Branch ("cluster_cell_region",&m_c_cellRegion);
  tree->Branch ("cluster_cell_energy",&m_c_cellEnergy);
  tree->Branch ("cluster_cell_time",&m_c_cellTime);
  tree->Branch ("cluster_cell_eta",&m_c_cellEta);
  tree->Branch ("cluster_cell_phi",&m_c_cellPhi);
  tree->Branch ("cluster_cell_deta",&m_c_cellDEta);
  tree->Branch ("cluster_cell_dphi",&m_c_cellDPhi);
  tree->Branch ("cluster_cellsDist_dphi",&m_c_cellToClusterDPhi);
  tree->Branch ("cluster_cellsDist_deta",&m_c_cellToClusterDEta);
  // Cluster channel (digits and cell ch)
  tree->Branch ("cluster_index_chLvl",&m_c_clusterIndex_chLvl);
  tree->Branch ("cluster_channel_index",&m_c_clusterChannelIndex);
  tree->Branch ("cluster_channel_digits",&m_c_channelDigits);
  tree->Branch ("cluster_channel_energy",&m_c_channelEnergy);
  tree->Branch ("cluster_channel_time",&m_c_channelTime);
  tree->Branch ("cluster_channel_layer",&m_c_channelLayer);
  if (!m_noBadCells) tree->Branch ("cluster_channel_bad",&m_c_channelBad);
  tree->Branch ("cluster_channel_chInfo", &m_c_channelChInfo);
  tree->Branch ("cluster_channel_hash",&m_c_channelHashMap); 
  tree->Branch ("cluster_channel_id",&m_c_channelChannelIdMap);
  if (m_getLArCalibConstants){
    tree->Branch ("cluster_channel_effSigma",&m_c_channelEffectiveSigma);
    tree->Branch ("cluster_channel_noise",&m_c_channelNoise);
    tree->Branch ("cluster_channel_DSPThreshold",&m_c_channelDSPThreshold);
    tree->Branch ("cluster_channel_OFCTimeOffset",&m_c_channelOFCTimeOffset);
    tree->Branch ("cluster_channel_ADC2MeV0",&m_c_channelADC2MEV0);
    tree->Branch ("cluster_channel_ADC2MeV1",&m_c_channelADC2MEV1);
    tree->Branch ("cluster_channel_pedestal",&m_c_channelPed);
    tree->Branch ("cluster_channel_OFCa",&m_c_channelOFCa);
    tree->Branch ("cluster_channel_OFCb",&m_c_channelOFCb);
    tree->Branch ("cluster_channel_MinBiasAvg",&m_c_channelMinBiasAvg);
    
    if (!m_isMC){
      tree->Branch ("cluster_channel_OfflineEnergyRescaler",&m_c_channelOfflEneRescaler);
      tree->Branch ("cluster_channel_OfflineHVScale",&m_c_channelOfflHVScale);
      tree->Branch ("cluster_channel_shape",&m_c_channelShape);
      tree->Branch ("cluster_channel_shapeDer",&m_c_channelShapeDer);
    }
  }

  // Cluster raw channel
  tree->Branch ("cluster_index_rawChLvl",&m_c_clusterIndex_rawChLvl);
  tree->Branch ("cluster_rawChannel_index",&m_c_clusterRawChannelIndex);
  tree->Branch ("cluster_rawChannel_id",&m_c_rawChannelIdMap);
  tree->Branch ("cluster_rawChannel_amplitude",&m_c_rawChannelAmplitude);
  tree->Branch ("cluster_rawChannel_time",&m_c_rawChannelTime);
  tree->Branch ("cluster_rawChannel_layer",&m_c_rawChannelLayer);
  tree->Branch ("cluster_rawChannel_Ped",&m_c_rawChannelPed);
  tree->Branch ("cluster_rawChannel_Prov",&m_c_rawChannelProv);
  tree->Branch ("cluster_rawChannel_qual",&m_c_rawChannelQuality);  
  tree->Branch ("cluster_rawChannel_chInfo",&m_c_rawChannelChInfo); // tree->Branch ("cluster_cell_caloGain",&c_cellGain); //modify
  if (!m_isMC){
    tree->Branch ("cluster_rawChannel_DSPThreshold",&m_c_rawChannelDSPThreshold);
  }  

  // ## Cluster EMB2 7_11
  if (m_doAssocTopoCluster711Dump){
    tree->Branch ("cluster711_index",&m_c711_clusterIndex);
    tree->Branch ("c711_electronIndex_clusterLvl",&m_c711_electronIndex_clusterLvl);
    tree->Branch ("cluster711_et",&m_c711_clusterEnergy);
    tree->Branch ("cluster711_pt",&m_c711_clusterPt);
    tree->Branch ("cluster711_time",&m_c711_clusterTime);
    tree->Branch ("cluster711_eta",&m_c711_clusterEta);
    tree->Branch ("cluster711_phi",&m_c711_clusterPhi);
    // Cluster cell
    tree->Branch ("cluster711_index_cellLvl",&m_c711_clusterIndex_cellLvl);
    tree->Branch ("cluster711_cell_index",&m_c711_clusterCellIndex);
    tree->Branch ("cluster711_cell_caloGain",&m_c711_cellGain);
    tree->Branch ("cluster711_cell_layer",&m_c711_cellLayer);
    tree->Branch ("cluster711_cell_region",&m_c711_cellRegion);
    tree->Branch ("cluster711_cell_energy",&m_c711_cellEnergy);
    tree->Branch ("cluster711_cell_time",&m_c711_cellTime);
    tree->Branch ("cluster711_cell_eta",&m_c711_cellEta);
    tree->Branch ("cluster711_cell_phi",&m_c711_cellPhi);
    tree->Branch ("cluster711_cell_deta",&m_c711_cellDEta);
    tree->Branch ("cluster711_cell_dphi",&m_c711_cellDPhi);
    tree->Branch ("cluster711_cellsDist_dphi",&m_c711_cellToClusterDPhi);
    tree->Branch ("cluster711_cellsDist_deta",&m_c711_cellToClusterDEta);
    // Cluster channel (digits and cell ch)
    tree->Branch ("cluster711_index_chLvl",&m_c711_clusterIndex_chLvl);
    tree->Branch ("cluster711_channel_index",&m_c711_clusterChannelIndex);
    tree->Branch ("cluster711_channel_digits",&m_c711_channelDigits);
    tree->Branch ("cluster711_channel_energy",&m_c711_channelEnergy);
    tree->Branch ("cluster711_channel_time",&m_c711_channelTime);
    tree->Branch ("cluster711_channel_layer", &m_c711_channelLayer);
    if (!m_noBadCells) tree->Branch ("cluster711_channel_bad",&m_c711_channelBad);
    tree->Branch ("cluster711_channel_chInfo", &m_c711_channelChInfo);
    tree->Branch ("cluster711_channel_hash",&m_c711_channelHashMap); 
    tree->Branch ("cluster711_channel_id",&m_c711_channelChannelIdMap);
    if (m_getLArCalibConstants){
      tree->Branch ("cluster711_channel_effSigma",&m_c711_channelEffectiveSigma);
      tree->Branch ("cluster711_channel_noise",&m_c711_channelNoise);
      tree->Branch ("cluster711_channel_DSPThreshold",&m_c711_channelDSPThreshold);
      tree->Branch ("cluster711_channel_OFCTimeOffset",&m_c711_channelOFCTimeOffset);
      tree->Branch ("cluster711_channel_ADC2MeV0",&m_c711_channelADC2MEV0);
      tree->Branch ("cluster711_channel_ADC2MeV1",&m_c711_channelADC2MEV1);
      tree->Branch ("cluster711_channel_pedestal",&m_c711_channelPed);
      tree->Branch ("cluster711_channel_OFCa",&m_c711_channelOFCa);
      tree->Branch ("cluster711_channel_OFCb",&m_c711_channelOFCb);
      tree->Branch ("cluster711_channel_MinBiasAvg",&m_c711_channelMinBiasAvg);
      
      if (!m_isMC){
        tree->Branch ("cluster711_channel_OfflineEnergyRescaler",&m_c711_channelOfflEneRescaler);
        tree->Branch ("cluster711_channel_OfflineHVScale",&m_c711_channelOfflHVScale);
        tree->Branch ("cluster711_channel_shape",&m_c711_channelShape);
        tree->Branch ("cluster711_channel_shapeDer",&m_c711_channelShapeDer);
      }
    }
    // Cluster raw channel
    tree->Branch ("cluster711_index_rawChLvl",&m_c711_clusterIndex_rawChLvl);
    tree->Branch ("cluster711_rawChannel_index",&m_c711_clusterRawChannelIndex);
    tree->Branch ("cluster711_rawChannel_id",&m_c711_rawChannelIdMap);
    tree->Branch ("cluster711_rawChannel_amplitude",&m_c711_rawChannelAmplitude);
    tree->Branch ("cluster711_rawChannel_time",&m_c711_rawChannelTime);
    tree->Branch ("cluster711_rawChannel_layer",&m_c711_rawChannelLayer);
    tree->Branch ("cluster711_rawChannel_Ped",&m_c711_rawChannelPed);
    tree->Branch ("cluster711_rawChannel_Prov",&m_c711_rawChannelProv);
    tree->Branch ("cluster711_rawChannel_qual",&m_c711_rawChannelQuality);  
    tree->Branch ("cluster711_rawChannel_chInfo",&m_c711_rawChannelChInfo);
    if (!m_isMC){
      tree->Branch ("cluster711_rawChannel_DSPThreshold",&m_c711_rawChannelDSPThreshold);
    }
  }
  // #############

  // ## Particle Truth ##
  if (m_isMC){
    if (m_doLArEMBHitsDump){
      tree->Branch("hits_sampling", &m_hits_sampling);
      tree->Branch("hits_clusterIndex_chLvl", &m_hits_clusterIndex_chLvl);
      tree->Branch("hits_clusterChannelIndex", &m_hits_clusterChannelIndex);
      tree->Branch("hits_hash", &m_hits_hash);
      tree->Branch("hits_energy", &m_hits_energy);
      tree->Branch("hits_time", &m_hits_time);
      tree->Branch("hits_sampFrac", &m_hits_sampFrac);
      tree->Branch("hits_energyConv", &m_hits_energyConv);
      tree->Branch("hits_cellEta", &m_hits_cellEta);
      tree->Branch("hits_cellPhi", &m_hits_cellPhi);
    }

    if (m_doTruthPartDump){
      tree->Branch("mc_part_energy",&m_mc_part_energy);
      tree->Branch("mc_part_pt",&m_mc_part_pt);
      tree->Branch("mc_part_m",&m_mc_part_m);
      tree->Branch("mc_part_eta",&m_mc_part_eta);
      tree->Branch("mc_part_phi",&m_mc_part_phi);
      tree->Branch("mc_part_pdgId",&m_mc_part_pdgId);
      tree->Branch("mc_part_status", &m_mc_part_status);
      tree->Branch("mc_part_barcode", &m_mc_part_barcode);
    }
    // ## Vertex Truth ##
    if (m_doTruthEventDump){
      tree->Branch("mc_vert_x", &m_mc_vert_x);
      tree->Branch("mc_vert_y", &m_mc_vert_y);
      tree->Branch("mc_vert_z", &m_mc_vert_z);
      tree->Branch("mc_vert_time", &m_mc_vert_time);
      tree->Branch("mc_vert_perp", &m_mc_vert_perp);
      tree->Branch("mc_vert_eta", &m_mc_vert_eta);
      tree->Branch("mc_vert_phi", &m_mc_vert_phi);
      tree->Branch("mc_vert_barcode", &m_mc_vert_barcode);
      tree->Branch("mc_vert_status", &m_mc_vert_status);
    }
  }
  
  // ## Photons ##
  if (m_doPhotonDump){
    tree->Branch("ph_energy",&m_ph_energy);
    tree->Branch("ph_pt",&m_ph_pt);  
    tree->Branch("ph_eta",&m_ph_eta);
    tree->Branch("ph_phi",&m_ph_phi);
    tree->Branch("ph_m",&m_ph_m);
  }  
  // ## Primary Vertex
  tree->Branch("vtx_x",&m_vtx_x);
  tree->Branch("vtx_y",&m_vtx_y);
  tree->Branch("vtx_z",&m_vtx_z);
  tree->Branch("vtx_deltaZ0",&m_vtx_deltaZ0);
  tree->Branch("vtx_delta_z0_sin",&m_vtx_delta_z0_sin);
  tree->Branch("vtx_d0sig",&m_vtx_d0sig);

  // ## Electrons ##
  tree->Branch("el_index",&m_el_index);
  tree->Branch("el_Pt",&m_el_Pt);
  tree->Branch("el_et",&m_el_et);
  tree->Branch("el_Eta",&m_el_Eta);
  tree->Branch("el_Phi",&m_el_Phi);
  tree->Branch("el_m",&m_el_m);
  tree->Branch("el_eoverp",&m_el_eoverp);
  tree->Branch("el_f1",&m_el_f1);
  tree->Branch("el_f3",&m_el_f3);
  tree->Branch("el_eratio",&m_el_eratio);
  tree->Branch("el_weta1",&m_el_weta1);
  tree->Branch("el_weta2",&m_el_weta2);
  tree->Branch("el_fracs1",&m_el_fracs1);
  tree->Branch("el_wtots1",&m_el_wtots1);
  tree->Branch("el_e277",&m_el_e277);
  tree->Branch("el_reta",&m_el_reta);
  tree->Branch("el_rphi",&m_el_rphi);
  tree->Branch("el_deltae",&m_el_deltae);
  tree->Branch("el_rhad",&m_el_rhad);
  tree->Branch("el_rhad1",&m_el_rhad1);

  // ## Tag and Probe ##
  // zee
  if (!m_doElecSelectByTrackOnly && m_doTagAndProbe){
    tree->Branch("zee_M", &m_zee_M);
    tree->Branch("zee_E", &m_zee_E);
    tree->Branch("zee_pt", &m_zee_pt);
    tree->Branch("zee_px", &m_zee_px);
    tree->Branch("zee_py", &m_zee_py);
    tree->Branch("zee_pz", &m_zee_pz);
    tree->Branch("zee_T", &m_zee_T);
    tree->Branch("zee_deltaR", &m_zee_deltaR);
  }
}

void EventReaderBaseAlg::clear(){
  // ## Event info
  m_e_runNumber       = 9999;
  m_e_eventNumber     = 9999;
  m_e_bcid            = 9999;
  m_e_lumiBlock       = 999; 
  m_e_inTimePileup    = -999;
  m_e_outOfTimePileUp = -999;
  // ############

  ATH_MSG_DEBUG("Clear Clusters..");
  // ## Cluster (TOPOCLUSTER or SUPERCLUSTER)
  m_c_clusterIndexCounter = 0;
  m_c_clusterIndex->clear();
  m_c_electronIndex_clusterLvl->clear();
  m_c_clusterEnergy->clear();
  m_c_clusterTime->clear();
  m_c_clusterPt->clear();
  m_c_clusterEta->clear();
  m_c_clusterPhi->clear();
  // Cluster cell
  ATH_MSG_DEBUG("Clear Clusters cell..");
  m_c_cellIndexCounter = 0;
  m_c_clusterIndex_cellLvl->clear();
  m_c_clusterCellIndex->clear();
  m_c_cellGain->clear(); 
  m_c_cellLayer->clear();
  m_c_cellRegion->clear();
  m_c_cellEnergy->clear();
  m_c_cellTime->clear();
  m_c_cellEta->clear();
  m_c_cellPhi->clear();
  m_c_cellDEta->clear();
  m_c_cellDPhi->clear();
  m_c_cellToClusterDPhi->clear();
  m_c_cellToClusterDEta->clear();   
  // Cluster channel
  ATH_MSG_DEBUG("Clear Clusters channel..");
  m_c_clusterIndex_chLvl->clear();
  m_c_clusterChannelIndex->clear();
  m_c_channelHashMap->clear(); // 
  m_c_channelChannelIdMap->clear();
  m_c_channelDigits->clear();
  m_c_channelEnergy->clear();
  m_c_channelTime->clear();
  m_c_channelLayer->clear();
  if (!m_noBadCells) m_c_channelBad->clear();
  m_c_channelChInfo->clear();
  if (m_getLArCalibConstants){
    ATH_MSG_DEBUG("Clear calib. constants..");
    m_c_channelEffectiveSigma->clear();
    m_c_channelNoise->clear();
    m_c_channelDSPThreshold->clear();
    m_c_channelOFCTimeOffset->clear();
    m_c_channelADC2MEV0->clear();
    m_c_channelADC2MEV1->clear();
    m_c_channelPed->clear();
    m_c_channelOFCa->clear();
    m_c_channelOFCb->clear();
    m_c_channelMinBiasAvg->clear();    
    if (!m_isMC){
      m_c_channelOfflEneRescaler->clear();
      m_c_channelOfflHVScale->clear();
      m_c_channelShapeDer->clear();
      m_c_channelShape->clear();
    }
  }
  // Cluster raw channel
  ATH_MSG_DEBUG("Clear Clusters rawch..");
  m_c_rawChannelIdMap->clear();
  m_c_rawChannelChInfo->clear();
  m_c_rawChannelAmplitude->clear();
  m_c_rawChannelTime->clear();
  m_c_rawChannelLayer->clear();
  m_c_rawChannelPed->clear();// raw channel estimated pedestal 
  m_c_rawChannelProv->clear();// raw channel LAr provenance (tile masked)
  m_c_rawChannelQuality->clear();
  m_c_clusterRawChannelIndex->clear();
  m_c_clusterIndex_rawChLvl->clear();
  if (!m_isMC){
    m_c_rawChannelDSPThreshold->clear();
  }
  // ############

  // ## Cluster EMB2_711
  if (m_doAssocTopoCluster711Dump){    
    *m_c711_clusterIndexCounter = 0;
    m_c711_clusterIndex->clear();
    m_c711_electronIndex_clusterLvl->clear();
    m_c711_clusterEnergy->clear();
    m_c711_clusterPt->clear();
    m_c711_clusterTime->clear();
    m_c711_clusterEta->clear();
    m_c711_clusterPhi->clear();
    // Cluster cell
    ATH_MSG_DEBUG("Clear Clusters 7x11 cell..");
    *m_c711_cellIndexCounter = 0;
    m_c711_clusterIndex_cellLvl->clear();
    m_c711_clusterCellIndex->clear();
    m_c711_cellGain->clear(); 
    m_c711_cellLayer->clear();
    m_c711_cellRegion->clear();
    m_c711_cellEnergy->clear();
    m_c711_cellTime->clear();
    m_c711_cellEta->clear();
    m_c711_cellPhi->clear();
    m_c711_cellDEta->clear();
    m_c711_cellDPhi->clear();
    m_c711_cellToClusterDPhi->clear();
    m_c711_cellToClusterDEta->clear();   
    // Cluster channel
    ATH_MSG_DEBUG("Clear Clusters 7x11 channel..");
    m_c711_clusterIndex_chLvl->clear();
    m_c711_clusterChannelIndex->clear();
    m_c711_channelHashMap->clear(); // 
    m_c711_channelChannelIdMap->clear();
    m_c711_channelDigits->clear();
    m_c711_channelEnergy->clear();
    m_c711_channelTime->clear();
    m_c711_channelLayer->clear();
    if (!m_noBadCells) m_c711_channelBad->clear();
    m_c711_channelChInfo->clear();
    if (m_getLArCalibConstants){
      m_c711_channelEffectiveSigma->clear();
      m_c711_channelNoise->clear();
      m_c711_channelDSPThreshold->clear();
      m_c711_channelOFCTimeOffset->clear();
      m_c711_channelADC2MEV0->clear();
      m_c711_channelADC2MEV1->clear();
      m_c711_channelPed->clear();
      m_c711_channelOFCa->clear();
      m_c711_channelOFCb->clear();
      m_c711_channelMinBiasAvg->clear();    
      if (!m_isMC){
        m_c711_channelOfflEneRescaler->clear();
        m_c711_channelOfflHVScale->clear();
        m_c711_channelShapeDer->clear();
        m_c711_channelShape->clear();
      }      
    }
    // Cluster raw channel
    ATH_MSG_DEBUG("Clear Clusters 7x11 rawch..");
    m_c711_rawChannelIdMap->clear();
    m_c711_rawChannelChInfo->clear();
    m_c711_rawChannelAmplitude->clear();
    m_c711_rawChannelTime->clear();
    m_c711_rawChannelLayer->clear();
    m_c711_rawChannelPed->clear();// raw channel estimated pedestal 
    m_c711_rawChannelProv->clear();// raw channel LAr provenance (tile masked)
    m_c711_rawChannelQuality->clear();
    m_c711_clusterRawChannelIndex->clear();
    m_c711_clusterIndex_rawChLvl->clear();
    if (!m_isMC){
      m_c711_rawChannelDSPThreshold->clear();
    }
}

  // ## Particle Truth ##
  if (m_isMC){
    if (m_doLArEMBHitsDump){
      m_hits_sampling->clear();
      m_hits_clusterIndex_chLvl->clear();
      m_hits_clusterChannelIndex->clear();
      m_hits_hash->clear();
      m_hits_energy->clear();
      m_hits_time->clear();
      m_hits_sampFrac->clear();
      m_hits_energyConv->clear();
      m_hits_cellEta->clear();
      m_hits_cellPhi->clear();
    }

    if (m_doTruthPartDump){
      ATH_MSG_DEBUG("particle truth..");
      m_mc_part_energy->clear();
      m_mc_part_pt->clear();
      m_mc_part_m->clear();
      m_mc_part_eta->clear();
      m_mc_part_phi->clear();
      m_mc_part_pdgId->clear();
      m_mc_part_status->clear();
      m_mc_part_barcode->clear();
    }
    // ## Vertex Truth ##
    if (m_doTruthEventDump){
      m_mc_vert_x->clear();
      m_mc_vert_y->clear();
      m_mc_vert_z->clear();
      m_mc_vert_time->clear();
      m_mc_vert_perp->clear();
      m_mc_vert_eta->clear();
      m_mc_vert_phi->clear();
      m_mc_vert_barcode->clear();
      m_mc_vert_status->clear();
    }
  }
  // ## Photons ##
  if (m_doPhotonDump){
    ATH_MSG_DEBUG("Clear Photons");
    m_ph_energy->clear();
    m_ph_eta->clear();
    m_ph_phi->clear();
    m_ph_pt->clear();
    m_ph_m->clear();
  }

  ATH_MSG_DEBUG("Clear: Vertexes");
  m_vtx_x->clear();
  m_vtx_y->clear();
  m_vtx_z->clear();
  m_vtx_deltaZ0->clear();
  m_vtx_delta_z0_sin->clear();
  m_vtx_d0sig->clear();

  // ## Electrons ##
  ATH_MSG_DEBUG("Clear: Electrons");
  m_el_index->clear();
  m_el_Pt->clear();
  m_el_et->clear();
  m_el_Eta->clear();
  m_el_Phi->clear();
  m_el_m->clear();
  m_el_eoverp->clear();
  
  // offline shower shapes 
  m_el_f1->clear();
  m_el_f3->clear();
  m_el_eratio->clear();
  m_el_weta1->clear();
  m_el_weta2->clear();
  m_el_fracs1->clear();
  m_el_wtots1->clear();
  m_el_e277->clear();
  m_el_reta->clear();
  m_el_rphi->clear();
  m_el_deltae->clear();
  m_el_rhad->clear();
  m_el_rhad1->clear();

  // ## Tag and Probe ##
  //zee
  if (!m_doElecSelectByTrackOnly && m_doTagAndProbe){
    ATH_MSG_DEBUG("Clear: T&P");
    m_zee_M->clear();
    m_zee_E->clear();
    m_zee_pt->clear();
    m_zee_px->clear();
    m_zee_py->clear();
    m_zee_pz->clear();
    m_zee_T->clear();
    m_zee_deltaR->clear();
  }
}
