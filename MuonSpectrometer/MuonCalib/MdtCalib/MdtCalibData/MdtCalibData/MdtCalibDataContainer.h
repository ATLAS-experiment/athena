/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCALIB_MDTCALIBDATACONTAINER_H
#define MUONCALIB_MDTCALIBDATACONTAINER_H


#include <AthenaKernel/CLASS_DEF.h>
#include <AthenaKernel/CondCont.h>
#include <GaudiKernel/MsgStream.h>

#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MdtCalibData/MdtFullCalibData.h>

#include <memory>
#include <optional>

namespace MuonCalib{
    
    class MdtCalibDataContainer{
    public:
        using CorrectionPtr = MdtFullCalibData::CorrectionPtr;
        using RtRelationPtr = MdtFullCalibData::RtRelationPtr;
        using TubeContainerPtr = MdtFullCalibData::TubeContainerPtr;

        enum class RegionGranularity { OneRt, OnePerChamber, OnePerMultiLayer };
        MdtCalibDataContainer(const Muon::IMuonIdHelperSvc* idHelprSvc,
                              const RegionGranularity granularity);
        
        ~MdtCalibDataContainer() = default;
        /// Returns the calibration data associated with this station        
        const MdtFullCalibData* getCalibData(const Identifier& measId,
                                             MsgStream& msg) const;
        /// Checks whether a calibration data object is already present
        bool hasDataForChannel(const Identifier& measId, MsgStream& msg) const;
        
        
        bool storeData(const Identifier& mlID, CorrectionPtr corrFuncSet, MsgStream& msg);
        bool storeData(const Identifier& mlID, RtRelationPtr rtRelation, MsgStream& msg);
        bool storeData(const Identifier& mlID, TubeContainerPtr tubeContainer, MsgStream& msg);
  
        RegionGranularity granularity() const;

        void setInversePropSpeed(const float speed);
        float inversePropSpeed() const;
    private:
        std::optional<unsigned int> containerIndex(const Identifier& measId,
                                                   MsgStream& msg) const;

        const Muon::IMuonIdHelperSvc* m_idHelperSvc{nullptr};
        const RegionGranularity m_granularity{RegionGranularity::OneRt};
        const MdtIdHelper& m_idHelper{m_idHelperSvc->mdtIdHelper()};
        std::vector<MdtFullCalibData> m_dataCache{};
        float m_invPropSpeed{0.f};
    
    };

}

CLASS_DEF( MuonCalib::MdtCalibDataContainer , 1228248101 , 1 );
CONDCONT_MIXED_DEF( MuonCalib::MdtCalibDataContainer , 1267664791 );

#endif
