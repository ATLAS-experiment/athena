/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMETRYCNV_ReadoutGeomCnvAlg_H
#define MUONGEOMETRYCNV_ReadoutGeomCnvAlg_H

#include "TrkSurfaces/Surface.h" // Work around cppcheck false positive
#include <AthenaBaseComps/AthCondAlgorithm.h>
#include <StoreGate/WriteCondHandleKey.h>
#include <StoreGate/ReadCondHandleKey.h>
#include <StoreGate/CondHandleKeyArray.h>

#include <MuonReadoutGeometry/MuonDetectorManager.h>
#include <MuonReadoutGeometry/MuonReadoutElement.h>
#include <MuonReadoutGeometryR4/MuonDetectorManager.h>

#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelHelpers/GeoDeDuplicator.h"
#include "GeoModelKernel/GeoVFullPhysVol.h"
#include "GeoModelKernel/GeoIdentifierTag.h"

#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <ActsGeometryInterfaces/GeometryContext.h>

/** The ReadoutGeomCnvAlg converts the Run4 Readout geometry build from the GeoModelXML into the legacy MuonReadoutGeometry.
 *  The algorithm is meant to serve as an adapter allowing to dynamically exchange individual components in the Muon processing chain
 *  by their Run4 / Acts equivalents
 * 
*/

namespace MuonGMR4{
class ReadoutGeomCnvAlg : public AthCondAlgorithm {
    public:
        using AthCondAlgorithm::AthCondAlgorithm;
        ~ReadoutGeomCnvAlg() = default;

        StatusCode execute(const EventContext& ctx) const override;
        StatusCode initialize() override;
    
    private:
        /** @brief Cache object holding the constructed detector manager,
         *         and the intermediate GeoModel objects needed to build the
         *         legacy readout geometry */
        struct ConstructionCache: public GeoDeDuplicator {
            public:
                ConstructionCache() = default;
                /** @brief Pointer to the legacy MuonDetectorManager*/
                std::unique_ptr<MuonGM::MuonDetectorManager> detMgr{};
                /** @brief Pointer to the world */
                PVLink world{};
                /** @brief Set of all translated Physical volumes */
                std::set<PVConstLink> translatedStations{};
                /** @brief Returns an identifier tag */
                GeoIntrusivePtr<GeoIdentifierTag> newIdTag() {
                    return geoId(++m_id);
                }
            private:
                unsigned m_id{0};
        };
        
