/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETGNNTRACKING_DUMPOBJECTS_H
#define INDETGNNTRACKING_DUMPOBJECTS_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"

#include "GeneratorObjects/McEventCollection.h"
#include "InDetPrepRawData/PixelClusterContainer.h"
#include "InDetPrepRawData/SCT_ClusterContainer.h"
#include "InDetSimData/InDetSimDataCollection.h"
#include "TrkSpacePoint/SpacePointContainer.h"
#include "TrkSpacePoint/SpacePointOverlapCollection.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkTruthData/DetailedTrackTruthCollection.h"
#include "TrkTruthData/TrackTruthCollection.h"
#include "xAODEventInfo/EventInfo.h"

#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "SCT_ReadoutGeometry/SCT_DetectorManager.h"

#include "GaudiKernel/IPartPropSvc.h"

class PixelID;
class SCT_ID;
class TTree;

namespace InDetDD {
class PixelDetectorManager;
class SCT_DetectorManager;
} // namespace InDetDD

namespace HepPDT {
class ParticleDataTable;
}

namespace InDet {

int compute_overlap_SP_flag(const int& eta_module_cl1,const int& phi_module_cl1,
                            const int& eta_module_cl2,const int& phi_module_cl2);

class DumpObjects : public AthAlgorithm {
public:
  DumpObjects(const std::string &name, ISvcLocator *pSvcLocator);
  ~DumpObjects() {}

  virtual StatusCode initialize() override final;
  virtual StatusCode execute() override final;
  virtual StatusCode finalize() override final;

private:
  bool isPassed(HepMC::ConstGenParticlePtr particle, float &px, float &py, float &pz, float &pt,
                float &eta, float &vx, float &vy, float &vz, float &radius, float &status, float &charge,
                std::vector<int> &vParentID, std::vector<int> &vParentBarcode, int &vProdNin, int &vProdNout,
                int &vProdStatus, int &vProdBarcode);
  const PixelID *m_pixelID;
  const SCT_ID *m_SCT_ID;
  const InDetDD::PixelDetectorManager *m_pixelManager;
  const InDetDD::SCT_DetectorManager *m_SCT_Manager;
  int m_event;
  int m_selected;
  ServiceHandle<IPartPropSvc> m_particlePropSvc;
  const HepPDT::ParticleDataTable *m_particleDataTable;
  std::string m_name;

  // ReadHandleKeys
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "EventInfoKey", "EventInfo"};
  SG::ReadHandleKey<McEventCollection> m_mcEventCollectionKey{this, "TruthEventKey", "TruthEvent"};

  SG::ReadHandleKey<InDet::PixelClusterContainer> m_pixelClusterKey{this, "PixelClusterKey", "ITkPixelClusters"};
  SG::ReadHandleKey<InDet::SCT_ClusterContainer> m_stripClusterKey{this, "StripClusterKey", "ITkStripClusters"};

  SG::ReadHandleKey<InDetSimDataCollection> m_pixelSDOKey{this, "PixelSDOKey", "ITkPixelSDO_Map"};
  SG::ReadHandleKey<InDetSimDataCollection> m_stripSDOKey{this, "StripSDOKey", "ITkStripSDO_Map"};

  SG::ReadHandleKey<SpacePointContainer> m_pixelSpacePointContainerKey{this, "PixelSpacePointContainerKey",
                                                                       "ITkPixelSpacePoints"};
  SG::ReadHandleKey<SpacePointContainer> m_stripSpacePointContainerKey{this, "StripSpacePointContainerKey",
                                                                       "ITkStripSpacePoints"};
  SG::ReadHandleKey<SpacePointOverlapCollection> m_overlapSpacePointCollectionKey{this, "OverlapSpacePointCollectionKey",
                                                                       "ITkOverlapSpacePoints"};


  SG::ReadHandleKey<TrackCollection> m_tracksKey{this, "TracksKey", "CombinedITkTracks"};
  SG::ReadHandleKey<TrackTruthCollection> m_tracksTruthKey{this, "TracksTruthKey", "CombinedITkTracksTruthCollection"};
  SG::ReadHandleKey<DetailedTrackTruthCollection> m_detailedTracksTruthKey{this, "DetailedTracksTruthKey",
                                                                           "CombinedITkTracksDetailedTruth"};

  int m_offset;

  // Accepting truth only if passes these requirements
  float m_max_eta = 4.0;
  float m_min_pt = 1000.;
  int m_max_barcode = 200e3;
  float m_maxProdVertex = 260.;

