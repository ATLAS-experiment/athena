#include "DumpObjects.h"
#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenParticle.h"
#include "GeneratorObjects/xAODTruthParticleLink.h"
#include "InDetPrepRawData/SiCluster.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "PixelReadoutGeometry/PixelDetectorManager.h"
#include "PixelReadoutGeometry/PixelModuleDesign.h"
#include "ReadoutGeometryBase/SiLocalPosition.h"
#include "SCT_ReadoutGeometry/SCT_ModuleSideDesign.h"
#include "TrkSpacePoint/SpacePointOverlapCollection.h"
#include "xAODTruth/TruthVertex.h"

#include "HepPDT/ParticleDataTable.hh"

#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "InDetRIO_OnTrack/SCT_ClusterOnTrack.h"
#include "InDetRIO_OnTrack/SiClusterOnTrack.h"
#include "TrkEventPrimitives/FitQuality.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkTrack/TrackInfo.h"

#include "EventInfo/EventID.h"
#include "EventInfo/EventInfo.h"

#include "GaudiKernel/ITHistSvc.h"
#include "TTree.h"

#include <fstream>

//-------------------------------------------------------------------------
InDet::DumpObjects::DumpObjects(const std::string &name, ISvcLocator *pSvcLocator)
    //-------------------------------------------------------------------------
    : AthAlgorithm(name, pSvcLocator), m_pixelID(nullptr), m_SCT_ID(nullptr), m_pixelManager(nullptr),
      m_SCT_Manager(nullptr), m_event(0), m_selected(0), m_particlePropSvc("PartPropSvc", name),
      m_particleDataTable(0), m_offset(0) {
  declareProperty("Offset", m_offset);
  declareProperty("FileName", m_name = "");
  //
  declareProperty("NtupleFileName", m_ntupleFileName);
  declareProperty("NtupleDirectoryName", m_ntupleDirName);
  declareProperty("NtupleTreeName", m_ntupleTreeName);
  declareProperty("maxCL", m_maxCL = 1500000);
  declareProperty("maxPart", m_maxPart = 1500000);
  declareProperty("maxSP", m_maxSP = 1500000);
  declareProperty("maxTRK", m_maxTRK = 1500000);
  declareProperty("maxDTT", m_maxDTT = 1500000);

  declareProperty("rootFile", m_rootFile);
}

