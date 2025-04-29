/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/


#include "FPGATrackSimObjects/FPGATrackSimEventInputHeader.h"
#include "FPGATrackSimObjects/FPGATrackSimEventInfo.h"
#include "FPGATrackSimObjects/FPGATrackSimOfflineTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimOfflineHit.h"
#include "FPGATrackSimObjects/FPGATrackSimTruthTrack.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"

#include "StoreGate/DataHandle.h"

#include "IdDictDetDescr/IdDictManager.h"
#include "InDetPrepRawData/SiClusterContainer.h"
#include "InDetPrepRawData/SiClusterCollection.h"
#include "InDetRawData/InDetRawDataCollection.h"
#include "InDetRawData/InDetRawDataContainer.h"
#include "InDetRawData/InDetRawDataCLASS_DEF.h"
#include "InDetSimData/SCT_SimHelper.h"
#include "InDetSimData/PixelSimHelper.h"
#include "InDetReadoutGeometry/SiDetectorDesign.h"

#include "ReadoutGeometryBase/SiCellId.h"
#include "ReadoutGeometryBase/SiReadoutCellId.h"
#include "SCT_ReadoutGeometry/StripStereoAnnulusDesign.h"

#include "TrkParameters/TrackParameters.h"

#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/GenParticle.h"
#include "TruthUtils/HepMCHelpers.h"

#include "InDetRIO_OnTrack/SiClusterOnTrack.h"

#include "GaudiKernel/IPartPropSvc.h"
#include "FPGATrackSimSGToRawHitsTool.h"

#include <bitset>


namespace {
  // A few constants for truth cuts
  const float FPGATrackSim_PT_TRUTHMIN = 400.;
  const float FPGATrackSim_Z_TRUTHMIN = 2300.;
}

FPGATrackSimSGToRawHitsTool::FPGATrackSimSGToRawHitsTool(const std::string& algname, const std::string& name, const IInterface* ifc) :
  base_class(algname, name, ifc)
{}

