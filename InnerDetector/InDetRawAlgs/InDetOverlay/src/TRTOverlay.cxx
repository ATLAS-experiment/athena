/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/RNGWrapper.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Random/RandFlat.h"
#include "AtlasHepMC/GenParticle.h"
//
#include "InDetIdentifier/TRT_ID.h"
#include "InDetOverlay/TRTOverlay.h"
#include "InDetRawData/InDetRawData.h"
#include "InDetRawData/TRT_LoLumRawData.h"
#include "InDetRawData/TRT_RDORawData.h"
#include "InDetSimData/InDetSimDataCollection.h"
#include "TRT_ConditionsData/StrawStatus.h"
//
#include "IDC_OverlayBase/IDC_OverlayHelpers.h" //debugPrint
//
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"


namespace
{
struct TRTRDOSorter {
  bool operator()(TRT_RDORawData *digit1, TRT_RDORawData *digit2) {
    return digit1->identify() < digit2->identify();
  }
} TRTRDOSorterObject;

std::unique_ptr<TRT_RDO_Collection> copyCollection(
    const IdentifierHash &hashId,
    const TRT_RDO_Collection *collection,
    DataPool<TRT_LoLumRawData>& dataItemsPool) {

  auto outputCollection = std::make_unique<TRT_RDO_Collection>(hashId);
  outputCollection->setIdentifier(collection->identify());

  //Elements created here are owned by the DataPool
  outputCollection->clear(SG::VIEW_ELEMENTS);
  outputCollection->reserve(collection->size());
  for (const TRT_RDORawData *existingDatum : *collection) {
    TRT_LoLumRawData* datumCopy = dataItemsPool.nextElementPtr();
    (*datumCopy) = TRT_LoLumRawData(existingDatum->identify(), existingDatum->getWord());
    outputCollection->push_back(datumCopy);
  }

  return outputCollection;
}

std::unique_ptr<TRT_RDO_Collection> copyCollectionAndSort(
    const IdentifierHash &hashId,
    const TRT_RDO_Collection *collection,
    DataPool<TRT_LoLumRawData>& dataItemsPool) {

  std::unique_ptr<TRT_RDO_Collection> outputCollection = copyCollection(hashId, collection,dataItemsPool);
  std::stable_sort(outputCollection->begin(), outputCollection->end(), TRTRDOSorterObject);
  return outputCollection;
}
} // anonymous namespace


TRTOverlay::TRTOverlay(const std::string &name, ISvcLocator *pSvcLocator)
  : AthReentrantAlgorithm(name, pSvcLocator)
{
}

StatusCode TRTOverlay::initialize()
{
  ATH_MSG_DEBUG("Initializing...");

  // Check and initialize keys
  ATH_CHECK( m_bkgInputKey.initialize(!m_bkgInputKey.key().empty()) );
  ATH_MSG_VERBOSE("Initialized ReadHandleKey: " << m_bkgInputKey);
  ATH_CHECK( m_signalInputKey.initialize() );
  ATH_MSG_VERBOSE("Initialized ReadHandleKey: " << m_signalInputKey);
  ATH_CHECK( m_outputKey.initialize() );
  ATH_MSG_VERBOSE("Initialized WriteHandleKey: " << m_outputKey);
  ATH_CHECK( m_signalInputSDOKey.initialize() );
  ATH_MSG_VERBOSE("Initialized ReadHandleKey for SDO: " << m_signalInputSDOKey);
  ATH_CHECK( m_strawStatusHTKey.initialize() );
  ATH_MSG_VERBOSE("Initialized ReadCondHandleKey: " << m_strawStatusHTKey);

  // Retrieve TRT ID helper
  if (!detStore()->retrieve(m_trtId, "TRT_ID").isSuccess() || !m_trtId) {
    ATH_MSG_FATAL("Cannot retrieve TRT ID helper");
    return StatusCode::FAILURE;
  }

  // Initialize random number generator
  CHECK(m_rndmSvc.retrieve());

  // Retrieve TRT local occupancy tool
  CHECK(m_TRT_LocalOccupancyTool.retrieve());

  return StatusCode::SUCCESS;
}


