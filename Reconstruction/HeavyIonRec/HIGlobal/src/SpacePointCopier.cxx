/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <algorithm>
#include "InDetPrepRawData/PixelCluster.h"
#include "InDetIdentifier/PixelID.h"
#include "InDetIdentifier/SCT_ID.h"
#include "SpacePointCopier.h"
#include "AthContainers/ConstAccessor.h"
#include "AthContainers/Accessor.h"

SpacePointCopier::SpacePointCopier(const std::string& name, ISvcLocator* pSvcLocator) :
  AthReentrantAlgorithm(name, pSvcLocator)
{
}

SpacePointCopier::~SpacePointCopier()
{
}

StatusCode SpacePointCopier::initialize()
{
  ATH_CHECK( m_pixelSPKey.initialize() );
  ATH_CHECK( m_SCTSPKey.initialize() );
  ATH_CHECK( m_tracksKey.initialize() );
  ATH_CHECK( m_outputKey.initialize() );
  return StatusCode::SUCCESS;
}

StatusCode SpacePointCopier::finalize()
{
  return StatusCode::SUCCESS;
}

StatusCode SpacePointCopier::execute(const EventContext& context) const
{
  auto pixelSPContainer = SG::makeHandle(m_pixelSPKey, context);
  auto SCTSPContainer = SG::makeHandle(m_SCTSPKey, context);

  auto output = std::make_unique<xAOD::BaseContainer>();
  auto outputAux = std::make_unique<xAOD::AuxContainerBase>();
  output->setStore(outputAux.get());

  size_t pixSize = 0;
  for ( auto coll: *pixelSPContainer ) {
      pixSize += coll->size();
  }
  size_t SCTSize = 0;
  for ( auto coll: *SCTSPContainer ) {
      SCTSize += coll->size();
  }
  // avoid recording for large events (for them the output collection will be empty)
  auto goodNumberOfSpacePoints = [this, SCTSize, pixSize](){    
    return pixSize <= m_maxPixSP 
    or SCTSize <= m_maxSCTSP 
    or pixSize + SCTSize <= m_maxTotalSP;
  };

  auto goodNumberOfTracks = [this, &context]() {
    auto tracksHandle = SG::makeHandle(m_tracksKey, context);
    return tracksHandle->size() <= m_maxTracks;
  };

  if ( goodNumberOfSpacePoints() and goodNumberOfTracks() ) {
    ATH_MSG_DEBUG("Converting "  << pixSize + SCTSize << " SPs");
    static const SG::Accessor< float > x ("x");
    static const SG::Accessor< float > y ("y");
    static const SG::Accessor< float > z ("z");
    static const SG::Accessor< float > tot ("tot");
    static const SG::Accessor< short > csize ("csize");
    static const SG::Accessor< unsigned int > module1 ("module1");
    static const SG::Accessor< unsigned int > module2 ("module2");

    const PixelID *pixelID = nullptr;
    const SCT_ID *stripID = nullptr;
    
    ATH_CHECK(detStore()->retrieve(pixelID, "PixelID"));
    ATH_CHECK(detStore()->retrieve(stripID, "SCT_ID"));


    for ( auto coll: *pixelSPContainer ) {
      for ( auto sp: *coll ) {
        auto *item = new SG::AuxElement();
        output->push_back( item );
        x(*item) =  float(sp->globalPosition().x());
        y(*item) =  float(sp->globalPosition().y());
        z(*item) =  float(sp->globalPosition().z());
        const InDet::PixelCluster* cluster = static_cast<const InDet::PixelCluster *>(sp->clusterList().first);
        tot(*item) = float(cluster->totalToT());
        csize(*item) = short(cluster->totList().size());
        auto[hashId1, hashId2]  = sp->elementIdList();
        module1(*item) = hashId1;
        module2(*item) = hashId2;     

      }
    }


    for ( auto coll: *SCTSPContainer ) {
      for ( auto sp: *coll ) {
        auto *item = new SG::AuxElement();
        output->push_back( item );
        x(*item) =  float(sp->globalPosition().x());
        y(*item) =  float(sp->globalPosition().y());
        z(*item) =  float(sp->globalPosition().z());   
        tot(*item) = 0;
        csize(*item) = 0;

        auto[hashId1, hashId2]  = sp->elementIdList();
        module1(*item) = hashId1;
        module2(*item) = hashId2;     
      }
    }
    for ( size_t i = 0; i < std::min(10ul, output->size()); ++i ) {
      static const SG::ConstAccessor<float> xAcc ("x");
      static const SG::ConstAccessor<float> yAcc ("y");
      static const SG::ConstAccessor<float> zAcc ("z");
      ATH_MSG_DEBUG("Saves SP x y z: " << xAcc(*output->at(i))
                                       << " " << yAcc(*output->at(i))
                                       << " " << zAcc(*output->at(i)) );
    }
    ATH_MSG_DEBUG("... and more ...");
  }

  auto outputHandle = SG::makeHandle(m_outputKey, context);
  ATH_CHECK( outputHandle.record(std::move(output), std::move(outputAux)));

  return StatusCode::SUCCESS;
}