StatusCode FPGATrackSimSGToRawHitsTool::initialize() {

  ATH_MSG_DEBUG("FPGATrackSimSGToRawHitsTool::initialize()");

  if(!m_truthToTrack.empty() ) ATH_CHECK(m_truthToTrack.retrieve());  
  if(!m_extrapolator.empty()) ATH_CHECK(m_extrapolator.retrieve());
  ATH_CHECK(m_beamSpotKey.initialize());

  SmartIF<IPartPropSvc> partPropSvc{service("PartPropSvc")};
  ATH_CHECK(partPropSvc.isValid());
  m_particleDataTable = partPropSvc->PDT();

  ATH_CHECK(detStore()->retrieve(m_PIX_mgr, "ITkPixel"));
  ATH_CHECK(detStore()->retrieve(m_pixelId, "PixelID"));
  ATH_CHECK(detStore()->retrieve(m_SCT_mgr, "ITkStrip"));
  ATH_CHECK(detStore()->retrieve(m_sctId, "SCT_ID"));

  ATH_CHECK(m_eventInfoKey.initialize());
  ATH_CHECK(m_pixelClusterContainerKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_sctClusterContainerKey.initialize(SG::AllowEmpty));

  ATH_CHECK(m_offlineTracksKey.initialize(SG::AllowEmpty));

  ATH_CHECK(m_mcCollectionKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_pixelSDOKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_stripSDOKey.initialize(SG::AllowEmpty));
  ATH_CHECK(m_pixelRDOKey.initialize());
  ATH_CHECK(m_stripRDOKey.initialize());

  ATH_MSG_DEBUG("Initialization complete");
  return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimSGToRawHitsTool::finalize() {
  return StatusCode::SUCCESS;
}


/** This function get from the SG the inner detector raw hits
  and prepares them for FPGATrackSim simulation */
StatusCode FPGATrackSimSGToRawHitsTool::readData(FPGATrackSimEventInputHeader* header, const EventContext& eventContext)
{
  m_eventHeader = header; //take the external pointer
  auto eventInfo = SG::makeHandle(m_eventInfoKey, eventContext);
  //Filled to variable / start event
  FPGATrackSimEventInfo event_info;
  event_info.setRunNumber(eventInfo->runNumber());
  event_info.setEventNumber(eventInfo->eventNumber());
  event_info.setLB(eventInfo->lumiBlock());
  event_info.setBCID(eventInfo->bcid());
  event_info.setaverageInteractionsPerCrossing(eventInfo->averageInteractionsPerCrossing());
  event_info.setactualInteractionsPerCrossing(eventInfo->actualInteractionsPerCrossing());
  event_info.setextendedLevel1ID(eventInfo->extendedLevel1ID());
  event_info.setlevel1TriggerType(eventInfo->level1TriggerType());
  //  event_info.setlevel1TriggerInfo(eventInfo->level1TriggerInfo ()); // unclear if needed, TODO come back to it
  m_eventHeader->newEvent(event_info);//this also reset all variables
  HitIndexMap hitIndexMap; // keep running index event-unique to each hit
  HitIndexMap pixelClusterIndexMap;
  // get pixel and sct cluster containers
  // dump raw silicon data
  ATH_MSG_DEBUG("Dump raw silicon data");
  ATH_CHECK(readRawSilicon(hitIndexMap,  eventContext));
  FPGATrackSimOptionalEventInfo optional;
  if (m_readOfflineClusters) {
    std::vector <FPGATrackSimCluster> clusters;
    ATH_CHECK(readOfflineClusters(clusters, eventContext));
    for (const auto& cluster : clusters) optional.addOfflineCluster(cluster);
    ATH_MSG_DEBUG("Saved " << optional.nOfflineClusters() << " offline clusters");
    ATH_CHECK(dumpPixelClusters(pixelClusterIndexMap, eventContext));
  }
  if (m_readTruthTracks) {
    std::vector <FPGATrackSimTruthTrack> truth;
    ATH_CHECK(readTruthTracks(truth, eventContext));
    for (const FPGATrackSimTruthTrack& trk : truth) optional.addTruthTrack(trk);
    ATH_MSG_DEBUG("Saved " << optional.nTruthTracks() << " truth tracks");
  }
  std::vector <FPGATrackSimOfflineTrack> offline;
  if (m_readOfflineTracks) {
    ATH_CHECK(readOfflineTracks(offline, eventContext));
    for (const FPGATrackSimOfflineTrack& trk : offline) optional.addOfflineTrack(trk);
    ATH_MSG_DEBUG("Saved " << optional.nOfflineTracks() << " offline tracks");
  }
  m_eventHeader->setOptional(optional);
  ATH_MSG_DEBUG(*m_eventHeader);
  ATH_MSG_DEBUG("End of execute()");
  return StatusCode::SUCCESS;
}


StatusCode FPGATrackSimSGToRawHitsTool::readOfflineTracks(std::vector<FPGATrackSimOfflineTrack>& offline, const EventContext& eventContext)
{
  auto offlineTracksHandle = SG::makeHandle(m_offlineTracksKey, eventContext);
  ATH_MSG_DEBUG("read Offline tracks, size= " << offlineTracksHandle->size());

  int iTrk = -1;
  for (const xAOD::TrackParticle* trackParticle : *offlineTracksHandle) {
    iTrk++;
    FPGATrackSimOfflineTrack tmpOfflineTrack;
    tmpOfflineTrack.setQOverPt(trackParticle->pt() > 0 ? trackParticle->charge() / trackParticle->pt() : 0);
    tmpOfflineTrack.setEta(trackParticle->eta());
    tmpOfflineTrack.setPhi(trackParticle->phi());
    tmpOfflineTrack.setD0(trackParticle->d0());
    tmpOfflineTrack.setZ0(trackParticle->z0());

    const Trk::TrackStates* trackStates = trackParticle->track()->trackStateOnSurfaces();
    if (trackStates == nullptr) {
      ATH_MSG_ERROR("missing trackStatesOnSurface");
      return StatusCode::FAILURE;
    }
    for (const Trk::TrackStateOnSurface* tsos : *trackStates) {
      if (tsos == nullptr) continue;
      if (tsos->type(Trk::TrackStateOnSurface::Measurement)) {
        const Trk::MeasurementBase* measurement = tsos->measurementOnTrack();
        if (tsos->trackParameters() != nullptr &&
          tsos->trackParameters()->associatedSurface().associatedDetectorElement() != nullptr &&
          tsos->trackParameters()->associatedSurface().associatedDetectorElement()->identify() != 0
          ) {
          const Trk::RIO_OnTrack* hit = dynamic_cast <const Trk::RIO_OnTrack*>(measurement);
          const Identifier& hitId = hit->identify();
          FPGATrackSimOfflineHit tmpOfflineHit;
          if (m_pixelId->is_pixel(hitId)) {
            tmpOfflineHit.setIsPixel(true);
            tmpOfflineHit.setIsBarrel(m_pixelId->is_barrel(hitId));

            const InDetDD::SiDetectorElement* sielement = m_PIX_mgr->getDetectorElement(hitId);
            tmpOfflineHit.setClusterID(sielement->identifyHash());
            tmpOfflineHit.setTrackNumber(iTrk);
            tmpOfflineHit.setLayer(m_pixelId->layer_disk(hitId));
            tmpOfflineHit.setLocX((float)measurement->localParameters()[Trk::locX]);
            tmpOfflineHit.setLocY((float)measurement->localParameters()[Trk::locY]);
          }
          else if (m_sctId->is_sct(hitId)) {
            tmpOfflineHit.setIsPixel(false);
            tmpOfflineHit.setIsBarrel(m_sctId->is_barrel(hitId));
            const InDetDD::SiDetectorElement* sielement = m_SCT_mgr->getDetectorElement(hitId);
            tmpOfflineHit.setClusterID(sielement->identifyHash());
            tmpOfflineHit.setTrackNumber(iTrk);
            tmpOfflineHit.setLayer(m_sctId->layer_disk(hitId));
            tmpOfflineHit.setLocX(((float)measurement->localParameters()[Trk::locX]));
            tmpOfflineHit.setLocY(-99999.9);
          }
          tmpOfflineTrack.addHit(tmpOfflineHit);
        }
      }
    }
    offline.push_back(tmpOfflineTrack);
  }//end of loop over tracks


  return StatusCode::SUCCESS;
}



// dump silicon channels with geant matching information.
StatusCode
FPGATrackSimSGToRawHitsTool::readRawSilicon(HitIndexMap& hitIndexMap, const EventContext& eventContext) // const cannot make variables push back to DataInput
{
  ATH_MSG_DEBUG("read silicon hits");
  unsigned int hitIndex = 0u;

  ATH_CHECK(readPixelSimulation(hitIndexMap, hitIndex, eventContext));
  ATH_CHECK(readStripSimulation(hitIndexMap, hitIndex, eventContext));

  return StatusCode::SUCCESS;
}


StatusCode
FPGATrackSimSGToRawHitsTool::readPixelSimulation(HitIndexMap& hitIndexMap, unsigned int& hitIndex, const EventContext& eventContext) {

  auto pixelSDOHandle = SG::makeHandle(m_pixelSDOKey, eventContext);
  auto pixelRDOHandle = SG::makeHandle(m_pixelRDOKey, eventContext);

  ATH_MSG_DEBUG("Found Pixel SDO Map");

  for (const InDetRawDataCollection<PixelRDORawData>* pixel_rdoCollection : *pixelRDOHandle) {
    if (pixel_rdoCollection == nullptr) { continue; }
    // loop on all RDOs
    for (const PixelRDORawData* pixelRawData : *pixel_rdoCollection) {
      Identifier rdoId = pixelRawData->identify();
      // get the det element from the det element collection
      const InDetDD::SiDetectorElement* sielement = m_PIX_mgr->getDetectorElement(rdoId); assert(sielement);

      Amg::Vector2D localPos = sielement->rawLocalPositionOfCell(rdoId);
      Amg::Vector3D globalPos = sielement->globalPosition(localPos);
      InDetDD::SiCellId cellID = sielement->cellIdFromIdentifier(rdoId);

      // update map between pixel identifier and event-unique hit index.
      // ganged pixels (nCells==2) get two entries.
      hitIndexMap[rdoId] = hitIndex;
      const int nCells = sielement->numberOfConnectedCells(cellID);
      if (nCells == 2) {
        const InDetDD::SiCellId tmpCell = sielement->connectedCell(cellID, 1);
        const Identifier tmpId = sielement->identifierFromCellId(tmpCell);
        hitIndexMap[tmpId] = hitIndex; // add second entry for ganged pixel ID
      }
      // if there is simulation truth available, try to retrieve the "most likely" barcode for this pixel.
      FPGATrackSimInputUtils::ParentBitmask parentMask;
      const HepMcParticleLink* bestTruthLink{};
      if (!m_pixelSDOKey.empty()) {
        InDetSimDataCollection::const_iterator iter(pixelSDOHandle->find(rdoId));
        if (nCells > 1 && iter == pixelSDOHandle->end()) {
          InDetDD::SiReadoutCellId SiRC(m_pixelId->phi_index(rdoId), m_pixelId->eta_index(rdoId));
          for (int ii = 0; ii < nCells && iter == pixelSDOHandle->end(); ++ii) {
            iter = pixelSDOHandle->find(sielement->identifierFromCellId(sielement->design().connectedCell(SiRC, ii)));
          }
        } // end search for correct ganged pixel
        // if SDO found for this pixel, associate the particle. otherwise leave unassociated.
        if (iter != pixelSDOHandle->end()) { bestTruthLink = getTruthInformation(iter, parentMask); }
      } // end if pixel truth available
      HepMC::ConstGenParticlePtr bestParent = (bestTruthLink) ? bestTruthLink->cptr() : nullptr;
      ++hitIndex;

      // push back the hit information  to DataInput for HitList
      FPGATrackSimHit tmpSGhit;
      tmpSGhit.setHitType(HitType::unmapped);
      tmpSGhit.setDetType(SiliconTech::pixel);
      tmpSGhit.setIdentifierHash(sielement->identifyHash());
      tmpSGhit.setIdentifier(sielement->identify().get_identifier32().get_compact());

      int barrel_ec = m_pixelId->barrel_ec(rdoId);
      if (barrel_ec == 0)
        tmpSGhit.setDetectorZone(DetectorZone::barrel);
      else if (barrel_ec == 2)
        tmpSGhit.setDetectorZone(DetectorZone::posEndcap);
      else if (barrel_ec == -2)
        tmpSGhit.setDetectorZone(DetectorZone::negEndcap);

      tmpSGhit.setLayerDisk(m_pixelId->layer_disk(rdoId));
      tmpSGhit.setPhiModule(m_pixelId->phi_module(rdoId));
      tmpSGhit.setEtaModule(m_pixelId->eta_module(rdoId));
      tmpSGhit.setPhiIndex(m_pixelId->phi_index(rdoId));
      tmpSGhit.setEtaIndex(m_pixelId->eta_index(rdoId));
      tmpSGhit.setPhiCoord(localPos[0]);
      tmpSGhit.setEtaCoord(localPos[1]);
      tmpSGhit.setEtaWidth(0);
      tmpSGhit.setPhiWidth(0);
      tmpSGhit.setX(globalPos[Amg::x]);
      tmpSGhit.setY(globalPos[Amg::y]);
      tmpSGhit.setZ(globalPos[Amg::z]);
      tmpSGhit.setToT(pixelRawData->getToT());
      tmpSGhit.setisValidForITkHit(true); // Pixel clusters are close enough right now that they all can be considered valid for ITK
      if (bestParent) {
        tmpSGhit.setEventIndex(bestTruthLink->eventIndex());
        tmpSGhit.setBarcode(bestTruthLink->barcode()); // FIXME barcode-based
        tmpSGhit.setUniqueID(bestTruthLink->id()); // May need fixing when uid will be used.
      }
      else {
        tmpSGhit.setEventIndex(std::numeric_limits<long>::max());
        tmpSGhit.setBarcode(std::numeric_limits<HepMcParticleLink::barcode_type>::max());
        tmpSGhit.setUniqueID(std::numeric_limits<HepMcParticleLink::barcode_type>::max());
      }

      tmpSGhit.setBarcodePt(static_cast<unsigned long>(std::ceil(bestParent ? bestParent->momentum().perp() : 0.)));
      tmpSGhit.setParentageMask(parentMask.to_ulong());

      // Add truth
      FPGATrackSimMultiTruth mt;
      FPGATrackSimMultiTruth::Barcode uniqueID(tmpSGhit.getEventIndex(), tmpSGhit.getBarcode()); // FIXME barcode-based
      mt.maximize(uniqueID, tmpSGhit.getBarcodePt()); // FIXME barcode-based
      tmpSGhit.setTruth(mt);

      m_eventHeader->addHit(tmpSGhit);
    } // end for each RDO in the collection
  } // for each pixel RDO collection

  return StatusCode::SUCCESS;
}

StatusCode
FPGATrackSimSGToRawHitsTool::readStripSimulation(HitIndexMap& hitIndexMap, unsigned int& hitIndex, const EventContext& eventContext) {

  constexpr int MaxChannelinStripRow = 128;

  auto stripSDOHandle = SG::makeHandle(m_stripSDOKey, eventContext);
  ATH_MSG_DEBUG("Found SCT SDO Map");
  auto stripRDOHandle = SG::makeHandle(m_stripRDOKey, eventContext);
  for (const InDetRawDataCollection<SCT_RDORawData>* SCT_Collection : *stripRDOHandle) {
    if (SCT_Collection == nullptr) { continue; }

    std::map<int, bool> firedStrips;
    std::map<int, const SCT_RDORawData*> firedStripsToRDO;
    // Preprocess the SCT collection hits to get information for encoding strip in ITK format
    // All strips fired read into a map to an overview of full module that should be used to encode
    // the data into the ITk formatl
    for (const SCT_RDORawData* sctRawData : *SCT_Collection) 
    {
      const Identifier rdoId = sctRawData->identify();
      const int baseLineStrip{m_sctId->strip(rdoId)};
      for(int i = 0; i < sctRawData->getGroupSize(); i++) {
        firedStrips[baseLineStrip+ i] = true;
        firedStripsToRDO[baseLineStrip + i] = sctRawData;
      }
    }

    // Loop over the fired hits and encode them in the ITk strips hit map
    // It find unique hits in the list that can be encoded and don't overlap
    std::map<int, int> stripEncodingForITK;
    std::map<int, const SCT_RDORawData* > stripEncodingForITKToRDO;
    for(const auto& [stripID, fired]: firedStrips)
    {
      // Don't use the strip that has been set false. 
      // This will be the case where neighbouring strip will "used up in the cluster"
      // And then we don't want to re use them 
      if(!fired) continue;

      // Check the next 3 hits if they are there and have a hit in them
      std::bitset<3> hitMap;


      // Get the current chip id of the strip
      int currChipID = stripID / MaxChannelinStripRow;
      // Compute the maximum stripID this chip can have
      int maxStripIDForCurrChip = (currChipID + 1) * MaxChannelinStripRow;

      for(int i = 0; i < 3; i++)
      {    
        // We don't want to "cluster" strips that are outside the range of this chip
        if((stripID + 1 + i) >= maxStripIDForCurrChip) continue;

        if(firedStrips.find(stripID + 1 + i) != firedStrips.end())
        {
          if(firedStrips.at(stripID + 1 + i))
          {
            hitMap[2 - i] = 1;
            firedStrips[stripID + 1 + i] = false;
          }
          else
          {
            hitMap[2 - i] = 0;
          }
        }
      }

      // Encode the hit map into a int
      stripEncodingForITK[stripID] = (int)(hitMap.to_ulong());
      stripEncodingForITKToRDO[stripID] = firedStripsToRDO[stripID];
    }

    // Actual creation of the FPGAHit objects
    for(const auto& [stripID, fired]: firedStrips)
    {
      const SCT_RDORawData* sctRawData = firedStripsToRDO[stripID];
      const Identifier rdoId = sctRawData->identify();
      // get the det element from the det element collection
      const InDetDD::SiDetectorElement* sielement = m_SCT_mgr->getDetectorElement(rdoId);
      const InDetDD::SiDetectorDesign& design = dynamic_cast<const InDetDD::SiDetectorDesign&>(sielement->design());

      InDetDD::SiCellId frontId(stripID);
      Amg::Vector2D localPos = design.localPositionOfCell(frontId);
      std::pair<Amg::Vector3D, Amg::Vector3D> endsOfStrip = sielement->endsOfStrip(localPos);

      hitIndexMap[rdoId] = hitIndex;
      ++hitIndex;
      // if there is simulation truth available, try to retrieve the
      // "most likely" barcode for this strip.
      FPGATrackSimInputUtils::ParentBitmask parentMask;
      const HepMcParticleLink* bestTruthLink{};
      if (!m_stripSDOKey.empty()) {
        InDetSimDataCollection::const_iterator iter(stripSDOHandle->find(rdoId));
        // if SDO found for this strip, associate the particle
        if (iter != stripSDOHandle->end()) { bestTruthLink = getTruthInformation(iter, parentMask); }
      } // end if sct truth available
      HepMC::ConstGenParticlePtr bestParent = (bestTruthLink) ? bestTruthLink->cptr() : nullptr;
      // push back the hit information  to DataInput for HitList , copy from RawInput.cxx

      FPGATrackSimHit tmpSGhit;
      tmpSGhit.setHitType(HitType::unmapped);
      tmpSGhit.setDetType(SiliconTech::strip);
      tmpSGhit.setIdentifierHash(sielement->identifyHash());
      tmpSGhit.setIdentifier(sielement->identify().get_identifier32().get_compact());

      int barrel_ec = m_sctId->barrel_ec(rdoId);
      if (barrel_ec == 0)
        tmpSGhit.setDetectorZone(DetectorZone::barrel);
      else if (barrel_ec == 2)
        tmpSGhit.setDetectorZone(DetectorZone::posEndcap);
      else if (barrel_ec == -2)
        tmpSGhit.setDetectorZone(DetectorZone::negEndcap);

      tmpSGhit.setLayerDisk(m_sctId->layer_disk(rdoId));
      tmpSGhit.setPhiModule(m_sctId->phi_module(rdoId));
      tmpSGhit.setEtaModule(m_sctId->eta_module(rdoId));
      tmpSGhit.setPhiIndex(stripID);
      tmpSGhit.setEtaIndex(m_sctId->row(rdoId));
      tmpSGhit.setPhiCoord(localPos[0]);
      tmpSGhit.setEtaCoord(localPos[1]);
      tmpSGhit.setSide(m_sctId->side(rdoId));
      tmpSGhit.setEtaWidth(0);
      tmpSGhit.setPhiWidth(1);
      if (bestParent) {
        tmpSGhit.setEventIndex(bestTruthLink->eventIndex());
        tmpSGhit.setBarcode(bestTruthLink->barcode()); // FIXME barcode-based
        tmpSGhit.setUniqueID(bestTruthLink->id());
      }
      else {
        tmpSGhit.setEventIndex(std::numeric_limits<long>::max());
        tmpSGhit.setBarcode(std::numeric_limits<HepMcParticleLink::barcode_type>::max());
        tmpSGhit.setUniqueID(std::numeric_limits<HepMcParticleLink::barcode_type>::max());
      }

      // If the strip has been identified by the previous for loop as a valid hit that can be encoded into ITk Strip format
      if(stripEncodingForITK.find(stripID) != stripEncodingForITK.end())
      { 
        // Each ITK ABC chip reads 128 channels in one row, so we just need to divide the current strip with 128 to get the chip index
        // for the Strip ID, it is the remainder left after dividing by 128
        int chipID = stripID / MaxChannelinStripRow;
        int ITkStripID = stripID % MaxChannelinStripRow;

        // for each ABC chip readout, each reads 256 channels actually. 0-127 corresponds to lower row and then 128-255 corresponds to the 
        // upper. This can be simulated in the code by using the eta module index. Even index are not offest, while odd index, the 
        // strip id is offest by 128
        // One point to not is that for barrel, the eta module index start at 1, and not zero. Hence a shift of 1 is needed
        int offset = m_sctId->eta_module(rdoId) % 2;
        if(m_sctId->barrel_ec(rdoId) == 0) offset = (std::abs(m_sctId->eta_module(rdoId)) - 1) % 2;

        ITkStripID += offset * MaxChannelinStripRow;

        tmpSGhit.setisValidForITkHit(true);
        tmpSGhit.setStripRowIDForITk(ITkStripID);
        tmpSGhit.setStripChipIDForITk(chipID);
        tmpSGhit.setStripHitMapForITk(stripEncodingForITK.at(stripID));
      }
      
      tmpSGhit.setBarcodePt(static_cast<unsigned long>(std::ceil(bestParent ? bestParent->momentum().perp() : 0.)));
      tmpSGhit.setParentageMask(parentMask.to_ulong());
      tmpSGhit.setX(0.5 * (endsOfStrip.first.x() + endsOfStrip.second.x()));
      tmpSGhit.setY(0.5 * (endsOfStrip.first.y() + endsOfStrip.second.y()));
      tmpSGhit.setZ(0.5 * (endsOfStrip.first.z() + endsOfStrip.second.z()));

      // Add truth
      FPGATrackSimMultiTruth mt;
      FPGATrackSimMultiTruth::Barcode uniqueID(tmpSGhit.getEventIndex(), tmpSGhit.getBarcode()); // FIXME barcode-based
      mt.maximize(uniqueID, tmpSGhit.getBarcodePt()); // FIMXE barcode-based
      tmpSGhit.setTruth(mt);

      m_eventHeader->addHit(tmpSGhit);
    } // end for each RDO in the strip collection
  } // end for each strip RDO collection
  // dump all RDO's and SDO's for a given event, for debugging purposes

  return StatusCode::SUCCESS;
}


StatusCode
FPGATrackSimSGToRawHitsTool::dumpPixelClusters(HitIndexMap& pixelClusterIndexMap, const EventContext& eventContext) {
  unsigned int pixelClusterIndex = 0;
  auto pixelSDOHandle = SG::makeHandle(m_pixelSDOKey, eventContext);
  auto pixelClusterContainerHandle = SG::makeHandle(m_pixelClusterContainerKey, eventContext);
  // Dump pixel clusters. They're in m_pixelContainer
  for (const InDet::SiClusterCollection* pixelClusterCollection : *pixelClusterContainerHandle) {
    if (pixelClusterCollection == nullptr) {
      ATH_MSG_DEBUG("pixelClusterCollection not available!");
      continue;
    }

    for (const InDet::SiCluster* cluster : *pixelClusterCollection) {
      Identifier theId = cluster->identify();
      // if there is simulation truth available, try to retrieve the "most likely" barcode for this pixel cluster.
      FPGATrackSimInputUtils::ParentBitmask parentMask; // FIXME set, but not used
      if (!m_pixelSDOKey.empty()) {
        for (const Identifier& rdoId : cluster->rdoList()) {
          const InDetDD::SiDetectorElement* sielement = m_PIX_mgr->getDetectorElement(rdoId);
          assert(sielement);
          InDetDD::SiCellId cellID = sielement->cellIdFromIdentifier(rdoId);

          const int nCells = sielement->numberOfConnectedCells(cellID);
          InDetSimDataCollection::const_iterator iter(pixelSDOHandle->find(rdoId));
          // this might be the ganged pixel copy.
          if (nCells > 1 && iter == pixelSDOHandle->end()) {
            InDetDD::SiReadoutCellId SiRC(m_pixelId->phi_index(rdoId), m_pixelId->eta_index(rdoId));
            for (int ii = 0; ii < nCells && iter == pixelSDOHandle->end(); ++ii) {
              iter = pixelSDOHandle->find(sielement->identifierFromCellId(sielement->design().connectedCell(SiRC, ii)));
            }
          } // end search for correct ganged pixel
          // if SDO found for this pixel, associate the particle. otherwise leave unassociated.
          if (iter != pixelSDOHandle->end()) { (void) getTruthInformation(iter, parentMask); } // FIXME not used??
        } // if we have pixel sdo's available
      }
      pixelClusterIndexMap[theId] = pixelClusterIndex;
      pixelClusterIndex++;
    } // End loop over pixel clusters
  } // End loop over pixel cluster collection

  return StatusCode::SUCCESS;
}

StatusCode
FPGATrackSimSGToRawHitsTool::readOfflineClusters(std::vector <FPGATrackSimCluster>& clusters, const EventContext& eventContext)
{

  //Lets do the Pixel clusters first
  //Loopover the pixel clusters and convert them into a FPGATrackSimCluster for storage
  // Dump pixel clusters. They're in m_pixelContainer
  auto pixelSDOHandle = SG::makeHandle(m_pixelSDOKey, eventContext);
  auto pixelClusterContainerHandler = SG::makeHandle(m_pixelClusterContainerKey, eventContext);
  for (const InDet::SiClusterCollection* pixelClusterCollection : *pixelClusterContainerHandler) {
    if (pixelClusterCollection == nullptr) {
      ATH_MSG_DEBUG("pixelClusterCollection not available!");
      continue;
    }
    const int size = pixelClusterCollection->size();
    ATH_MSG_DEBUG("PixelClusterCollection found with " << size << " clusters");
    for (const InDet::SiCluster* cluster : *pixelClusterCollection) {

      // if there is simulation truth available, try to retrieve the "most likely" barcode for this pixel cluster.
      FPGATrackSimInputUtils::ParentBitmask parentMask;
      const HepMcParticleLink* bestTruthLink{};
      if (!m_pixelSDOKey.empty()) {
        for (const Identifier& rdoId : cluster->rdoList()) {
          const InDetDD::SiDetectorElement* sielement = m_PIX_mgr->getDetectorElement(rdoId);
          assert(sielement);
          InDetDD::SiCellId cellID = sielement->cellIdFromIdentifier(rdoId);
          const int nCells = sielement->numberOfConnectedCells(cellID);
          InDetSimDataCollection::const_iterator iter(pixelSDOHandle->find(rdoId));
          // this might be the ganged pixel copy.
          if (nCells > 1 && iter == pixelSDOHandle->end()) {
            InDetDD::SiReadoutCellId SiRC(m_pixelId->phi_index(rdoId), m_pixelId->eta_index(rdoId));
            for (int ii = 0; ii < nCells && iter == pixelSDOHandle->end(); ++ii) {
              iter = pixelSDOHandle->find(sielement->identifierFromCellId(sielement->design().connectedCell(SiRC, ii)));
            }
          } // end search for correct ganged pixel
          // if SDO found for this pixel, associate the particle. otherwise leave unassociated.
          if (iter != pixelSDOHandle->end()) { bestTruthLink = getTruthInformation(iter, parentMask); }
        } // if we have pixel sdo's available
      }
      HepMC::ConstGenParticlePtr bestParent = (bestTruthLink) ? bestTruthLink->cptr() : nullptr;

      Identifier theID = cluster->identify();
      //cluster object to be written out
      FPGATrackSimCluster clusterOut;
      //Rawhit object to represent the cluster
      FPGATrackSimHit clusterEquiv;
      //Lets get the information of this pixel cluster
      const InDetDD::SiDetectorElement* sielement = m_PIX_mgr->getDetectorElement(theID);
      assert(sielement);
      const InDetDD::SiLocalPosition localPos = sielement->rawLocalPositionOfCell(theID);
      const Amg::Vector3D globalPos(sielement->globalPosition(localPos));
      clusterEquiv.setHitType(HitType::clustered);
      clusterEquiv.setX(globalPos.x());
      clusterEquiv.setY(globalPos.y());
      clusterEquiv.setZ(globalPos.z());
      clusterEquiv.setDetType(SiliconTech::pixel);
      clusterEquiv.setIdentifierHash(sielement->identifyHash());
      clusterEquiv.setIdentifier(sielement->identify().get_identifier32().get_compact());

      int barrel_ec = m_pixelId->barrel_ec(theID);
      if (barrel_ec == 0)
        clusterEquiv.setDetectorZone(DetectorZone::barrel);
      else if (barrel_ec == 2)
        clusterEquiv.setDetectorZone(DetectorZone::posEndcap);
      else if (barrel_ec == -2)
        clusterEquiv.setDetectorZone(DetectorZone::negEndcap);

      clusterEquiv.setLayerDisk(m_pixelId->layer_disk(theID));
      clusterEquiv.setPhiModule(m_pixelId->phi_module(theID));
      clusterEquiv.setEtaModule(m_pixelId->eta_module(theID));
      clusterEquiv.setPhiIndex(m_pixelId->phi_index(theID));
      clusterEquiv.setEtaIndex(m_pixelId->eta_index(theID));
      clusterEquiv.setPhiCoord(localPos.xPhi());
      clusterEquiv.setEtaCoord(localPos.xEta());

      clusterEquiv.setPhiWidth(cluster->width().colRow()[1]);
      clusterEquiv.setEtaWidth(cluster->width().colRow()[0]);
      //Save the truth here as the MultiTruth object is only transient
      if (bestParent) {
        clusterEquiv.setEventIndex(bestTruthLink->eventIndex());
        clusterEquiv.setBarcode(bestTruthLink->barcode()); // FIXME barcode-based
        clusterEquiv.setUniqueID(bestTruthLink->id());
      }
      else {
        clusterEquiv.setEventIndex(std::numeric_limits<long>::max());
        clusterEquiv.setBarcode(std::numeric_limits<HepMcParticleLink::barcode_type>::max());
        clusterEquiv.setUniqueID(std::numeric_limits<HepMcParticleLink::barcode_type>::max());
      }

      clusterEquiv.setBarcodePt(static_cast<unsigned long>(std::ceil(bestParent ? bestParent->momentum().perp() : 0.)));
      clusterEquiv.setParentageMask(parentMask.to_ulong());
      clusterOut.setClusterEquiv(clusterEquiv);
      clusters.push_back(clusterOut);
    }
  }

  //Now lets do the strip clusters
  //Loopover the pixel clusters and convert them into a FPGATrackSimCluster for storage
  // Dump pixel clusters. They're in m_pixelContainer
  auto stripSDOHandle = SG::makeHandle(m_stripSDOKey, eventContext);
  ATH_MSG_DEBUG("Found SCT SDO Map");
  auto stripRDOHandle = SG::makeHandle(m_stripRDOKey, eventContext);

  for (const InDetRawDataCollection<SCT_RDORawData>* SCT_Collection : *stripRDOHandle) {
    if (SCT_Collection == nullptr) { continue; }
    for (const SCT_RDORawData* sctRawData : *SCT_Collection) {
      const Identifier rdoId = sctRawData->identify();
      // get the det element from the det element collection
      const InDetDD::SiDetectorElement* sielement = m_SCT_mgr->getDetectorElement(rdoId);
      const InDetDD::SiDetectorDesign& design = dynamic_cast<const InDetDD::SiDetectorDesign&>(sielement->design());
      const InDetDD::SiLocalPosition localPos = design.localPositionOfCell(m_sctId->strip(rdoId));
      const Amg::Vector3D gPos = sielement->globalPosition(localPos);
      // if there is simulation truth available, try to retrieve the
      // "most likely" barcode for this strip.
      FPGATrackSimInputUtils::ParentBitmask parentMask;
      const HepMcParticleLink* bestTruthLink{};
      if (!m_stripSDOKey.empty()) {
        InDetSimDataCollection::const_iterator iter(stripSDOHandle->find(rdoId));
        // if SDO found for this pixel, associate the particle
        if (iter != stripSDOHandle->end()) { bestTruthLink = getTruthInformation(iter, parentMask); }
      } // end if sct truth available
      HepMC::ConstGenParticlePtr bestParent = (bestTruthLink) ? bestTruthLink->cptr() : nullptr;

      // push back the hit information  to DataInput for HitList , copy from RawInput.cxx
      FPGATrackSimCluster clusterOut;
      FPGATrackSimHit clusterEquiv;
      clusterEquiv.setHitType(HitType::clustered);
      clusterEquiv.setX(gPos.x());
      clusterEquiv.setY(gPos.y());
      clusterEquiv.setZ(gPos.z());
      clusterEquiv.setDetType(SiliconTech::strip);
      clusterEquiv.setIdentifierHash(sielement->identifyHash());
      clusterEquiv.setIdentifier(sielement->identify().get_identifier32().get_compact());

      int barrel_ec = m_sctId->barrel_ec(rdoId);
      if (barrel_ec == 0)
        clusterEquiv.setDetectorZone(DetectorZone::barrel);
      else if (barrel_ec == 2)
        clusterEquiv.setDetectorZone(DetectorZone::posEndcap);
      else if (barrel_ec == -2)
        clusterEquiv.setDetectorZone(DetectorZone::negEndcap);

      clusterEquiv.setLayerDisk(m_sctId->layer_disk(rdoId));
      clusterEquiv.setPhiModule(m_sctId->phi_module(rdoId));
      clusterEquiv.setEtaModule(m_sctId->eta_module(rdoId));
      clusterEquiv.setPhiIndex(m_sctId->strip(rdoId));
      clusterEquiv.setEtaIndex(m_sctId->row(rdoId));
      clusterEquiv.setPhiCoord(localPos.xPhi());
      clusterEquiv.setEtaCoord(localPos.xEta());
      clusterEquiv.setSide(m_sctId->side(rdoId));
      //I think this is the strip "cluster" width
      clusterEquiv.setPhiWidth(sctRawData->getGroupSize());
      //Save the truth here as the MultiTruth object is only transient
      if (bestParent) {
        clusterEquiv.setEventIndex(bestTruthLink->eventIndex());
        clusterEquiv.setBarcode(bestTruthLink->barcode()); // FIXME barcode-based
        clusterEquiv.setUniqueID(bestTruthLink->id());
      }
      else {
        clusterEquiv.setEventIndex(std::numeric_limits<long>::max());
        clusterEquiv.setBarcode(std::numeric_limits<HepMcParticleLink::barcode_type>::max());
        clusterEquiv.setUniqueID(std::numeric_limits<HepMcParticleLink::barcode_type>::max());
      }

      clusterEquiv.setBarcodePt(static_cast<unsigned long>(std::ceil(bestParent ? bestParent->momentum().perp() : 0.)));
      clusterEquiv.setParentageMask(parentMask.to_ulong());
      clusterOut.setClusterEquiv(clusterEquiv);
      clusters.push_back(clusterOut);
    } // end for each RDO in the strip collection
  } // end for each strip RDO collection
  // dump all RDO's and SDO's for a given event, for debugging purposes

  return StatusCode::SUCCESS;
}

StatusCode
FPGATrackSimSGToRawHitsTool::readTruthTracks(std::vector <FPGATrackSimTruthTrack>& truth, const EventContext& eventContext)
{
  auto simTracksHandle = SG::makeHandle(m_mcCollectionKey, eventContext);
  ATH_MSG_DEBUG("Dump truth tracks, size " << simTracksHandle->size());

  // dump each truth track
  for (unsigned int ievt = 0; ievt < simTracksHandle->size(); ++ievt) {
    const HepMC::GenEvent* genEvent = simTracksHandle->at(ievt);
    // retrieve the primary interaction vertex here. for now, use the dummy origin.
    HepGeom::Point3D<double>  primaryVtx(0., 0., 0.);
    // the event should have signal process vertex unless it was generated as single particles.
    // if it exists, use it for the primary vertex.
    HepMC::ConstGenVertexPtr spv = HepMC::signal_process_vertex(genEvent);
    if (spv) {
        primaryVtx.set(spv->position().x(),
                       spv->position().y(),
                       spv->position().z());
      ATH_MSG_DEBUG("using signal process vertex for eventIndex " << ievt << ":"
        << primaryVtx.x() << "\t" << primaryVtx.y() << "\t" << primaryVtx.z());
    }
    for (const auto& particle: *genEvent) {
      const int pdgcode = particle->pdg_id();
      // reject generated particles without a production vertex.
      if (particle->production_vertex() == nullptr) {
        continue;
      }
      // reject neutral or unstable particles
      const HepPDT::ParticleData* pd = m_particleDataTable->particle(abs(pdgcode));
      if (pd == nullptr) {
        continue;
      }
      float charge = pd->charge();
      if (pdgcode < 0) charge *= -1.; // since we took absolute value above
      if (std::abs(charge) < 0.5) {
        continue;
      }
      if (!MC::isStable(particle)) {
        continue;
      }
      // truth-to-track tool
      const Amg::Vector3D momentum(particle->momentum().px(), particle->momentum().py(), particle->momentum().pz());
      const Amg::Vector3D position(particle->production_vertex()->position().x(), particle->production_vertex()->position().y(), particle->production_vertex()->position().z());
      const Trk::CurvilinearParameters cParameters(position, momentum, charge);
      Trk::PerigeeSurface persf;
      if (m_UseNominalOrigin) {
        Amg::Vector3D    origin(0, 0, 0);
        persf = Trk::PerigeeSurface(origin);
      }
      else {
        SG::ReadCondHandle<InDet::BeamSpotData> beamSpotHandle{ m_beamSpotKey, eventContext };
        Trk::PerigeeSurface persf(beamSpotHandle->beamPos());
      }
      const std::unique_ptr<Trk::TrackParameters> tP = m_extrapolator->extrapolate(eventContext, cParameters, persf, Trk::anyDirection, false);
      const double track_truth_d0 = tP ? tP->parameters()[Trk::d0] : 999.;
      const double track_truth_phi = tP ? tP->parameters()[Trk::phi] : 999.;
      const double track_truth_p = (tP && fabs(tP->parameters()[Trk::qOverP]) > 1.e-8) ?
        tP->charge() / tP->parameters()[Trk::qOverP] : 10E7;
      const double track_truth_x0 = tP ? tP->position().x() : 999.;
      const double track_truth_y0 = tP ? tP->position().y() : 999.;
      const double track_truth_z0 = tP ? tP->parameters()[Trk::z0] : 999.;
      const double track_truth_q = tP ? tP->charge() : 0.;
      const double track_truth_sinphi = tP ? std::sin(tP->parameters()[Trk::phi]) : -1.;
      const double track_truth_cosphi = tP ? std::cos(tP->parameters()[Trk::phi]) : -1.;
      const double track_truth_sintheta = tP ? std::sin(tP->parameters()[Trk::theta]) : -1.;
      const double track_truth_costheta = tP ? std::cos(tP->parameters()[Trk::theta]) : -1.;
      double truth_d0corr = track_truth_d0 - (primaryVtx.y() * cos(track_truth_phi) - primaryVtx.x() * sin(track_truth_phi));
      double truth_zvertex = 0.;
      const HepGeom::Point3D<double> startVertex(particle->production_vertex()->position().x(), particle->production_vertex()->position().y(), particle->production_vertex()->position().z());
      // categorize particle (prompt, secondary, etc.) based on InDetPerformanceRTT/detector paper criteria.
      bool isPrimary = true;
      if (std::abs(truth_d0corr) > 2.) { isPrimary = false; }
      const int bc = HepMC::barcode(particle); // FIXME update barcode-based syntax
      const int uid = HepMC::uniqueID(particle);
      if (HepMC::is_simulation_particle(particle) || bc == 0) { isPrimary = false; } // FIXME update barcode-based syntax
      if (isPrimary && particle->production_vertex()) {
        const HepGeom::Point3D<double> startVertex(particle->production_vertex()->position().x(), particle->production_vertex()->position().y(), particle->production_vertex()->position().z());
        if (std::abs(startVertex.z() - truth_zvertex) > 100.) { isPrimary = false; }
        if (particle->end_vertex()) {
          HepGeom::Point3D<double> endVertex(particle->end_vertex()->position().x(), particle->end_vertex()->position().y(), particle->end_vertex()->position().z());
          if (endVertex.perp() < FPGATrackSim_PT_TRUTHMIN && std::abs(endVertex.z()) < FPGATrackSim_Z_TRUTHMIN) { isPrimary = false; }
        }
      }
      else {
        isPrimary = false;
      }

      HepMcParticleLink truthLink2(uid, ievt, HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_ID);
      
      FPGATrackSimTruthTrack tmpSGTrack;
      tmpSGTrack.setVtxX(track_truth_x0);
      tmpSGTrack.setVtxY(track_truth_y0);
      tmpSGTrack.setVtxZ(track_truth_z0);
      tmpSGTrack.setD0(track_truth_d0);
      tmpSGTrack.setZ0(track_truth_z0);
      tmpSGTrack.setVtxZ(primaryVtx.z());
      tmpSGTrack.setQ(track_truth_q);
      tmpSGTrack.setPX(track_truth_p * (track_truth_cosphi * track_truth_sintheta));
      tmpSGTrack.setPY(track_truth_p * (track_truth_sinphi * track_truth_sintheta));
      tmpSGTrack.setPZ(track_truth_p * track_truth_costheta);
      tmpSGTrack.setPDGCode(pdgcode);
      tmpSGTrack.setStatus(particle->status());

      tmpSGTrack.setBarcode(truthLink2.barcode());
      tmpSGTrack.setUniqueID(truthLink2.id());
      tmpSGTrack.setEventIndex(truthLink2.eventIndex());

      truth.push_back(tmpSGTrack);
    } // end for each GenParticle in this GenEvent
  } // end for each GenEvent


  return StatusCode::SUCCESS;
}


const HepMcParticleLink* FPGATrackSimSGToRawHitsTool::getTruthInformation(InDetSimDataCollection::const_iterator& iter,
                                                                          FPGATrackSimInputUtils::ParentBitmask& parentMask) {
  const HepMcParticleLink* bestTruthLink{};
  const InDetSimData& sdo(iter->second);
  const std::vector<InDetSimData::Deposit>& deposits(sdo.getdeposits());
  float bestPt{-999.f};
  for (const InDetSimData::Deposit& dep : deposits) {

    const HepMcParticleLink& particleLink = dep.first;
    // RDO's without SDO's are delta rays or detector noise.
    if (!particleLink.isValid()) { continue; }
    const float genEta = particleLink->momentum().pseudoRapidity();
    const float genPt = particleLink->momentum().perp(); // MeV
    // reject unstable particles
    if (!MC::isStable(particleLink.cptr())) { continue; }
    // reject secondaries and low pT (<400 MeV) pileup
    if (HepMC::is_simulation_particle(particleLink.cptr()) || particleLink.barcode() == 0 /*HepMC::no_truth_link(particleLink)*/) { continue; }  // FIXME
    // reject far forward particles
    if (std::fabs(genEta) > m_maxEta) { continue; }
    // "bestTruthLink" links to the highest pt particle
    if (bestPt < genPt) {
      bestPt = genPt;
      bestTruthLink = &particleLink;
    }
 #ifdef HEPMC3
     parentMask |= FPGATrackSimInputUtils::construct_truth_bitmap(std::shared_ptr<const HepMC3::GenParticle>(particleLink.cptr()));
 #else
     parentMask |= FPGATrackSimInputUtils::construct_truth_bitmap(particleLink.cptr());
 #endif
     // check SDO
  } // end for each contributing particle
  return bestTruthLink;
}
