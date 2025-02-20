/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "MmRdoToPrepDataToolMT.h"
#include "MuonPrepRawData/MMPrepDataContainer.h"
#include "MuonReadoutGeometry/MMReadoutElement.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "TrkEventPrimitives/LocalDirection.h"
#include "xAODMuonPrepData/MMClusterAuxContainer.h"

// BS access
#include "ByteStreamCnvSvcBase/IROBDataProviderSvc.h"
#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"
#include "MuonIdHelpers/IdentifierByDetElSorter.h"
using namespace MuonGM;
using namespace Trk;

namespace {
  // Count hits with negative charge, which indicates bad calibration
  std::atomic<bool> hitNegativeCharge{false};
}  // namespace

namespace Muon{
MmRdoToPrepDataToolMT::MmRdoToPrepDataToolMT(const std::string& t,
                                             const std::string& n,
                                             const IInterface* p)
    : base_class(t, n, p) {}

StatusCode MmRdoToPrepDataToolMT::initialize() {
  ATH_MSG_DEBUG(" in initialize()");
  ATH_CHECK(m_idHelperSvc.retrieve());
  // check if the initialization of the data container is success
  ATH_CHECK(m_writeKey.initialize());
  ATH_CHECK(m_readKey.initialize());
  ATH_CHECK(m_muDetMgrKey.initialize());
  ATH_CHECK(m_calibTool.retrieve());
  ATH_CHECK(m_updateKey.initialize(!m_updateKey.key().empty()));
  ATH_CHECK(m_xAODKey.initialize(!m_xAODKey.empty()));
  return StatusCode::SUCCESS;
}

StatusCode MmRdoToPrepDataToolMT::processCollection(const EventContext& ctx, 
                                                    MMPrepDataContainer* mmPrepDataContainer,
                                                    xAOD::MMClusterContainer* xAODContainer,
                                                    const std::vector<IdentifierHash>& idsToDecode,
                                                    const MM_RawDataCollection* rdoColl) const {
  ATH_MSG_DEBUG(" ***************** Start of process MM Collection");
  const MmIdHelper& id_helper = m_idHelperSvc->mmIdHelper();
  // protect for large splashes
  bool merge = m_merge || rdoColl->size() > 100;

  const IdentifierHash hash = rdoColl->identifierHash();

  // check if we actually want to decode this RDO collection
  if (!idsToDecode.empty() and std::find(idsToDecode.begin(), idsToDecode.end(),
                                         hash) == idsToDecode.end()) {
    ATH_MSG_DEBUG("Hash ID " << hash << " not in input list, ignore");
    return StatusCode::SUCCESS;
  } else
    ATH_MSG_DEBUG("Going to decode " << hash);

  // check if the collection already exists, otherwise add it
  if (mmPrepDataContainer->indexFindPtr(hash) != nullptr) {

    ATH_MSG_DEBUG("In processCollection: collection already contained in the MM PrepData container");
    return StatusCode::SUCCESS;
  }

  // Get write handle for this collection
  MMPrepDataContainer::IDC_WriteHandle lock = mmPrepDataContainer->getWriteHandle(hash);
  // Check if collection already exists (via the cache, i.e. in online trigger
  // mode)
  if (lock.OnlineAndPresentInAnotherView()) {
    ATH_MSG_DEBUG("In processCollection: collection already available in the MM PrepData container (via cache)");
    return StatusCode::SUCCESS;
  }
  auto prdColl = std::make_unique<MMPrepDataCollection>(hash);

  // set the offline identifier of the collection Id
  IdContext context = id_helper.module_context();
  Identifier moduleId{};
  if (id_helper.get_id(hash, moduleId, &context) != 0) {
    ATH_MSG_ERROR("Could not convert the hash Id: " << hash << " to identifier");
  } else {
    ATH_MSG_DEBUG(" dump moduleId " << moduleId);
    prdColl->setIdentifier(moduleId);
  }

  // MuonDetectorManager from the conditions store
  SG::ReadCondHandle<MuonGM::MuonDetectorManager> MuonDetMgr{m_muDetMgrKey, ctx};
  if (!MuonDetMgr.isValid()) {
    ATH_MSG_ERROR("Null pointer to the read MuonDetectorManager conditions object");
    return StatusCode::FAILURE;
  }

  std::vector<MMPrepData> MMprds;
  // convert the RDO collection to a PRD collection  
  for (const MM_RawData* rdo : *rdoColl) {
    ATH_MSG_DEBUG("Adding a new MM PrepRawData");

    const Identifier rdoId = rdo->identify();
    ATH_MSG_DEBUG(" dump rdo " << m_idHelperSvc->toString(rdoId));

    int channel = rdo->channel();
    std::vector<Identifier> rdoList;
    Identifier prdId = id_helper.channelID(rdoId, id_helper.multilayer(rdoId), id_helper.gasGap(rdoId), channel);
    ATH_MSG_DEBUG(" channel RDO " << channel << " channel from rdoID " << id_helper.channel(rdoId));
    rdoList.push_back(prdId);

    // get the local and global positions
    const MuonGM::MMReadoutElement* detEl = MuonDetMgr->getMMReadoutElement(prdId);
    Amg::Vector2D localPos{Amg::Vector2D::Zero()};

    bool getLocalPos = detEl->stripPosition(prdId, localPos);
    if (!getLocalPos) {
      if (msgLvl(MSG::DEBUG)) { // We should still keep this a warning and fix it properly but silence it for now (ATLASRECTS-7520)
        ATH_MSG_WARNING("Could not get the local strip position for "<< m_idHelperSvc->toString(prdId));
      }
      continue;
    }

    Amg::Vector3D globalPos{Amg::Vector3D::Zero()};
    bool getGlobalPos = detEl->stripGlobalPosition(prdId, globalPos);
    if (!getGlobalPos) {
      ATH_MSG_WARNING("Could not get the global strip position for MM");
      continue;
    }
    NSWCalib::CalibratedStrip calibStrip;
    ATH_CHECK(m_calibTool->calibrateStrip(ctx, rdo, calibStrip));
    if (calibStrip.charge < 0) {
      if (!hitNegativeCharge || msgLvl(MSG::DEBUG)) {
        ATH_MSG_WARNING("One MM RDO or more, such as one with pdo = "<< rdo->charge()
                        << " counts, corresponds to a negative charge ("<< calibStrip.charge << "). Skipping these RDOs");
        hitNegativeCharge = true;
      }
      continue;
    }

    Trk::LocalDirection localDir;
    const Trk::PlaneSurface& psurf = detEl->surface(prdId);
    Amg::Vector2D lpos{Amg::Vector2D::Zero()};
    psurf.globalToLocal(globalPos, globalPos, lpos);
    psurf.globalToLocalDirection(globalPos, localDir);

    ATH_MSG_DEBUG(" Surface centre x " << Amg::toString(psurf.center()));
    ATH_MSG_DEBUG(" localPos x " <<Amg::toString(localPos)<< " localPos y " << Amg::toString(lpos));

    Amg::Vector3D gdir = psurf.transform().linear() * Amg::Vector3D::UnitY();
    ATH_MSG_DEBUG(" MM detector surface direction phi "<< gdir.phi() << " global radius hit " << globalPos.perp()
                  << " phi pos " << globalPos.phi() << " global z "<< globalPos.z());

    auto cov = Amg::MatrixX(2, 2);
    cov.setIdentity();
    (cov)(0, 0) = calibStrip.resTransDistDrift;
    (cov)(1, 1) = calibStrip.resLongDistDrift;
    localPos.x() += calibStrip.dx;

    if (!merge) {
      // storage will be handeled by Store Gate
     auto mpd = std::make_unique<MMPrepData>(prdId, hash, 
                                             std::move(localPos), std::move(rdoList), std::move(cov),
                                             detEl, 
                                             calibStrip.time, calibStrip.charge, calibStrip.distDrift);
      mpd->setAuthor(MMPrepData::Author::RDOTOPRDConverter);

      prdColl->push_back(std::move(mpd));

    } else {
      if (calibStrip.charge < m_singleStripChargeCut) {
        continue;
      }
      MMPrepData mpd(prdId, hash, std::move(localPos), std::move(rdoList),
                     std::move(cov), detEl, calibStrip.time, calibStrip.charge,
                     calibStrip.distDrift);
      // set the hash of the MMPrepData such that it contains the correct value
      // in case it gets used in SimpleMMClusterBuilderTool::getClusters
      mpd.setHashAndIndex(hash, 0);
      mpd.setAuthor(MMPrepData::Author::RDOTOPRDConverter);
      MMprds.push_back(std::move(mpd));
    }
  }

  if (merge) {
    std::vector<std::unique_ptr<MMPrepData>> clusters;

    /// reconstruct the clusters
    ATH_CHECK(m_clusterBuilderTool->getClusters(ctx, std::move(MMprds), clusters));

    for (std::unique_ptr<MMPrepData>& prdN : clusters) {
      prdN->setHashAndIndex(prdColl->identifyHash(), prdColl->size());
      prdColl->push_back(std::move(prdN));
    }
  }  // merge
  
  if (xAODContainer) {
      // Lambda to fill xprd from prd
      std::vector<const MMPrepData*> sortMe{prdColl->begin(), prdColl->end()};
      std::ranges::sort(sortMe, IdentifierByDetElSorter{m_idHelperSvc.get()});
      for (const  MMPrepData* prd : sortMe) {
          xAOD::MMCluster* cluster{xAODContainer->push_back(std::make_unique<xAOD::MMCluster>())};
          cluster->setIdentifier(prd->identify().get_compact());
          cluster->setMeasurement(m_idHelperSvc->detElementHash(prd->identify()), 
                                  xAOD::MeasVector<1>{prd->localPosition().x()},
                                  xAOD::MeasMatrix<1>{prd->localCovariance()(0, 0)});
          cluster->setGasGap(id_helper.gasGap(prd->identify()));
          cluster->setChannelNumber(id_helper.channel(prd->identify()));
          cluster->setTime(prd->time());
          cluster->setCharge(prd->charge());
          cluster->setDriftDist(prd->driftDist());
          cluster->setAngle(prd->angle());
          cluster->setChiSqProb(prd->chisqProb());
          cluster->setAuthor(prd->author());
          cluster->setQuality(prd->quality());
          cluster->setStripNumbers(prd->stripNumbers());
          cluster->setStripTimes(prd->stripTimes());
          cluster->setStripCharges(prd->stripCharges());
          cluster->setStripDriftDist(prd->stripDriftDist());
          cluster->setStripDriftErrors(prd->stripDriftErrors());
      }
  }
  // now write the collection
  ATH_CHECK(lock.addOrDelete(std::move(prdColl)));
  ATH_MSG_DEBUG("PRD hash " << hash << " has been moved to container");

  return StatusCode::SUCCESS;
}

const MM_RawDataContainer* 
  MmRdoToPrepDataToolMT::getRdoContainer(const EventContext& ctx) const {
  auto rdoContainerHandle = SG::makeHandle(m_readKey, ctx);
  if (rdoContainerHandle.isValid()) {
    ATH_MSG_DEBUG("MM_getRdoContainer success");
    return rdoContainerHandle.cptr();
  }
  ATH_MSG_WARNING("Retrieval of MM_RawDataContainer failed !");

  return nullptr;
}

void MmRdoToPrepDataToolMT::processRDOContainer(const EventContext& ctx, 
                                                MMPrepDataContainer* mmPrepDataContainer,
                                                xAOD::MMClusterContainer* xAODContainer,
                                                const std::vector<IdentifierHash>& idsToDecode) const {
  ATH_MSG_DEBUG("In processRDOContainer");
  const MM_RawDataContainer* rdoContainer = getRdoContainer(ctx);
  if (!rdoContainer) {
    return;
  }

  for (const MM_RawDataCollection* rdoColl : *rdoContainer) {

    if (rdoColl->empty()){
      continue;
    }
    ATH_MSG_DEBUG("New RDO collection with " << rdoColl->size() << "MM Hits");

    if (processCollection(ctx, mmPrepDataContainer, xAODContainer, idsToDecode, rdoColl).isFailure()) {
      ATH_MSG_DEBUG(
          "processCsm returns a bad StatusCode - keep going for new data "
          "collections in this event");
    }
  }
}

// methods for ROB-based decoding
StatusCode MmRdoToPrepDataToolMT::decode(const EventContext& ctx, 
                                               const std::vector<IdentifierHash>& idVect) const {

  // is idVect a right thing to use here? to be reviewed maybe
  ATH_MSG_DEBUG("Size of the RDO container to be decoded: " << idVect.size());

  MMPrepDataContainer* mmPrepDataContainer = setupMM_PrepDataContainer(ctx);

  if (!mmPrepDataContainer) {
    return StatusCode::FAILURE;
  }
  
  SG::WriteHandle<xAOD::MMClusterContainer> outputContainer{};
  xAOD::MMClusterContainer* xAODContainer{nullptr};
  if (!m_xAODKey.empty()) {
    outputContainer = SG::WriteHandle{m_xAODKey, ctx};
    ATH_CHECK(outputContainer.record(std::make_unique<xAOD::MMClusterContainer>(),
                                             std::make_unique<xAOD::MMClusterAuxContainer>()));
    xAODContainer = outputContainer.ptr();
  }

  processRDOContainer(ctx, mmPrepDataContainer, xAODContainer, idVect);

  return StatusCode::SUCCESS;
}

StatusCode MmRdoToPrepDataToolMT::decode(const EventContext&, 
                                         const std::vector<uint32_t>&) const {
  ATH_MSG_FATAL("ROB based decoding is not supported....");
  return StatusCode::FAILURE;
}
StatusCode MmRdoToPrepDataToolMT::provideEmptyContainer(const EventContext& ctx) const {
  if (!m_xAODKey.empty()) {
    SG::WriteHandle writeHandle{m_xAODKey, ctx};
    ATH_CHECK(writeHandle.record(std::make_unique<xAOD::MMClusterContainer>(),
                                std::make_unique<xAOD::MMClusterAuxContainer>()));
  }

  return setupMM_PrepDataContainer(ctx) ? StatusCode::SUCCESS
                                        : StatusCode::FAILURE;
}

MMPrepDataContainer*
MmRdoToPrepDataToolMT::setupMM_PrepDataContainer(const EventContext& ctx) const {

  SG::WriteHandle handle{m_writeKey, ctx};
  if (m_updateKey.key().empty()) {
    // No external cache, just record the container
    StatusCode status =
        handle.record(std::make_unique<MMPrepDataContainer>(
            m_idHelperSvc->mmIdHelper().module_hash_max()));

    if (status.isFailure() || !handle.isValid()) {
      ATH_MSG_FATAL(
          "Could not record container of MicroMega PrepData Container at "
          << m_writeKey.key());
      return nullptr;
    }
  } else {
    // use the cache to get the container
    SG::UpdateHandle update(m_updateKey, ctx);
    if (!update.isValid()) {
      ATH_MSG_FATAL("Invalid UpdateHandle " << m_updateKey.key());
      return nullptr;
    }
    StatusCode status = handle.record(std::make_unique<MMPrepDataContainer>(update.ptr()));
    if (status.isFailure() || !handle.isValid()) {
      ATH_MSG_FATAL(
          "Could not record container of MM PrepData Container using cache "
          << m_updateKey.key() << " - "
          << m_writeKey.key());
      return nullptr;
    }
    ATH_MSG_DEBUG("Created container using cache for "
                  << m_updateKey.key());
  }
  return handle.ptr();
}

}