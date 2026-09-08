/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// SCT_PrepDataToxAOD.cxx
//   Implementation file for class SCT_PrepDataToxAOD
///////////////////////////////////////////////////////////////////

#include "SCT_PrepDataToxAOD.h"

#include "Identifier/Identifier.h"
#include "InDetIdentifier/SCT_ID.h"
#include "InDetRawData/SCT_RDO_Collection.h"
#include "InDetSimEvent/SiHit.h"
#include "InDetReadoutGeometry/SiDetectorElement.h"
#include "ReadoutGeometryBase/SiCellId.h"
#include "ReadoutGeometryBase/SiLocalPosition.h"
#include "SCT_ReadoutGeometry/SCT_ModuleSideDesign.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "xAODTracking/TrackMeasurementValidationAuxContainer.h"
#include "TruthUtils/MagicNumbers.h"

#include "AtlasHepMC/GenParticle.h"

#include "CLHEP/Geometry/Point3D.h"

#include <algorithm>
#include <cmath>

#define AUXDATA(OBJ, TYP, NAME)                                         \
  static const SG::AuxElement::Accessor<TYP> acc_##NAME (#NAME);  acc_##NAME(*(OBJ))


/////////////////////////////////////////////////////////////////////
//
//        Initialize method: 
//
/////////////////////////////////////////////////////////////////////
StatusCode SCT_PrepDataToxAOD::initialize()
{
  ATH_CHECK(detStore()->retrieve(m_SCTHelper, "SCT_ID"));

  //make sure we don't write what we don't have
  if (not m_useTruthInfo) {
    m_writeSDOs.set(false);
    m_writeSiHits.set(false);
  }

  ATH_CHECK(m_clustercontainer.initialize());
  ATH_CHECK(m_SDOcontainer.initialize(m_writeSDOs));
  ATH_CHECK(m_sihitContainer.initialize(m_writeSiHits));
  ATH_CHECK(m_multiTruth.initialize(m_useTruthInfo));
  ATH_CHECK(m_rdoContainer.initialize(m_writeRDOinformation));
  ATH_CHECK(m_xAodContainer.initialize());
  ATH_CHECK(m_xAodOffset.initialize());
  ATH_CHECK(m_truthParticleLinks.initialize( m_useTruthInfo && !m_truthParticleLinks.empty()));

  ATH_CHECK(m_SCTDetEleCollKey.initialize());

  return StatusCode::SUCCESS;
}