  // Information related to the TTree
  //
  std::string m_ntupleFileName; //!< jobOption: Ntuple file name
  std::string m_ntupleDirName;  //!< jobOption: Ntuple directory name
  std::string m_ntupleTreeName; //!< jobOption: Ntuple tree name
  int m_maxCL;                  //!< jobOption: maximum number of clusters
  int m_maxPart;                //!< jobOption: maximum number of particles
  int m_maxSP;                  //!< jobOption: maximum number of space points
  int m_maxTRK;
  int m_maxDTT;
  bool m_rootFile; //!< jobOption: save data in root format
  TTree *m_nt;

  unsigned int m_run_number;
  unsigned long long m_event_number;

  int m_nSE;
  int *m_SEID;

  int m_nCL;
  int *m_CLindex;
  std::vector<std::string> *m_CLhardware;
  double *m_CLx;
  double *m_CLy;
  double *m_CLz;
  int *m_CLbarrel_endcap;
  int *m_CLlayer_disk;
  int *m_CLeta_module;
  int *m_CLphi_module;
  int *m_CLside;
  uint64_t *m_CLmoduleID;
  std::vector<std::vector<int>> *m_CLparticleLink_eventIndex;
  std::vector<std::vector<int>> *m_CLparticleLink_barcode;
  std::vector<std::vector<bool>> *m_CLbarcodesLinked;
  std::vector<std::vector<float>> *m_CLparticle_charge;
  std::vector<std::vector<int>> *m_CLphis, *m_CLetas, *m_CLtots;
  double *m_CLloc_direction1, *m_CLloc_direction2, *m_CLloc_direction3;
  double *m_CLJan_loc_direction1, *m_CLJan_loc_direction2, *m_CLJan_loc_direction3;
  int *m_CLpixel_count;
  float *m_CLcharge_count;
  float *m_CLloc_eta, *m_CLloc_phi;
  float *m_CLglob_eta, *m_CLglob_phi;
  double *m_CLeta_angle, *m_CLphi_angle;
  float *m_CLnorm_x, *m_CLnorm_y, *m_CLnorm_z;
  std::vector<std::vector<double>> *m_CLlocal_cov;

  int m_nPartEVT;
  int *m_Part_event_number;
  int *m_Part_barcode;
  float *m_Part_px, *m_Part_py, *m_Part_pz;
  float *m_Part_pt;
  float *m_Part_eta;
  float *m_Part_vx, *m_Part_vy, *m_Part_vz;
  float *m_Part_radius;
  float *m_Part_status;
  float *m_Part_charge;
  int *m_Part_pdg_id;
  int *m_Part_passed;
  int *m_Part_vProdNin, *m_Part_vProdNout, *m_Part_vProdStatus, *m_Part_vProdBarcode;
  std::vector<std::vector<int>> *m_Part_vParentID, *m_Part_vParentBarcode;

  int m_nSP;
  int *m_SPindex;
  double *m_SPx, *m_SPy, *m_SPz;
  int *m_SPCL1_index, *m_SPCL2_index;
  int *m_SPisOverlap; // -1: pixel, 0: strip not overlap, 1: strip overlap eta, 2: strip overlap phi, 3: overlap eta & phi

  int m_nTRK;
  int *m_TRKindex;
  int *m_TRKtrack_fitter, *m_TRKparticle_hypothesis;
  std::vector<std::vector<int>> *m_TRKproperties, *m_TRKpattern;
  int *m_TRKndof, *m_TRKmot, *m_TRKoot;
  float *m_TRKchiSq;
  std::vector<std::vector<int>> *m_TRKmeasurementsOnTrack_pixcl_sctcl_index, *m_TRKoutliersOnTrack_pixcl_sctcl_index;
  int *m_TRKcharge;
  std::vector<std::vector<double>> *m_TRKperigee_position, *m_TRKperigee_momentum;
  int *m_TTCindex, *m_TTCevent_index, *m_TTCparticle_link;
  float *m_TTCprobability;

  int m_nDTT;
  int *m_DTTindex, *m_DTTsize;
  std::vector<std::vector<int>> *m_DTTtrajectory_eventindex, *m_DTTtrajectory_barcode, *m_DTTstTruth_subDetType,
      *m_DTTstTrack_subDetType, *m_DTTstCommon_subDetType;
};
} // namespace InDet

#endif // INDETGNNTRACKING_DUMPOBJECTS_H
