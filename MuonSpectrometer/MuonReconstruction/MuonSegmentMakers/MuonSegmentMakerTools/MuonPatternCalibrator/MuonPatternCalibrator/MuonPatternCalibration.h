/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUON_MUONPATTERNCALIBRATION_H
#define MUON_MUONPATTERNCALIBRATION_H

#include <map>
#include <string>
#include <vector>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPattern/MuonPatternCombination.h"
#include "MuonPattern/MuonPatternCombinationCollection.h"
#include "MuonPrepRawData/MuonPrepDataContainer.h"
#include "MuonRecHelperTools/MuonEDMPrinterTool.h"
#include "MuonRecToolInterfaces/IMdtDriftCircleOnTrackCreator.h"
#include "MuonRecToolInterfaces/IMuonClusterOnTrackCreator.h"
#include "MuonSegmentMakerToolInterfaces/IMuonPatternCalibration.h"

class MdtPrepData;

namespace Muon {
class MdtPrepData;
class MuonClusterOnTrack;
class MdtDriftCircleOnTrack;

class MuonPatternCalibration :  public extends<AthAlgTool, IMuonPatternCalibration> {
  public:
    using ISPrd = std::pair<Amg::Vector3D, const MuonCluster*>;
    using ISPrdVec = std::vector<ISPrd>;    

    using ISPrdMdt = std::pair<Amg::Vector3D, const MdtPrepData*>;
    using ISPrdMdtVec = std::vector<ISPrdMdt>;
    using RegionIdMap = std::map<int, ISPrdMdtVec>;
   
    struct Region {
        Region() = default;
        Amg::Vector3D regionPos{Amg::Vector3D::Zero()};
        Amg::Vector3D regionDir{Amg::Vector3D::Zero()};
        ISPrdVec      triggerPrds{};
        RegionIdMap   mdtPrdsPerChamber{};
        bool init{false};
    };

  
    using RegionMap =  std::map<int, Region>;
 
    struct EtaPhiHits {
        EtaPhiHits() = default;
        unsigned int neta{0};
        unsigned int nphi{0};
    };

  public:
    MuonPatternCalibration(const std::string&, const std::string&, const IInterface*);
    virtual ~MuonPatternCalibration() = default;

    virtual StatusCode initialize() override;

    StatusCode calibrate(const EventContext& ctx, const MuonPatternCombination& pat, ROTsPerRegion& hitsPerRegion) const override;
    int  getRegionId(const Identifier& id) const override;
    bool checkForPhiMeasurements(const MuonPatternCombination& pat) const override;

  private:
    StatusCode createRegionMap(const EventContext& ctx, const MuonPatternCombination& pat, 
                               RegionMap& regionMap, bool hasPhiMeasurements) const;
    
    void printRegionMap(const RegionMap& regionMap) const;

    void calibrateRegionMap(const RegionMap& regionMap, IMuonPatternCalibration::ROTsPerRegion& hitsPerRegion) const;


    void insertCluster(const MuonCluster& mdt, RegionMap& regionMap, const Amg::Vector3D& patpose,
                       const Amg::Vector3D& patdire, bool hasPhiMeasurements) const;

    void insertMdt(const MdtPrepData& clus, RegionMap& regionMap, const Amg::Vector3D& patpose,
                   const Amg::Vector3D& patdire, bool hasPhiMeasurements) const;


    ToolHandle<IMdtDriftCircleOnTrackCreator> m_mdtCreator{
        this,
        "MdtCreator",
        "",
    };  //<! pointer to mdt rio ontrack creator
    ToolHandle<IMuonClusterOnTrackCreator> m_clusterCreator{
        this,
        "ClusterCreator",
        "Muon::MuonClusterOnTrackCreator/MuonClusterOnTrackCreator",
    };  //<! pointer to muon cluster rio ontrack creator
    PublicToolHandle<MuonEDMPrinterTool> m_printer{
        this,
        "Printer",
        "Muon::MuonEDMPrinterTool",
    };  //<! tool to print EDM objects

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{
        this,
        "MuonIdHelperSvc",
        "Muon::MuonIdHelperSvc/MuonIdHelperSvc",
    };

    Gaudi::Property<bool> m_doMultiAnalysis{this, "DoMultiChamberAnalysis", true};  //<! use neighbouring chambers during segment finding
    Gaudi::Property<double> m_dropDistance{this, "DropDistance", 1500.};     //<! hits that are further away than the distance are not added to segmentmaker input
    Gaudi::Property<double> m_phiAngleCut{this, "AngleCutPhi", 1.e9};      //<! cut on the phi opening angle between chamber and pattern
    Gaudi::Property<bool> m_doSummary{this, "DoSummary", false};
    Gaudi::Property<bool> m_recoverTriggerHits{this, "RecoverTriggerHits", true};
    Gaudi::Property<bool> m_removeDoubleMdtHits{this, "RemoveDoubleMdtHits", true};

    SG::ReadHandleKey<Muon::RpcPrepDataContainer> m_keyRpc{this, "RpcPrepDataContainer","RPC_Measurements"};
    SG::ReadHandleKey<Muon::TgcPrepDataContainer> m_keyTgc{this, "TgcPrepDataContainer","TGC_Measurements"};

    /// load the container from storegate given a ReadHandleKey. If the key is empty
    /// a nullptr will be returned
    template <class ContType> StatusCode loadFromStoreGate(const EventContext& ctx,
                                                           const SG::ReadHandleKey<ContType>& key,
                                                           const ContType* & cont_ptr) const;
};

}  // namespace Muon

#endif  // MUON_MUONPATTERNCALIBRATION_H