StatusCode TRTOverlay::execute(const EventContext& ctx) const
{
  ATH_MSG_DEBUG("execute() begin");

  // Reading the input RDOs
  const TRT_RDO_Container *bkgContainerPtr = nullptr;
  if (!m_bkgInputKey.empty()) {
    SG::ReadHandle<TRT_RDO_Container> bkgContainer(m_bkgInputKey, ctx);
    if (!bkgContainer.isValid()) {
      ATH_MSG_ERROR("Could not get background TRT RDO container " << bkgContainer.name() << " from store " << bkgContainer.store());
      return StatusCode::FAILURE;
    }
    bkgContainerPtr = bkgContainer.cptr();

    ATH_MSG_DEBUG("Found background TRT RDO container " << bkgContainer.name() << " in store " << bkgContainer.store());
    ATH_MSG_DEBUG("TRT Background = " << Overlay::debugPrint(bkgContainer.cptr()));
  }

  SG::ReadHandle<TRT_RDO_Container> signalContainer(m_signalInputKey, ctx);
  if (!signalContainer.isValid()) {
    ATH_MSG_ERROR("Could not get signal TRT RDO container " << signalContainer.name() << " from store " << signalContainer.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Found signal TRT RDO container " << signalContainer.name() << " in store " << signalContainer.store());
  ATH_MSG_DEBUG("TRT Signal     = " << Overlay::debugPrint(signalContainer.cptr()));

  SG::ReadHandle<InDetSimDataCollection> signalSDOContainer(m_signalInputSDOKey, ctx);
  if (!signalSDOContainer.isValid()) {
    ATH_MSG_ERROR("Could not get signal TRT SDO map container " << signalSDOContainer.name() << " from store " << signalSDOContainer.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Found signal TRT SDO map container " << signalSDOContainer.name() << " in store " << signalSDOContainer.store());

  // The DataPool, this is what will actually own the elements
  // we create during this algorithm. The containers are
  // views.
  DataPool<TRT_LoLumRawData> dataItemsPool(ctx);
  dataItemsPool.prepareToAdd(200000);

  // Creating output RDO container
  SG::WriteHandle<TRT_RDO_Container> outputContainer(m_outputKey, ctx);
  ATH_CHECK(outputContainer.record(std::make_unique<TRT_RDO_Container>(signalContainer->size())));
  if (!outputContainer.isValid()) {
    ATH_MSG_ERROR("Could not record output TRT RDO container " << outputContainer.name() << " to store " << outputContainer.store());
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Recorded output TRT RDO container " << outputContainer.name() << " in store " << outputContainer.store());

  ATH_CHECK(overlayContainer(ctx, bkgContainerPtr, signalContainer.cptr(),
                             outputContainer.ptr(), signalSDOContainer.cptr(),
                             dataItemsPool));
  ATH_MSG_DEBUG(
      "TRT Result   = " << Overlay::debugPrint(outputContainer.ptr()));

  ATH_MSG_DEBUG("execute() end");
  return StatusCode::SUCCESS;
}


StatusCode TRTOverlay::overlayContainer(const EventContext &ctx,
                                        const TRT_RDO_Container *bkgContainer,
                                        const TRT_RDO_Container *signalContainer,
                                        TRT_RDO_Container *outputContainer,
                                        const InDetSimDataCollection *signalSDOCollection,
                                        DataPool<TRT_LoLumRawData>& dataItemsPool) const
{

  // There are some use cases where background is empty
  if (!bkgContainer) {
    // Only loop through the signal collections and copy them over
    for (const auto [hashId, ptr] : signalContainer->GetAllHashPtrPair()) {
      // Copy the signal collection
      // pools own the individual elements
      std::unique_ptr<TRT_RDO_Collection> signalCollection = copyCollection(hashId, ptr, dataItemsPool);

      if (outputContainer->addCollection(signalCollection.get(), hashId).isFailure()) {
        ATH_MSG_ERROR("Adding signal Collection with hashId " << hashId << " failed");
        return StatusCode::FAILURE;
      } else {
        (void)signalCollection.release();
      }
    }

    return StatusCode::SUCCESS;
  }

  // Setup the random engine
  ATHRNG::RNGWrapper* rngWrapper = m_rndmSvc->getEngine(this);
  rngWrapper->setSeed( name(), ctx );
  CLHEP::HepRandomEngine *rndmEngine = rngWrapper->getEngine(ctx);

  // Load TRT conditions
  SG::ReadCondHandle<TRTCond::StrawStatusData> strawStatusHTHandle{ m_strawStatusHTKey, ctx };
  const TRTCond::StrawStatusData *strawStatusHT{*strawStatusHTHandle};

  // Retrieve the occupancy map
  std::map<int, double> occupancyMap = m_TRT_LocalOccupancyTool->getDetectorOccupancy(ctx, bkgContainer);

  // The MC signal container should typically be smaller than bkgContainer,
  // because the latter contains all the noise, minimum bias and pile up.
  // Thus we firstly iterate over signal hashes and store them in a map.
  std::vector < std::pair<IdentifierHash, bool> > overlapMap;
  overlapMap.reserve(signalContainer->numberOfCollections());
  for (const auto [hashId, ptr] : signalContainer->GetAllHashPtrPair()) {
    overlapMap.emplace_back(hashId, false);
  }

  // Now loop through the background hashes and copy unique ones over
  for (const auto [hashId, ptr] : bkgContainer->GetAllHashPtrPair()) {
    auto search = std::lower_bound( overlapMap.begin(), overlapMap.end(), hashId,
     [](const std::pair<IdentifierHash, bool> &lhs,  IdentifierHash rhs) -> bool { return lhs.first < rhs; } );
    if (search == overlapMap.end() || search->first != hashId) {
      // Copy the background collection
      std::unique_ptr<TRT_RDO_Collection> bkgCollection{};
      if (m_sortBkgInput) {
        // copy the bkg again Pool owns the individual elements
        bkgCollection = copyCollectionAndSort(hashId, bkgContainer->indexFindPtr(hashId),dataItemsPool);
      } else {
        bkgCollection = copyCollection(hashId, bkgContainer->indexFindPtr(hashId),dataItemsPool);
      }

      if (outputContainer->addCollection(bkgCollection.get(), hashId).isFailure()) {
        ATH_MSG_ERROR("Adding background Collection with hashId " << hashId << " failed");
        return StatusCode::FAILURE;
      } else {
        //coverity[RESOURCE_LEAK]
        (void)bkgCollection.release();
      }
    } else {
      // Flip the overlap flag
      search->second = true;
    }
  }

  // Finally loop through the map and process the signal and overlay if
  // necessary
  for (const auto &[hashId, overlap] : overlapMap) {
    // Copy the signal collection the pool owns the individual elements
    std::unique_ptr<TRT_RDO_Collection> signalCollection =
        copyCollection(hashId, signalContainer->indexFindPtr(hashId),dataItemsPool);

    if (overlap) { // Do overlay
      // Create the output collection, only works for Inner Detector
      auto outputCollection = std::make_unique<TRT_RDO_Collection>(hashId);
      outputCollection->setIdentifier(signalCollection->identify());
      // This will receive merged elements from the other containers.
      // There elements are owned actually by the DataPool
      outputCollection->clear(SG::VIEW_ELEMENTS);

      // Copy the background collection the pool owns the individual elements
      std::unique_ptr<TRT_RDO_Collection> bkgCollection{};
      if (m_sortBkgInput) {
        bkgCollection = copyCollectionAndSort(hashId, bkgContainer->indexFindPtr(hashId),dataItemsPool);
      } else {
        bkgCollection = copyCollection(hashId, bkgContainer->indexFindPtr(hashId),dataItemsPool);
      }

      // Merge collections
      int det = m_trtId->barrel_ec(signalCollection->identify());
      mergeCollections(bkgCollection.get(),
                       signalCollection.get(),
                       outputCollection.get(),
                       occupancyMap[det],
                       signalSDOCollection,
                       strawStatusHT,
                       rndmEngine);

      if (outputContainer->addCollection(outputCollection.get(), hashId).isFailure()) {
        ATH_MSG_ERROR("Adding overlaid Collection with hashId " << hashId << " failed");
        return StatusCode::FAILURE;
      } else {
        outputCollection.release();
      }
    } else { // Only write signal out
      if (outputContainer->addCollection(signalCollection.get(), hashId).isFailure()) {
        ATH_MSG_ERROR("Adding signal Collection with hashId " << hashId << " failed");
        return StatusCode::FAILURE;
      } else {
        (void)signalCollection.release();
      }
    }
  }

  return StatusCode::SUCCESS;
}

/// Here we take 2 view containers with elements owned by the DataPool
/// we modify some of them and push them to a 3rd view container.
void TRTOverlay::mergeCollections(TRT_RDO_Collection *bkgCollection,
                                  TRT_RDO_Collection *signalCollection,
                                  TRT_RDO_Collection *outputCollection,
                                  double occupancy,
                                  const InDetSimDataCollection *signalSDOCollection,
                                  const TRTCond::StrawStatusData *strawStatusHT,
                                  CLHEP::HepRandomEngine *rndmEngine) const
{
  if (bkgCollection->identify() != signalCollection->identify()) {
    throw std::runtime_error("mergeCollections(): collection Id mismatch");
  }

  // Merge by copying ptrs from background and signal to output collection
  TRT_RDO_Collection::size_type ibkg = 0, isig = 0;

  outputCollection->reserve(
      std::max(bkgCollection->size(), signalCollection->size()));
  // Below we have swapElements.
  // Remember the elements of the signalCollection and bkgCollection
  // containers are owned by the DataPool.
  // tmpBkg and tmp are whaterver elements we take out of the containers.
  //
  // So
  // A) We can not delete them. dataPool will do that at the end of the event.
  // B) We can push them back only to a View so outputCollection is a view
  // collection
  // C) We pass nullptr so no need to get another item from the pool

  while ((ibkg < bkgCollection->size()) || (isig < signalCollection->size())) {
    // The RDO that goes to the output at the end of this step.
    TRT_RDORawData *tmp{};

    if (isig == signalCollection->size()) {
      // just copy the remaining background digits
      bkgCollection->swapElement(ibkg++, nullptr, tmp);
    } else if (ibkg == bkgCollection->size()) {
      // just copy the remaining signal digits
      signalCollection->swapElement(isig++, nullptr, tmp);
    } else {
      // Need to decide which one goes first.
      // See comments in TRTDigitization.cxx about the assumption that id1<id2 <=> hash1<hash2
      if (signalCollection->at(isig)->identify() < bkgCollection->at(ibkg)->identify()) {
        signalCollection->swapElement(isig++, nullptr, tmp);
      } else if (bkgCollection->at(ibkg)->identify() < signalCollection->at(isig)->identify()) {
        bkgCollection->swapElement(ibkg++, nullptr, tmp);
      } else {
        // The hits are on the same channel.
        TRT_RDORawData *tmpBkg{};

        bkgCollection->swapElement(ibkg++, nullptr, tmpBkg);
        signalCollection->swapElement(isig++, nullptr, tmp);

        TRT_LoLumRawData *sigRdo = dynamic_cast<TRT_LoLumRawData *>(tmp);
        const TRT_LoLumRawData *bkgRdo = dynamic_cast<const TRT_LoLumRawData *>(tmpBkg);

        if (sigRdo && bkgRdo) {
          // the actual merging
          sigRdo->merge(*bkgRdo);

          // If the hit is not already a high level hit
          if (!(sigRdo->getWord() & 0x04020100)) {
            // Determine if the hit is from an electron or not
            bool isElectron = false;
            Identifier rdoId = sigRdo->identify();
            InDetSimDataCollection::const_iterator sdoIter = signalSDOCollection->find(rdoId);
            if (sdoIter != signalSDOCollection->end()) {
              const std::vector<InDetSimData::Deposit> &deposits = sdoIter->second.getdeposits();
              for (const InDetSimData::Deposit &deposit : deposits) {
                const HepMcParticleLink &particleLink = deposit.first;
                if (particleLink.isValid()) {
                  if (std::abs(particleLink->pdg_id()) == 11) {
                    isElectron = true;
                    break;
                  }
                }
              }
            }

            // Determine what type of straw was hit
            bool isXenonStraw = false;
            if (strawStatusHT != nullptr) {
              if (strawStatusHT->findStatus(m_trtId->straw_hash(rdoId)) == TRTCond::StrawStatus::Good) {
                isXenonStraw = true;
              }
            }

            // Get random number
            int det = m_trtId->barrel_ec(rdoId);
            float HTOccupancyCorrection = 0.;
            if (isXenonStraw) {
              if (isElectron) {
                HTOccupancyCorrection = std::abs(det) > 1 ? m_HTOccupancyCorrectionEC : m_HTOccupancyCorrectionB;
              } else {
                HTOccupancyCorrection = std::abs(det) > 1 ? m_HTOccupancyCorrectionEC_noE : m_HTOccupancyCorrectionB_noE;
              }
            } else {
              if (isElectron) {
                HTOccupancyCorrection = std::abs(det) > 1 ? m_HTOccupancyCorrectionEC_Ar : m_HTOccupancyCorrectionB_Ar;
              } else {
                HTOccupancyCorrection = std::abs(det) > 1 ? m_HTOccupancyCorrectionEC_Ar_noE : m_HTOccupancyCorrectionB_Ar_noE;
              }
            }

            unsigned int newWord = 0;
            if (HTOccupancyCorrection != 0. && occupancy * HTOccupancyCorrection > CLHEP::RandFlat::shoot(rndmEngine, 0, 1)) {
              newWord += 1 << (26-9);
            }

            TRT_LoLumRawData newRdo(rdoId, newWord);
            sigRdo->merge(newRdo);
          }
        } else {
          ATH_MSG_WARNING("TRT RDO is the wrong format");
        }
      }
    }

    outputCollection->push_back(tmp);
  } // <= while
}
