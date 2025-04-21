/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MuonDetectorManager_H
#define MuonDetectorManager_H

#include "GeoPrimitives/GeoPrimitives.h"
/// Ensure that the Athena extensions are properly loaded
#include <iostream>
#include <map>
#include <sstream>
#include <vector>
#include "AthenaBaseComps/AthMessaging.h"
#include "GaudiKernel/StatusCode.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoVDetectorManager.h"
#include "MuonAlignmentData/CorrContainer.h"
#include "MuonAlignmentData/NswAsBuiltDbData.h"
#include "MuonAlignmentData/sTGCAsBuiltData.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"


#ifndef SIMULATIONBASE
#include "MuonNSWAsBuilt/StripCalculator.h"
#include "MuonNSWAsBuilt/StgcStripCalculator.h"
#endif

namespace MuonGM {


    class MuonReadoutElement;
    class CscReadoutElement;
    class TgcReadoutElement;
    class RpcReadoutElement;
    class MdtReadoutElement;
    class sTgcReadoutElement;
    class MMReadoutElement;
    class MuonClusterReadoutElement;
    class MuonStation;

    /**
       The MuonDetectorManager stores the transient representation of the Muon Spectrometer geometry
       and provides access to all main blocks of information needed by generic clients of the
       geometry:
       XxxReadoutElements, MuonStations, MuonIdHelpers, geometryVersion, etc.
       More details below.
    */

    class MuonDetectorManager : public GeoVDetectorManager, public AthMessaging {
    public:
        // Constructor:
        MuonDetectorManager();

        // Destructor:
        ~MuonDetectorManager();

        // Gets the number of tree tops:  required:
        virtual unsigned int getNumTreeTops() const;

        // Gets the ith tree top:  required:
        virtual PVConstLink getTreeTop(unsigned int i) const;
        PVLink getTreeTop(unsigned int i);

        // Add a tree top:
        void addTreeTop(PVLink);

        // Add a XxxReadoutElement to the Collection
        void addMdtReadoutElement(std::unique_ptr<MdtReadoutElement>&& reEle);    //!< store the MdtReadoutElement using as "key" the identifier
        void addRpcReadoutElement(std::unique_ptr<RpcReadoutElement>&& reEle);    //!< store the RpcReadoutElement using as "key" the identifier
        void addTgcReadoutElement(std::unique_ptr<TgcReadoutElement>&& reEle);    //!< store the TgcReadoutElement using as "key" the identifier
        void addCscReadoutElement(std::unique_ptr<CscReadoutElement>&& reEle);    //!< store the CscReadoutElement using as "key" the identifier
        void addsTgcReadoutElement(std::unique_ptr<sTgcReadoutElement>&& reEle);  //!< store the sTGCReadoutElement using as "key" the identifier
        void addMMReadoutElement(std::unique_ptr<MMReadoutElement>&& reEle);      //!< store the MMReadoutElement using as "key" the identifier


        // access to Readout Elements
        const MdtReadoutElement* getMdtReadoutElement(const Identifier& id) const;    //!< access via extended identifier (requires unpacking)
        const RpcReadoutElement* getRpcReadoutElement(const Identifier& id) const;    //!< access via extended identifier (requires unpacking)
        const TgcReadoutElement* getTgcReadoutElement(const Identifier& id) const;    //!< access via extended identifier (requires unpacking)
        const CscReadoutElement* getCscReadoutElement(const Identifier& id) const;    //!< access via extended identifier (requires unpacking)
        const MMReadoutElement* getMMReadoutElement(const Identifier& id) const;      //!< access via extended identifier (requires unpacking)
        const sTgcReadoutElement* getsTgcReadoutElement(const Identifier& id) const;  //!< access via extended identifier (requires unpacking)
        /// Get any read out element
        const MuonReadoutElement* getReadoutElement(const Identifier& id) const; 
        //// Non const version
        MdtReadoutElement* getMdtReadoutElement(const Identifier& id);
        CscReadoutElement* getCscReadoutElement(const Identifier& id);
        TgcReadoutElement* getTgcReadoutElement(const Identifier& id);
      

        const MdtReadoutElement* getMdtReadoutElement(const IdentifierHash& id) const;  //!< access via detector-element hash id
        const RpcReadoutElement* getRpcReadoutElement(const IdentifierHash& id) const;  //!< access via detector-element hash id
        const TgcReadoutElement* getTgcReadoutElement(const IdentifierHash& id) const;  //!< access via detector-element hash id
        const CscReadoutElement* getCscReadoutElement(const IdentifierHash& id) const;  //!< access via detector-element hash id

        inline unsigned int nMuonStation() const;  //!< Number of MuonStations

