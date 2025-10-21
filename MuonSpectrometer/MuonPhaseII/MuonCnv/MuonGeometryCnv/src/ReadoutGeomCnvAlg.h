/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONGEOMETRYCNV_ReadoutGeomCnvAlg_H
#define MUONGEOMETRYCNV_ReadoutGeomCnvAlg_H

#include "TrkSurfaces/Surface.h" // Work around cppcheck false positive
#include <AthenaBaseComps/AthReentrantAlgorithm.h>
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
class ReadoutGeomCnvAlg : public AthReentrantAlgorithm {
    public:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;
        ~ReadoutGeomCnvAlg() = default;

        StatusCode execute(const EventContext& ctx) const override;
        StatusCode initialize() override;
        bool isReEntrant() const override { return false; }
    
    private:
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
                unsigned int m_id{0};
        };
        
        /** @brief builds a station object from readout element. The parent PhysVol of the readoutElement
         *         is interpreted as embedding station volume and all children which are not fullPhysical 
         *         volumes are attached to the copied clone. 
         * 
         */
        StatusCode buildStation(const ActsTrk::GeometryContext& gctx,
                                const Identifier& stationId,
                                ConstructionCache& cacheObj) const;
        /** @brief Clones the fullPhysical volume of the readoutElement and embeds it into the associated station.
         *         If creations of the needed station fails, failure is returned. The references to the clonedPhysVol
         *         & to the station are set if the procedure was successful.
         */
        StatusCode cloneReadoutVolume(const ActsTrk::GeometryContext& gctx,
                                      const Identifier& stationId,
                                      ConstructionCache& cacheObj,
                                      GeoIntrusivePtr<GeoVFullPhysVol>& clonedPhysVol,
                                      MuonGM::MuonStation* & station) const;
        /** @brief Clones the fullPhysicalVolume of the  */
        GeoIntrusivePtr<GeoVFullPhysVol> cloneNswWedge(const ActsTrk::GeometryContext& gctx,
                                                       const MuonGMR4::MuonReadoutElement* nswRE,
                                                       ConstructionCache& cacheObj) const;
        
        StatusCode buildMdt(const ActsTrk::GeometryContext& gctx,
                            ConstructionCache& cacheObj) const;

        StatusCode buildRpc(const ActsTrk::GeometryContext& gctx,
                            ConstructionCache& cacheObj) const;

        StatusCode buildSTGC(const ActsTrk::GeometryContext& gctx,
                             ConstructionCache& cacheObj) const;

        StatusCode buildMM(const ActsTrk::GeometryContext& gctx,
                           ConstructionCache& cacheObj) const;

        StatusCode buildTgc(const ActsTrk::GeometryContext& gctx,
                            ConstructionCache& cacheObj) const;

        
        StatusCode dumpAndCompare(const ActsTrk::GeometryContext& gctx,
                                  const MuonGMR4::RpcReadoutElement& refEle,
                                  const MuonGM::RpcReadoutElement& testEle) const;

        StatusCode dumpAndCompare(const ActsTrk::GeometryContext& gctx,
                                  const MuonGMR4::MdtReadoutElement& refEle,
                                  const MuonGM::MdtReadoutElement& testEle) const;

        StatusCode dumpAndCompare(const ActsTrk::GeometryContext& gctx,
                                  const MuonGMR4::MmReadoutElement& refEle,
                                  const MuonGM::MMReadoutElement& testEle) const;        

        StatusCode dumpAndCompare(const ActsTrk::GeometryContext& gctx,
                                  const MuonGMR4::TgcReadoutElement& refEle,
                                  const MuonGM::TgcReadoutElement& testEle) const;

        StatusCode dumpAndCompare(const ActsTrk::GeometryContext& gctx,
                                  const MuonGMR4::sTgcReadoutElement& refEle,
                                  const MuonGM::sTgcReadoutElement& testEle) const;

        StatusCode checkIdCompability(const MuonGMR4::MuonReadoutElement& refEle,
                                      const MuonGM::MuonReadoutElement& testEle) const;
        
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        SG::WriteCondHandleKey<MuonGM::MuonDetectorManager> m_writeKey{this, "WriteKey", "MuonDetectorManager"};
        
        SG::ReadCondHandleKeyArray<ActsTrk::DetectorAlignStore> m_alignStoreKeys{this, "AlignmentKeys", {}, "Alignment key"};
        
        Gaudi::Property<bool> m_checkGeo{this, "checkGeo", false, "Checks the positions of the sensors"};
        Gaudi::Property<bool> m_dumpGeo{this, "dumpGeo", false, "Dumps the constructed geometry"};
        Gaudi::Property<std::string> m_geoDumpName{this,"geoDumpName", "ConvMuonGeoModel.db",};
        const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

 
};
}
#endif
