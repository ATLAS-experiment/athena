/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FastCaloSimParamHitAnalysis.h"
#include <GaudiKernel/StatusCode.h>
#include "FastCaloSim/Core/TFCSTruthState.h"
#include "FastCaloSim/Core/TFCSExtrapolationState.h"

// Geant4 includes for building the input track to the FastCaloSim transport
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4DynamicParticle.hh"
#include "G4Track.hh"
#include "G4FieldTrack.hh"
#include "G4ThreeVector.hh"
#include <memory>

// Section of includes for LAr calo tests
#include "LArSimEvent/LArHitContainer.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"

// Section of includes for tile calo tests
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/LArFCAL_ID.h"
#include "CaloIdentifier/LArHEC_ID.h"
#include "TileConditions/TileInfo.h"

#include "TileDetDescr/TileDetDescrManager.h"
#include "CaloIdentifier/TileID.h"
#include "TileIdentifier/TileHWID.h"
#include "TileSimEvent/TileHit.h"
#include "TileSimEvent/TileHitVector.h"

//Track Record
#include "TrackRecord/TrackRecordCollection.h"

//CaloCell
#include "CaloEvent/CaloCellContainer.h"
#include "CaloEvent/CaloClusterCellLinkContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODCaloEvent/CaloCluster.h"

#include "GaudiKernel/MsgStream.h"

#include "ISF_FastCaloSimEvent/FCS_StepInfoCollection.h"

#include "TTree.h"
#include "TFile.h"
#include "TString.h"
#include "TVector3.h"
#include <sstream>

// For MC Truth information:
#include "GeneratorObjects/McEventCollection.h"


#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>

namespace {
  /// Build a Geant4 track from a FastCaloSim truth state, to be transported
  /// through the calorimeter geometry. Returns nullptr if the pdgid is not
  /// known to the Geant4 particle table.
  std::unique_ptr<G4Track> makeG4Track(const TFCSTruthState& truth) {
    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    if (!particleTable) return nullptr;
    G4ParticleDefinition* particle = particleTable->FindParticle(truth.pdgid());
    if (!particle) return nullptr;

    // Dynamic particle from the truth momentum (in MeV)
    const G4ThreeVector momentum(truth.Px(), truth.Py(), truth.Pz());
    auto* dynamicParticle = new G4DynamicParticle(particle, momentum);

    // Track at the truth production vertex (G4Track takes ownership of the
    // dynamic particle)
    const G4ThreeVector position(truth.vertex().X(), truth.vertex().Y(),
                                 truth.vertex().Z());
    return std::make_unique<G4Track>(dynamicParticle, /*time=*/0.0, position);
  }
}

FastCaloSimParamHitAnalysis::FastCaloSimParamHitAnalysis(const std::string& name, ISvcLocator* pSvcLocator)
: AthAlgorithm(name, pSvcLocator)
  //Note that m_xxx are pointers to vectors set to 0, not set to empty vector! see note around TBranch
{
  m_surfacelist.resize(0);
  m_surfacelist.push_back(CaloCell_ID_FCS::PreSamplerB);
  m_surfacelist.push_back(CaloCell_ID_FCS::PreSamplerE);
  m_surfacelist.push_back(CaloCell_ID_FCS::EME1);
  m_surfacelist.push_back(CaloCell_ID_FCS::EME2);
  m_surfacelist.push_back(CaloCell_ID_FCS::FCAL0);
}

FastCaloSimParamHitAnalysis::~FastCaloSimParamHitAnalysis()
= default;