        inline unsigned int nMdtRE() const;   //!< Number of Mdt ReadoutElements
        inline unsigned int nsTgcRE() const;  //!< Number of sTgc ReadoutElements
        inline unsigned int nMMRE() const;    //!< Number of MM ReadoutElements
        inline unsigned int nCscRE() const;   //!< Number of Csc ReadoutElements
        inline unsigned int nRpcRE() const;   //!< Number of Rpc ReadoutElements
        inline unsigned int nTgcRE() const;   //!< Number of Tgc ReadoutElements

        inline unsigned int nMdtDE() const;  //!< Number of Mdt DetectorElements
        inline unsigned int nCscDE() const;  //!< Number of Csc DetectorElements
        inline unsigned int nRpcDE() const;  //!< Number of Rpc DetectorElements
        inline unsigned int nTgcDE() const;  //!< Number of Tgc DetectorElements

        // Geometry versioning
        inline const std::string& geometryVersion() const;  //!< it can be Rome-Initial or P03, or ... it's the name of the layout
        void setGeometryVersion(const std::string& version);
        inline const std::string& get_DBMuonVersion() const;  //!< the name of the MuonSpectrometer tag (in the geometry DB) actually accessed
        void set_DBMuonVersion(const std::string& version);

        // Access to identifier helpers
        inline const MdtIdHelper* mdtIdHelper() const;
        inline const CscIdHelper* cscIdHelper() const;
        inline const RpcIdHelper* rpcIdHelper() const;
        inline const TgcIdHelper* tgcIdHelper() const;
        inline const sTgcIdHelper* stgcIdHelper() const;
        inline const MmIdHelper* mmIdHelper() const;


        void setMinimalGeoFlag(int flag);
        inline int  MinimalGeoFlag() const;
        void setCutoutsFlag(int flag);
        inline int  IncludeCutoutsFlag() const;
        void setCutoutsBogFlag(int flag);
        inline int  IncludeCutoutsBogFlag() const;

        // Add a MuonStation to the list
        void addMuonStation(std::unique_ptr<MuonStation>&& mst);
        const MuonStation* getMuonStation(const std::string& stName, int eta, int phi) const;
        MuonStation* getMuonStation(const std::string& stName, int eta, int phi);
        //<! access to the MuonStation by StationName, Jzz, Jff (amdb indices!!!! not stationPhi and Eta)
        static std::string muonStationKey(const std::string& stName, int statEtaIndex, int statPhiIndex) ;

        void clearCache();
        void fillCache();

        StatusCode updateAlignment(const ALineContainer& a);
        StatusCode updateDeformations(const BLineContainer& a);
        StatusCode updateMdtAsBuiltParams(const MdtAsBuiltContainer& a);
        StatusCode updateCSCInternalAlignmentMap(const ALineContainer& cscIntAline);

        void setNswAsBuilt(const NswAsBuiltDbData* nswAsBuiltData);
        void setsTGCAsBuilt(const sTGCAsBuiltData* stgcAsBuilt);
#ifndef SIMULATIONBASE
        const NswAsBuilt::StripCalculator* getMMAsBuiltCalculator() const { 
            return  m_nswAsBuilt ? m_nswAsBuilt->microMegaData.get() : nullptr; 
        }
#endif
        /**
         *  Identifier <-> AMDB conversion constants in use
        */
        static constexpr int NCscStEtaOffset = 1;
        static constexpr int NTgcStatTypeOff = -41;
        static constexpr int NTgcStEtaOffset = 5;

        const sTGCAsBuiltData* getsTGCAsBuilt() const {
            return m_stgcAsBuildData;
        }

        // map the RPC station indices (0-NRpcStatType) back to the RpcIdHelper stationNames
        int rpcStationName(const int stationIndex) const;
      
    private:
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{"Muon::MuonIdHelperSvc/MuonIdHelperSvc", "MuonDetectorManager"};
        void loadStationIndices();
        unsigned int rpcStationTypeIdx(const int stationName) const;  // map the RPC stationNames from the RpcIdHelper to 0-NRpcStatType
        enum RpcStatType { BML = 0, BMS, BOL, BOS, BMF, BOF, BOG, BME, BIR, BIM, BIL, BIS, UNKNOWN };
        /// Helper method to convert the Identifier into the corresponding index accessing the array
        int rpcIdentToArrayIdx(const Identifier& id) const;        
        int tgcIdentToArrayIdx(const Identifier& id) const;
        int cscIdentToArrayIdx(const Identifier& id) const;
        int stgcIdentToArrayIdx(const Identifier& id) const;
        int mmIdenToArrayIdx(const Identifier& id) const;
        int mdtIdentToArrayIdx(const Identifier& id) const;

        int m_minimalgeo{0};
        int m_includeCutouts{0};
        int m_includeCutoutsBog{0};

        std::vector<PVLink> m_envelope;  // Tree-top...