//----------------------------------
StatusCode InDet::DumpObjects::initialize() {
  //----------------------------------
  m_event = m_offset;

  // ReadHandle keys
  ATH_CHECK(m_eventInfoKey.initialize());
  ATH_CHECK(m_mcEventCollectionKey.initialize());
  ATH_CHECK(m_stripClusterKey.initialize());
  ATH_CHECK(m_pixelClusterKey.initialize());
  ATH_CHECK(m_pixelSDOKey.initialize());
  ATH_CHECK(m_stripSDOKey.initialize());
  ATH_CHECK(m_pixelSpacePointContainerKey.initialize());
  ATH_CHECK(m_stripSpacePointContainerKey.initialize());
  ATH_CHECK(m_overlapSpacePointCollectionKey.initialize());
  ATH_CHECK(m_tracksKey.initialize());
  ATH_CHECK(m_tracksTruthKey.initialize());
  ATH_CHECK(m_detailedTracksTruthKey.initialize());

  // Grab PixelID helper
  if (detStore()->retrieve(m_pixelID, "PixelID").isFailure()) {
    return StatusCode::FAILURE;
  }

  if (!detStore()->contains<InDetDD::PixelDetectorManager>("Pixel") ||
      detStore()->retrieve(m_pixelManager, "Pixel").isFailure()) {
    // if Pixel retrieval fails, try ITkPixel
    if (!detStore()->contains<InDetDD::PixelDetectorManager>("ITkPixel") ||
        detStore()->retrieve(m_pixelManager, "ITkPixel").isFailure()) {
      return StatusCode::FAILURE;
    }
  }

  // Grab SCT_ID helper
  if (detStore()->retrieve(m_SCT_ID, "SCT_ID").isFailure()) {
    return StatusCode::FAILURE;
  }

  if (!detStore()->contains<InDetDD::SCT_DetectorManager>("SCT") ||
      detStore()->retrieve(m_SCT_Manager, "SCT").isFailure()) {
    // if SCT retrieval fails, try ITkStrip
    if (!detStore()->contains<InDetDD::SCT_DetectorManager>("ITkStrip") ||
        detStore()->retrieve(m_SCT_Manager, "ITkStrip").isFailure()) {
      return StatusCode::FAILURE;
    }
  }

  // particle property service
  if (m_particlePropSvc.retrieve().isFailure()) {
    ATH_MSG_ERROR("Can not retrieve " << m_particlePropSvc << " . Aborting ... ");
    return StatusCode::FAILURE;
  }

  // and the particle data table
  m_particleDataTable = m_particlePropSvc->PDT();
  if (m_particleDataTable == 0) {
    ATH_MSG_ERROR("Could not get ParticleDataTable! Cannot associate pdg code with charge. Aborting. ");
    return StatusCode::FAILURE;
  }

  // Define the TTree
  //
  ITHistSvc *tHistSvc;
  StatusCode sc = service("THistSvc", tHistSvc);
  if (sc.isFailure()) {
    ATH_MSG_ERROR("Unable to retrieve pointer to THistSvc");
    return sc;
  }
  m_nt = new TTree(TString(m_ntupleTreeName), "Athena Dump for GNN4ITk");
  // NB: we must not delete the tree, this is done by THistSvc
  std::string fullNtupleName = m_ntupleFileName + m_ntupleDirName + m_ntupleTreeName;
  sc = tHistSvc->regTree(fullNtupleName, m_nt);
  if (sc.isFailure()) {
    ATH_MSG_ERROR("Unable to register TTree: " << fullNtupleName);
    return sc;
  }

  if (m_rootFile) {
    m_SEID = new int[m_maxCL];

    m_CLindex = new int[m_maxCL];
    m_CLhardware = new std::vector<std::string>;
    m_CLx = new double[m_maxCL];
    m_CLy = new double[m_maxCL];
    m_CLz = new double[m_maxCL];
    m_CLbarrel_endcap = new int[m_maxCL];
    m_CLlayer_disk = new int[m_maxCL];
    m_CLeta_module = new int[m_maxCL];
    m_CLphi_module = new int[m_maxCL];
    m_CLside = new int[m_maxCL];
    m_CLmoduleID = new uint64_t[m_maxCL];
    m_CLparticleLink_eventIndex = new std::vector<std::vector<int>>;
    m_CLparticleLink_barcode = new std::vector<std::vector<int>>;
    m_CLbarcodesLinked = new std::vector<std::vector<bool>>;
    m_CLphis = new std::vector<std::vector<int>>;
    m_CLetas = new std::vector<std::vector<int>>;
    m_CLtots = new std::vector<std::vector<int>>;
    m_CLloc_direction1 = new double[m_maxCL];
    m_CLloc_direction2 = new double[m_maxCL];
    m_CLloc_direction3 = new double[m_maxCL];
    m_CLJan_loc_direction1 = new double[m_maxCL];
    m_CLJan_loc_direction2 = new double[m_maxCL];
    m_CLJan_loc_direction3 = new double[m_maxCL];
    m_CLpixel_count = new int[m_maxCL];
    m_CLcharge_count = new float[m_maxCL];
    m_CLloc_eta = new float[m_maxCL];
    m_CLloc_phi = new float[m_maxCL];
    m_CLglob_eta = new float[m_maxCL];
    m_CLglob_phi = new float[m_maxCL];
    m_CLeta_angle = new double[m_maxCL];
    m_CLphi_angle = new double[m_maxCL];
    m_CLnorm_x = new float[m_maxCL];
    m_CLnorm_y = new float[m_maxCL];
    m_CLnorm_z = new float[m_maxCL];
    m_CLlocal_cov = new std::vector<std::vector<double>>;

    m_Part_event_number = new int[m_maxPart];
    m_Part_barcode = new int[m_maxPart];
    m_Part_px = new float[m_maxPart];
    m_Part_py = new float[m_maxPart];
    m_Part_pz = new float[m_maxPart];
    m_Part_pt = new float[m_maxPart];
    m_Part_eta = new float[m_maxPart];
    m_Part_vx = new float[m_maxPart];
    m_Part_vy = new float[m_maxPart];
    m_Part_vz = new float[m_maxPart];
    m_Part_radius = new float[m_maxPart];
    m_Part_status = new float[m_maxPart];
    m_Part_charge = new float[m_maxPart];
    m_Part_pdg_id = new int[m_maxPart];
    m_Part_passed = new int[m_maxPart];
    m_Part_vProdNin = new int[m_maxPart];
    m_Part_vProdNout = new int[m_maxPart];
    m_Part_vProdStatus = new int[m_maxPart];
    m_Part_vProdBarcode = new int[m_maxPart];
    m_Part_vParentID = new std::vector<std::vector<int>>;
    m_Part_vParentBarcode = new std::vector<std::vector<int>>;

    m_SPindex = new int[m_maxSP];
    m_SPx = new double[m_maxSP];
    m_SPy = new double[m_maxSP];
    m_SPz = new double[m_maxSP];
    m_SPCL1_index = new int[m_maxSP];
    m_SPCL2_index = new int[m_maxSP];
    m_SPisOverlap = new int[m_maxSP];

    m_TRKindex = new int[m_maxTRK];
    m_TRKtrack_fitter = new int[m_maxTRK];
    m_TRKparticle_hypothesis = new int[m_maxTRK];
    m_TRKproperties = new std::vector<std::vector<int>>;
    m_TRKpattern = new std::vector<std::vector<int>>;
    m_TRKndof = new int[m_maxTRK];
    m_TRKmot = new int[m_maxTRK];
    m_TRKoot = new int[m_maxTRK];
    m_TRKchiSq = new float[m_maxTRK];
    m_TRKmeasurementsOnTrack_pixcl_sctcl_index = new std::vector<std::vector<int>>;
    m_TRKoutliersOnTrack_pixcl_sctcl_index = new std::vector<std::vector<int>>;
    m_TRKcharge = new int[m_maxTRK];
    m_TRKperigee_position = new std::vector<std::vector<double>>;
    m_TRKperigee_momentum = new std::vector<std::vector<double>>;
    m_TTCindex = new int[m_maxTRK];
    m_TTCevent_index = new int[m_maxTRK];
    m_TTCparticle_link = new int[m_maxTRK];
    m_TTCprobability = new float[m_maxTRK];

    m_DTTindex = new int[m_maxDTT];
    m_DTTsize = new int[m_maxDTT];
    m_DTTtrajectory_eventindex = new std::vector<std::vector<int>>;
    m_DTTtrajectory_barcode = new std::vector<std::vector<int>>;
    m_DTTstTruth_subDetType = new std::vector<std::vector<int>>;
    m_DTTstTrack_subDetType = new std::vector<std::vector<int>>;
    m_DTTstCommon_subDetType = new std::vector<std::vector<int>>;

    m_nt->Branch("run_number", &m_run_number, "run_number/i");
    m_nt->Branch("event_number", &m_event_number, "event_number/l");

    m_nt->Branch("nSE", &m_nSE, "nSE/I");
    m_nt->Branch("SEID", m_SEID, "SEID[nSE]/I");

    m_nt->Branch("nCL", &m_nCL, "nCL/I");
    m_nt->Branch("CLindex", m_CLindex, "CLindex[nCL]/I");
    m_nt->Branch("CLhardware", &m_CLhardware);
    m_nt->Branch("CLx", m_CLx, "CLx[nCL]/D");
    m_nt->Branch("CLy", m_CLy, "CLy[nCL]/D");
    m_nt->Branch("CLz", m_CLz, "CLz[nCL]/D");
    m_nt->Branch("CLbarrel_endcap", m_CLbarrel_endcap, "CLbarrel_endcap[nCL]/I");
    m_nt->Branch("CLlayer_disk", m_CLlayer_disk, "CLlayer_disk[nCL]/I");
    m_nt->Branch("CLeta_module", m_CLeta_module, "CLeta_module[nCL]/I");
    m_nt->Branch("CLphi_module", m_CLphi_module, "CLphi_module[nCL]/I");
    m_nt->Branch("CLside", m_CLside, "CLside[nCL]/I");
    m_nt->Branch("CLmoduleID", m_CLmoduleID, "CLmoduleID[nCL]/l");
    m_nt->Branch("CLparticleLink_eventIndex", &m_CLparticleLink_eventIndex);
    m_nt->Branch("CLparticleLink_barcode", &m_CLparticleLink_barcode);
    m_nt->Branch("CLbarcodesLinked", &m_CLbarcodesLinked);
    m_nt->Branch("CLphis", &m_CLphis);
    m_nt->Branch("CLetas", &m_CLetas);
    m_nt->Branch("CLtots", &m_CLtots);
    m_nt->Branch("CLloc_direction1", m_CLloc_direction1, "CLloc_direction1[nCL]/D");
    m_nt->Branch("CLloc_direction2", m_CLloc_direction2, "CLloc_direction2[nCL]/D");
    m_nt->Branch("CLloc_direction3", m_CLloc_direction3, "CLloc_direction3[nCL]/D");
    m_nt->Branch("CLJan_loc_direction1", m_CLJan_loc_direction1, "CLJan_loc_direction1[nCL]/D");
    m_nt->Branch("CLJan_loc_direction2", m_CLJan_loc_direction2, "CLJan_loc_direction2[nCL]/D");
    m_nt->Branch("CLJan_loc_direction3", m_CLJan_loc_direction3, "CLJan_loc_direction3[nCL]/D");
    m_nt->Branch("CLpixel_count", m_CLpixel_count, "CLpixel_count[nCL]/I");
    m_nt->Branch("CLcharge_count", m_CLcharge_count, "CLcharge_count[nCL]/F");
    m_nt->Branch("CLloc_eta", m_CLloc_eta, "CLloc_eta[nCL]/F");
    m_nt->Branch("CLloc_phi", m_CLloc_phi, "CLloc_phi[nCL]/F");
    m_nt->Branch("CLglob_eta", m_CLglob_eta, "CLglob_eta[nCL]/F");
    m_nt->Branch("CLglob_phi", m_CLglob_phi, "CLglob_phi[nCL]/F");
    m_nt->Branch("CLeta_angle", m_CLeta_angle, "CLeta_angle[nCL]/D");
    m_nt->Branch("CLphi_angle", m_CLphi_angle, "CLphi_angle[nCL]/D");
    m_nt->Branch("CLnorm_x", m_CLnorm_x, "CLnorm_x[nCL]/F");
    m_nt->Branch("CLnorm_y", m_CLnorm_y, "CLnorm_y[nCL]/F");
    m_nt->Branch("CLnorm_z", m_CLnorm_z, "CLnorm_z[nCL]/F");
    m_nt->Branch("CLlocal_cov", &m_CLlocal_cov);

    m_nt->Branch("nPartEVT", &m_nPartEVT, "nPartEVT/I");
    m_nt->Branch("Part_event_number", m_Part_event_number, "Part_event_number[nPartEVT]/I");
    m_nt->Branch("Part_barcode", m_Part_barcode, "Part_barcode[nPartEVT]/I");
    m_nt->Branch("Part_px", m_Part_px, "Part_px[nPartEVT]/F");
    m_nt->Branch("Part_py", m_Part_py, "Part_py[nPartEVT]/F");
    m_nt->Branch("Part_pz", m_Part_pz, "Part_pz[nPartEVT]/F");
    m_nt->Branch("Part_pt", m_Part_pt, "Part_pt[nPartEVT]/F");
    m_nt->Branch("Part_eta", m_Part_eta, "Part_eta[nPartEVT]/F");
    m_nt->Branch("Part_vx", m_Part_vx, "Part_vx[nPartEVT]/F");
    m_nt->Branch("Part_vy", m_Part_vy, "Part_vy[nPartEVT]/F");
    m_nt->Branch("Part_vz", m_Part_vz, "Part_vz[nPartEVT]/F");
    m_nt->Branch("Part_radius", m_Part_radius, "Part_radius[nPartEVT]/F");
    m_nt->Branch("Part_status", m_Part_status, "Part_status[nPartEVT]/F");
    m_nt->Branch("Part_charge", m_Part_charge, "Part_charge[nPartEVT]/F");
    m_nt->Branch("Part_pdg_id", m_Part_pdg_id, "Part_pdg_id[nPartEVT]/I");
    m_nt->Branch("Part_passed", m_Part_passed, "Part_passed[nPartEVT]/I");
    m_nt->Branch("Part_vProdNin", m_Part_vProdNin, "Part_vProdNin[nPartEVT]/I");
    m_nt->Branch("Part_vProdNout", m_Part_vProdNout, "Part_vProdNout[nPartEVT]/I");
    m_nt->Branch("Part_vProdStatus", m_Part_vProdStatus, "Part_vProdStatus[nPartEVT]/I");
    m_nt->Branch("Part_vProdBarcode", m_Part_vProdBarcode, "Part_vProdBarcode[nPartEVT]/I");
    m_nt->Branch("Part_vParentID", &m_Part_vParentID);
    m_nt->Branch("Part_vParentBarcode", &m_Part_vParentBarcode);

    m_nt->Branch("nSP", &m_nSP, "nSP/I");
    m_nt->Branch("SPindex", m_SPindex, "SPindex[nSP]/I");
    m_nt->Branch("SPx", m_SPx, "SPx[nSP]/D");
    m_nt->Branch("SPy", m_SPy, "SPy[nSP]/D");
    m_nt->Branch("SPz", m_SPz, "SPz[nSP]/D");
    m_nt->Branch("SPCL1_index", m_SPCL1_index, "SPCL1_index[nSP]/I");
    m_nt->Branch("SPCL2_index", m_SPCL2_index, "SPCL2_index[nSP]/I");
    m_nt->Branch("SPisOverlap", m_SPisOverlap, "SPisOverlap[nSP]/I");

    m_nt->Branch("nTRK", &m_nTRK, "nTRK/I");
    m_nt->Branch("TRKindex", m_TRKindex, "TRKindex[nTRK]/I");
    m_nt->Branch("TRKtrack_fitter", m_TRKtrack_fitter, "TRKtrack_fitter[nTRK]/I");
    m_nt->Branch("TRKparticle_hypothesis", m_TRKparticle_hypothesis, "TRKparticle_hypothesis[nTRK]/I");
    m_nt->Branch("TRKproperties", &m_TRKproperties);
    m_nt->Branch("TRKpattern", &m_TRKpattern);
    m_nt->Branch("TRKndof", m_TRKndof, "TRKndof[nTRK]/I");
    m_nt->Branch("TRKmot", m_TRKmot, "TRKmot[nTRK]/I");
    m_nt->Branch("TRKoot", m_TRKoot, "TRKoot[nTRK]/I");
    m_nt->Branch("TRKchiSq", m_TRKchiSq, "TRKchiSq[nTRK]/F");
    m_nt->Branch("TRKmeasurementsOnTrack_pixcl_sctcl_index", &m_TRKmeasurementsOnTrack_pixcl_sctcl_index);
    m_nt->Branch("TRKoutliersOnTrack_pixcl_sctcl_index", &m_TRKoutliersOnTrack_pixcl_sctcl_index);
    m_nt->Branch("TRKcharge", m_TRKcharge, "TRKcharge[nTRK]/I");
    m_nt->Branch("TRKperigee_position", &m_TRKperigee_position);
    m_nt->Branch("TRKperigee_momentum", &m_TRKperigee_momentum);
    m_nt->Branch("TTCindex", m_TTCindex, "TTCindex[nTRK]/I");
    m_nt->Branch("TTCevent_index", m_TTCevent_index, "TTCevent_index[nTRK]/I");
    m_nt->Branch("TTCparticle_link", m_TTCparticle_link, "TTCparticle_link[nTRK]/I");
    m_nt->Branch("TTCprobability", m_TTCprobability, "TTCprobability[nTRK]/F");

    m_nt->Branch("nDTT", &m_nDTT, "nDTT/I");
    m_nt->Branch("DTTindex", m_DTTindex, "DTTindex[nDTT]/I");
    m_nt->Branch("DTTsize", m_DTTsize, "DTTsize[nDTT]/I");
    m_nt->Branch("DTTtrajectory_eventindex", &m_DTTtrajectory_eventindex);
    m_nt->Branch("DTTtrajectory_barcode", &m_DTTtrajectory_barcode);
    m_nt->Branch("DTTstTruth_subDetType", &m_DTTstTruth_subDetType);
    m_nt->Branch("DTTstTrack_subDetType", &m_DTTstTrack_subDetType);
    m_nt->Branch("DTTstCommon_subDetType", &m_DTTstCommon_subDetType);
  }

  return StatusCode::SUCCESS;
}