StatusCode FastCaloSimParamHitAnalysis::initialize ATLAS_NOT_THREAD_SAFE ()
{
  ATH_MSG_INFO( "Initializing FastCaloSimParamHitAnalysis" );
  //
  // Register the callback(s):
  //
  ATH_CHECK(m_geoModel.retrieve());
  ATH_CHECK(detStore()->retrieve(m_tileMgr));
  ATH_CHECK(detStore()->retrieve(m_tileID));

  const CaloIdManager* caloIdManager{nullptr};
  ATH_CHECK(detStore()->retrieve(caloIdManager));
  m_larEmID=caloIdManager->getEM_ID();
  if(m_larEmID==nullptr)
    throw std::runtime_error("ISF_HitAnalysis: Invalid LAr EM ID helper");
  m_larFcalID=caloIdManager->getFCAL_ID();
  if(m_larFcalID==nullptr)
    throw std::runtime_error("ISF_HitAnalysis: Invalid FCAL ID helper");
  m_larHecID=caloIdManager->getHEC_ID();
  if(m_larHecID==nullptr)
    throw std::runtime_error("ISF_HitAnalysis: Invalid HEC ID helper");
  m_tileID=caloIdManager->getTileID();
  if(m_tileID==nullptr)
    throw std::runtime_error("ISF_HitAnalysis: Invalid Tile ID helper");

  ATH_CHECK( m_fSamplKey.initialize() );

  ATH_CHECK(detStore()->retrieve(m_tileHWID));
  ATH_CHECK( m_tileSamplingFractionKey.initialize() );

  ATH_CHECK( m_tileCablingSvc.retrieve() );
  m_tileCabling = m_tileCablingSvc->cablingService();

  ATH_CHECK(m_caloMgrKey.initialize());

  // Get the FastCaloSim transport + extrapolation tool
  ATH_CHECK (m_FastCaloSimParametrizationTool.retrieve());
  // Build the shared transport world volume now, on the master thread. This is
  // a no-op when the Geant4 FastCaloSim fast-sim model is also configured (it
  // runs the same call in its constructor); when this algorithm is the only
  // consumer it loads the simplified transport geometry and creates the world.
  ATH_CHECK(m_FastCaloSimParametrizationTool->initializeTransportGeometry());

  // Grab the Ntuple and histogramming service for the tree
  ATH_CHECK(m_thistSvc.retrieve());

  // Give ROOT a file context while THistSvc takes ownership of the tree.
  std::unique_ptr<TFile> dummyFile{
      TFile::Open("dummyFile.root", "RECREATE")};
  m_tree = new TTree("FCS_ParametrizationInput", "FCS_ParametrizationInput");
  std::string fullNtupleName =  "/"+m_ntupleFileName+"/"+m_ntupleTreeName;
  StatusCode sc = m_thistSvc->regTree(fullNtupleName, m_tree);
  if (sc.isFailure() || !m_tree )
    {
      ATH_MSG_ERROR("Unable to register TTree: " << fullNtupleName);
      return StatusCode::FAILURE;
    }

  /** now add branches and leaves to the tree */
  if (m_tree)
    {
      ATH_MSG_INFO("Registered TTree: " << fullNtupleName);
      //initialize the variables before creating the branches
      m_hit_x = new std::vector<float>;
      m_hit_y = new std::vector<float>;
      m_hit_z = new std::vector<float>;
      m_hit_energy = new std::vector<float>;
      m_hit_time = new std::vector<float>;
      m_hit_identifier = new std::vector<Long64_t>;
      m_hit_cellidentifier = new std::vector<Long64_t>;
      m_islarbarrel = new std::vector<bool>;
      m_islarendcap = new std::vector<bool>;
      m_islarhec = new std::vector<bool>;
      m_islarfcal = new std::vector<bool>;
      m_istile = new std::vector<bool>;
      m_hit_sampling = new std::vector<int>;
      m_hit_samplingfraction = new std::vector<float>;

      m_truth_energy = new std::vector<float>;
      m_truth_px = new std::vector<float>;
      m_truth_py = new std::vector<float>;
      m_truth_pz = new std::vector<float>;
      m_truth_pdg = new std::vector<int>;
      m_truth_barcode = new std::vector<int>;
      m_truth_vtxbarcode = new std::vector<int>;

      m_cluster_energy = new std::vector<float>;
      m_cluster_eta    = new std::vector<float>;
      m_cluster_phi    = new std::vector<float>;
      m_cluster_size   = new std::vector<unsigned>;
      m_cluster_cellID = new std::vector<std::vector<Long64_t > >;

      m_cell_identifier = new std::vector<Long64_t>;
      m_cell_energy = new std::vector<float>;
      m_cell_sampling = new std::vector<int>;

      m_g4hit_energy = new std::vector<float>;
      m_g4hit_time = new std::vector<float>;
      m_g4hit_identifier = new std::vector<Long64_t>;
      m_g4hit_cellidentifier = new std::vector<Long64_t>;
      m_g4hit_samplingfraction = new std::vector<float>;
      m_g4hit_sampling = new std::vector<int>;

      m_total_cell_e = 0;
      m_total_hit_e = 0;
      m_total_g4hit_e = 0;

      m_final_cell_energy = new std::vector<Float_t>;
      m_final_hit_energy = new std::vector<Float_t>;
      m_final_g4hit_energy = new std::vector<Float_t>;

      m_newTTC_entrance_eta = new std::vector<std::vector<float> >;
      m_newTTC_entrance_phi = new std::vector<std::vector<float> >;
      m_newTTC_entrance_r = new std::vector<std::vector<float> >;
      m_newTTC_entrance_z = new std::vector<std::vector<float> >;
      m_newTTC_entrance_detaBorder = new std::vector<std::vector<float> >;
      m_newTTC_entrance_OK = new std::vector<std::vector<bool> >;
      m_newTTC_back_eta = new std::vector<std::vector<float> >;
      m_newTTC_back_phi = new std::vector<std::vector<float> >;
      m_newTTC_back_r = new std::vector<std::vector<float> >;
      m_newTTC_back_z = new std::vector<std::vector<float> >;
      m_newTTC_back_detaBorder = new std::vector<std::vector<float> >;
      m_newTTC_back_OK = new std::vector<std::vector<bool> >;
      m_newTTC_mid_eta = new std::vector<std::vector<float> >;
      m_newTTC_mid_phi = new std::vector<std::vector<float> >;
      m_newTTC_mid_r = new std::vector<std::vector<float> >;
      m_newTTC_mid_z = new std::vector<std::vector<float> >;
      m_newTTC_mid_detaBorder = new std::vector<std::vector<float> >;
      m_newTTC_mid_OK = new std::vector<std::vector<bool> >;
      m_newTTC_IDCaloBoundary_eta = new std::vector<float>;
      m_newTTC_IDCaloBoundary_phi = new std::vector<float>;
      m_newTTC_IDCaloBoundary_r = new std::vector<float>;
      m_newTTC_IDCaloBoundary_z = new std::vector<float>;
      m_newTTC_Angle3D = new std::vector<float>;
      m_newTTC_AngleEta = new std::vector<float>;

      m_MuonEntryLayer_E = new std::vector<float>;
      m_MuonEntryLayer_px = new std::vector<float>;
      m_MuonEntryLayer_py = new std::vector<float>;
      m_MuonEntryLayer_pz = new std::vector<float>;
      m_MuonEntryLayer_x = new std::vector<float>;
      m_MuonEntryLayer_y = new std::vector<float>;
      m_MuonEntryLayer_z = new std::vector<float>;
      m_MuonEntryLayer_pdg = new std::vector<int>;

      // Optional branches
      if(m_saveAllBranches){
        m_tree->Branch("HitX",                 &m_hit_x);
        m_tree->Branch("HitY",                 &m_hit_y);
        m_tree->Branch("HitZ",                 &m_hit_z);
        m_tree->Branch("HitE",                 &m_hit_energy);
        m_tree->Branch("HitT",                 &m_hit_time);
        m_tree->Branch("HitIdentifier",        &m_hit_identifier);
        m_tree->Branch("HitCellIdentifier",    &m_hit_cellidentifier);
        m_tree->Branch("HitIsLArBarrel",       &m_islarbarrel);
        m_tree->Branch("HitIsLArEndCap",       &m_islarendcap);
        m_tree->Branch("HitIsHEC",             &m_islarhec);
        m_tree->Branch("HitIsFCAL",            &m_islarfcal);
        m_tree->Branch("HitIsTile",            &m_istile);
        m_tree->Branch("HitSampling",          &m_hit_sampling);
        m_tree->Branch("HitSamplingFraction",  &m_hit_samplingfraction);

        m_tree->Branch("CellIdentifier",       &m_cell_identifier);
        m_tree->Branch("CellE",                &m_cell_energy);
        m_tree->Branch("CellSampling",         &m_cell_sampling);

        m_tree->Branch("G4HitE",               &m_g4hit_energy);
        m_tree->Branch("G4HitT",               &m_g4hit_time);
        m_tree->Branch("G4HitIdentifier",      &m_g4hit_identifier);
        m_tree->Branch("G4HitCellIdentifier",  &m_g4hit_cellidentifier);
        m_tree->Branch("G4HitSamplingFraction",&m_g4hit_samplingfraction);
        m_tree->Branch("G4HitSampling",        &m_g4hit_sampling);
      }

      //CaloHitAna output variables
      m_tree->Branch("TruthE",               &m_truth_energy);
      m_tree->Branch("TruthPx",              &m_truth_px);
      m_tree->Branch("TruthPy",              &m_truth_py);
      m_tree->Branch("TruthPz",              &m_truth_pz);
      m_tree->Branch("TruthPDG",             &m_truth_pdg);
      m_tree->Branch("TruthBarcode",         &m_truth_barcode);
      m_tree->Branch("TruthVtxBarcode",      &m_truth_vtxbarcode);

      if(m_doClusterInfo){
        m_tree->Branch("ClusterE",               &m_cluster_energy);
        m_tree->Branch("ClusterEta",             &m_cluster_eta);
        m_tree->Branch("ClusterPhi",             &m_cluster_phi);
        m_tree->Branch("ClusterSize",            &m_cluster_size);
        m_tree->Branch("ClusterCellID",          &m_cluster_cellID);
      }

      m_oneeventcells = new FCS_matchedcellvector;
      if(m_doAllCells){
        m_tree->Branch("AllCells", &m_oneeventcells);
      }

      //write cells per layer
      if(m_doLayers){
        for (Int_t i = 0; i < MAX_LAYER; i++)
          {
            TString branchname = "Sampling_";
            branchname += i;
            m_layercells[i] = new FCS_matchedcellvector;
            m_tree->Branch(branchname, &m_layercells[i]);
          }
      }

      if(m_doLayerSums){
        //write also energies per layer:
        m_tree->Branch("cell_energy", &m_final_cell_energy);
        m_tree->Branch("hit_energy",  &m_final_hit_energy);
        m_tree->Branch("g4hit_energy", &m_final_g4hit_energy);

        //This is a duplicate of cell_energy[25]
        m_tree->Branch("total_cell_energy", &m_total_cell_e);
        m_tree->Branch("total_hit_energy",  &m_total_hit_e);
        m_tree->Branch("total_g4hit_energy", &m_total_g4hit_e);
      }

      m_tree->Branch("newTTC_back_eta",&m_newTTC_back_eta);
      m_tree->Branch("newTTC_back_phi",&m_newTTC_back_phi);
      m_tree->Branch("newTTC_back_r",&m_newTTC_back_r);
      m_tree->Branch("newTTC_back_z",&m_newTTC_back_z);
      m_tree->Branch("newTTC_back_detaBorder",&m_newTTC_back_detaBorder);
      m_tree->Branch("newTTC_back_OK",&m_newTTC_back_OK);
      m_tree->Branch("newTTC_entrance_eta",&m_newTTC_entrance_eta);
      m_tree->Branch("newTTC_entrance_phi",&m_newTTC_entrance_phi);
      m_tree->Branch("newTTC_entrance_r",&m_newTTC_entrance_r);
      m_tree->Branch("newTTC_entrance_z",&m_newTTC_entrance_z);
      m_tree->Branch("newTTC_entrance_detaBorder",&m_newTTC_entrance_detaBorder);
      m_tree->Branch("newTTC_entrance_OK",&m_newTTC_entrance_OK);
      m_tree->Branch("newTTC_mid_eta",&m_newTTC_mid_eta);
      m_tree->Branch("newTTC_mid_phi",&m_newTTC_mid_phi);
      m_tree->Branch("newTTC_mid_r",&m_newTTC_mid_r);
      m_tree->Branch("newTTC_mid_z",&m_newTTC_mid_z);
      m_tree->Branch("newTTC_mid_detaBorder",&m_newTTC_mid_detaBorder);
      m_tree->Branch("newTTC_mid_OK",&m_newTTC_mid_OK);
      m_tree->Branch("newTTC_IDCaloBoundary_eta",&m_newTTC_IDCaloBoundary_eta);
      m_tree->Branch("newTTC_IDCaloBoundary_phi",&m_newTTC_IDCaloBoundary_phi);
      m_tree->Branch("newTTC_IDCaloBoundary_r",&m_newTTC_IDCaloBoundary_r);
      m_tree->Branch("newTTC_IDCaloBoundary_z",&m_newTTC_IDCaloBoundary_z);
      m_tree->Branch("newTTC_Angle3D",&m_newTTC_Angle3D);
      m_tree->Branch("newTTC_AngleEta",&m_newTTC_AngleEta);

      m_tree->Branch("MuonEntryLayer_E",&m_MuonEntryLayer_E);
      m_tree->Branch("MuonEntryLayer_px",&m_MuonEntryLayer_px);
      m_tree->Branch("MuonEntryLayer_py",&m_MuonEntryLayer_py);
      m_tree->Branch("MuonEntryLayer_pz",&m_MuonEntryLayer_pz);
      m_tree->Branch("MuonEntryLayer_x",&m_MuonEntryLayer_x);
      m_tree->Branch("MuonEntryLayer_y",&m_MuonEntryLayer_y);
      m_tree->Branch("MuonEntryLayer_z",&m_MuonEntryLayer_z);
      m_tree->Branch("MuonEntryLayer_pdg",&m_MuonEntryLayer_pdg);
    }
  dummyFile->Close();
  return StatusCode::SUCCESS;
} //initialize