/////////////////////////////////////////////////////////////////////
//
//        Execute method: 
//
/////////////////////////////////////////////////////////////////////
StatusCode SCT_PrepDataToxAOD::execute(const EventContext& ctx) const
{
  // the cluster ambiguity map
  std::map<Identifier, const SCT_RDORawData*> idToRAWDataMap;
  if (m_writeRDOinformation) {
    SG::ReadHandle<SCT_RDO_Container> rdoContainer{m_rdoContainer, ctx};
    if (rdoContainer.isValid()) {
      // get all the RIO_Collections in the container
      for (const auto collection: *rdoContainer) {
        //get all the RDOs in the collection
        for (const auto rdo : *collection) {
          if (rdo==nullptr) {
            ATH_MSG_WARNING("Null SCT RDO. Skipping it");
            continue;
          }
          Identifier rdoId{rdo->identify()};
          idToRAWDataMap.insert(std::pair<Identifier, const SCT_RDORawData*>{rdoId, rdo});
        } // collection
      } // Have container;
    } else if (m_firstEventWarnings) {
      ATH_MSG_WARNING("Failed to retrieve SCT RDO container");
    }
  }
  ATH_MSG_DEBUG("Size of RDO map is " << idToRAWDataMap.size());

  const PRD_MultiTruthCollection* prdmtColl{nullptr};
  const xAODTruthParticleLinkVector *truth_particle_links{nullptr};
  if (m_useTruthInfo) {
    SG::ReadHandle<PRD_MultiTruthCollection> prdmtCollHandle{m_multiTruth, ctx};
    if (prdmtCollHandle.isValid()) {
      prdmtColl = &*prdmtCollHandle;
    }
    if (!m_truthParticleLinks.empty()) {
       SG::ReadHandle<xAODTruthParticleLinkVector> truthParticleLinksHandle{m_truthParticleLinks, ctx};
       if (truthParticleLinksHandle.isValid()) {
          truth_particle_links = truthParticleLinksHandle.cptr();
       }
    }
  }

  const InDetSimDataCollection* sdoCollection{nullptr};
  if (m_writeSDOs) {
    SG::ReadHandle<InDetSimDataCollection> sdoCollectionHandle{m_SDOcontainer, ctx};
    if (sdoCollectionHandle.isValid()) {
      sdoCollection = &*sdoCollectionHandle;
    }
  }

  std::vector<std::vector<const SiHit*>> siHits(m_SCTHelper->wafer_hash_max());
  if (m_writeSiHits) {
    SG::ReadHandle<SiHitCollection> sihitCollection{m_sihitContainer, ctx};
    // ITk split sensors: a SiHit carries the mother sensor's eta_module while
    // the cluster lives on a child-row wafer (eta_module = mother + row), so
    // route each hit to its child wafer hash as StripDigitizationTool does.
    // Unsplit sensors have no mother and are unaffected. Without the element
    // collection the hits stay on the mother wafer.
    SG::ReadCondHandle<InDetDD::SiDetectorElementCollection> sctDetEleHandle{m_SCTDetEleCollKey, ctx};
    const InDetDD::SiDetectorElementCollection* sctElements =
        sctDetEleHandle.isValid() ? sctDetEleHandle.cptr() : nullptr;
    if (sihitCollection.isValid()) {
      for (const SiHit& siHit: *sihitCollection) {
        // Check if it is an SCT hit
        if (not siHit.isSCT()) continue;

        Identifier wafer_id{m_SCTHelper->wafer_id(siHit.getBarrelEndcap(),
                                                  siHit.getLayerDisk(),
                                                  siHit.getPhiModule(),
                                                  siHit.getEtaModule(),
                                                  siHit.getSide())};
        IdentifierHash wafer_hash{m_SCTHelper->wafer_hash(wafer_id)};

        if (sctElements) {
          const InDetDD::SiDetectorElement* baseEl = sctElements->getDetectorElement(wafer_hash);
          if (baseEl) {
            const auto* design =
                dynamic_cast<const InDetDD::SCT_ModuleSideDesign*>(&baseEl->design());
            const InDetDD::SCT_ModuleSideDesign* mother = design ? design->getMother() : nullptr;
            if (mother) {
              HepGeom::Point3D<double> avg{siHit.localStartPosition() + siHit.localEndPosition()};
              avg *= 0.5;
              // {xPhi, xEta, xDepth}; the mother-design row is the child eta offset.
              const HepGeom::Point3D<double> p3{baseEl->hitLocalToLocal3D(avg)};
              const InDetDD::SiLocalPosition slp(p3.y(), p3.x()); // (xEta along, xPhi across)
              const InDetDD::SiCellId mDiode = mother->cellIdOfPosition(slp);
              if (mDiode.isValid()) {
                const int rowOffset = mother->getStripRow(mDiode).second;
                if (rowOffset != 0) {
                  const Identifier child_id{m_SCTHelper->wafer_id(siHit.getBarrelEndcap(),
                                                                  siHit.getLayerDisk(),
                                                                  siHit.getPhiModule(),
                                                                  siHit.getEtaModule() + rowOffset,
                                                                  siHit.getSide())};
                  wafer_hash = m_SCTHelper->wafer_hash(child_id);
                }
              }
            }
          }
        }

        if (wafer_hash < siHits.size()) {
          siHits[wafer_hash].push_back(&siHit);
        } else {
          ATH_MSG_WARNING("SiHit routed to an unknown SCT wafer (eta_module "
                          << siHit.getEtaModule() << ", hash " << wafer_hash << "), dropped");
        }
      }
    }
  }
  
  // Mandatory. This is needed and required if this algorithm is scheduled.
  SG::ReadHandle<InDet::SCT_ClusterContainer> sctClusterContainer{m_clustercontainer, ctx};
  if (not sctClusterContainer.isValid()) {
    ATH_MSG_FATAL("Cannot retrieve SCT PrepDataContainer " << m_clustercontainer.key());
    return StatusCode::FAILURE;
  }

  // Create the xAOD container and its auxiliary store:
  SG::WriteHandle<xAOD::TrackMeasurementValidationContainer> xaod{m_xAodContainer, ctx};
  ATH_CHECK(xaod.record(std::make_unique<xAOD::TrackMeasurementValidationContainer>(),
                        std::make_unique<xAOD::TrackMeasurementValidationAuxContainer>()));
  
  SG::WriteHandle<std::vector<unsigned int>> offsets{m_xAodOffset, ctx};
  ATH_CHECK(offsets.record(std::make_unique<std::vector<unsigned int>>(m_SCTHelper->wafer_hash_max(), 0)));

  unsigned int have_truth_link=0u;
  unsigned int missing_truth_particle=0u;
  unsigned int missing_parent_particle=0u;
  // Loop over the container
  unsigned int counter{0};
  for (const auto clusterCollection: *sctClusterContainer) {
    //Fill Offset container
    (*offsets)[clusterCollection->identifyHash()] = counter;
    
    // skip empty collections
    if (clusterCollection->empty()) continue;

    xaod->resize(counter + clusterCollection->size());
    // loop over collection and convert to xAOD
    for (const InDet::SCT_Cluster* prd: *clusterCollection) {
      Identifier clusterId{prd->identify()};
      if (not clusterId.is_valid()) {
        ATH_MSG_WARNING("SCT cluster identifier is not valid!");
      }

      // create and add xAOD object
      xAOD::TrackMeasurementValidation* xprd{new xAOD::TrackMeasurementValidation()};
      xaod->at(counter) = xprd;
      ++counter;

      //Set Identifier
      xprd->setIdentifier(clusterId.get_compact());

      //Set Global Position
      Amg::Vector3D gpos{prd->globalPosition()};
      xprd->setGlobalPosition(gpos.x(), gpos.y(), gpos.z());

      //Set Local Position
      const Amg::Vector2D& locpos{prd->localPosition()};
      float locY{0.};
      float locX{static_cast<float>(locpos.x())};
      if ((not std::isinf(locpos.y()) or std::isnan(locpos.y()))) {
        if (locpos.y()>=1e-07) locY = locpos.y();
      } else { 
        locY = -9999.;
      }

      // Set local error matrix
      xprd->setLocalPosition(locX, locY);

      const Amg::MatrixX& localCov{prd->localCovariance()};
      if (localCov.size() == 1) {
        xprd->setLocalPositionError(localCov(0, 0), 0., 0.);
      } else if (localCov.size() == 4) {
        xprd->setLocalPositionError(localCov(0, 0), localCov(1, 1), localCov(0, 1));
      } else {
        xprd->setLocalPositionError(0., 0., 0.);
      }
         
      // Set vector of hit identifiers
      std::vector<uint64_t> rdoIdentifierList;
      rdoIdentifierList.reserve(prd->rdoList().size());
      for (const auto& hitIdentifier: prd->rdoList()) {
        rdoIdentifierList.push_back(hitIdentifier.get_compact());
      }
      xprd->setRdoIdentifierList(rdoIdentifierList);

      //Add SCT specific information
      const InDet::SiWidth cw{prd->width()};
      AUXDATA(xprd, int, SiWidth) = static_cast<int>(cw.colRow()[0]);
      AUXDATA(xprd, int, hitsInThirdTimeBin) = static_cast<int>(prd->hitsInThirdTimeBin());

      AUXDATA(xprd, int, bec)        = m_SCTHelper->barrel_ec(clusterId);
      AUXDATA(xprd, int, layer)      = m_SCTHelper->layer_disk(clusterId);
      AUXDATA(xprd, int, phi_module) = m_SCTHelper->phi_module(clusterId);
      AUXDATA(xprd, int, eta_module) = m_SCTHelper->eta_module(clusterId);
      AUXDATA(xprd, int, side)       = m_SCTHelper->side(clusterId);
   
      // Add the Detector element ID  --  not sure if needed as we have the informations above
      const InDetDD::SiDetectorElement* de{prd->detectorElement()};

      uint64_t detElementId{0};
      if (de) {
        Identifier detId{de->identify()};
        if (detId.is_valid()) {
          detElementId = detId.get_compact();
        }
      }
      AUXDATA(xprd, uint64_t, detectorElementID) = detElementId;
   
      //Add details about the individual hits 
      if (m_writeRDOinformation) {
        addRDOInformation(xprd, prd, idToRAWDataMap);
      }
      
      // Use the MultiTruth Collection to get a list of all true particle contributing to the cluster
      if (m_useTruthInfo) {
        if (prdmtColl) {
          auto range{prdmtColl->equal_range(clusterId)};
          if (truth_particle_links) {
             std::vector<unsigned int> tp_indices;
             for (auto i{range.first}; i!=range.second; ++i) {
                ElementLink<xAOD::TruthParticleContainer> a_truth_particle_link = truth_particle_links->find(i->second);
                if (a_truth_particle_link) {
                   const xAOD::TruthParticle *truth_particle = *a_truth_particle_link;
                   if (truth_particle) {
                      ++have_truth_link;
                      tp_indices.push_back(static_cast<int>(truth_particle->index()));
                   }
                   else {
                      ++missing_parent_particle;
                   }
                }
                else {
                   tp_indices.push_back(std::numeric_limits<unsigned int>::max());
                   ++missing_truth_particle;
                }
             }
             AUXDATA(xprd, std::vector<unsigned int>, truth_index) = std::move(tp_indices);
          }
          std::vector<int> uniqueIDs;
          for (auto& i{range.first}; i!=range.second; ++i) {
            uniqueIDs.push_back(HepMC::uniqueID(i->second));
          }
          AUXDATA(xprd, std::vector<int>, truth_barcode) = std::move(uniqueIDs); // TODO rename variable to be consistent?
        }
      }

      // Use the SDO Collection to get a list of all true particle contributing to the cluster per readout element
      //  Also get the energy deposited by each true particle per readout element   
      std::vector<std::vector<int>> sdoTruthUIDs;
      if (m_writeSDOs) {
        if (sdoCollection) {
          sdoTruthUIDs = addSDOInformation(xprd, prd, sdoCollection);
        }
      }
    
      // Now Get the most detailed truth from the SiHits
      // Note that this could get really slow if there are a lot of hits and clusters
      if (m_writeSiHits) {
        addSiHitInformation(xprd, prd, &siHits[prd->detectorElement()->identifyHash()], sdoTruthUIDs);
      }
    }
  }
  ATH_MSG_DEBUG(" recorded SCT_PrepData objects: size " << xaod->size());

  m_haveTruthLink += have_truth_link;
  m_missingTruthParticle += missing_truth_particle;
  m_missingParentParticle += missing_parent_particle;
  m_firstEventWarnings = false; //disable one-time warnings

  return StatusCode::SUCCESS;
}