//-------------------------------
StatusCode InDet::DumpObjects::execute() {
  //-------------------------------
  //
  const EventContext &ctx = Gaudi::Hive::currentContext();

  m_selected = 0;
  m_event++;

  // map cluster ID to an index
  // in order to connect the cluster to spacepoints
  std::map<Identifier, long int> clusterIDMapIdx;
  m_selected = 0; // global indices for clusters

  std::map<Identifier, long int> clusterIDMapSpacePointIdx; // key: cluster indentifier, value: spacepoint index

  // create a container with HepMcParticleLink and list of clusters
  // particle barcode --> is accepted and number of clusters
  std::map<std::pair<int, int>, std::pair<bool, int>> allTruthParticles;

  const McEventCollection *mcCollptr = nullptr;
  SG::ReadHandle<McEventCollection> mcEventCollectionHandle{m_mcEventCollectionKey, ctx};
  if (not mcEventCollectionHandle.isValid()) {
    ATH_MSG_WARNING(" McEventCollection not found: " << m_mcEventCollectionKey.key());
    return StatusCode::FAILURE;
  }
  mcCollptr = mcEventCollectionHandle.cptr();

  // dump out event ID
  const xAOD::EventInfo *eventInfo = nullptr;
  SG::ReadHandle<xAOD::EventInfo> eventInfoHandle(m_eventInfoKey, ctx);
  if (not eventInfoHandle.isValid()) {
    ATH_MSG_WARNING(" EventInfo not found: " << m_eventInfoKey.key());
    return StatusCode::FAILURE;
  }
  eventInfo = eventInfoHandle.cptr();

  m_run_number = eventInfo->runNumber();
  m_event_number = eventInfo->eventNumber();

  std::map<int, int> allSubEvents;

  m_nSE = 0;

  bool duplicateSubeventID = false;
  for (unsigned int cntr = 0; cntr < mcCollptr->size(); ++cntr) {
    int ID = mcCollptr->at(cntr)->event_number();
    if (m_rootFile)
      m_SEID[m_nSE++] = ID;

    if (m_nSE == m_maxCL) {
      ATH_MSG_WARNING("DUMP : hit max number of subevent ID");
      break;
    }
    std::map<int, int>::iterator it = allSubEvents.find(ID);
    if (it == allSubEvents.end())
      allSubEvents.insert(std::make_pair(ID, 1));
    else {
      it->second++;
      duplicateSubeventID = true;
    }
  }

  if (duplicateSubeventID) {
    ATH_MSG_WARNING("Duplicate subevent ID in event " << m_event);
  }

  m_nPartEVT = 0;

  if (m_rootFile) {
    (*m_Part_vParentID).clear();
    (*m_Part_vParentBarcode).clear();
  }

  for (unsigned int cntr = 0; cntr < mcCollptr->size(); ++cntr) {
    const HepMC::GenEvent *genEvt = (mcCollptr->at(cntr));

    // for ( HepMC::GenEvent::particle_const_iterator p = genEvt->particles_begin(); p != genEvt->particles_end(); ++p )
    // {
    for (auto p : *genEvt) {
      //*p is a GenParticle
      float px, py, pz, pt, eta, vx, vy, vz, radius, status, charge = 0.;
      std::vector<int> vParentID;
      std::vector<int> vParentBarcode;

      int vProdNin, vProdNout, vProdStatus, vProdBarcode;
      bool passed = isPassed(p, px, py, pz, pt, eta, vx, vy, vz, radius, status, charge, vParentID, vParentBarcode,
                             vProdNin, vProdNout, vProdStatus, vProdBarcode);
      allTruthParticles.insert(std::make_pair(std::make_pair(genEvt->event_number(), HepMC::barcode(p)),
                                              std::make_pair(passed, 0))); // JB: HEPMC3 barcode() -> HepMC::barcode(p)
      // subevent, barcode, px, py, pz, pt, eta, vx, vy, vz, radius, status, charge
      if (m_rootFile) {
        m_Part_event_number[m_nPartEVT] = genEvt->event_number();
        m_Part_barcode[m_nPartEVT] = HepMC::barcode(p); // JB: HEPMC3 barcode() -> HepMC::barcode(p)
        m_Part_px[m_nPartEVT] = px;
        m_Part_py[m_nPartEVT] = py;
        m_Part_pz[m_nPartEVT] = pz;
        m_Part_pt[m_nPartEVT] = pt;
        m_Part_eta[m_nPartEVT] = eta;
        m_Part_vx[m_nPartEVT] = vx;
        m_Part_vy[m_nPartEVT] = vy;
        m_Part_vz[m_nPartEVT] = vz;
        m_Part_radius[m_nPartEVT] = radius;
        m_Part_status[m_nPartEVT] = status;
        m_Part_charge[m_nPartEVT] = charge;
        m_Part_pdg_id[m_nPartEVT] = p->pdg_id();
        m_Part_passed[m_nPartEVT] = (passed ? true : false);
        m_Part_vProdNin[m_nPartEVT] = vProdNin;
        m_Part_vProdNout[m_nPartEVT] = vProdNout;
        m_Part_vProdStatus[m_nPartEVT] = vProdStatus;
        m_Part_vProdBarcode[m_nPartEVT] = vProdBarcode;
        (*m_Part_vParentID).push_back(vParentID);
        (*m_Part_vParentBarcode).push_back(vParentBarcode);
      }

      m_nPartEVT++;
      if (m_nPartEVT == m_maxPart) {
        ATH_MSG_WARNING("DUMP : hit max number of particle events");
        break;
      }
    }
  }

  const InDet::PixelClusterContainer *PixelClusterContainer = 0;
  SG::ReadHandle<InDet::PixelClusterContainer> pixelClusterContainerHandle{m_pixelClusterKey, ctx};
  if (not pixelClusterContainerHandle.isValid()) {
    ATH_MSG_WARNING(" PixelClusterContainer not found: " << m_pixelClusterKey.key());
    return StatusCode::FAILURE;
  }
  PixelClusterContainer = pixelClusterContainerHandle.cptr();

  const InDet::SCT_ClusterContainer *SCT_ClusterContainer = 0;
  SG::ReadHandle<InDet::SCT_ClusterContainer> stripClusterContainerHandle{m_stripClusterKey, ctx};
  if (not stripClusterContainerHandle.isValid()) {
    ATH_MSG_WARNING(" SCT_ClusterContainer not found: " << m_stripClusterKey.key());
    return StatusCode::FAILURE;
  }
  SCT_ClusterContainer = stripClusterContainerHandle.cptr();

  auto cartesion_to_spherical = [](const Amg::Vector3D &xyzVec, float &eta_, float &phi_) {
    float r3 = 0;
    for (int idx = 0; idx < 3; ++idx) {
      r3 += xyzVec[idx] * xyzVec[idx];
    }
    r3 = sqrt(r3);
    phi_ = atan2(xyzVec[1], xyzVec[0]);
    float theta_ = acos(xyzVec[2] / r3);
    eta_ = log(tan(0.5 * theta_));
  };

  ///////////////////////////////////////////////////////////////////////////
  //////////////////////////////PIXEL CONTAINER//////////////////////////////
  ///////////////////////////////////////////////////////////////////////////

  m_nCL = 0;
  if (m_rootFile) {
    (*m_CLhardware).clear();
    (*m_CLparticleLink_eventIndex).clear();
    (*m_CLparticleLink_barcode).clear();
    (*m_CLbarcodesLinked).clear();
    (*m_CLphis).clear();
    (*m_CLetas).clear();
    (*m_CLtots).clear();
    (*m_CLlocal_cov).clear();
  }

  if (PixelClusterContainer->size() > 0) {

    const InDetSimDataCollection *sdoCollection = 0;
    SG::ReadHandle<InDetSimDataCollection> sdoCollectionHandle{m_pixelSDOKey, ctx};
    if (not sdoCollectionHandle.isValid()) {
      ATH_MSG_WARNING(" InDetSimDataCollection not found: " << m_pixelSDOKey.key());
      return StatusCode::FAILURE;
    }
    sdoCollection = sdoCollectionHandle.cptr();

    for (const auto &clusterCollection : *PixelClusterContainer) {
      // skip empty collections
      if (clusterCollection->empty())
        continue;

      int barrel_endcap = m_pixelID->barrel_ec(clusterCollection->identify());
      int layer_disk = m_pixelID->layer_disk(clusterCollection->identify());
      int eta_module = m_pixelID->eta_module(clusterCollection->identify());
      int phi_module = m_pixelID->phi_module(clusterCollection->identify());

      InDetDD::SiDetectorElement *element = m_pixelManager->getDetectorElement(clusterCollection->identify());

      Amg::Vector3D my_normal = element->normal();
      float norm_x = fabs(my_normal.x()) > 1e-5 ? my_normal.x() : 0.;
      float norm_y = fabs(my_normal.y()) > 1e-5 ? my_normal.y() : 0.;
      float norm_z = fabs(my_normal.z()) > 1e-5 ? my_normal.z() : 0.;

      const InDetDD::PixelModuleDesign *design(dynamic_cast<const InDetDD::PixelModuleDesign *>(&element->design()));

      if (not design) {
        ATH_MSG_ERROR("Dynamic cast failed at " << __LINE__ << " of MergedPixelsTool.cxx.");
        return StatusCode::FAILURE;
      }

      // loop over collection
      for (const auto &cluster : *clusterCollection) {
        Identifier clusterId = cluster->identify();
        if (!clusterId.is_valid()) {
          ATH_MSG_WARNING("Pixel cluster identifier is not valid");
        }

        const Amg::MatrixX &local_cov = cluster->localCovariance();

        std::vector<std::pair<int, int>> barcodes = {};
        std::vector<int> particleLink_eventIndex = {};
        std::vector<int> particleLink_barcode = {};
        std::vector<bool> barcodesLinked = {};
        std::vector<int> phis = {};
        std::vector<int> etas = {};
        std::vector<int> tots = {};
        int min_eta = 999;
        int min_phi = 999;
        int max_eta = -999;
        int max_phi = -999;

        float charge_count = 0;
        int pixel_count = 0;

        for (unsigned int rdo = 0; rdo < cluster->rdoList().size(); rdo++) {
          const auto &rdoID = cluster->rdoList().at(rdo);
          int phi = m_pixelID->phi_index(rdoID);
          int eta = m_pixelID->eta_index(rdoID);
          if (min_eta > eta)
            min_eta = eta;
          if (min_phi > phi)
            min_phi = phi;
          if (max_eta < eta)
            max_eta = eta;
          if (max_phi < phi)
            max_phi = phi;

          ++pixel_count;
          charge_count += cluster->totList().at(rdo);

          phis.push_back(phi);
          etas.push_back(eta);
          tots.push_back(cluster->totList().at(rdo));

          auto pos = sdoCollection->find(rdoID);
          if (pos != sdoCollection->end()) {
            for (auto deposit : pos->second.getdeposits()) {
              const HepMcParticleLink &particleLink = deposit.first;
              std::pair<int, int> barcode(particleLink.eventIndex(), particleLink.barcode());
              // if (particleLink.isValid()) allTruthParticles.at(barcode).second++; // JB comment this out
              if (std::find(barcodes.begin(), barcodes.end(), barcode) == barcodes.end()) {
                barcodes.push_back(barcode);
                particleLink_eventIndex.push_back(particleLink.eventIndex());
                particleLink_barcode.push_back(particleLink.barcode());
                barcodesLinked.push_back(particleLink.isValid());
              }
            }
          }
        }

        InDetDD::SiLocalPosition localPos_entry = design->localPositionOfCell(InDetDD::SiCellId(min_phi, min_eta));
        InDetDD::SiLocalPosition localPos_exit = design->localPositionOfCell(InDetDD::SiCellId(max_phi, max_eta));

        Amg::Vector3D localStartPosition(localPos_entry.xEta() - 0.5 * element->etaPitch(),
                                         localPos_entry.xPhi() - 0.5 * element->phiPitch(),
                                         -0.5 * element->thickness());
        Amg::Vector3D localEndPosition(localPos_exit.xEta() + 0.5 * element->etaPitch(),
                                       localPos_exit.xPhi() + 0.5 * element->phiPitch(), 0.5 * element->thickness());

        // local direction in local coordinates
        // clusterShape: [lx, ly, lz]
        Amg::Vector3D localDirection = localEndPosition - localStartPosition;

        float loc_eta = 0, loc_phi = 0; // clusterShape: [leta, lphi]
        cartesion_to_spherical(localDirection, loc_eta, loc_phi);

        Amg::Vector3D globalStartPosition = element->globalPosition(localStartPosition);
        Amg::Vector3D globalEndPosition = element->globalPosition(localEndPosition);

        Amg::Vector3D direction = globalEndPosition - globalStartPosition;
        float glob_eta = 0, glob_phi = 0; // clusterShape: [geta, gphi]
        cartesion_to_spherical(direction, glob_eta, glob_phi);

        Amg::Vector3D my_phiax = element->phiAxis();
        Amg::Vector3D my_etaax = element->etaAxis();

        float trkphicomp = direction.dot(my_phiax);
        float trketacomp = direction.dot(my_etaax);
        float trknormcomp = direction.dot(my_normal);
        double phi_angle = atan2(trknormcomp, trkphicomp);
        double eta_angle = atan2(trknormcomp, trketacomp);
        // now dumping all the values now
        clusterIDMapIdx[cluster->identify()] = m_selected++;
        std::vector<double> v_local_cov;
        if (local_cov.size() > 0) {
          for (size_t i = 0, nRows = local_cov.rows(), nCols = local_cov.cols(); i < nRows; i++) {
            for (size_t j = 0; j < nCols; ++j) {
              v_local_cov.push_back(local_cov(i, j));
            }
          }
        } 
        if (m_rootFile) {
          // fill TTree
          m_CLindex[m_nCL] = m_selected;
          (*m_CLhardware).push_back("PIXEL");
          m_CLx[m_nCL] = cluster->globalPosition().x();
          m_CLy[m_nCL] = cluster->globalPosition().y();
          m_CLz[m_nCL] = cluster->globalPosition().z();
          m_CLbarrel_endcap[m_nCL] = barrel_endcap;
          m_CLlayer_disk[m_nCL] = layer_disk;
          m_CLeta_module[m_nCL] = eta_module;
          m_CLphi_module[m_nCL] = phi_module;
          m_CLside[m_nCL] = 0;
          m_CLmoduleID[m_nCL] = clusterCollection->identify().get_compact();
          (*m_CLparticleLink_eventIndex).push_back(particleLink_eventIndex);
          (*m_CLparticleLink_barcode).push_back(particleLink_barcode);
          (*m_CLbarcodesLinked).push_back(barcodesLinked);
          (*m_CLetas).push_back(etas);
          (*m_CLphis).push_back(phis);
          (*m_CLtots).push_back(tots);
          m_CLloc_direction1[m_nCL] = localDirection[0];
          m_CLloc_direction2[m_nCL] = localDirection[1];
          m_CLloc_direction3[m_nCL] = localDirection[2];
          m_CLJan_loc_direction1[m_nCL] = 0;
          m_CLJan_loc_direction2[m_nCL] = 0;
          m_CLJan_loc_direction3[m_nCL] = 0;
          m_CLpixel_count[m_nCL] = pixel_count;
          m_CLcharge_count[m_nCL] = charge_count;
          m_CLloc_eta[m_nCL] = loc_eta;
          m_CLloc_phi[m_nCL] = loc_phi;
          m_CLglob_eta[m_nCL] = glob_eta;
          m_CLglob_phi[m_nCL] = glob_phi;
          m_CLeta_angle[m_nCL] = eta_angle;
          m_CLphi_angle[m_nCL] = phi_angle;
          m_CLnorm_x[m_nCL] = norm_x;
          m_CLnorm_y[m_nCL] = norm_y;
          m_CLnorm_z[m_nCL] = norm_z;
          (*m_CLlocal_cov).push_back(v_local_cov);
        }
        m_nCL++;
        if (m_nCL == m_maxCL) {
          ATH_MSG_WARNING("DUMP : hit max number of clusters");
          break;
        }
      }
    }
  }

  ///////////////////////////////////////////////////////////////////////////
  ////////////////////////////////SCT CONTAINER//////////////////////////////
  ///////////////////////////////////////////////////////////////////////////

  if (SCT_ClusterContainer->size() > 0) {
    const InDetSimDataCollection *sdoCollection = 0;
    SG::ReadHandle<InDetSimDataCollection> sdoCollectionHandle{m_stripSDOKey, ctx};
    if (not sdoCollectionHandle.isValid()) {
      ATH_MSG_WARNING(" InDetSimDataCollection not found: " << m_stripSDOKey.key());
      return StatusCode::FAILURE;
    }
    sdoCollection = sdoCollectionHandle.cptr();

    for (const auto &clusterCollection : *SCT_ClusterContainer) {
      // skip empty collections
      if (clusterCollection->empty())
        continue;

      int barrel_endcap = m_SCT_ID->barrel_ec(clusterCollection->identify());
      int layer_disk = m_SCT_ID->layer_disk(clusterCollection->identify());
      int eta_module = m_SCT_ID->eta_module(clusterCollection->identify());
      int phi_module = m_SCT_ID->phi_module(clusterCollection->identify());
      int side = m_SCT_ID->side(clusterCollection->identify());

      InDetDD::SiDetectorElement *element = m_SCT_Manager->getDetectorElement(clusterCollection->identify());

      Amg::Vector3D my_normal = element->normal();
      float norm_x = fabs(my_normal.x()) > 1e-5 ? my_normal.x() : 0.;
      float norm_y = fabs(my_normal.y()) > 1e-5 ? my_normal.y() : 0.;
      float norm_z = fabs(my_normal.z()) > 1e-5 ? my_normal.z() : 0.;

      // loop over collection
      for (const auto &cluster : *clusterCollection) {
        Identifier clusterId = cluster->identify();
        if (!clusterId.is_valid()) {
          ATH_MSG_WARNING("SCT cluster identifier is not valid");
        }

        const Amg::MatrixX &local_cov = cluster->localCovariance();

        std::vector<std::pair<int, int>> barcodes = {};
        std::vector<int> particleLink_eventIndex = {};
        std::vector<int> particleLink_barcode = {};
        std::vector<bool> barcodesLinked = {};

        std::vector<int> tots = {};
        std::vector<int> strip_ids = {};
        int min_strip = 999;
        int max_strip = -999;

        float charge_count = 0;
        int pixel_count = 0;

        for (unsigned int rdo = 0; rdo < cluster->rdoList().size(); rdo++) {
          const auto &rdoID = cluster->rdoList().at(rdo);

          int strip = m_SCT_ID->strip(rdoID);

          if (min_strip > strip)
            min_strip = strip;
          if (max_strip < strip)
            max_strip = strip;
          strip_ids.push_back(strip);
          // tots.push_back(cluster->totList().at(rdo));
          tots.push_back(0); // FIXME
          ++pixel_count;
          // find barcodes of the truth particles
          auto pos = sdoCollection->find(rdoID);
          if (pos != sdoCollection->end()) {
            for (auto deposit : pos->second.getdeposits()) {
              const HepMcParticleLink &particleLink = deposit.first;
              std::pair<int, int> barcode(particleLink.eventIndex(), particleLink.barcode());
              // note that we are not filling the map allTruthParticles here - OK, we are not using this map for
              // anything
              if (std::find(barcodes.begin(), barcodes.end(), barcode) == barcodes.end()) {
                barcodes.push_back(barcode);
                particleLink_eventIndex.push_back(particleLink.eventIndex());
                particleLink_barcode.push_back(particleLink.barcode());
                barcodesLinked.push_back(particleLink.isValid());
              }
            }
          }
        }

        // retrieve cluster shape
        const InDetDD::SCT_ModuleSideDesign *design(
            dynamic_cast<const InDetDD::SCT_ModuleSideDesign *>(&element->design()));
        if (not design) {
          ATH_MSG_ERROR("Failed at " << __LINE__ << " of accessing SCT ModuleSide Design");
          return StatusCode::FAILURE;
        }

        Amg::Vector2D locpos = cluster->localPosition();
        std::pair<Amg::Vector3D, Amg::Vector3D> ends(
            element->endsOfStrip(InDetDD::SiLocalPosition(locpos.y(), locpos.x(), 0)));

        Amg::Vector3D JanDirection = ends.second - ends.first;

        InDetDD::SiLocalPosition localPos_entry = design->localPositionOfCell(InDetDD::SiCellId(min_strip));
        InDetDD::SiLocalPosition localPos_exit = design->localPositionOfCell(InDetDD::SiCellId(max_strip));

        Amg::Vector3D localStartPosition(localPos_entry.xEta() - 0.5 * element->etaPitch(),
                                         localPos_entry.xPhi() - 0.5 * element->phiPitch(),
                                         -0.5 * element->thickness());
        Amg::Vector3D localEndPosition(localPos_exit.xEta() + 0.5 * element->etaPitch(),
                                       localPos_exit.xPhi() + 0.5 * element->phiPitch(), 0.5 * element->thickness());

        Amg::Vector3D localDirection = localEndPosition - localStartPosition;
        float loc_eta = 0, loc_phi = 0; // clusterShape: [leta, lphi]
        cartesion_to_spherical(localDirection, loc_eta, loc_phi);

        Amg::Vector3D globalStartPosition = element->globalPosition(localStartPosition);
        Amg::Vector3D globalEndPosition = element->globalPosition(localEndPosition);

        Amg::Vector3D direction = globalEndPosition - globalStartPosition;
        float glob_eta = 0, glob_phi = 0; // clusterShape: [geta, gphi]
        cartesion_to_spherical(direction, glob_eta, glob_phi);

        Amg::Vector3D my_phiax = element->phiAxis();
        Amg::Vector3D my_etaax = element->etaAxis();

        float trkphicomp = direction.dot(my_phiax);
        float trketacomp = direction.dot(my_etaax);
        float trknormcomp = direction.dot(my_normal);
        double phi_angle = atan2(trknormcomp, trkphicomp);
        double eta_angle = atan2(trknormcomp, trketacomp);

        // now dumping all the values now
        clusterIDMapIdx[cluster->identify()] = m_selected++;
        // cluster shape
        std::vector<int> cst;
        for (unsigned strip = 0; strip < strip_ids.size(); strip++) {
          cst.push_back(-1);
        }
        std::vector<double> v_local_cov;
        if (local_cov.size() > 0) {
          for (size_t i = 0, nRows = local_cov.rows(), nCols = local_cov.cols(); i < nRows; i++) {
            for (size_t j = 0; j < nCols; ++j) {
              v_local_cov.push_back(local_cov(i, j));
            }
          }
        } 
        if (m_rootFile) {
          m_CLindex[m_nCL] = m_selected;
          (*m_CLhardware).push_back("STRIP");
          m_CLx[m_nCL] = cluster->globalPosition().x();
          m_CLy[m_nCL] = cluster->globalPosition().y();
          m_CLz[m_nCL] = cluster->globalPosition().z();
          m_CLbarrel_endcap[m_nCL] = barrel_endcap;
          m_CLlayer_disk[m_nCL] = layer_disk;
          m_CLeta_module[m_nCL] = eta_module;
          m_CLphi_module[m_nCL] = phi_module;
          m_CLside[m_nCL] = side;
          m_CLmoduleID[m_nCL] = clusterCollection->identify().get_compact();
          (*m_CLparticleLink_eventIndex).push_back(particleLink_eventIndex);
          (*m_CLparticleLink_barcode).push_back(particleLink_barcode);
          (*m_CLbarcodesLinked).push_back(barcodesLinked);
          (*m_CLetas).push_back(strip_ids);
          (*m_CLphis).push_back(cst);
          (*m_CLtots).push_back(tots);
          m_CLloc_direction1[m_nCL] = localDirection[0];
          m_CLloc_direction2[m_nCL] = localDirection[1];
          m_CLloc_direction3[m_nCL] = localDirection[2];
          m_CLJan_loc_direction1[m_nCL] = JanDirection[0];
          m_CLJan_loc_direction2[m_nCL] = JanDirection[1];
          m_CLJan_loc_direction3[m_nCL] = JanDirection[2];
          m_CLpixel_count[m_nCL] = pixel_count;
          m_CLcharge_count[m_nCL] = charge_count;
          m_CLloc_eta[m_nCL] = loc_eta;
          m_CLloc_phi[m_nCL] = loc_phi;
          m_CLglob_eta[m_nCL] = glob_eta;
          m_CLglob_phi[m_nCL] = glob_phi;
          m_CLeta_angle[m_nCL] = eta_angle;
          m_CLphi_angle[m_nCL] = phi_angle;
          m_CLnorm_x[m_nCL] = norm_x;
          m_CLnorm_y[m_nCL] = norm_y;
          m_CLnorm_z[m_nCL] = norm_z;
          (*m_CLlocal_cov).push_back(v_local_cov);
        }

        m_nCL++;
        if (m_nCL == m_maxCL) {
          ATH_MSG_WARNING("DUMP : hit max number of clusters");
          break;
        }
      }
    }
  }


  ////////////////////////////////////////////////////////////////////////////
  /////////////////////////////// SPACE POINTS ///////////////////////////////
  ///////////////////////////////////////////////////////////////////////////

  const SpacePointContainer *PixelSpacePointContainer = 0;
  SG::ReadHandle<SpacePointContainer> pixelSpacePointContainerHandle{m_pixelSpacePointContainerKey, ctx};
  if (not pixelSpacePointContainerHandle.isValid()) {
    ATH_MSG_ERROR(" SpacePointContainer not found: " << m_pixelSpacePointContainerKey.key());
    return StatusCode::FAILURE;
  }
  PixelSpacePointContainer = pixelSpacePointContainerHandle.cptr();

  const SpacePointContainer *SCT_SpacePointContainer = 0;
  SG::ReadHandle<SpacePointContainer> stripSpacePointContainerHandle{m_stripSpacePointContainerKey, ctx};
  if (not stripSpacePointContainerHandle.isValid()) {
    ATH_MSG_ERROR(" SpacePointContainer not found: " << m_stripSpacePointContainerKey.key());
    return StatusCode::FAILURE;
  }
  SCT_SpacePointContainer = stripSpacePointContainerHandle.cptr();

  const SpacePointOverlapCollection *OverlapSpacePointCollection =0;
  SG::ReadHandle<SpacePointOverlapCollection> overlapSpacePointCollectionHandle{m_overlapSpacePointCollectionKey, ctx};
  if (not overlapSpacePointCollectionHandle.isValid()) {
    ATH_MSG_ERROR(" SpacePointContainer not found: " << m_overlapSpacePointCollectionKey.key());
    return StatusCode::FAILURE;
  }
  OverlapSpacePointCollection = overlapSpacePointCollectionHandle.cptr();

  int sp_index = 0;

  m_nSP = 0;

  if (PixelSpacePointContainer && PixelSpacePointContainer->size() > 0) {
    for (const auto &spCollection : *PixelSpacePointContainer) {
      // skip empty collections
      if (spCollection->empty())
        continue;

      // loop over collection
      for (const auto &sp : *spCollection) {
        // save sp x, y, z and the index of the cluster associated to that one
        const InDet::SiCluster *cl = static_cast<const InDet::SiCluster *>(sp->clusterList().first);
        if (m_rootFile) {
          m_SPindex[m_nSP] = sp_index;
          m_SPx[m_nSP] = sp->globalPosition().x();
          m_SPy[m_nSP] = sp->globalPosition().y();
          m_SPz[m_nSP] = sp->globalPosition().z();
          m_SPCL1_index[m_nSP] = clusterIDMapIdx[cl->identify()];
          m_SPCL2_index[m_nSP] = -1;
          m_SPisOverlap[m_nSP] = -1;
        }
        sp_index++;

        m_nSP++;
        if (m_nSP == m_maxSP) {
          ATH_MSG_WARNING("DUMP : hit max number of space points");
          break;
        }
      }
    }
  }

  if (SCT_SpacePointContainer && SCT_SpacePointContainer->size() > 0) {
    for (const auto &spCollection : *SCT_SpacePointContainer) {
      // skip empty collections
      if (spCollection->empty())
        continue;

      // loop over collection
      for (const auto &sp : *spCollection) {
        // save sp x, y, z and the index of the cluster associated to that one
        const InDet::SiCluster *cl_1 = static_cast<const InDet::SiCluster *>(sp->clusterList().first);
        const InDet::SiCluster *cl_2 = static_cast<const InDet::SiCluster *>(sp->clusterList().second);
        if (m_rootFile) {
          m_SPindex[m_nSP] = sp_index;
          m_SPx[m_nSP] = sp->globalPosition().x();
          m_SPy[m_nSP] = sp->globalPosition().y();
          m_SPz[m_nSP] = sp->globalPosition().z();
          m_SPCL1_index[m_nSP] = clusterIDMapIdx[cl_1->identify()];
          m_SPCL2_index[m_nSP] = clusterIDMapIdx[cl_2->identify()];
          m_SPisOverlap[m_nSP] = 0;
        }
        sp_index++;

        m_nSP++;
        if (m_nSP == m_maxSP) {
          ATH_MSG_WARNING("DUMP : hit max number of space points");
          break;
        }
      }
    }
  }

  // loop over collection
  for (const auto &sp : *OverlapSpacePointCollection) {
    // save sp x, y, z and the index of the cluster associated to that one
    const InDet::SiCluster *cl_1 = static_cast<const InDet::SiCluster *>(sp->clusterList().first);
    const InDet::SiCluster *cl_2 = static_cast<const InDet::SiCluster *>(sp->clusterList().second);
    if (m_rootFile) {
      m_SPindex[m_nSP] = sp_index;
      m_SPx[m_nSP] = sp->globalPosition().x();
      m_SPy[m_nSP] = sp->globalPosition().y();
      m_SPz[m_nSP] = sp->globalPosition().z();
      m_SPCL1_index[m_nSP] = clusterIDMapIdx[cl_1->identify()];
      m_SPCL2_index[m_nSP] = clusterIDMapIdx[cl_2->identify()];
      m_SPisOverlap[m_nSP] = 1;
    }
    sp_index++;

    m_nSP++;
    if (m_nSP == m_maxSP) {
      ATH_MSG_WARNING("DUMP : hit max number of space points");
      break;
    }
  }

  //////////////////////////////////////////////////////////////////////
  /////////////////////////////// TRACKS ///////////////////////////////
  //////////////////////////////////////////////////////////////////////

  const TrackCollection *trackCollection = 0;
  SG::ReadHandle<TrackCollection> trackCollectionHandle{m_tracksKey, ctx};
  if (not trackCollectionHandle.isValid()) {
    ATH_MSG_WARNING(" TrackCollection not found: " << m_tracksKey.key());
    return StatusCode::FAILURE;
  }
  trackCollection = trackCollectionHandle.cptr();

  const TrackTruthCollection *trackTruthCollection = 0;
  SG::ReadHandle<TrackTruthCollection> trackTruthCollectionHandle{m_tracksTruthKey, ctx};
  if (not trackTruthCollectionHandle.isValid()) {
    ATH_MSG_WARNING(" TrackTruthCollection not found: " << m_tracksTruthKey.key());
    return StatusCode::FAILURE;
  }
  trackTruthCollection = trackTruthCollectionHandle.cptr();

  int trk_index = 0;

  // loop over tracks (and track truth) objects
  TrackCollection::const_iterator trackIterator = (*trackCollection).begin();
  m_nTRK = 0;
  if (m_rootFile) {
    (*m_TRKproperties).clear();
    (*m_TRKpattern).clear();
    (*m_TRKperigee_position).clear();
    (*m_TRKperigee_momentum).clear();
    (*m_TRKmeasurementsOnTrack_pixcl_sctcl_index).clear();
    (*m_TRKoutliersOnTrack_pixcl_sctcl_index).clear();
  }

  for (; trackIterator < (*trackCollection).end(); ++trackIterator) {
    if (!((*trackIterator))) {
      ATH_MSG_WARNING("TrackCollection contains empty entries");
      continue;
    }
    const Trk::TrackInfo &info = (*trackIterator)->info();
    const Trk::FitQuality *fitQuality = (*trackIterator)->fitQuality();
    const Trk::Perigee *perigeeParameters = (*trackIterator)->perigeeParameters();
    const DataVector<const Trk::MeasurementBase> *measurementsOnTrack = (*trackIterator)->measurementsOnTrack();
    const DataVector<const Trk::MeasurementBase> *outliersOnTrack = (*trackIterator)->outliersOnTrack();

    ElementLink<TrackCollection> tracklink;
    tracklink.setElement(const_cast<Trk::Track *>(*trackIterator));
    tracklink.setStorableObject(*trackCollection);
    const ElementLink<TrackCollection> tracklink2 = tracklink;
    TrackTruthCollection::const_iterator found = trackTruthCollection->find(tracklink2);

    const std::bitset<Trk::TrackInfo::NumberOfTrackProperties> &properties = info.properties();
    std::vector<int> v_properties;
    for (std::size_t i = 0; i < properties.size(); i++) {
      if (properties[i]) {
        v_properties.push_back(i);
      }
    }

    const std::bitset<Trk::TrackInfo::NumberOfTrackRecoInfo> &pattern = info.patternRecognition();
    std::vector<int> v_pattern;
    for (std::size_t i = 0; i < pattern.size(); i++) {
      if (pattern[i]) {
        v_pattern.push_back(i);
      }
    }

    int ndof = -1;
    float chiSq = 0;
    if (fitQuality) {
      ndof = fitQuality->numberDoF();
      chiSq = fitQuality->chiSquared();
    }
    std::vector<double> position, momentum;
    int charge = 0;
    if (perigeeParameters) {
      position.push_back(perigeeParameters->position()[0]);
      position.push_back(perigeeParameters->position()[1]);
      position.push_back(perigeeParameters->position()[2]);
      momentum.push_back(perigeeParameters->momentum()[0]);
      momentum.push_back(perigeeParameters->momentum()[1]);
      momentum.push_back(perigeeParameters->momentum()[2]);
      charge = perigeeParameters->charge();
    } else {
      position.push_back(0);
      position.push_back(0);
      position.push_back(0);
      momentum.push_back(0);
      momentum.push_back(0);
      momentum.push_back(0);
    }
    int mot = 0;
    int oot = 0;
    if (measurementsOnTrack)
      mot = measurementsOnTrack->size();
    if (outliersOnTrack)
      oot = outliersOnTrack->size();
    std::vector<int> measurementsOnTrack_pixcl_sctcl_index, outliersOnTrack_pixcl_sctcl_index;
    int TTCindex, TTCevent_index, TTCparticle_link;
    float TTCprobability;
    if (measurementsOnTrack) {
      for (size_t i = 0; i < measurementsOnTrack->size(); i++) {
        const Trk::MeasurementBase *mb = (*measurementsOnTrack)[i];
        const InDet::PixelClusterOnTrack *pixcl = dynamic_cast<const InDet::PixelClusterOnTrack *>(mb);
        const InDet::SCT_ClusterOnTrack *sctcl = dynamic_cast<const InDet::SCT_ClusterOnTrack *>(mb);
        if (pixcl) {
          measurementsOnTrack_pixcl_sctcl_index.push_back(clusterIDMapIdx[pixcl->prepRawData()->identify()]);
        }
        else if (sctcl) {
          measurementsOnTrack_pixcl_sctcl_index.push_back(clusterIDMapIdx[sctcl->prepRawData()->identify()]);
        } else {
          measurementsOnTrack_pixcl_sctcl_index.push_back(-1);
        }
      }
    }
    if (outliersOnTrack) {
      for (size_t i = 0; i < outliersOnTrack->size(); i++) {
        const Trk::MeasurementBase *mb = (*outliersOnTrack)[i];
        const InDet::PixelClusterOnTrack *pixcl = dynamic_cast<const InDet::PixelClusterOnTrack *>(mb);
        const InDet::SCT_ClusterOnTrack *sctcl = dynamic_cast<const InDet::SCT_ClusterOnTrack *>(mb);
        if (pixcl) {
          outliersOnTrack_pixcl_sctcl_index.push_back(clusterIDMapIdx[pixcl->prepRawData()->identify()]);
        } else if (sctcl) {
          outliersOnTrack_pixcl_sctcl_index.push_back(clusterIDMapIdx[sctcl->prepRawData()->identify()]);
        } else {
          outliersOnTrack_pixcl_sctcl_index.push_back(-1);
        }
      }
    }
    if (found != trackTruthCollection->end()) {
      TTCindex = found->first.index();
      TTCevent_index = found->second.particleLink().eventIndex();
      TTCparticle_link = found->second.particleLink().barcode();
      TTCprobability = found->second.probability();
    } else {
      TTCindex = TTCevent_index = TTCparticle_link = -999;
      TTCprobability = -1;
    }

    if (m_rootFile) {
      m_TRKindex[m_nTRK] = trk_index;
      m_TRKtrack_fitter[m_nTRK] = info.trackFitter();
      m_TRKndof[m_nTRK] = info.trackFitter();
      m_TRKparticle_hypothesis[m_nTRK] = info.particleHypothesis();
      (*m_TRKproperties).push_back(v_properties);
      (*m_TRKpattern).push_back(v_pattern);
      m_TRKndof[m_nTRK] = ndof;
      m_TRKchiSq[m_nTRK] = chiSq;
      (*m_TRKmeasurementsOnTrack_pixcl_sctcl_index).push_back(measurementsOnTrack_pixcl_sctcl_index);
      (*m_TRKoutliersOnTrack_pixcl_sctcl_index).push_back(outliersOnTrack_pixcl_sctcl_index);
      m_TRKcharge[m_nTRK] = charge;
      (*m_TRKperigee_position).push_back(position);
      (*m_TRKperigee_momentum).push_back(momentum);
      m_TRKmot[m_nTRK] = mot;
      m_TRKoot[m_nTRK] = oot;
      m_TTCindex[m_nTRK] = TTCindex;
      m_TTCevent_index[m_nTRK] = TTCevent_index;
      m_TTCparticle_link[m_nTRK] = TTCparticle_link;
      m_TTCprobability[m_nTRK] = TTCprobability;
    }

    trk_index++;
    // index
    m_nTRK++;
    if (m_nTRK == m_maxTRK) {
      ATH_MSG_WARNING("DUMP : hit max number of track events");
      break;
    }
  }

  const DetailedTrackTruthCollection *detailedTrackTruthCollection = 0;
  SG::ReadHandle<DetailedTrackTruthCollection> detailedTrackTruthCollectionHandle{m_detailedTracksTruthKey, ctx};
  if (not detailedTrackTruthCollectionHandle.isValid()) {
    ATH_MSG_WARNING(" DetailedTrackTruthCollection not found: " << m_detailedTracksTruthKey.key());
    return StatusCode::FAILURE;
  }
  detailedTrackTruthCollection = detailedTrackTruthCollectionHandle.cptr();

  m_nDTT = 0;
  if (m_rootFile) {
    (*m_DTTtrajectory_eventindex).clear();
    (*m_DTTtrajectory_barcode).clear();
    (*m_DTTstTruth_subDetType).clear();
    (*m_DTTstTrack_subDetType).clear();
    (*m_DTTstCommon_subDetType).clear();
  }

  // loop over DetailedTrackTruth objects
  DetailedTrackTruthCollection::const_iterator detailedTrackTruthIterator = (*detailedTrackTruthCollection).begin();
  for (; detailedTrackTruthIterator != (*detailedTrackTruthCollection).end(); ++detailedTrackTruthIterator) {
    std::vector<int> DTTtrajectory_eventindex, DTTtrajectory_barcode, DTTstTruth_subDetType, DTTstTrack_subDetType,
        DTTstCommon_subDetType;
    const TruthTrajectory &traj = detailedTrackTruthIterator->second.trajectory();
    for (size_t j = 0; j < traj.size(); j++) {
      DTTtrajectory_eventindex.push_back(traj[j].eventIndex());
      DTTtrajectory_barcode.push_back(traj[j].barcode());
    }
    const SubDetHitStatistics &stTruth = detailedTrackTruthIterator->second.statsTruth();
    const SubDetHitStatistics &stTrack = detailedTrackTruthIterator->second.statsTrack();
    const SubDetHitStatistics &stCommon = detailedTrackTruthIterator->second.statsCommon();
    for (unsigned j = 0; j < SubDetHitStatistics::NUM_SUBDETECTORS; j++) {
      DTTstTruth_subDetType.push_back(stTruth[SubDetHitStatistics::SubDetType(j)]);
    }
    for (unsigned j = 0; j < SubDetHitStatistics::NUM_SUBDETECTORS; j++) {
      DTTstTrack_subDetType.push_back(stTrack[SubDetHitStatistics::SubDetType(j)]);
    }
    for (unsigned j = 0; j < SubDetHitStatistics::NUM_SUBDETECTORS; j++) {
      DTTstCommon_subDetType.push_back(stCommon[SubDetHitStatistics::SubDetType(j)]);
    }

    if (m_rootFile) {
      m_DTTindex[m_nDTT] = detailedTrackTruthIterator->first.index();
      m_DTTsize[m_nDTT] = traj.size();
      (*m_DTTtrajectory_eventindex).push_back(DTTtrajectory_eventindex);
      (*m_DTTtrajectory_barcode).push_back(DTTtrajectory_barcode);
      (*m_DTTstTruth_subDetType).push_back(DTTstTruth_subDetType);
      (*m_DTTstTrack_subDetType).push_back(DTTstTrack_subDetType);
      (*m_DTTstCommon_subDetType).push_back(DTTstCommon_subDetType);
    }

    m_nDTT++;
  }

  // Once all the information for this event has been filled in the arrays,
  // copy content of the arrays to the TTree
  if (m_rootFile)
    m_nt->Fill();

  return StatusCode::SUCCESS;
}