StatusCode FastCaloSimParamHitAnalysis::finalize ATLAS_NOT_THREAD_SAFE ()
{

 ATH_MSG_INFO( "doing finalize()" );


 const AthenaAttributeList* simParam = nullptr;
 if (detStore()->retrieve(simParam, m_MC_SIM_PARAM).isFailure()) {
   ATH_MSG_ERROR("Could not retrieve Simulation parameters");
   return StatusCode::FAILURE;
 } else {
   ATH_MSG_DEBUG("Retrieved Simulation parameters");
   for (auto attrItr = simParam->begin(); attrItr != simParam->end();
        ++attrItr) {
     std::stringstream outstr;
     attrItr->toOutputStream(outstr);
     ATH_MSG_INFO("Simulation MetaData: " << outstr.str());
   }
 }

 const AthenaAttributeList* digiParam = nullptr;
 if (detStore()->retrieve(digiParam, m_MC_DIGI_PARAM).isFailure()) {
   ATH_MSG_DEBUG("Digitization parameters are not available");
 } else {
   ATH_MSG_DEBUG("Retrieved Digitization parameters");
   for (auto attrItr = digiParam->begin(); attrItr != digiParam->end();
        ++attrItr) {
     std::stringstream outstr;
     attrItr->toOutputStream(outstr);
     ATH_MSG_INFO("Digitization MetaData: " << outstr.str());
   }
 }
 std::unique_ptr<TFile> dummyGeoFile{
     TFile::Open("dummyGeoFile.root", "RECREATE")};
 TTree* geo = new TTree( m_geoModel->atlasVersion().c_str() , m_geoModel->atlasVersion().c_str() );
 std::string fullNtupleName =  "/"+m_geoFileName+"/"+m_geoModel->atlasVersion();
 StatusCode sc = m_thistSvc->regTree(fullNtupleName, geo);
 if(sc.isFailure() || !geo )
 {
  ATH_MSG_ERROR("Unable to register TTree: " << fullNtupleName);
  return StatusCode::FAILURE;
 }

 using GEOCELL = struct
 {
  Long64_t identifier;
  Int_t calosample;
  float eta,phi,r,eta_raw,phi_raw,r_raw,x,y,z,x_raw,y_raw,z_raw;
  float deta,dphi,dr,dx,dy,dz;
 };

 static GEOCELL geocell;

 if(geo)
 {
  ATH_MSG_INFO("Registered TTree: " << fullNtupleName);
  geo->Branch("identifier", &geocell.identifier,"identifier/L");
  geo->Branch("calosample", &geocell.calosample,"calosample/I");

  geo->Branch("eta", &geocell.eta,"eta/F");
  geo->Branch("phi", &geocell.phi,"phi/F");
  geo->Branch("r", &geocell.r,"r/F");
  geo->Branch("eta_raw", &geocell.eta_raw,"eta_raw/F");
  geo->Branch("phi_raw", &geocell.phi_raw,"phi_raw/F");
  geo->Branch("r_raw", &geocell.r_raw,"r_raw/F");

  geo->Branch("x", &geocell.x,"x/F");
  geo->Branch("y", &geocell.y,"y/F");
  geo->Branch("z", &geocell.z,"z/F");
  geo->Branch("x_raw", &geocell.x_raw,"x_raw/F");
  geo->Branch("y_raw", &geocell.y_raw,"y_raw/F");
  geo->Branch("z_raw", &geocell.z_raw,"z_raw/F");

  geo->Branch("deta", &geocell.deta,"deta/F");
  geo->Branch("dphi", &geocell.dphi,"dphi/F");
  geo->Branch("dr", &geocell.dr,"dr/F");
  geo->Branch("dx", &geocell.dx,"dx/F");
  geo->Branch("dy", &geocell.dy,"dy/F");
  geo->Branch("dz", &geocell.dz,"dz/F");
 }

 SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey,Gaudi::Hive::currentContext()};
 ATH_CHECK(caloMgrHandle.isValid());
 const CaloDetDescrManager* calo_dd_man = *caloMgrHandle;

 int ncells=0;
 for (const CaloDetDescrElement* theDDE : calo_dd_man->element_range())
 {
   if(theDDE)
   {
    CaloCell_ID::CaloSample sample=theDDE->getSampling();
    ++ncells;
    if(geo)
    {
     geocell.identifier=theDDE->identify().get_compact();
     geocell.calosample=sample;
     geocell.eta=theDDE->eta();
     geocell.phi=theDDE->phi();
     geocell.r=theDDE->r();
     geocell.eta_raw=theDDE->eta_raw();
     geocell.phi_raw=theDDE->phi_raw();
     geocell.r_raw=theDDE->r_raw();
     geocell.x=theDDE->x();
     geocell.y=theDDE->y();
     geocell.z=theDDE->z();
     geocell.x_raw=theDDE->x_raw();
     geocell.y_raw=theDDE->y_raw();
     geocell.z_raw=theDDE->z_raw();
     geocell.deta=theDDE->deta();
     geocell.dphi=theDDE->dphi();
     geocell.dr=theDDE->dr();
     geocell.dx=theDDE->dx();
     geocell.dy=theDDE->dy();
     geocell.dz=theDDE->dz();

     geo->Fill();
    }
   }
 }

 ATH_MSG_INFO( ncells<<" cells found" );

 dummyGeoFile->Close();
 return StatusCode::SUCCESS;
} //finalize