std::vector<std::vector<int>> SCT_PrepDataToxAOD::addSDOInformation(xAOD::TrackMeasurementValidation* xprd,
                                                                    const InDet::SCT_Cluster* prd,
                                                                    const InDetSimDataCollection* sdoCollection) const
{
  std::vector<int> sdo_word;
  std::vector<std::vector<int>> sdo_depositsUniqueID;
  std::vector<std::vector<float>> sdo_depositsEnergy;
  // find hit
  for (const auto& hitIdentifier: prd->rdoList()) {
    auto pos{sdoCollection->find(hitIdentifier)};
    if (pos == sdoCollection->end()) continue;
    sdo_word.push_back(pos->second.word());
    std::vector<int> sdoDepUniqueID(pos->second.getdeposits().size(), HepMC::INVALID_PARTICLE_ID);
    std::vector<float> sdoDepEnergy(pos->second.getdeposits().size());
    unsigned int nDepos{0};
    for (auto& deposit: pos->second.getdeposits()) {
      if (deposit.first) sdoDepUniqueID[nDepos] = HepMC::uniqueID(deposit.first);
      ATH_MSG_DEBUG(" SDO Energy Deposit " << deposit.second);
      sdoDepEnergy[nDepos] = deposit.second;
      nDepos++;
    }
    sdo_depositsUniqueID.push_back(std::move(sdoDepUniqueID));
    sdo_depositsEnergy.push_back(std::move(sdoDepEnergy));
  }
  AUXDATA(xprd, std::vector<int>, sdo_words) = std::move(sdo_word);
  AUXDATA(xprd, std::vector<std::vector<int>>, sdo_depositsBarcode) = sdo_depositsUniqueID; // TODO rename variable to be consistent?
  AUXDATA(xprd, std::vector<std::vector<float>>, sdo_depositsEnergy) = std::move(sdo_depositsEnergy);
  return sdo_depositsUniqueID;
}


