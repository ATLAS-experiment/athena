/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FASTCALOSIMPARAMETRIZATION_ISF_HIT_ANALYSIS_H
#define ISF_FASTCALOSIMPARAMETRIZATION_ISF_HIT_ANALYSIS_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaKernel/IOVSvcDefs.h"

#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "LArElecCalib/ILArfSampl.h"
#include "TileConditions/TileSamplingFraction.h"
#include "TileConditions/TileCablingSvc.h"
#include "CaloDetDescr/ICaloCoordinateTool.h"

#include "ISF_FastCaloSimParametrization/IFastCaloSimCaloExtrapolation.h"
#include "ISF_FastCaloSimEvent/FastCaloSim_CaloCell_ID.h"
#include "ISF_FastCaloSimParametrization/FCS_Cell.h"
#include "ISF_FastCaloSimParametrization/FSmap.h"

#include "TrkExInterfaces/ITimedExtrapolator.h"
#include "TrkEventPrimitives/PdgToParticleHypothesis.h"
#include "TrkParameters/TrackParameters.h"

#include "StoreGate/ReadCondHandle.h"

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/IPartPropSvc.h"

#include "CxxUtils/checker_macros.h"
#include "CxxUtils/CachedPointer.h"

#include "AtlasHepMC/GenParticle.h"
#include "HepPDT/ParticleDataTable.hh"

#include "CLHEP/Units/SystemOfUnits.h"
#include <TLorentzVector.h>
#include <Rtypes.h>
#include <string>

/* *************************************************************
   This is a modified copy of Simulation/Tools/CaloHitAnalysis
   Aug 27, 2013 Zdenek Hubacek (CERN)
   ************************************************************** */

namespace Trk
{
  class TrackingVolume;
}

class TileID;
class TileHWID;
class TileDetDescrManager;
class TTree;
class LArEM_ID;
class LArFCAL_ID;
class LArHEC_ID;

//############################

class ISF_HitAnalysis : public AthAlgorithm {

public:

  ISF_HitAnalysis(const std::string& name, ISvcLocator* pSvcLocator);
  ~ISF_HitAnalysis();

  virtual StatusCode initialize ATLAS_NOT_THREAD_SAFE () override;
  virtual StatusCode finalize ATLAS_NOT_THREAD_SAFE () override;
  virtual StatusCode execute() override;
  StatusCode updateMetaData(IOVSVC_CALLBACK_ARGS);

  const static int MAX_LAYER = 25;

private:
  // extrapolation through Calo
  std::vector<Trk::HitInfo>* caloHits(const HepMC::GenParticle& part ) const;
  void extrapolate(const HepMC::ConstGenParticlePtr&  part,std::vector<Trk::HitInfo>* hitVector);
  void extrapolate_to_ID(const HepMC::ConstGenParticlePtr&  part,std::vector<Trk::HitInfo>* hitVector);