StatusCode FastCaloSimParamHitAnalysis::execute(const EventContext& ctx)
{

 ATH_MSG_DEBUG( "In FastCaloSimParamHitAnalysis::execute()" );

 if (! m_tree)
 {
  ATH_MSG_ERROR( "tree not registered" );
  return StatusCode::FAILURE;
 }

 SG::ReadCondHandle<ILArfSampl> fSamplHdl(m_fSamplKey, ctx);
 ATH_CHECK(fSamplHdl.isValid());
 const ILArfSampl* fSampl=*fSamplHdl;

 SG::ReadCondHandle<TileSamplingFraction> tileSamplingFraction(m_tileSamplingFractionKey, ctx);
 ATH_CHECK( tileSamplingFraction.isValid() );


 //now if the branches were created correctly, the pointers point to something and it is possible to clear the vectors
 TVector3 vectest;
 vectest.SetPtEtaPhi(1.,1.,1.);
 m_hit_x->clear();
 m_hit_y->clear();
 m_hit_z->clear();
 m_hit_energy->clear();
 m_hit_time->clear();
 m_hit_identifier->clear();
 m_hit_cellidentifier->clear();
 m_islarbarrel->clear();
 m_islarendcap->clear();
 m_islarhec->clear();
 m_islarfcal->clear();
 m_istile->clear();
 m_hit_sampling->clear();
 m_hit_samplingfraction->clear();
 m_truth_energy->clear();
 m_truth_px->clear();
 m_truth_py->clear();
 m_truth_pz->clear();
 m_truth_pdg->clear();
 m_truth_barcode->clear();
 m_truth_vtxbarcode->clear();
 m_cluster_energy->clear();
 m_cluster_eta->clear();
 m_cluster_phi->clear();
 m_cluster_size->clear();
 m_cluster_cellID->clear();
 m_cell_identifier->clear();
 m_cell_energy->clear();
 m_cell_sampling->clear();
 m_g4hit_energy->clear();
 m_g4hit_time->clear();
 m_g4hit_identifier->clear();
 m_g4hit_cellidentifier->clear();
 m_g4hit_sampling->clear();
 m_g4hit_samplingfraction->clear();
 //which fails for this one!!
 //m_matched_cells->clear();
 std::map<Long64_t, FCS_cell> cells; //read all objects and collect them by identifier (Long64_t)
 std::map<Long64_t, std::vector<FCS_g4hit> > g4hits;
 std::map<Long64_t, std::vector<FCS_hit> > hits;

 cells.clear();
 g4hits.clear();
 hits.clear();

 FCS_cell   one_cell{};
 FCS_g4hit  one_g4hit{};
 FCS_hit    one_hit{};
 FCS_matchedcell one_matchedcell;

 m_oneeventcells->m_vector.clear();
 m_final_g4hit_energy->clear();
 m_final_hit_energy->clear();
 m_final_cell_energy->clear();

 m_newTTC_back_eta->clear();
 m_newTTC_back_phi->clear();
 m_newTTC_back_r->clear();
 m_newTTC_back_z->clear();
 m_newTTC_back_detaBorder->clear();
 m_newTTC_back_OK->clear();
 m_newTTC_entrance_eta->clear();
 m_newTTC_entrance_phi->clear();
 m_newTTC_entrance_r->clear();
 m_newTTC_entrance_z->clear();
 m_newTTC_entrance_detaBorder->clear();
 m_newTTC_entrance_OK->clear();
 m_newTTC_mid_eta->clear();
 m_newTTC_mid_phi->clear();
 m_newTTC_mid_r->clear();
 m_newTTC_mid_z->clear();
 m_newTTC_mid_detaBorder->clear();
 m_newTTC_mid_OK->clear();
 m_newTTC_IDCaloBoundary_eta->clear();
 m_newTTC_IDCaloBoundary_phi->clear();
 m_newTTC_IDCaloBoundary_r->clear();
 m_newTTC_IDCaloBoundary_z->clear();
 m_newTTC_Angle3D->clear();
 m_newTTC_AngleEta->clear();


 m_MuonEntryLayer_E->clear();
 m_MuonEntryLayer_x->clear();
 m_MuonEntryLayer_y->clear();
 m_MuonEntryLayer_z->clear();
 m_MuonEntryLayer_px->clear();
 m_MuonEntryLayer_py->clear();
 m_MuonEntryLayer_pz->clear();
 m_MuonEntryLayer_pdg->clear();

 //##########################

 SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey, ctx};
 ATH_CHECK(caloMgrHandle.isValid());
 const CaloDetDescrManager* calo_dd_man = *caloMgrHandle;

 //Get the FastCaloSim step info collection from store
 const ISF_FCS_Parametrization::FCS_StepInfoCollection* eventStepsES;
 StatusCode sc = evtStore()->retrieve(eventStepsES, "MergedEventSteps");
 if (sc.isFailure()) {
   ATH_MSG_WARNING( "No FastCaloSim steps read from StoreGate?" );
   //return StatusCode::FAILURE;
 } else {
   ATH_MSG_INFO("Read: "<<eventStepsES->size()<<" position hits");
   for (ISF_FCS_Parametrization::FCS_StepInfoCollection::const_iterator it = eventStepsES->begin(); it != eventStepsES->end(); ++it) {
     m_hit_x->push_back( (*it)->x() );
     m_hit_y->push_back( (*it)->y() );
     m_hit_z->push_back( (*it)->z() );
     m_hit_energy->push_back( (*it)->energy() );
     m_hit_time->push_back( (*it)->time());

     //Try to get the samplings, sampling fractions from identifiers
     bool larbarrel=false;
     bool larendcap=false;
     bool larhec=false;
     bool larfcal=false;
     bool tile=false;
     int sampling=-1;
     double sampfrac=0.0;

     Identifier id = (*it)->identify();
     Identifier cell_id = (*it)->identify(); //to be replaced by cell_id in tile

     if(calo_dd_man->get_element(id)) {
       CaloCell_ID::CaloSample layer = calo_dd_man->get_element(id)->getSampling();
       sampling = layer; //use CaloCell layer immediately
     } else {
       ATH_MSG_WARNING( "Warning no sampling info for "<<id.getString());
     }

     if(m_larEmID->is_lar_em(id) || m_larHecID->is_lar_hec(id) || m_larFcalID->is_lar_fcal(id)) sampfrac=fSampl->FSAMPL(id);
     if (m_tileID->is_tile(id)) {
       HWIdentifier channel_id = m_tileCabling->s2h_channel_id(id);
       int channel = m_tileHWID->channel(channel_id);
       int drawerIdx = m_tileHWID->drawerIdx(channel_id);
       sampfrac = tileSamplingFraction->getSamplingFraction(drawerIdx, channel);
     }
     if(m_larEmID->is_lar_em(id)) {
       //LAr EM cells
       if (m_larEmID->is_em_barrel(id)) larbarrel=true;
        else if(m_larEmID->is_em_endcap(id)) larendcap=true;
     } else if(m_larHecID->is_lar_hec(id)) {
       //LAr HEC cells
       larhec = true;
     } else if(m_larFcalID->is_lar_fcal(id)) {
       //LAr FCal cells
       larfcal = true;
     } else if (m_tileID->is_tile_aux(id)) {
       // special case for E4'
       tile = true;
       cell_id = m_tileID->cell_id(id);
       sampling = CaloCell_ID::TileGap3;
     } else if(m_tileID->is_tile_barrel(id) || m_tileID->is_tile_extbarrel(id) || m_tileID->is_tile_gap(id)) {
       // all other Tile cells
       tile = true;
       cell_id = m_tileID->cell_id(id);
       Int_t tile_sampling = -1;
       if(calo_dd_man->get_element(cell_id)) {
         tile_sampling = calo_dd_man->get_element(cell_id)->getSampling();
       }
       if(tile_sampling!= -1) sampling = tile_sampling; //calo_dd_man needs to be called with cell_id not pmt_id!!
     } else {
       ATH_MSG_WARNING( "This hit is somewhere. Please check!");
     }

     m_hit_identifier->push_back(id.get_compact());
     m_hit_cellidentifier->push_back(cell_id.get_compact());
     //push things into vectors:
     m_islarbarrel->push_back(larbarrel);
     m_islarendcap->push_back(larendcap);
     m_islarhec->push_back(larhec);
     m_islarfcal->push_back(larfcal);
     m_istile->push_back(tile);
     m_hit_sampling->push_back(sampling);
     m_hit_samplingfraction->push_back(sampfrac);

   } //event steps
 }//event steps read correctly

 // Read the requested number of particles from the first truth event.
 const McEventCollection* mcEvent;
 sc = evtStore()->retrieve(mcEvent,"TruthEvent");
 if(sc.isFailure()) {
   ATH_MSG_WARNING( "No truth event!");
 } else {
   if(mcEvent) {
     if(!mcEvent->empty()) {
       int particleIndex=0;
       int loopEnd = m_NtruthParticles;
       int particles_size=(*mcEvent->begin())->particles_size();
       if(loopEnd==-1) {
         loopEnd = particles_size;
       }
       for (const auto& part: *(*mcEvent->begin()))
       {
         
         ATH_MSG_DEBUG("Number truth particles="<<particles_size<<" loopEnd="<<loopEnd);
         particleIndex++;

         if (particleIndex>loopEnd) break;

         TFCSTruthState truth(part->momentum().px(),part->momentum().py(),part->momentum().pz(),part->momentum().e(),part->pdg_id());

         // Project the momentum direction to the calorimeter boundary.
         TVector3 moment;
         moment.SetXYZ(part->momentum().px(),part->momentum().py(),part->momentum().pz());
         if (moment.Mag2() == 0.) {
           ATH_MSG_WARNING("Skipping truth particle with zero momentum");
           continue;
         }
         TVector3 direction=moment.Unit();

         if(std::abs(direction.Z())/m_CaloBoundaryZ < direction.Perp()/m_CaloBoundaryR) {
           direction*=m_CaloBoundaryR/direction.Perp();
         } else {
           direction*=m_CaloBoundaryZ/std::abs(direction.Z());
         }  

         if((part)->production_vertex()) {
           truth.set_vertex((part)->production_vertex()->position().x(), (part)->production_vertex()->position().y(), (part)->production_vertex()->position().z());
         } else {
           truth.set_vertex(direction.X(),direction.Y(),direction.Z());
           ATH_MSG_WARNING("No production vertex; using the projected position: x "
                           << direction.X() << " y " << direction.Y()
                           << " z " << direction.Z());
         }  
         
         if( std::abs(direction.X()-truth.vertex().X())>0.1 || std::abs(direction.Y()-truth.vertex().Y())>0.1 || std::abs(direction.Z()-truth.vertex().Z())>0.1 ) {
           ATH_MSG_WARNING("VERTEX from direction: x "<<direction.X()<<" y "<<direction.Y()<<" z "<<direction.Z());
           ATH_MSG_WARNING("but VERTEX from hepmc: x "<<truth.vertex().X()<<" y "<<truth.vertex().Y()<<" z "<<truth.vertex().Z());
         }  

         TFCSExtrapolationState result;
         // Build a Geant4 track for the truth particle, transport it through the
         // calorimeter geometry and extrapolate the steps -- same path as the
         // Geant4 FastCaloSim fast-sim model.
         std::unique_ptr<G4Track> g4track = makeG4Track(truth);
         if (g4track) {
           const std::vector<G4FieldTrack> caloSteps =
               m_FastCaloSimParametrizationTool->transport(*g4track);
           m_FastCaloSimParametrizationTool->extrapolate(result, &truth, caloSteps);
         } else {
           ATH_MSG_WARNING("Could not build G4Track for pdgid " << truth.pdgid()
                           << ", skipping extrapolation for this particle");
         }

         ATH_MSG_DEBUG("IDCaloBoundary_eta() "<<result.IDCaloBoundary_eta());
         ATH_MSG_DEBUG("IDCaloBoundary_phi() "<<result.IDCaloBoundary_phi());
         ATH_MSG_DEBUG("IDCaloBoundary_r() "<<result.IDCaloBoundary_r());
         ATH_MSG_DEBUG("IDCaloBoundary_z() "<<result.IDCaloBoundary_z());
         ATH_MSG_DEBUG("AngleEta "<<result.IDCaloBoundary_AngleEta());
         ATH_MSG_DEBUG("Angle3D "<<result.IDCaloBoundary_Angle3D());

         m_newTTC_IDCaloBoundary_eta->push_back(float(result.IDCaloBoundary_eta()));
         m_newTTC_IDCaloBoundary_phi->push_back(float(result.IDCaloBoundary_phi()));
         m_newTTC_IDCaloBoundary_r->push_back(float(result.IDCaloBoundary_r()));
         m_newTTC_IDCaloBoundary_z->push_back(float(result.IDCaloBoundary_z()));
         m_newTTC_Angle3D ->push_back(float(result.IDCaloBoundary_Angle3D()));
         m_newTTC_AngleEta->push_back(float(result.IDCaloBoundary_AngleEta()));

         std::vector<float> eta_vec_ENT;
         std::vector<float> phi_vec_ENT;
         std::vector<float> r_vec_ENT;
         std::vector<float> z_vec_ENT;
         std::vector<float> detaBorder_vec_ENT;
         std::vector<bool>  OK_vec_ENT;

         std::vector<float> eta_vec_EXT;
         std::vector<float> phi_vec_EXT;
         std::vector<float> r_vec_EXT;
         std::vector<float> z_vec_EXT;
         std::vector<float> detaBorder_vec_EXT;
         std::vector<bool>  OK_vec_EXT;

         std::vector<float> eta_vec_MID;
         std::vector<float> phi_vec_MID;
         std::vector<float> r_vec_MID;
         std::vector<float> z_vec_MID;
         std::vector<float> detaBorder_vec_MID;
         std::vector<bool>  OK_vec_MID;

         for(int sample=CaloCell_ID_FCS::FirstSample;sample<CaloCell_ID_FCS::MaxSample;++sample) {
           ATH_MSG_DEBUG("sample "<<sample);
           ATH_MSG_DEBUG(" eta ENT "<<result.eta(sample,1)<<" eta EXT "<<result.eta(sample,2));
           ATH_MSG_DEBUG(" phi ENT "<<result.phi(sample,1)<<" phi EXT "<<result.phi(sample,2));
           ATH_MSG_DEBUG(" r   ENT "<<result.r(sample,1)  <<" r   EXT "<<result.r(sample,2)  );
           ATH_MSG_DEBUG(" z   ENT "<<result.z(sample,1)  <<" z   EXT "<<result.z(sample,2)  );
           ATH_MSG_DEBUG(" detaBorder   ENT "<<result.detaBorder(sample,1)  <<" detaBorder   EXT "<<result.detaBorder(sample,2)  );
           ATH_MSG_DEBUG(" OK  ENT "<<result.OK(sample,1) <<" OK  EXT "<<result.OK(sample,2)  );
           eta_vec_ENT.push_back(float(result.eta(sample,CaloSubPos::SUBPOS_ENT)));
           eta_vec_EXT.push_back(float(result.eta(sample,CaloSubPos::SUBPOS_EXT)));
           eta_vec_MID.push_back(float(result.eta(sample,CaloSubPos::SUBPOS_MID)));
           phi_vec_ENT.push_back(float(result.phi(sample,CaloSubPos::SUBPOS_ENT)));
           phi_vec_EXT.push_back(float(result.phi(sample,CaloSubPos::SUBPOS_EXT)));
           phi_vec_MID.push_back(float(result.phi(sample,CaloSubPos::SUBPOS_MID)));
           r_vec_ENT.push_back(float(result.r(sample,CaloSubPos::SUBPOS_ENT)));
           r_vec_EXT.push_back(float(result.r(sample,CaloSubPos::SUBPOS_EXT)));
           r_vec_MID.push_back(float(result.r(sample,CaloSubPos::SUBPOS_MID)));
           z_vec_ENT.push_back(float(result.z(sample,CaloSubPos::SUBPOS_ENT)));
           z_vec_EXT.push_back(float(result.z(sample,CaloSubPos::SUBPOS_EXT)));
           z_vec_MID.push_back(float(result.z(sample,CaloSubPos::SUBPOS_MID)));
           detaBorder_vec_ENT.push_back(float(result.detaBorder(sample,CaloSubPos::SUBPOS_ENT)));
           detaBorder_vec_EXT.push_back(float(result.detaBorder(sample,CaloSubPos::SUBPOS_EXT)));
           detaBorder_vec_MID.push_back(float(result.detaBorder(sample,CaloSubPos::SUBPOS_MID)));
           OK_vec_ENT.push_back(result.OK(sample,CaloSubPos::SUBPOS_ENT));
           OK_vec_EXT.push_back(result.OK(sample,CaloSubPos::SUBPOS_EXT));
           OK_vec_MID.push_back(result.OK(sample,CaloSubPos::SUBPOS_MID));
         }

         m_newTTC_back_eta->push_back(eta_vec_EXT);
         m_newTTC_back_phi->push_back(phi_vec_EXT);
         m_newTTC_back_r  ->push_back(r_vec_EXT);
         m_newTTC_back_z  ->push_back(z_vec_EXT);
         m_newTTC_back_detaBorder  ->push_back(detaBorder_vec_EXT);
         m_newTTC_back_OK  ->push_back(OK_vec_EXT);
         m_newTTC_entrance_eta->push_back(eta_vec_ENT);
         m_newTTC_entrance_phi->push_back(phi_vec_ENT);
         m_newTTC_entrance_r  ->push_back(r_vec_ENT);
         m_newTTC_entrance_z  ->push_back(z_vec_ENT);
         m_newTTC_entrance_detaBorder  ->push_back(detaBorder_vec_ENT);
         m_newTTC_entrance_OK  ->push_back(OK_vec_ENT);
         m_newTTC_mid_eta->push_back(eta_vec_MID);
         m_newTTC_mid_phi->push_back(phi_vec_MID);
         m_newTTC_mid_r  ->push_back(r_vec_MID);
         m_newTTC_mid_z  ->push_back(z_vec_MID);
         m_newTTC_mid_detaBorder  ->push_back(detaBorder_vec_MID);
         m_newTTC_mid_OK  ->push_back(OK_vec_MID);

         m_truth_energy->push_back((part)->momentum().e());
         m_truth_px->push_back((part)->momentum().px());
         m_truth_py->push_back((part)->momentum().py());
         m_truth_pz->push_back((part)->momentum().pz());
         m_truth_pdg->push_back((part)->pdg_id());
         m_truth_barcode->push_back(HepMC::barcode(part));

       } //for mcevent
     } //mcevent size
   } //mcEvent
 }//truth event

 //Retrieve and save MuonEntryLayer information 
 const TrackRecordCollection *MuonEntry = nullptr;
 sc = evtStore()->retrieve(MuonEntry, "MuonEntryLayer");
 if (sc.isFailure())
 {
 ATH_MSG_WARNING( "Couldn't read MuonEntry from StoreGate");
 //return NULL;
 }
 else{
  for ( const TrackRecord &record : *MuonEntry){
    m_MuonEntryLayer_E->push_back((record).GetEnergy());
    m_MuonEntryLayer_px->push_back((record).GetMomentum().getX());
    m_MuonEntryLayer_py->push_back((record).GetMomentum().getY());
    m_MuonEntryLayer_pz->push_back((record).GetMomentum().getZ());
    m_MuonEntryLayer_x->push_back((record).GetPosition().getX());
    m_MuonEntryLayer_y->push_back((record).GetPosition().getY());
    m_MuonEntryLayer_z->push_back((record).GetPosition().getZ());
    m_MuonEntryLayer_pdg->push_back((record).GetPDGCode());
  }
 }

 // Get the reco clusters if available