void SCT_PrepDataToxAOD::addSiHitInformation(xAOD::TrackMeasurementValidation* xprd, 
                                             const InDet::SCT_Cluster* prd,
                                             const std::vector<const SiHit*>* siHits,
                                             const std::vector<std::vector<int>>& sdoTruthUIDs) const
{
  std::vector<SiHit> matchingHits;
  findAllHitsCompatibleWithCluster(prd, siHits, sdoTruthUIDs, matchingHits);

  long unsigned int numHits{matchingHits.size()};

  std::vector<float> sihit_energyDeposit(numHits, 0.);
  std::vector<float> sihit_meanTime(numHits, 0.);
  std::vector<int> sihit_uniqueID(numHits, HepMC::UNDEFINED_ID);
  
  std::vector<float> sihit_startPosX(numHits, 0.);
  std::vector<float> sihit_startPosY(numHits, 0.);
  std::vector<float> sihit_startPosZ(numHits, 0.);

  std::vector<float> sihit_endPosX(numHits, 0);
  std::vector<float> sihit_endPosY(numHits, 0);
  std::vector<float> sihit_endPosZ(numHits, 0);

  int hitNumber{0};
  const InDetDD::SiDetectorElement* de{prd->detectorElement()};
  if (de) {
    // ITk split sensors: hitLocalToLocal3D is an axis permutation without
    // translation, so the along-strip coordinate comes out in the mother frame.
    // Subtract the child's row shift to land in the child (cluster) frame.
    // StripBoxDesign keeps that shift as a translation along the design's
    // local x, which is the strip direction (it calls it zShift because the
    // strips run along global z in the barrel); in the {xPhi, xEta, xDepth}
    // frame returned by hitLocalToLocal3D it acts on xEta. StripGmxInterface
    // signs the shift per side, but the mother eta direction is the same on
    // both sides, so undo the sign on side 1.
    double alongStripShift = 0.;
    if (const auto* d = dynamic_cast<const InDetDD::SCT_ModuleSideDesign*>(&de->design())) {
      if (d->getMother()) {
        alongStripShift = d->moduleShift().translation().x();
        if (m_SCTHelper->side(prd->identify()) != 0) alongStripShift = -alongStripShift;
      }
    }
    for (const SiHit& sihit : matchingHits) {
      sihit_energyDeposit[hitNumber] = sihit.energyLoss();
      sihit_meanTime[hitNumber] = sihit.meanTime();
      sihit_uniqueID[hitNumber] = HepMC::uniqueID(sihit.particleLink());

      // hitLocalToLocal3D respects each design's axes (Run-3 SCT and ITk strips
      // order the SiHit components differently) and returns {xPhi, xEta, xDepth}.
      const HepGeom::Point3D<double> s{de->hitLocalToLocal3D(sihit.localStartPosition())};
      sihit_startPosX[hitNumber] = s.x();
      sihit_startPosY[hitNumber] = s.y() - alongStripShift;  // xEta, along the strip
      sihit_startPosZ[hitNumber] = s.z();

      const HepGeom::Point3D<double> e{de->hitLocalToLocal3D(sihit.localEndPosition())};
      sihit_endPosX[hitNumber] = e.x();
      sihit_endPosY[hitNumber] = e.y() - alongStripShift;
      sihit_endPosZ[hitNumber] = e.z();
      ++hitNumber;
    }
  }

  AUXDATA(xprd, std::vector<float>, sihit_energyDeposit) = std::move(sihit_energyDeposit);
  AUXDATA(xprd, std::vector<float>, sihit_meanTime) = std::move(sihit_meanTime);
  AUXDATA(xprd, std::vector<int>, sihit_barcode) = std::move(sihit_uniqueID); // TODO rename variable to be consistent?
  
  AUXDATA(xprd, std::vector<float>, sihit_startPosX) = std::move(sihit_startPosX);
  AUXDATA(xprd, std::vector<float>, sihit_startPosY) = std::move(sihit_startPosY);
  AUXDATA(xprd, std::vector<float>, sihit_startPosZ) = std::move(sihit_startPosZ);

  AUXDATA(xprd, std::vector<float>, sihit_endPosX) = std::move(sihit_endPosX);
  AUXDATA(xprd, std::vector<float>, sihit_endPosY) = std::move(sihit_endPosY);
  AUXDATA(xprd, std::vector<float>, sihit_endPosZ) = std::move(sihit_endPosZ);
}

