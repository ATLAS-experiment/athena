/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_FASTCALOSIMPARAMETRIZATION_ISF_HIT_ANALYSIS_H
#define ISF_FASTCALOSIMPARAMETRIZATION_ISF_HIT_ANALYSIS_H

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/Algorithm.h"
#include "GaudiKernel/ObjectVector.h"
#include "GaudiKernel/ITHistSvc.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "StoreGate/ReadCondHandle.h"

#include "AthenaBaseComps/AthAlgorithm.h"
#include "CxxUtils/checker_macros.h"
#include "CxxUtils/CachedPointer.h"
#include "LArElecCalib/ILArfSampl.h"
#include "GeoModelInterfaces/IGeoModelSvc.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "TileConditions/TileSamplingFraction.h"
#include "TileConditions/TileCablingSvc.h"

//#####################################
#include "CaloDetDescr/ICaloCoordinateTool.h"
#include "ISF_FastCaloSimParametrization/FSmap.h"
#include "AtlasHepMC/GenParticle.h"
#include "HepPDT/ParticleData.hh"
#include "GaudiKernel/IPartPropSvc.h"
#include "TrkParameters/TrackParameters.h"
//#####################################

#include "ISF_FastCaloSimParametrization/IFastCaloSimCaloExtrapolation.h"
#include "ISF_FastCaloSimEvent/FastCaloSim_CaloCell_ID.h"
#include "ISF_FastCaloSimParametrization/FCS_Cell.h"


namespace Trk
{
  class TrackingVolume;
}

#include "TrkExInterfaces/ITimedExtrapolator.h"
#include "TrkEventPrimitives/PdgToParticleHypothesis.h"

#include <string>
#include <Rtypes.h>
#include <TLorentzVector.h>

/* *************************************************************
   This is a modified copy of Simulation/Tools/CaloHitAnalysis
   Aug 27, 2013 Zdenek Hubacek (CERN)
   ************************************************************** */

class TileID;
class TileHWID;
class TileDetDescrManager;
class TTree;
class LArEM_ID;
class LArFCAL_ID;
class LArHEC_ID;

//############################
class ICaloCoordinateTool;
class IExtrapolateToCaloTool;
class CaloDepthTool;
namespace Trk {
  class IExtrapolator;
}

class ISF_HitAnalysis : public AthAlgorithm {

public:

  ISF_HitAnalysis(const std::string& name, ISvcLocator* pSvcLocator);
  ~ISF_HitAnalysis();

  virtual StatusCode initialize ATLAS_NOT_THREAD_SAFE () override;
  virtual StatusCode finalize ATLAS_NOT_THREAD_SAFE () override;
  virtual StatusCode execute() override;

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

  const LArEM_ID *m_larEmID{nullptr};
  const LArFCAL_ID *m_larFcalID{nullptr};
  const LArHEC_ID *m_larHecID{nullptr};
  const TileID * m_tileID{nullptr};
  const TileHWID*     m_tileHWID{nullptr};
  const TileCablingService* m_tileCabling{nullptr};

  const TileDetDescrManager * m_tileMgr{nullptr};

  /** Simple variables by Ketevi */
  std::vector<float>* m_hit_x{nullptr};
  std::vector<float>* m_hit_y{nullptr};
  std::vector<float>* m_hit_z{nullptr};
  std::vector<float>* m_hit_energy{nullptr};
  std::vector<float>* m_hit_time{nullptr};
  std::vector<Long64_t>* m_hit_identifier{nullptr};
  std::vector<Long64_t>* m_hit_cellidentifier{nullptr};
  std::vector<bool>*  m_islarbarrel{nullptr};
  std::vector<bool>*  m_islarendcap{nullptr};
  std::vector<bool>*  m_islarhec{nullptr};
  std::vector<bool>*  m_islarfcal{nullptr};
  std::vector<bool>*  m_istile{nullptr};
  std::vector<int>*   m_hit_sampling{nullptr};
  std::vector<float>* m_hit_samplingfraction{nullptr};

  std::vector<float>* m_truth_energy{nullptr};
  std::vector<float>* m_truth_px{nullptr};
  std::vector<float>* m_truth_py{nullptr};
  std::vector<float>* m_truth_pz{nullptr};
  std::vector<int>*   m_truth_pdg{nullptr};
  std::vector<int>*   m_truth_barcode{nullptr};
  std::vector<int>*   m_truth_vtxbarcode{nullptr}; //production vertex barcode

  std::vector<float>* m_cluster_energy{nullptr};
  std::vector<float>* m_cluster_eta{nullptr};
  std::vector<float>* m_cluster_phi{nullptr};
  std::vector<unsigned>* m_cluster_size{nullptr};
  std::vector<std::vector<Long64_t >>* m_cluster_cellID{nullptr};


  std::vector<Long64_t>*    m_cell_identifier{nullptr};
  std::vector<float>*       m_cell_energy{nullptr};
  std::vector<int>*         m_cell_sampling{nullptr};