        // Geometry versioning
        std::string m_geometryVersion{};  // generic name of the Layout
        std::string m_DBMuonVersion{};    // name of the MuonVersion table-collection in Oracle
       
        // 115.6 kBytes.
        std::vector<std::unique_ptr<MdtReadoutElement>> m_mdtArray;
        std::vector<std::unique_ptr<CscReadoutElement>> m_cscArray;
        std::vector<std::unique_ptr<TgcReadoutElement>> m_tgcArray;
        
        std::vector<std::unique_ptr<RpcReadoutElement>> m_rpcArray;
        std::vector<std::unique_ptr<sTgcReadoutElement>> m_stgArray;
        std::vector<std::unique_ptr<MMReadoutElement>> m_mmcArray;
        std::map<std::string, std::unique_ptr<MuonStation> > m_MuonStationMap;

        unsigned int m_n_mdtRE{0};
        unsigned int m_n_cscRE{0};
        unsigned int m_n_rpcRE{0};
        unsigned int m_n_tgcRE{0};
        unsigned int m_n_stgRE{0};
        unsigned int m_n_mmcRE{0};

        unsigned int m_n_mdtDE{0};
        unsigned int m_n_cscDE{0};
        unsigned int m_n_rpcDE{0};
        unsigned int m_n_tgcDE{0};

        
        const NswAsBuiltDbData* m_nswAsBuilt{nullptr};
        const sTGCAsBuiltData* m_stgcAsBuildData {nullptr};
    
        /// RPC name caches
        std::map<int, int> m_rpcStatToIdx;
        std::map<int, int> m_rpcIdxToStat;


    };

    const MdtIdHelper* MuonDetectorManager::mdtIdHelper() const { 
        return m_idHelperSvc->hasMDT() ? &(m_idHelperSvc->mdtIdHelper()) : nullptr;
    }
    const CscIdHelper* MuonDetectorManager::cscIdHelper() const { 
        return m_idHelperSvc->hasCSC() ?  &(m_idHelperSvc->cscIdHelper()) : nullptr; 
    }
    const RpcIdHelper* MuonDetectorManager::rpcIdHelper() const { 
        return m_idHelperSvc->hasRPC() ? &(m_idHelperSvc->rpcIdHelper()): nullptr; 
    }
    const TgcIdHelper* MuonDetectorManager::tgcIdHelper() const { 
        return m_idHelperSvc->hasTGC() ? &(m_idHelperSvc->tgcIdHelper()) : nullptr; 
    }
    const sTgcIdHelper* MuonDetectorManager::stgcIdHelper() const { 
        return  m_idHelperSvc->hasSTGC() ? &(m_idHelperSvc->stgcIdHelper()) : nullptr; 
    }
    const MmIdHelper* MuonDetectorManager::mmIdHelper() const { 
        return  m_idHelperSvc->hasMM() ? &(m_idHelperSvc->mmIdHelper()) : nullptr; 
    }



    int MuonDetectorManager::MinimalGeoFlag() const { return m_minimalgeo; }
    int MuonDetectorManager::IncludeCutoutsFlag() const { return m_includeCutouts; }
    int MuonDetectorManager::IncludeCutoutsBogFlag() const { return m_includeCutoutsBog; }

    const std::string& MuonDetectorManager::geometryVersion() const { return m_geometryVersion; }
    const std::string& MuonDetectorManager::get_DBMuonVersion() const { return m_DBMuonVersion; }

    
    unsigned int MuonDetectorManager::nMuonStation() const { return m_MuonStationMap.size(); }
    unsigned int MuonDetectorManager::nMdtRE() const { return m_n_mdtRE; }
    unsigned int MuonDetectorManager::nCscRE() const { return m_n_cscRE; }
    unsigned int MuonDetectorManager::nRpcRE() const { return m_n_rpcRE; }
    unsigned int MuonDetectorManager::nTgcRE() const { return m_n_tgcRE; }
    unsigned int MuonDetectorManager::nsTgcRE() const { return m_n_stgRE; }
    unsigned int MuonDetectorManager::nMMRE() const { return m_n_mmcRE; }

    unsigned int MuonDetectorManager::nMdtDE() const { return m_n_mdtDE; }
    unsigned int MuonDetectorManager::nCscDE() const { return m_n_cscDE; }
    unsigned int MuonDetectorManager::nRpcDE() const { return m_n_rpcDE; }
    unsigned int MuonDetectorManager::nTgcDE() const { return m_n_tgcDE; }

}  // namespace MuonGM

#ifndef GAUDI_NEUTRAL
namespace MuonGM {
    class MuonDetectorManager;
}
CLASS_DEF(MuonGM::MuonDetectorManager, 4500, 1);
CONDCONT_MIXED_DEF(MuonGM::MuonDetectorManager, 205781622);
#endif

#endif