void SCT_PrepDataToxAOD::findAllHitsCompatibleWithCluster(const InDet::SCT_Cluster* prd, 
                                                          const std::vector<const SiHit*>* siHits,
                                                          const std::vector<std::vector<int>>& sdoTruthUIDs,
                                                          std::vector<SiHit>& matchingHits) const
{
  ATH_MSG_VERBOSE("Got " << siHits->size() << " SiHits to look through");

  // Check if we have detector element  --  needed to find the local position of the SiHits
  const InDetDD::SiDetectorElement* de{prd->detectorElement()};
  if (de==nullptr) return;

  std::vector<const SiHit*> multiMatchingHits;

  for (const SiHit* siHit: *siHits) {
    // Match by geometry (SiHit centroid within +/-1 strip of a cluster RDO) or,
    // failing that, by the SiHit's truth particle having deposited charge in one
    // of the cluster's RDOs. Charge sharing and steep angles on ITk strips push
    // the centroid several strips away from the fired strips.
    bool matched = false;
    if (m_useSiHitsGeometryMatching) {
      HepGeom::Point3D<double> averagePosition{siHit->localStartPosition() + siHit->localEndPosition()};
      averagePosition *= 0.5;
      const HepGeom::Point3D<double> p3{de->hitLocalToLocal3D(averagePosition)};
      const Amg::Vector2D pos{p3.x(), p3.y()}; // (xPhi across, xEta along)
      InDetDD::SiCellId diode{de->cellIdOfPosition(pos)};

      for (const auto& hitIdentifier: prd->rdoList()) {
        ATH_MSG_DEBUG("Truth Strip " <<  diode.phiIndex() << " Cluster Strip " << m_SCTHelper->strip(hitIdentifier));

        if (std::abs(static_cast<int>(diode.phiIndex()) - m_SCTHelper->strip(hitIdentifier))<=1) {
          multiMatchingHits.push_back(siHit);
          matched = true;
          break;
        }
      }
    }
    if (!matched) {
      const auto uid = HepMC::uniqueID(siHit->particleLink());
      for (const auto& uniqueIDSDOColl: sdoTruthUIDs) {
        if (std::find(uniqueIDSDOColl.begin(), uniqueIDSDOColl.end(), uid) == uniqueIDSDOColl.end()) continue;
        multiMatchingHits.push_back(siHit);
        break;
      }
    }
  }

  matchingHits.reserve(multiMatchingHits.size());
  //Now we will now make 1 SiHit for each true particle if the SiHits "touch" other 
  std::vector<const SiHit*>::iterator siHitIter{multiMatchingHits.begin()};
  std::vector<const SiHit*>::iterator siHitIter2{multiMatchingHits.begin()};
  ATH_MSG_DEBUG("Found " << multiMatchingHits.size() << " SiHit ");
  for (; siHitIter != multiMatchingHits.end(); ++siHitIter) {
    const SiHit* lowestXPos{*siHitIter};
    const SiHit* highestXPos{*siHitIter};

    // We will merge these hits
    std::vector<const SiHit*> ajoiningHits;
    ajoiningHits.push_back(*siHitIter);
  
    siHitIter2 = siHitIter+1;
    auto uniqueID = HepMC::uniqueID((*siHitIter)->particleLink());
    while (siHitIter2 != multiMatchingHits.end()) {
      // Need to come from the same truth particle
      if ( uniqueID != HepMC::uniqueID((*siHitIter2)->particleLink())) {
        ++siHitIter2;
        continue;
      }

      constexpr double maxDiff = 0.00005;
      // Check to see if the SiHits are compatible with each other.
      if (std::abs((highestXPos->localEndPosition().x()-(*siHitIter2)->localStartPosition().x()))<maxDiff and
          std::abs((highestXPos->localEndPosition().y()-(*siHitIter2)->localStartPosition().y()))<maxDiff and
          std::abs((highestXPos->localEndPosition().z()-(*siHitIter2)->localStartPosition().z()))<maxDiff) {
        highestXPos = *siHitIter2;
        ajoiningHits.push_back(*siHitIter2);
        // Dont use hit  more than once
        siHitIter2 = multiMatchingHits.erase(siHitIter2);
      } else if (std::abs((lowestXPos->localStartPosition().x()-(*siHitIter2)->localEndPosition().x()))<maxDiff and
                 std::abs((lowestXPos->localStartPosition().y()-(*siHitIter2)->localEndPosition().y()))<maxDiff and
                 std::abs((lowestXPos->localStartPosition().z()-(*siHitIter2)->localEndPosition().z()))<maxDiff) {
        lowestXPos = *siHitIter2;
        ajoiningHits.push_back(*siHitIter2);
        // Dont use hit  more than once
        siHitIter2 = multiMatchingHits.erase(siHitIter2);
      } else {
        ++siHitIter2;
      }
    }
    
    if (ajoiningHits.size()==0) {
      ATH_MSG_ERROR("This should really never happen");
      continue;
    } else if (ajoiningHits.size()==1) {
      // Copy Si Hit ready to return
      matchingHits.emplace_back(*ajoiningHits[0]);
      continue;
    } else {
      //  Build new SiHit and merge information together.
      ATH_MSG_DEBUG("Merging " << ajoiningHits.size() << " SiHits together.");
      float energyDep{0.};
      float time{0.};
      for (auto& siHit: ajoiningHits) {
        energyDep += siHit->energyLoss();
        time += siHit->meanTime();
      }
      time /= static_cast<float>(ajoiningHits.size());
      matchingHits.emplace_back(lowestXPos->localStartPosition(), 
                                highestXPos->localEndPosition(),
                                energyDep,
                                time,
                                (*siHitIter)->particleLink(),
                                1, // 0 for pixel 1 for SCT
                                (*siHitIter)->getBarrelEndcap(),
                                (*siHitIter)->getLayerDisk(),
                                (*siHitIter)->getEtaModule(),
                                (*siHitIter)->getPhiModule(),
                                (*siHitIter)->getSide());
    }
  }
}