//--------------------------------
StatusCode InDet::DumpObjects::finalize() {
  //--------------------------------
  if (m_rootFile) {
    delete[] m_SEID;

    delete[] m_CLindex;
    delete m_CLhardware;
    delete[] m_CLx;
    delete[] m_CLy;
    delete[] m_CLz;
    delete[] m_CLbarrel_endcap;
    delete[] m_CLlayer_disk;
    delete[] m_CLeta_module;
    delete[] m_CLphi_module;
    delete[] m_CLside;
    delete[] m_CLmoduleID;
    delete m_CLparticleLink_eventIndex;
    delete m_CLparticleLink_barcode;
    delete m_CLbarcodesLinked;
    delete m_CLphis;
    delete m_CLetas;
    delete m_CLtots;
    delete[] m_CLloc_direction1;
    delete[] m_CLloc_direction2;
    delete[] m_CLloc_direction3;
    delete[] m_CLJan_loc_direction1;
    delete[] m_CLJan_loc_direction2;
    delete[] m_CLJan_loc_direction3;
    delete[] m_CLpixel_count;
    delete[] m_CLcharge_count;
    delete[] m_CLloc_eta;
    delete[] m_CLloc_phi;
    delete[] m_CLglob_eta;
    delete[] m_CLglob_phi;
    delete[] m_CLeta_angle;
    delete[] m_CLphi_angle;
    delete[] m_CLnorm_x;
    delete[] m_CLnorm_y;
    delete[] m_CLnorm_z;
    delete m_CLlocal_cov;

    delete[] m_Part_event_number;
    delete[] m_Part_barcode;
    delete[] m_Part_px;
    delete[] m_Part_py;
    delete[] m_Part_pz;
    delete[] m_Part_pt;
    delete[] m_Part_eta;
    delete[] m_Part_vx;
    delete[] m_Part_vy;
    delete[] m_Part_vz;
    delete[] m_Part_radius;
    delete[] m_Part_status;
    delete[] m_Part_charge;
    delete[] m_Part_pdg_id;
    delete[] m_Part_passed;

    delete[] m_Part_vProdNin;
    delete[] m_Part_vProdNout;
    delete[] m_Part_vProdStatus;
    delete[] m_Part_vProdBarcode;
    delete m_Part_vParentID;
    delete m_Part_vParentBarcode;

    delete[] m_SPindex;
    delete[] m_SPx;
    delete[] m_SPy;
    delete[] m_SPz;
    delete[] m_SPCL1_index;
    delete[] m_SPCL2_index;
    delete[] m_SPisOverlap;

    delete[] m_TRKindex;
    delete[] m_TRKtrack_fitter;
    delete[] m_TRKparticle_hypothesis;
    delete m_TRKproperties;
    delete m_TRKpattern;
    delete[] m_TRKndof;
    delete[] m_TRKmot;
    delete[] m_TRKoot;
    delete[] m_TRKchiSq;
    delete m_TRKmeasurementsOnTrack_pixcl_sctcl_index;
    delete m_TRKoutliersOnTrack_pixcl_sctcl_index;
    delete[] m_TRKcharge;
    delete m_TRKperigee_position;
    delete m_TRKperigee_momentum;
    delete[] m_TTCindex;
    delete[] m_TTCevent_index;
    delete[] m_TTCparticle_link;
    delete[] m_TTCprobability;

    delete[] m_DTTindex;
    delete[] m_DTTsize;
    delete m_DTTtrajectory_eventindex;
    delete m_DTTtrajectory_barcode;
    delete m_DTTstTruth_subDetType;
    delete m_DTTstTrack_subDetType;
    delete m_DTTstCommon_subDetType;
  }

  return StatusCode::SUCCESS;
}

