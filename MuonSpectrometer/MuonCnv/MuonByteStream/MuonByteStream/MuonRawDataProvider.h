/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONBYTESTREAM_MUONRAWDATAPROVIDER_H
#define MUONBYTESTREAM_MUONRAWDATAPROVIDER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "IRegionSelector/IRegSelTool.h"
#include "MuonCnvToolInterfaces/IMuonRawDataProviderTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "TrigSteeringEvent/TrigRoiDescriptor.h"

namespace Muon {

    class MuonRawDataProvider : public AthReentrantAlgorithm {
    public:
        using AthReentrantAlgorithm::AthReentrantAlgorithm;
        StatusCode initialize() override;
        StatusCode execute(const EventContext &ctx) const override;
        ~MuonRawDataProvider() = default;

    private:
        ToolHandle<Muon::IMuonRawDataProviderTool> m_rawDataTool{this, "ProviderTool", "", "Raw data provider tool"};
        ToolHandle<IRegSelTool> m_regselTool{this, "RegionSelectionTool", "", "Region selector tool"};

        Gaudi::Property<bool> m_seededDecoding{this, "DoSeededDecoding", false, "If true do decoding in RoIs"};
        Gaudi::Property<bool> m_useHashIds{this, "UseHashIds", false, "Use HashIDList for seeded decoding"};
        Gaudi::Property<bool> m_decodePerRoI{this, "DecodePerRoI", true, "Decode each RoI separately when seeded"};
        Gaudi::Property<bool> m_failOnConvertError{this, "FailOnConvertError", false, "Return failure if conversion fails"};
        Gaudi::Property<bool> m_ignoreMissingRoIs{this, "IgnoreMissingRoIs", false, "Return success when RoIs are missing"};

        SG::ReadHandleKey<TrigRoiDescriptorCollection> m_roiCollectionKey{this, "RoIs", "OutputRoIs", "Name of RoI collection to read in"};
    };

}  // namespace Muon

#endif