void SCT_PrepDataToxAOD::addRDOInformation(xAOD::TrackMeasurementValidation* xprd, 
                                           const InDet::SCT_Cluster* prd,
                                           const std::map<Identifier, const SCT_RDORawData*>& idToRAWDataMap) const {
  std::vector<int> strip(prd->rdoList().size(), -1);
  std::vector<int> timebin(prd->rdoList().size(), -1);
  std::vector<int> groupsize(prd->rdoList().size(), -1);

  unsigned int nRDOs{0};
  for (const auto& hitIdentifier: prd->rdoList()) {
    auto result{idToRAWDataMap.find(hitIdentifier)};
    if (result != idToRAWDataMap.end()) {
      const SCT_RDORawData* sctRdo{result->second};
      if (sctRdo){
        strip[nRDOs] = m_SCTHelper->strip(sctRdo->identify());
        const SCT3_RawData* rdo3{dynamic_cast<const SCT3_RawData*>(sctRdo)};
        if (rdo3) {
          timebin[nRDOs] = rdo3->getTimeBin();
          groupsize[nRDOs] = rdo3->getGroupSize();
        }
      }
    }
    nRDOs++;
  }
  
  AUXDATA(xprd, std::vector<int>, rdo_strip) = std::move(strip);
  AUXDATA(xprd, std::vector<int>, rdo_timebin) = std::move(timebin);
  AUXDATA(xprd, std::vector<int>, rdo_groupsize) = std::move(groupsize);
}

/////////////////////////////////////////////////////////////////////
//
//        Finalize method: 
//
/////////////////////////////////////////////////////////////////////
StatusCode SCT_PrepDataToxAOD::finalize()
{
   if (m_useTruthInfo && !m_truthParticleLinks.empty()) {
      ATH_MSG_INFO("Missing truth particles " << m_missingTruthParticle << " missing parent: " << m_missingParentParticle
                    << " have " << m_haveTruthLink);
   }
  return StatusCode::SUCCESS;
}