  std::vector<float>*       m_g4hit_energy{nullptr};
  std::vector<float>*       m_g4hit_time{nullptr};
  std::vector<Long64_t>*    m_g4hit_identifier{nullptr};
  std::vector<Long64_t>*    m_g4hit_cellidentifier{nullptr};
  std::vector<float>*       m_g4hit_samplingfraction{nullptr};
  std::vector<int>*         m_g4hit_sampling{nullptr};

  //CaloHitAna variables
  FCS_matchedcellvector* m_oneeventcells = nullptr; //these are all matched cells in a single event
  FCS_matchedcellvector* m_layercells[MAX_LAYER]; //these are all matched cells in a given layer in a given event

  Float_t m_total_cell_e = 0;
  Float_t m_total_hit_e = 0;
  Float_t m_total_g4hit_e = 0;

  std::vector<Float_t>* m_final_cell_energy{nullptr};
  std::vector<Float_t>* m_final_hit_energy{nullptr};
  std::vector<Float_t>* m_final_g4hit_energy{nullptr};

  TTree * m_tree{nullptr};
  //####################################################
  double m_eta_calo_surf{0.};
  double m_phi_calo_surf{0.};
  double m_d_calo_surf{0.};
  double m_ptruth_eta{0.};
  double m_ptruth_phi{0.};
  double m_ptruth_e{0.};
  double m_ptruth_et{0.};
  double m_ptruth_pt{0.};
  double m_ptruth_p{0.};
  int m_pdgid{0};

  std::vector<std::vector<float> >* m_newTTC_entrance_eta{nullptr};
  std::vector<std::vector<float> >* m_newTTC_entrance_phi{nullptr};
  std::vector<std::vector<float> >* m_newTTC_entrance_r{nullptr};
  std::vector<std::vector<float> >* m_newTTC_entrance_z{nullptr};
  std::vector<std::vector<float> >* m_newTTC_entrance_detaBorder{nullptr};
  std::vector<std::vector<bool> >*  m_newTTC_entrance_OK{nullptr};
  std::vector<std::vector<float> >* m_newTTC_back_eta{nullptr};
  std::vector<std::vector<float> >* m_newTTC_back_phi{nullptr};
  std::vector<std::vector<float> >* m_newTTC_back_r{nullptr};
  std::vector<std::vector<float> >* m_newTTC_back_z{nullptr};
  std::vector<std::vector<float> >* m_newTTC_back_detaBorder{nullptr};
  std::vector<std::vector<bool> >*  m_newTTC_back_OK{nullptr};
  std::vector<std::vector<float> >* m_newTTC_mid_eta{nullptr};
  std::vector<std::vector<float> >* m_newTTC_mid_phi{nullptr};
  std::vector<std::vector<float> >* m_newTTC_mid_r{nullptr};
  std::vector<std::vector<float> >* m_newTTC_mid_z{nullptr};
  std::vector<std::vector<float> >* m_newTTC_mid_detaBorder{nullptr};
  std::vector<std::vector<bool> >*  m_newTTC_mid_OK{nullptr};
  std::vector<float>* m_newTTC_IDCaloBoundary_eta{nullptr};
  std::vector<float>* m_newTTC_IDCaloBoundary_phi{nullptr};
  std::vector<float>* m_newTTC_IDCaloBoundary_r{nullptr};
  std::vector<float>* m_newTTC_IDCaloBoundary_z{nullptr};
  std::vector<float>* m_newTTC_Angle3D{nullptr};
  std::vector<float>* m_newTTC_AngleEta{nullptr};

  std::vector<float>* m_MuonEntryLayer_E{nullptr};
  std::vector<float>* m_MuonEntryLayer_px{nullptr};
  std::vector<float>* m_MuonEntryLayer_py{nullptr};
  std::vector<float>* m_MuonEntryLayer_pz{nullptr};
  std::vector<float>* m_MuonEntryLayer_x{nullptr};
  std::vector<float>* m_MuonEntryLayer_y{nullptr};
  std::vector<float>* m_MuonEntryLayer_z{nullptr};
  std::vector<int>*   m_MuonEntryLayer_pdg{nullptr};

  /** The new Extrapolator setup */
  CxxUtils::CachedPointer<const Trk::TrackingVolume> m_caloEntrance;
  // extrapolation through Calo
  Trk::PdgToParticleHypothesis        m_pdgToParticleHypothesis;
  /** End new Extrapolator setup */

  CaloCell_ID_FCS::CaloSample    m_sample_calo_surf{CaloCell_ID_FCS::noSample};
  std::vector< CaloCell_ID_FCS::CaloSample > m_surfacelist;

  bool   m_layerCaloOK[CaloCell_ID_FCS::MaxSample][3];
  double m_letaCalo[CaloCell_ID_FCS::MaxSample][3];
  double m_lphiCalo[CaloCell_ID_FCS::MaxSample][3];
  double m_lrCalo[CaloCell_ID_FCS::MaxSample][3];
  double m_lzCalo[CaloCell_ID_FCS::MaxSample][3];
  double m_dCalo[CaloCell_ID_FCS::MaxSample][3];
  double m_distetaCaloBorder[CaloCell_ID_FCS::MaxSample][3];


  HepPDT::ParticleDataTable*     m_particleDataTable{nullptr};

  //###################################################################


};

#endif // ISF_HIT_ANALYSIS_H