// retreiving cluster container
 const xAOD::CaloClusterContainer* theClusters;
 std::string clusterContainerName = "CaloCalTopoClusters";  //Local hadron calibrated Topo-clusters , raw is the EM scale
  sc = evtStore()->retrieve(theClusters, clusterContainerName);
  if (sc.isFailure()) {
    ATH_MSG_WARNING(" Couldn't get cluster container '" << clusterContainerName << "'");
    return StatusCode::SUCCESS; 
  }
  xAOD::CaloClusterContainer::const_iterator itrClus = theClusters->begin();
  xAOD::CaloClusterContainer::const_iterator itrLastClus = theClusters->end();
  for ( ; itrClus!=itrLastClus; ++itrClus){
    const xAOD::CaloCluster *cluster =(*itrClus);
    m_cluster_energy->push_back(cluster->e(xAOD::CaloCluster::UNCALIBRATED)); // getRawE, cluster->e() is the Local hadron calibrated topo-clusters
    m_cluster_eta->push_back(cluster->eta(xAOD::CaloCluster::UNCALIBRATED));
    m_cluster_phi->push_back(cluster->phi(xAOD::CaloCluster::UNCALIBRATED));
    ATH_MSG_VERBOSE("Cluster energy: " << cluster->e() << " EMscale: " << cluster->e(xAOD::CaloCluster::UNCALIBRATED) << " cells: " << " links: " << cluster->getCellLinks());

    const CaloClusterCellLink* cellLinks = cluster->getCellLinks();
    if (!cellLinks) {
      ATH_MSG_DEBUG( "No cell links for this cluster"  );
      continue;
    }

    const CaloCellContainer* cellCont=cellLinks->getCellContainer();
    if (!cellCont) {
      ATH_MSG_DEBUG( "DataLink to cell container is broken"  );
      continue;
    }
    unsigned cellcount = 0;
    std::vector<Long64_t> cellIDs_in_cluster;
    xAOD::CaloCluster::const_cell_iterator cellIter =cluster->cell_begin();
    xAOD::CaloCluster::const_cell_iterator cellIterEnd =cluster->cell_end();
    for ( ;cellIter !=cellIterEnd;cellIter++) {
      ++cellcount;
      const CaloCell* cell= (*cellIter);
      cellIDs_in_cluster.push_back(cell->ID().get_compact());
      float EnergyCell=cell->energy(); //ID, time, phi, eta
      ATH_MSG_DEBUG("   Cell energy: " << EnergyCell);
    }// end of cells inside cluster loop
    m_cluster_size->push_back(cellcount);
    m_cluster_cellID->push_back(cellIDs_in_cluster);
  }

 //Get reco cells if available
 const CaloCellContainer *cellColl = nullptr;
 sc = evtStore()->retrieve(cellColl, "AllCalo");

 if (sc.isFailure())
 {
   ATH_MSG_WARNING( "Couldn't read AllCalo cells from StoreGate");
 }
 else
 {
  ATH_MSG_INFO( "Found: "<<cellColl->size()<<" calorimeter cells");
  CaloCellContainer::const_iterator itrCell = cellColl->begin();
  CaloCellContainer::const_iterator itrLastCell = cellColl->end();
  for ( ; itrCell!=itrLastCell; ++itrCell)
  {
    m_cell_energy->push_back((*itrCell)->energy());
    m_cell_identifier->push_back((*itrCell)->ID().get_compact());
    if (m_tileID->is_tile_aux((*itrCell)->ID())) {
      // special case for E4'
      m_cell_sampling->push_back(CaloCell_ID::TileGap3);
    }
    else if (calo_dd_man->get_element((*itrCell)->ID()))
    {
    // all other Tile cells
    CaloCell_ID::CaloSample layer = calo_dd_man->get_element((*itrCell)->ID())->getSampling();
    m_cell_sampling->push_back(layer);
    }
    else
    m_cell_sampling->push_back(-1);
  }
 } //calorimeter cells

 //Get all G4Hits (from CaloHitAnalysis)
 std::string  lArKey [4] = {"LArHitEMB", "LArHitEMEC", "LArHitFCAL", "LArHitHEC"};
 for (unsigned int i=0;i<4;i++)
 {
  const LArHitContainer* iter;
  ATH_MSG_DEBUG( "Checking G4Hits: "<<lArKey[i]);
  if(evtStore()->retrieve(iter,lArKey[i])==StatusCode::SUCCESS)
  {
    LArHitContainer::const_iterator hi;
    int hitnumber = 0;
    for (hi=(*iter).begin();hi!=(*iter).end();++hi) {
      hitnumber++;
      const LArHit* larHit = *hi;
      const CaloDetDescrElement *hitElement = calo_dd_man->get_element(larHit->cellID());
      if(!hitElement)
        continue;
      Identifier larhitid = hitElement->identify();
      if(calo_dd_man->get_element(larhitid)) {
      CaloCell_ID::CaloSample larlayer = calo_dd_man->get_element(larhitid)->getSampling();

      float larsampfrac=fSampl->FSAMPL(larhitid);
      m_g4hit_energy->push_back( larHit->energy() );
      m_g4hit_time->push_back( larHit->time() );
      m_g4hit_identifier->push_back( larhitid.get_compact() );
      m_g4hit_cellidentifier->push_back( larhitid.get_compact() );
      m_g4hit_sampling->push_back( larlayer);
      m_g4hit_samplingfraction->push_back( larsampfrac );
      }
    } // End while LAr hits
    ATH_MSG_INFO( "Read "<<hitnumber<<" G4Hits from "<<lArKey[i]);
  }
  else
  {
         ATH_MSG_INFO( "Can't retrieve LAr hits");
  }// End statuscode success upon retrieval of hits
  //std::cout <<"ZH G4Hit size: "<<m_g4hit_e->size()<<std::endl;
 }// End detector type loop

 const TileHitVector * hitVec = nullptr;
 if (evtStore()->retrieve(hitVec,"TileHitVec")==StatusCode::SUCCESS &&  m_tileMgr &&  m_tileID )
 {
  int hitnumber = 0;
  for(TileHitVecConstIterator i_hit=hitVec->begin() ; i_hit!=hitVec->end() ; ++i_hit)
  {
   hitnumber++;
   Identifier pmt_id = (*i_hit).identify();
   Identifier cell_id = m_tileID->cell_id(pmt_id);

   if (calo_dd_man->get_element(cell_id)){
      CaloCell_ID::CaloSample layer = calo_dd_man->get_element(cell_id)->getSampling();

      HWIdentifier channel_id = m_tileCabling->s2h_channel_id(pmt_id);
      int channel = m_tileHWID->channel(channel_id);
      int drawerIdx = m_tileHWID->drawerIdx(channel_id);
      float tilesampfrac = tileSamplingFraction->getSamplingFraction(drawerIdx, channel);

      //could there be more subhits??
      for (int tilesubhit_i = 0; tilesubhit_i<(*i_hit).size(); tilesubhit_i++)
      {
        m_g4hit_energy->push_back( (*i_hit).energy(tilesubhit_i) );
        m_g4hit_time->push_back(   (*i_hit).time(tilesubhit_i)   );
        m_g4hit_identifier->push_back( pmt_id.get_compact() );
        m_g4hit_cellidentifier->push_back( cell_id.get_compact() );
        m_g4hit_sampling->push_back( layer );
        m_g4hit_samplingfraction->push_back( tilesampfrac );
      }
    }
  }
  ATH_MSG_INFO( "Read "<<hitnumber<<" G4Hits from TileHitVec");
 }


  ATH_MSG_DEBUG("Building matched calorimeter-hit data");

  for (unsigned int cell_i = 0; cell_i < m_cell_identifier->size(); cell_i++){
    if (cells.find((*m_cell_identifier)[cell_i]) == cells.end()) {
      one_cell.cell_identifier = (*m_cell_identifier)[cell_i];
      one_cell.sampling = (*m_cell_sampling)[cell_i];
      one_cell.energy = (*m_cell_energy)[cell_i];
      one_cell.center_x = 0.0;
      one_cell.center_y = 0.0;
      one_cell.center_z = 0.0;
      cells.insert(std::pair<Long64_t, FCS_cell>(one_cell.cell_identifier, one_cell));
    }
    else
    {
      ATH_MSG_DEBUG("Duplicate cell identifier " << (*m_cell_identifier)[cell_i]);
    }
  }

  if(m_doG4Hits){
    for (unsigned int g4hit_i = 0; g4hit_i < m_g4hit_identifier->size(); g4hit_i++)
    {
      if ((*m_g4hit_sampling)[g4hit_i] >= 0 && (*m_g4hit_sampling)[g4hit_i] <= 25 && (*m_g4hit_time)[g4hit_i] > m_TimingCut)
      {
        ATH_MSG_DEBUG("Ignoring G4hit, time too large: " << g4hit_i << " time: " << (*m_g4hit_time)[g4hit_i]);
        continue;
      }

      if (g4hits.find((*m_g4hit_cellidentifier)[g4hit_i]) == g4hits.end())
      {
        one_g4hit.identifier = (*m_g4hit_identifier)[g4hit_i];
        one_g4hit.cell_identifier = (*m_g4hit_cellidentifier)[g4hit_i];
        one_g4hit.sampling = (*m_g4hit_sampling)[g4hit_i];
        one_g4hit.hit_time = (*m_g4hit_time)[g4hit_i];
        // Convert the hit energy using the detector sampling fraction.
        if (one_g4hit.sampling >= 12 && one_g4hit.sampling <= 20)
        {
          if ((*m_g4hit_samplingfraction)[g4hit_i])
          {
            one_g4hit.hit_energy = (*m_g4hit_energy)[g4hit_i] * (*m_g4hit_samplingfraction)[g4hit_i];
          }
          else one_g4hit.hit_energy = 0.;
        }
        else
        {
          one_g4hit.hit_energy = (*m_g4hit_energy)[g4hit_i] / (*m_g4hit_samplingfraction)[g4hit_i];
        }
        g4hits.insert(std::pair<Long64_t, std::vector<FCS_g4hit> >(one_g4hit.cell_identifier, std::vector<FCS_g4hit>(1, one_g4hit)));
      }
      else
      {
        one_g4hit.identifier = (*m_g4hit_identifier)[g4hit_i];
        one_g4hit.cell_identifier = (*m_g4hit_cellidentifier)[g4hit_i];
        one_g4hit.sampling = (*m_g4hit_sampling)[g4hit_i];
        one_g4hit.hit_time = (*m_g4hit_time)[g4hit_i];
        if (one_g4hit.sampling >= 12 && one_g4hit.sampling <= 20)
        {
          if ((*m_g4hit_samplingfraction)[g4hit_i])
          {
            one_g4hit.hit_energy = (*m_g4hit_energy)[g4hit_i] * (*m_g4hit_samplingfraction)[g4hit_i];
          }
          else one_g4hit.hit_energy = 0.;
        }
        else
        {
          one_g4hit.hit_energy = (*m_g4hit_energy)[g4hit_i] / (*m_g4hit_samplingfraction)[g4hit_i];
        }
        g4hits[(*m_g4hit_cellidentifier)[g4hit_i]].push_back(one_g4hit);
      }
    }
  }

  for (unsigned int hit_i = 0; hit_i < m_hit_identifier->size(); hit_i++)
  {
    if ((*m_hit_sampling)[hit_i] >= 0 && (*m_hit_sampling)[hit_i] <= 25 && (*m_hit_time)[hit_i] > m_TimingCut)
    {
      ATH_MSG_DEBUG("Ignoring FCS hit, time too large: " << hit_i << " time: " << (*m_hit_time)[hit_i]);
      continue;
    }
    if (hits.find((*m_hit_cellidentifier)[hit_i]) == hits.end())
    {
      one_hit.identifier = (*m_hit_identifier)[hit_i];
      one_hit.cell_identifier = (*m_hit_cellidentifier)[hit_i];
      one_hit.sampling = (*m_hit_sampling)[hit_i];

      if (one_hit.sampling >= 12 && one_hit.sampling <= 20)
      {
        if ((*m_hit_samplingfraction)[hit_i])
        {
          one_hit.hit_energy = (*m_hit_energy)[hit_i] * (*m_hit_samplingfraction)[hit_i];
        }
        else one_hit.hit_energy = 0.;
      }
      else
      {
        one_hit.hit_energy = (*m_hit_energy)[hit_i] / (*m_hit_samplingfraction)[hit_i];
      }
      //one_hit.hit_sampfrac = (*m_hit_samplingfraction)[hit_i];
      one_hit.hit_time = (*m_hit_time)[hit_i];
      one_hit.hit_x = (*m_hit_x)[hit_i];
      one_hit.hit_y = (*m_hit_y)[hit_i];
      one_hit.hit_z = (*m_hit_z)[hit_i];
      hits.insert(std::pair<Long64_t, std::vector<FCS_hit> >(one_hit.cell_identifier, std::vector<FCS_hit>(1, one_hit)));
    }
    else
    {
      one_hit.identifier = (*m_hit_identifier)[hit_i];
      one_hit.cell_identifier = (*m_hit_cellidentifier)[hit_i];
      one_hit.sampling = (*m_hit_sampling)[hit_i];
      //one_hit.hit_energy = (*m_hit_energy)[hit_i];
      if (one_hit.sampling >= 12 && one_hit.sampling <= 20)
      { //tile
        if ((*m_hit_samplingfraction)[hit_i])
        {
          one_hit.hit_energy = (*m_hit_energy)[hit_i] * (*m_hit_samplingfraction)[hit_i];
        }
        else one_hit.hit_energy = 0.;
      }
      else
      {
        one_hit.hit_energy = (*m_hit_energy)[hit_i] / (*m_hit_samplingfraction)[hit_i];
      }
      //one_hit.hit_sampfrac = (*m_hit_samplingfraction)[hit_i];
      one_hit.hit_time = (*m_hit_time)[hit_i];
      one_hit.hit_x = (*m_hit_x)[hit_i];
      one_hit.hit_y = (*m_hit_y)[hit_i];
      one_hit.hit_z = (*m_hit_z)[hit_i];
      hits[(*m_hit_cellidentifier)[hit_i]].push_back(one_hit);
    }
  }

  //Start matching:
  for (std::map<Long64_t, FCS_cell>::iterator it = cells.begin(); it != cells.end(); )
  {
    one_matchedcell.clear(); //maybe not completely necessery, as we're not pushing_back into vectors
    //set the cell part
    one_matchedcell.cell = it->second;
    //now look for FCS detailed hits in this cell
    std::map<Long64_t, std::vector<FCS_hit> >::iterator it2 = hits.find(it->first);
    if (it2 != hits.end())
    {
      //std::cout <<"FCS hits found in this cell"<<std::endl;
      one_matchedcell.hit = it2->second;
      hits.erase(it2); //remove it
    }
    else
    {
      //no hit found for this cell
      one_matchedcell.hit.clear(); //important!
    }
    //now look for G4hits in this cell
    std::map<Long64_t, std::vector<FCS_g4hit> >::iterator it3 = g4hits.find(it->first);
    if (it3 != g4hits.end())
    {
      one_matchedcell.g4hit = it3->second;
      g4hits.erase(it3);
    }
    else
    {
      //no g4hit found for this cell
      one_matchedcell.g4hit.clear();//important!
    }
    cells.erase(it++);
    //push_back matched cell for event jentry
    m_oneeventcells->push_back(one_matchedcell);
  }

  //ok, cells should be empty, what about hits and g4hits?
  //There could be G4hits/FCS hits for which we don't have a cell ->create a dummy empty cell with 0 energy, take the cell identifier from the hit
  ATH_MSG_DEBUG("ISF_HitAnalysis Check after cells: " << cells.size() << " " << g4hits.size() << " " << hits.size());

  for (std::map<Long64_t, std::vector<FCS_hit> >::iterator it = hits.begin(); it != hits.end();)
  {
    one_matchedcell.clear();
    one_matchedcell.cell.cell_identifier = it->first;
    //std::cout <<"This hit didn't exist in cell: "<<it->first<<std::endl;
    if (!it->second.empty())
    {
      one_matchedcell.cell.sampling = (it->second)[0].sampling;
    }
    else
    {
      one_matchedcell.cell.sampling = -1; //
      //ok, but you really shouldn't be here
      ATH_MSG_DEBUG("ERROR: You shouldn't really be here");
    }
    one_matchedcell.cell.energy = 0.;
    one_matchedcell.cell.center_x = 0.0;
    one_matchedcell.cell.center_y = 0.0;
    one_matchedcell.cell.center_z = 0.0;
    one_matchedcell.hit = it->second;
    std::map<Long64_t, std::vector<FCS_g4hit> >::iterator it3 = g4hits.find(it->first);
    if (it3 != g4hits.end())
    {
      one_matchedcell.g4hit = it3->second;
      g4hits.erase(it3);
    }
    else
    {
      //no g4hit found for this cell
      one_matchedcell.g4hit.clear(); //important!
    }
    hits.erase(it++);
    m_oneeventcells->push_back(one_matchedcell);

  }

  //ok, hits should be empty, what about g4hits?
  ATH_MSG_DEBUG("ISF_HitAnalysis Check after hits: " << cells.size() << " " << g4hits.size() << " " << hits.size());
  for (std::map<Long64_t, std::vector<FCS_g4hit> >::iterator it = g4hits.begin(); it != g4hits.end();)
  {
    one_matchedcell.clear(); //maybe not so important
    one_matchedcell.cell.cell_identifier = it->first;
    if (!it->second.empty())
    {
      one_matchedcell.cell.sampling = (it->second)[0].sampling;
    }
    else
    {
      one_matchedcell.cell.sampling = -1; //
      //not really
      ATH_MSG_DEBUG("ERROR: You shouldn't really be here");
    }
    one_matchedcell.cell.energy = 0.;
    one_matchedcell.cell.center_x = 0.0;
    one_matchedcell.cell.center_y = 0.0;
    one_matchedcell.cell.center_z = 0.0;
    one_matchedcell.g4hit = it->second;
    one_matchedcell.hit.clear(); //important!!
    g4hits.erase(it++);
    m_oneeventcells->push_back(one_matchedcell);
  }

  //Can fill the output tree already here:
  m_total_cell_e  = 0;
  m_total_hit_e   = 0;
  m_total_g4hit_e = 0;

  for (int j = 0; j < MAX_LAYER - 1; j++)
  {
    m_layercells[j]->m_vector = m_oneeventcells->GetLayer(j);
  }

  //this is for invalid cells
  m_layercells[MAX_LAYER - 1]->m_vector = m_oneeventcells->GetLayer(-1);
  for (int i = 0; i < MAX_LAYER; i++)
  {
    m_final_cell_energy->push_back(0.0); //zero for each event!
    m_final_hit_energy->push_back(0.0);
    m_final_g4hit_energy->push_back(0.0);

    for (unsigned int cellindex = 0; cellindex < m_layercells[i]->size(); cellindex++)
    {
      if (i != MAX_LAYER - 1)
      {
        m_final_cell_energy->at(i) += m_layercells[i]->m_vector.at(cellindex).cell.energy;
        m_total_cell_e += m_layercells[i]->m_vector.at(cellindex).cell.energy;
      }
      else
      {
        //don't add the energy in the invalid layer to the total energy (if there is any (shouldn't)
        m_final_cell_energy->at(i) += m_layercells[i]->m_vector.at(cellindex).cell.energy; //this should be here anyway
      }

      //sum energy of all FCS detailed hits in this layer/cell
      for (unsigned int j = 0; j < m_layercells[i]->m_vector.at(cellindex).hit.size(); j++)
      {
        if (i != MAX_LAYER - 1)
        {
          m_total_hit_e += m_layercells[i]->m_vector.at(cellindex).hit[j].hit_energy;
          m_final_hit_energy->at(i) += m_layercells[i]->m_vector.at(cellindex).hit[j].hit_energy;
        }
        else
        {
          //again, don't add invalid layer energy to the sum
          m_final_hit_energy->at(i) += m_layercells[i]->m_vector.at(cellindex).hit[j].hit_energy;
        }
      }

      //sum energy of all G4 hits in this layer/cell
      for (unsigned int j = 0; j < m_layercells[i]->m_vector.at(cellindex).g4hit.size(); j++)
      {
        if (i != MAX_LAYER - 1)
        {
          m_total_g4hit_e += m_layercells[i]->m_vector.at(cellindex).g4hit[j].hit_energy;
          m_final_g4hit_energy->at(i) += m_layercells[i]->m_vector.at(cellindex).g4hit[j].hit_energy;
        }
        else
        {
          //don't add invalied layer energy to the sum
          m_final_g4hit_energy->at(i) += m_layercells[i]->m_vector.at(cellindex).g4hit[j].hit_energy;
        }
      }
    }
  }

  // push_back for total energy
  m_final_cell_energy->push_back(0.0); 
  m_final_hit_energy->push_back(0.0);
  m_final_g4hit_energy->push_back(0.0);

  m_final_cell_energy->at(MAX_LAYER)  = m_total_cell_e;
  m_final_hit_energy->at(MAX_LAYER)   = m_total_hit_e;
  m_final_g4hit_energy->at(MAX_LAYER) = m_total_g4hit_e;

 //Fill the tree and finish
 if (m_tree) m_tree->Fill();

 return StatusCode::SUCCESS;

} //execute