  // Configurable properties
  ServiceHandle<IGeoModelSvc> m_geoModel{this,
      "GeoModelSvc", "GeoModelSvc", "GeoModel service"};
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey { this
      , "CaloDetDescrManager"
      , "CaloDetDescrManager"
      , "SG Key for CaloDetDescrManager in the Condition Store" };
  SG::ReadCondHandleKey<ILArfSampl> m_fSamplKey{this,
      "fSamplKey", "LArfSamplSym", "SG Key of LArfSampl object"};
  /**
   * @brief Name of TileSamplingFraction in condition store
   */
  SG::ReadCondHandleKey<TileSamplingFraction> m_tileSamplingFractionKey{this,
      "TileSamplingFraction", "TileSamplingFraction", "Input Tile sampling fraction"};
  /**
   * @brief Name of Tile cabling service
   */
  ServiceHandle<TileCablingSvc> m_tileCablingSvc{ this,
      "TileCablingSvc", "TileCablingSvc", "Tile cabling service"};
  StringProperty m_ntupleFileName{this, "NtupleFileName", "ISF_HitAnalysis"};
  StringProperty m_ntupleTreeName{this, "NtupleTreeName", "CaloHitAna"};
  StringProperty m_metadataTreeName{this, "MetadataTreeName", "MetaData"};
  StringProperty m_geoFileName{this, "GeoFileName", "ISF_Geometry"};
  IntegerProperty m_NtruthParticles{this, "NTruthParticles", 1, "Number of truth particles saved from the truth collection, -1 to save all"};
  ServiceHandle<ITHistSvc> m_thistSvc{ this,
      "THistSvc", "THistSvc", "Tile histogramming service"};
  PublicToolHandle<Trk::ITimedExtrapolator>  m_extrapolator{this, "Extrapolator", {}};
  PublicToolHandle<ICaloCoordinateTool> m_calo_tb_coord{this, "CaloCoordinateTool", "TBCaloCoordinate"};
  StringProperty m_caloEntranceName{this, "CaloEntrance", ""};
  /** The FastCaloSimCaloExtrapolation tool */
  PublicToolHandle<IFastCaloSimCaloExtrapolation> m_FastCaloSimCaloExtrapolation{this, "FastCaloSimCaloExtrapolation", {} };
  DoubleProperty m_CaloBoundaryR{this, "CaloBoundaryR", 1148};
  DoubleProperty m_CaloBoundaryZ{this, "CaloBoundaryZ", 3550};
  DoubleProperty m_calomargin{this, "CaloMargin", 0.0};
  BooleanProperty m_saveAllBranches{this,"SaveAllBranches",  false};
  BooleanProperty m_doAllCells{this, "DoAllCells", false};
  BooleanProperty m_doClusterInfo{this, "DoClusterInfo", false};
  BooleanProperty m_doLayers{this, "DoLayers", false};
  BooleanProperty m_doLayerSums{this, "DoLayerSums", true};
  BooleanProperty m_doG4Hits{this, "DoG4Hits", false};
  IntegerProperty m_TimingCut{this, "TimingCut", 999999};
  StringProperty m_MC_DIGI_PARAM{this, "MetaDataDigi", "/Digitization/Parameters"};
  StringProperty m_MC_SIM_PARAM{this, "MetaDataSim", "/Simulation/Parameters"};

  const LArEM_ID *m_larEmID{};
  const LArFCAL_ID *m_larFcalID{};
  const LArHEC_ID *m_larHecID{};
  const TileID * m_tileID{};
  const TileHWID*     m_tileHWID{};
  const TileCablingService* m_tileCabling{};

  const TileDetDescrManager * m_tileMgr{};

  /** Simple variables by Ketevi */
  std::vector<float>* m_hit_x{};
  std::vector<float>* m_hit_y{};
  std::vector<float>* m_hit_z{};
  std::vector<float>* m_hit_energy{};
  std::vector<float>* m_hit_time{};
  std::vector<Long64_t>* m_hit_identifier{};
  std::vector<Long64_t>* m_hit_cellidentifier{};
  std::vector<bool>*  m_islarbarrel{};
  std::vector<bool>*  m_islarendcap{};
  std::vector<bool>*  m_islarhec{};
  std::vector<bool>*  m_islarfcal{};
  std::vector<bool>*  m_istile{};
  std::vector<int>*   m_hit_sampling{};
  std::vector<float>* m_hit_samplingfraction{};

  std::vector<float>* m_truth_energy{};
  std::vector<float>* m_truth_px{};
  std::vector<float>* m_truth_py{};
  std::vector<float>* m_truth_pz{};
  std::vector<int>*   m_truth_pdg{};
  std::vector<int>*   m_truth_barcode{};
  std::vector<int>*   m_truth_vtxbarcode{}; //production vertex barcode

  std::vector<float>* m_cluster_energy{};
  std::vector<float>* m_cluster_eta{};
  std::vector<float>* m_cluster_phi{};
  std::vector<unsigned>* m_cluster_size{};
  std::vector<std::vector<Long64_t >>* m_cluster_cellID{};


  std::vector<Long64_t>*    m_cell_identifier{};
  std::vector<float>*       m_cell_energy{};
  std::vector<int>*         m_cell_sampling{};

  std::vector<float>*       m_g4hit_energy{};
  std::vector<float>*       m_g4hit_time{};
  std::vector<Long64_t>*    m_g4hit_identifier{};
  std::vector<Long64_t>*    m_g4hit_cellidentifier{};
  std::vector<float>*       m_g4hit_samplingfraction{};
  std::vector<int>*         m_g4hit_sampling{};

  //CaloHitAna variables
  FCS_matchedcellvector* m_oneeventcells = nullptr; //these are all matched cells in a single event
  FCS_matchedcellvector* m_layercells[MAX_LAYER]{}; //these are all matched cells in a given layer in a given event