        /** @brief builds a station object from readout element. The parent PhysVol of the readoutElement
         *         is interpreted as embedding station volume and all children which are not fullPhysical 
         *         volumes are attached to the copied clone. 
         * @param gctx: Current geometry context carrying the current alignment
         * @param stationId: Identifier of the station encoding stName, stEta, stPhi
         * @param cacheObj: Mutable reference to the GeoModel constuction cache */
        StatusCode buildStation(const ActsTrk::GeometryContext& gctx,
                                const Identifier& stationId,
                                ConstructionCache& cacheObj) const;
        /** @brief Clones the fullPhysical volume of the readoutElement and embeds it into the associated station.
         *         If creations of the needed station fails, failure is returned. The references to the clonedPhysVol
         *         & to the station are set if the procedure was successful.
         * @param gctx: Current geometry context carrying the current alignment
         * @param stationId: Identifier of the station encoding stName, stEta, stPhi
         * @param cacheObj: Mutable reference to the GeoModel constuction cache
         * @param clonedPhysVol: Mutable reference into which the cloned physical volume is attached
         * @param station: Mutable reference to the muon station pointer which is filled during the cloning */
        StatusCode cloneReadoutVolume(const ActsTrk::GeometryContext& gctx,
                                      const Identifier& stationId,
                                      ConstructionCache& cacheObj,
                                      GeoIntrusivePtr<GeoVFullPhysVol>& clonedPhysVol,
                                      MuonGM::MuonStation* & station) const;
        /** @brief Clones the full phyical volume associated to the NSw readout element
          * @param gctx: Current geometry context carrying the current alignment
          * @param nswRE: Readout element from which the full PhysVol to clone is retrieved
          * @param cacheObj: Mutable reference to the GeoModel constuction cache */
        GeoIntrusivePtr<GeoVFullPhysVol> cloneNswWedge(const ActsTrk::GeometryContext& gctx,
                                                       const MuonGMR4::MuonReadoutElement& nswRE,
                                                       ConstructionCache& cacheObj) const;
        /** @brief Converts all Mdt readout elements from the R4 format into
          *         the legacy Trk format
          * @param gctx: Current geometry context carrying the current alignment
          * @param cacheObj: Mutable reference to the GeoModel constuction cache */
        StatusCode buildMdt(const ActsTrk::GeometryContext& gctx,
                            ConstructionCache& cacheObj) const;
        /** @brief Converts all Rpc readout elements from the R4 format into
          *         the legacy Trk format
          * @param gctx: Current geometry context carrying the current alignment
          * @param cacheObj: Mutable reference to the GeoModel constuction cache */
        StatusCode buildRpc(const ActsTrk::GeometryContext& gctx,
                            ConstructionCache& cacheObj) const;
        /** @brief Converts all sTgc readout elements from the R4 format into
          *         the legacy Trk format
          * @param gctx: Current geometry context carrying the current alignment
          * @param cacheObj: Mutable reference to the GeoModel constuction cache */
        StatusCode buildSTGC(const ActsTrk::GeometryContext& gctx,
                             ConstructionCache& cacheObj) const;
        /** @brief Converts all Mm readout elements from the R4 format into
          *         the legacy Trk format
          * @param gctx: Current geometry context carrying the current alignment
          * @param cacheObj: Mutable reference to the GeoModel constuction cache */
        StatusCode buildMM(const ActsTrk::GeometryContext& gctx,
                           ConstructionCache& cacheObj) const;
        /** @brief Converts all Tgc readout elements from the R4 format into
          *         the legacy Trk format
          * @param gctx: Current geometry context carrying the current alignment
          * @param cacheObj: Mutable reference to the GeoModel constuction cache */
        StatusCode buildTgc(const ActsTrk::GeometryContext& gctx,
                            ConstructionCache& cacheObj) const;
        /** @brief Compares the R4 readout element with the constructed Trk
         *         readout element. Transforms and position of the sensors
         *         are required to be identical in both descriptions
         * @param gctx: Current geometry context carrying the current alignment
         * @param refEle: R4 readout element taken as blueprint to build the Trk
         *                readout element.
         * @param testEle: The constructed Trk readout element that is to be checked */
        StatusCode dumpAndCompare(const ActsTrk::GeometryContext& gctx,
                                  const MuonGMR4::RpcReadoutElement& refEle,
                                  const MuonGM::RpcReadoutElement& testEle) const;
        /** @brief Compares the R4 readout element with the constructed Trk
         *         readout element. Transforms and position of the sensors
         *         are required to be identical in both descriptions
         * @param gctx: Current geometry context carrying the current alignment
         * @param refEle: R4 readout element taken as blueprint to build the Trk
         *                readout element.
         * @param testEle: The constructed Trk readout element that is to be checked */
        StatusCode dumpAndCompare(const ActsTrk::GeometryContext& gctx,
                                  const MuonGMR4::MdtReadoutElement& refEle,
                                  const MuonGM::MdtReadoutElement& testEle) const;
        /** @brief Compares the R4 readout element with the constructed Trk
         *         readout element. Transforms and position of the sensors
         *         are required to be identical in both descriptions
         * @param gctx: Current geometry context carrying the current alignment
         * @param refEle: R4 readout element taken as blueprint to build the Trk
         *                readout element.
         * @param testEle: The constructed Trk readout element that is to be checked */
        StatusCode dumpAndCompare(const ActsTrk::GeometryContext& gctx,
                                  const MuonGMR4::MmReadoutElement& refEle,
                                  const MuonGM::MMReadoutElement& testEle) const;        
        /** @brief Compares the R4 readout element with the constructed Trk
         *         readout element. Transforms and position of the sensors
         *         are required to be identical in both descriptions
         * @param gctx: Current geometry context carrying the current alignment
         * @param refEle: R4 readout element taken as blueprint to build the Trk
         *                readout element.
         * @param testEle: The constructed Trk readout element that is to be checked */
        StatusCode dumpAndCompare(const ActsTrk::GeometryContext& gctx,
                                  const MuonGMR4::TgcReadoutElement& refEle,
                                  const MuonGM::TgcReadoutElement& testEle) const;
        /** @brief Compares the R4 readout element with the constructed Trk
         *         readout element. Transforms and position of the sensors
         *         are required to be identical in both descriptions
         * @param gctx: Current geometry context carrying the current alignment
         * @param refEle: R4 readout element taken as blueprint to build the Trk
         *                readout element.
         * @param testEle: The constructed Trk readout element that is to be checked */
        StatusCode dumpAndCompare(const ActsTrk::GeometryContext& gctx,
                                  const MuonGMR4::sTgcReadoutElement& refEle,
                                  const MuonGM::sTgcReadoutElement& testEle) const;
        /** @brief Checks whether the Identifier fields of both readout elements are
         *         identical
         * @param refEle: R4 readout element taken as blueprint to build the Trk
         *                readout element.
         * @param testEle: The constructed Trk readout element that is to be checked */
        StatusCode checkIdCompability(const MuonGMR4::MuonReadoutElement& refEle,
                                      const MuonGM::MuonReadoutElement& testEle) const;
        
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        SG::WriteCondHandleKey<MuonGM::MuonDetectorManager> m_writeKey{this, "WriteKey", "MuonDetectorManager"};
        
        SG::ReadCondHandleKeyArray<ActsTrk::DetectorAlignStore> m_alignStoreKeys{this, "AlignmentKeys", {}, "Alignment key"};
        
        Gaudi::Property<bool> m_checkGeo{this, "checkGeo", false, "Checks the positions of the sensors"};
        Gaudi::Property<bool> m_dumpGeo{this, "dumpGeo", false, "Dumps the constructed geometry"};
        /** @brief Instantiate a new transform cache to ensure lazy transform population in the event processing */
        Gaudi::Property<bool> m_splitTrfCache{this, "splitTrfCache", false, ""};
        Gaudi::Property<std::string> m_geoDumpName{this,"geoDumpName", "ConvMuonGeoModel.db",};
        const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

 
};
}
#endif
