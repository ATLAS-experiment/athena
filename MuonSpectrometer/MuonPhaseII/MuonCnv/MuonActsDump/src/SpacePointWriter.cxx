/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

 #include "SpacePointWriter.h"
 #include "StoreGate/ReadHandle.h"
 #include "MuonReadoutGeometryR4/Chamber.h" 
 #include "MuonPatternEvent/Segment.h"
 #include "xAODMuonPrepData/UtilFunctions.h"
 #include "MuonSpacePoint/SpacePointPerLayerSorter.h"

 #include "Acts/Definitions/Tolerance.hpp"
 #include "Acts/Definitions/Units.hpp"
 #include "Acts/Utilities/Helpers.hpp"
 #include "Acts/Surfaces/PlaneSurface.hpp"

 using namespace Acts::UnitLiterals;

namespace {
    constexpr float toFloat(const double x) {

        if (std::abs(x) < Acts::s_epsilon) {
            return 0.f;
        }
        constexpr double min = 3.*static_cast<double>(std::numeric_limits<float>::min());
        constexpr double max = static_cast<double>(std::numeric_limits<float>::max());
        const double clampedX = std::copysign(std::clamp(std::abs(x), min, max), x);
      return static_cast<float>(clampedX);
   }

}
 namespace MuonValR4{
    StatusCode SpacePointWriter::initialize(){
       ATH_CHECK(m_tree.init(this));
       ATH_CHECK(m_spacePointKeys.initialize());
       ATH_CHECK(m_trackingGeometryTool.retrieve());
       ATH_MSG_DEBUG("Successfully initialized");
       return StatusCode::SUCCESS;
    }
    StatusCode SpacePointWriter::execute(const EventContext& ctx){
      unsigned bucketCounter{0u};
      m_eventId = ctx.eventID().event_number();

      const Acts::GeometryContext tgContext = m_trackingGeometryTool->getGeometryContext(ctx).context();

      const MuonR4::SpacePointPerLayerSorter layerSorter{};

      for (const SG::ReadHandleKey<MuonR4::SpacePointContainer>& key : m_spacePointKeys) {
         const MuonR4::SpacePointContainer* container{nullptr};
         ATH_CHECK(SG::get(container,key, ctx));
         for (const auto& bucket : *container) {
            std::vector<std::uint32_t> layNumbers{};
            for (const auto& spacePoint : *bucket) {
               const std::uint32_t layNum = layerSorter.sectorLayerNum(*spacePoint);
               if (!Acts::rangeContainsValue(layNumbers, layNum)) {
                    layNumbers.push_back(layNum);
                }

               m_bucketId += bucketCounter;
               m_muonId += encodeId(*spacePoint, layNum);
               m_localPosition+=spacePoint->localPosition();
               m_sensorDirection+=spacePoint->sensorDirection();
               m_toNextSensor += spacePoint->toNextSensor();
               
               const Acts::Surface& measSurface = xAOD::muonSurface(spacePoint->primaryMeasurement()); 
               m_geometryId += measSurface.geometryId().value();
               m_driftR += toFloat(spacePoint->driftRadius());
               m_time += toFloat(spacePoint->time());

               m_toMeasFrame += spacePoint->msSector()->surface().localToGlobalTransform(tgContext).inverse()*
                                measSurface.localToGlobalTransform(tgContext);
               using namespace MuonR4::SegmentFit;
               m_covLoc0 += toFloat(spacePoint->covariance()[Acts::toUnderlying(AxisDefs::etaCov)]);
               m_covLoc1 += toFloat(spacePoint->covariance()[Acts::toUnderlying(AxisDefs::phiCov)]);
               m_covT    += toFloat(spacePoint->covariance()[Acts::toUnderlying(AxisDefs::timeCov)]);
            }
            ++bucketCounter;
         }
      }
      ATH_MSG_DEBUG("Dumped "<<bucketCounter<<" buckets");
      ATH_CHECK(m_tree.fill(ctx));
      return StatusCode::SUCCESS;
    }
    StatusCode SpacePointWriter::finalize(){
      ATH_CHECK(m_tree.write());
      return StatusCode::SUCCESS;
    }
    std::uint32_t SpacePointWriter::encodeId(const MuonR4::SpacePoint& spacePoint,
                                             std::uint32_t gasGap) const {
      constexpr std::uint32_t threeBit = 0x7;
      constexpr std::uint32_t fourBit = 0xF;
      constexpr std::uint32_t sixBit = 0x3F;   
      std::uint32_t rawRep{0u};
      const MuonGMR4::Chamber* ch = spacePoint.chamber();
      rawRep |= (static_cast<std::uint32_t>(ch->chamberIndex()) & fourBit);
      if (ch->side() == 1) {
        rawRep |= (1u << 4);
      }

      const Muon::MuonStationIndex::TechnologyIndex techIdx = m_idHelperSvc->technologyIndex(spacePoint.identify());
      rawRep |= ((static_cast<std::uint32_t>(techIdx) & threeBit) << 5);
      rawRep |= ((static_cast<std::uint32_t>(ch->sector() - 1u) & sixBit) << 8);
      rawRep |= (static_cast<std::uint32_t>(spacePoint.measuresEta()) << 14);
      rawRep |= (static_cast<std::uint32_t>(spacePoint.measuresPhi()) << 15);
      rawRep |= (static_cast<std::uint32_t>(spacePoint.hasTime()) << 16);
      
      int primaryCh{0};
      switch (techIdx) {
         using enum Muon::MuonStationIndex::TechnologyIndex;
         case MDT: {
            const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()}; 
            primaryCh = idHelper.tube(spacePoint.identify());
            break;
         } case RPC: {
            const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
            primaryCh = idHelper.channel(spacePoint.identify());
            break;
         } case TGC: {
            const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
            primaryCh = idHelper.channel(spacePoint.identify());
            break;
         } case STGC: {
            const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
            primaryCh = idHelper.channel(spacePoint.identify());
            break;
         } case MM: {
            const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
            primaryCh = idHelper.channel(spacePoint.identify());
            break;
         } default:
               ATH_MSG_WARNING("Dude you can't have CSCs in R4 "<<spacePoint);
      };

      rawRep |= ((static_cast<std::uint32_t>(gasGap - 1u) & fourBit) << 17);
      rawRep |= (static_cast<std::uint32_t>(primaryCh - 1u) << 21);
      return rawRep;
    }
}