//--------------------------------------------------------------------------------------------
bool InDet::DumpObjects::isPassed(HepMC::ConstGenParticlePtr particle, float &px, float &py, float &pz,
                                  float &pt, float &eta, float &vx, float &vy, float &vz, float &radius, float &status,
                                  float &charge, std::vector<int> &vParentID, std::vector<int> &vParentBarcode,
                                  int &vProdNin, int &vProdNout, int &vProdStatus, int &vProdBarcode) {
  //--------------------------------------------------------------------------------------------

  px = particle->momentum().px();
  py = particle->momentum().py();
  pz = particle->momentum().pz();

  pt = std::sqrt(px * px + py * py);
  eta = particle->momentum().eta();

  int pdgCode = particle->pdg_id();

  int absPdgCode = std::abs(pdgCode);
  // get the charge: ap->charge() is used later, DOES NOT WORK RIGHT NOW
  const HepPDT::ParticleData *ap = m_particleDataTable->particle(absPdgCode);
  charge = 1.;
  if (ap)
    charge = ap->charge();
  // since the PDT table only has abs(PID) values for the charge
  charge *= (pdgCode > 0.) ? 1. : -1.;

  status = particle->status();

  if (particle->production_vertex()) {
    vx = particle->production_vertex()->position().x();
    vy = particle->production_vertex()->position().y();
    vz = particle->production_vertex()->position().z();
    radius = particle->production_vertex()->position().perp();
  } else {
    vx = vy = vz = -1;
    radius = 999;
    if (status == 1)
      ATH_MSG_WARNING("no vertex for particle with status 1");
  }

  if (particle->production_vertex()) {
    vProdNin = particle->production_vertex()->particles_in_size();
    vProdNout = particle->production_vertex()->particles_out_size();
    vProdStatus = particle->production_vertex()->id();
    vProdBarcode = HepMC::barcode(particle->production_vertex()); // JB: HEPMC3 barcode() -> HepMC::barcode(p)
#ifdef HEPMC3
    for (const auto &p : particle->production_vertex()->particles_in()) {
      vParentID.push_back(p->pdg_id());
      vParentBarcode.push_back(HepMC::barcode(p)); // JB: HEPMC3 barcode() -> HepMC::barcode(p)
    }
#else
    for (auto ip = particle->production_vertex()->particles_in_const_begin();
         ip != particle->production_vertex()->particles_in_const_end();
         ++ip)
    {
      vParentID.push_back((*ip)->pdg_id());
      vParentBarcode.push_back(HepMC::barcode(*ip)); // JB: HEPMC3 barcode() -> HepMC::barcode(p)
    }
#endif    
  } else {
    vProdNin = 0;
    vProdNout = 0;
    vProdStatus = -999;
    vProdBarcode = 999;
  }

  bool passEta = (pt > 0.1) ? (std::abs(eta) < m_max_eta) : false;
  if (not passEta)
    return false;

  bool passPt = (pt > m_min_pt);
  if (not passPt)
    return false;

  bool passBarcode = (HepMC::barcode(particle) < m_max_barcode); // JB: HEPMC3 barcode() -> HepMC::barcode(p)
  if (not passBarcode)
    return false;

  bool passCharge = not(charge == 0.);
  if (not passCharge)
    return false;

  bool passStatus = (status == 1);
  if (not passStatus)
    return false;

  bool passProdRadius = (radius < m_maxProdVertex);
  if (not passProdRadius)
    return false;

  return true;
}
