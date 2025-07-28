/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "SpacePointCsvDumperAlg.h"

#include "StoreGate/ReadHandle.h"
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "xAODMuonPrepData/UtilFunctions.h"
#include <fstream>
#include <TString.h>

namespace {
    constexpr int encodeId(const int8_t stName, const int8_t stEta,
                 const int8_t sector, const int8_t tech) {
        return ( tech <<24 | sector <<16 |stEta << 8| stName);
    }
    constexpr double precCutOff(const double value, const double cutOff = 1.e-15) {
        return std::abs(value) > cutOff ? value : 0.;
    }
}


namespace MuonR4{

StatusCode SpacePointCsvDumperAlg::initialize() {
   ATH_CHECK(m_readKeys.initialize());
   ATH_CHECK(m_idHelperSvc.retrieve());
   return StatusCode::SUCCESS;
 }

StatusCode SpacePointCsvDumperAlg::execute(){

   const EventContext& ctx{Gaudi::Hive::currentContext()};
 
   constexpr std::string_view delim = ",";
   std::ofstream file{std::string(Form("event%09zu-",++m_event))+"SpacePoints.csv"};
   
    /// Identifier to check whether the bucket is in the same sector
    file<<"sectorId"<<delim;
    // Bucket inside the sector layer    
    file<<"bucketId"<<delim;
    /// Local position of the hit
    file<<"locPositionX"<<delim;
    file<<"locPositionY"<<delim;
    file<<"locPositionZ"<<delim;
    /// Local sensor direction of the hit
    file<<"locSensorDirX"<<delim;
    file<<"locSensorDirY"<<delim;
    file<<"locSensorDirZ"<<delim;
    /// Vector normal to the direction of the strip direction (inside plane)
    file<<"locSensorNormX"<<delim;
    file<<"locSensorNormY"<<delim;
    file<<"locSensorNormZ"<<delim;
    /// Normal vector on the sensor plane
    file<<"locPlaneNormX"<<delim;
    file<<"locPlaneNormY"<<delim;
    file<<"locPlaneNormZ"<<delim;
    /// Covariance entries of the uncalibrated space point
    file<<"covX"<<delim;
    file<<"covY"<<delim;
    file<<"covT"<<delim;   
    /// Drift radius
    file<<"driftR"<<delim;
    /// Properties of the space point Identifier
    file<<"gasGap"<<delim;
    file<<"primaryCh"<<delim;
    file<<"measuresEta"<<delim;
    file<<"measuresPhi"<<delim;
    file<<"measuresTime"<<delim;
    file<<std::endl;

   auto dumpToFile = [&](const unsigned bucketId,
                         const SpacePoint& spacePoint,
                         const unsigned gasGap) {
        
        const Identifier& measId = spacePoint.identify();
        ATH_MSG_VERBOSE("Dump space point "<<m_idHelperSvc->toString(measId)<<", gasGap: "<<gasGap);
        int primaryCh{0};
        using TechIndex = Muon::MuonStationIndex::TechnologyIndex; 
        const TechIndex techIdx = m_idHelperSvc->technologyIndex(measId);
        switch (techIdx) {
            case TechIndex::MDT: {
                const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()}; 
                primaryCh = idHelper.tube(measId);
                break;
            }                
            case TechIndex::RPC: {
                const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
                primaryCh = idHelper.channel(measId);
                break;
            }
            case TechIndex::TGC: {
                const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
                primaryCh = idHelper.channel(measId);
                break;
            }
            case TechIndex::STGC: {
                const sTgcIdHelper& idHelper{m_idHelperSvc->stgcIdHelper()};
                primaryCh = idHelper.channel(measId);
                break;
            }
            case TechIndex::MM: {
                const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
                primaryCh = idHelper.channel(measId);
                break;
            }
            default:
                ATH_MSG_WARNING("Dude you can't have CSCs in R4 "<<m_idHelperSvc->toString(measId));
        };

        const int secId = encodeId(static_cast<int8_t>(spacePoint.msSector()->chamberIndex()),
                                   spacePoint.msSector()->side(),
                                   spacePoint.msSector()->sector(),
                                   static_cast<int8_t>(techIdx));

        file<<secId<<delim;
        file<<bucketId<<delim;
        file<<precCutOff(spacePoint.positionInChamber().x())<<delim;
        file<<precCutOff(spacePoint.positionInChamber().y())<<delim;
        file<<precCutOff(spacePoint.positionInChamber().z())<<delim;
        //
        file<<precCutOff(spacePoint.normalInChamber().x())<<delim;
        file<<precCutOff(spacePoint.normalInChamber().y())<<delim;
        file<<precCutOff(spacePoint.normalInChamber().z())<<delim;
        //
        file<<precCutOff(spacePoint.directionInChamber().x())<<delim;
        file<<precCutOff(spacePoint.directionInChamber().y())<<delim;
        file<<precCutOff(spacePoint.directionInChamber().z())<<delim;
        //
        file<<precCutOff(spacePoint.planeNormal().x())<<delim;
        file<<precCutOff(spacePoint.planeNormal().y())<<delim;
        file<<precCutOff(spacePoint.planeNormal().z())<<delim;
        //
        file<<precCutOff(spacePoint.covariance()(Amg::x, Amg::x))<<delim;
        file<<precCutOff(spacePoint.covariance()(Amg::y, Amg::y))<<delim;
        /// Dummy value for the moment
        file<<1.<<delim;
        file<<precCutOff(spacePoint.driftRadius())<<delim;
        file<<gasGap<<delim;
        file<<primaryCh<<delim;
        file<<spacePoint.measuresEta()<<delim;
        file<<spacePoint.measuresPhi()<<delim;
        /// Dummy value for the moment
        file<<false<<delim;
        file<<std::endl;
   };

   for (const SG::ReadHandleKey<SpacePointContainer>& key : m_readKeys) {
        const SpacePointContainer* spContainer{nullptr};
        ATH_CHECK(SG::get(spContainer, key, ctx));

        const SpacePointPerLayerSorter layerSorter{m_idHelperSvc.get()};
        for(const SpacePointBucket* bucket : *spContainer) {
         std::unordered_map<Identifier, unsigned> gasNumbers{};
         for (const SpacePointBucket::value_type& spacePoint:  *bucket) {
                unsigned int gasGap{gasNumbers.insert(
                                    std::make_pair(layerSorter.detectorLayerId(spacePoint->identify()), 
                                                   gasNumbers.size())).first->second};
                dumpToFile(bucket->bucketId(), *spacePoint, gasGap);
            }
        }
   }
   return StatusCode::SUCCESS;
}
}