  Float_t m_total_cell_e{};
  Float_t m_total_hit_e{};
  Float_t m_total_g4hit_e{};

  std::vector<Float_t>* m_final_cell_energy{};
  std::vector<Float_t>* m_final_hit_energy{};
  std::vector<Float_t>* m_final_g4hit_energy{};

  TTree * m_tree{};
  //####################################################
  double m_eta_calo_surf{};
  double m_phi_calo_surf{};
  double m_d_calo_surf{};
  double m_ptruth_eta{};
  double m_ptruth_phi{};
  double m_ptruth_e{};
  double m_ptruth_et{};
  double m_ptruth_pt{};
  double m_ptruth_p{};
  int m_pdgid{};

  std::vector<std::vector<float> >* m_newTTC_entrance_eta{};
  std::vector<std::vector<float> >* m_newTTC_entrance_phi{};
  std::vector<std::vector<float> >* m_newTTC_entrance_r{};
  std::vector<std::vector<float> >* m_newTTC_entrance_z{};
  std::vector<std::vector<float> >* m_newTTC_entrance_detaBorder{};
  std::vector<std::vector<bool> >*  m_newTTC_entrance_OK{};
  std::vector<std::vector<float> >* m_newTTC_back_eta{};
  std::vector<std::vector<float> >* m_newTTC_back_phi{};
  std::vector<std::vector<float> >* m_newTTC_back_r{};
  std::vector<std::vector<float> >* m_newTTC_back_z{};
  std::vector<std::vector<float> >* m_newTTC_back_detaBorder{};
  std::vector<std::vector<bool> >*  m_newTTC_back_OK{};
  std::vector<std::vector<float> >* m_newTTC_mid_eta{};
  std::vector<std::vector<float> >* m_newTTC_mid_phi{};
  std::vector<std::vector<float> >* m_newTTC_mid_r{};
  std::vector<std::vector<float> >* m_newTTC_mid_z{};
  std::vector<std::vector<float> >* m_newTTC_mid_detaBorder{};
  std::vector<std::vector<bool> >*  m_newTTC_mid_OK{};
  std::vector<float>* m_newTTC_IDCaloBoundary_eta{};
  std::vector<float>* m_newTTC_IDCaloBoundary_phi{};
  std::vector<float>* m_newTTC_IDCaloBoundary_r{};
  std::vector<float>* m_newTTC_IDCaloBoundary_z{};
  std::vector<float>* m_newTTC_Angle3D{};
  std::vector<float>* m_newTTC_AngleEta{};

  std::vector<float>* m_MuonEntryLayer_E{};
  std::vector<float>* m_MuonEntryLayer_px{};
  std::vector<float>* m_MuonEntryLayer_py{};
  std::vector<float>* m_MuonEntryLayer_pz{};
  std::vector<float>* m_MuonEntryLayer_x{};
  std::vector<float>* m_MuonEntryLayer_y{};
  std::vector<float>* m_MuonEntryLayer_z{};
  std::vector<int>*   m_MuonEntryLayer_pdg{};

  /** The new Extrapolator setup */
  CxxUtils::CachedPointer<const Trk::TrackingVolume> m_caloEntrance;
  // extrapolation through Calo
  Trk::PdgToParticleHypothesis        m_pdgToParticleHypothesis;
  /** End new Extrapolator setup */

  CaloCell_ID_FCS::CaloSample    m_sample_calo_surf{CaloCell_ID_FCS::noSample};
  std::vector< CaloCell_ID_FCS::CaloSample > m_surfacelist;

  bool   m_layerCaloOK[CaloCell_ID_FCS::MaxSample][3]{};
  double m_letaCalo[CaloCell_ID_FCS::MaxSample][3]{};
  double m_lphiCalo[CaloCell_ID_FCS::MaxSample][3]{};
  double m_lrCalo[CaloCell_ID_FCS::MaxSample][3]{};
  double m_lzCalo[CaloCell_ID_FCS::MaxSample][3]{};
  double m_dCalo[CaloCell_ID_FCS::MaxSample][3]{};
  double m_distetaCaloBorder[CaloCell_ID_FCS::MaxSample][3]{};

  /// Handle on the particle property service
  ServiceHandle<IPartPropSvc> m_partPropSvc{this, "PartPropSvc", "PartPropSvc"};

  HepPDT::ParticleDataTable*     m_particleDataTable{};

  //###################################################################


};

#endif // ISF_HIT_ANALYSIS_H
