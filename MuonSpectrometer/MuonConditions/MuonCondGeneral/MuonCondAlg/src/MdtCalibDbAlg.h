/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
   MdtCalibDbAlg reads raw condition data and writes derived condition data to the condition store
*/

#ifndef MDTCALIBDBCOOLSTRTOOL_MDTCALIBDBALG_H
#define MDTCALIBDBCOOLSTRTOOL_MDTCALIBDBALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"

#include "AthenaKernel/IAthRNGSvc.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CoralBase/Blob.h"
#include "CoralUtilities/blobaccess.h"
#include "MdtCalibData/MdtCalibDataContainer.h"
#include "MdtCalibData/RtResolutionLookUp.h"
#include "MuonCalibITools/IIdToFixedIdTool.h"
#include "MuonCalibMath/SamplePoint.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonCondData/MdtCondDbData.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "nlohmann/json.hpp"
#include "zlib.h"


/** @brief Algorithm to load the Mdt calibration constants from the 
 *         conditions database
 */
namespace Muon{
class MdtCalibDbAlg : public AthCondAlgorithm {
public:
    using AthCondAlgorithm::AthCondAlgorithm;
    virtual ~MdtCalibDbAlg() = default;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;
private:
    StatusCode declareDependency(const EventContext& ctx, 
                                 SG::WriteCondHandle<MuonCalib::MdtCalibDataContainer>& writeHandle) const;
    StatusCode loadRt(const EventContext& ctx, MuonCalib::MdtCalibDataContainer& writeCdo) const;
    StatusCode loadTube(const EventContext& ctx, MuonCalib::MdtCalibDataContainer& writeCdo) const;
    
    StatusCode defaultT0s(MuonCalib::MdtCalibDataContainer& writeCdoTube) const;
    
    using RtRelationPtr = MuonCalib::MdtFullCalibData::RtRelationPtr;
    using LoadedRtMap = std::map<Identifier, RtRelationPtr>;

    StatusCode defaultRt(MuonCalib::MdtCalibDataContainer& writeCdoRt,
                         LoadedRtMap& loadedRts) const;

    std::optional<double> getInnerTubeRadius(const Identifier& id) const;

    /// Parses the legacy payload for the RT functions to a json format
    StatusCode legacyRtPayloadToJSON(const coral::AttributeList& attr, nlohmann::json & json) const;
    ///
    StatusCode legacyTubePayloadToJSON(const coral::AttributeList& attr,nlohmann::json & json) const;
    
    
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    ToolHandle<MuonCalib::IIdToFixedIdTool> m_idToFixedIdTool{this, "IdToFixedIdTool", "MuonCalib::IdToFixedIdTool"};
 
    const MuonGM::MuonDetectorManager* m_detMgr{nullptr}; 
    const MuonGMR4::MuonDetectorManager* m_r4detMgr{nullptr};

    Gaudi::Property<bool> m_useNewGeo{this, "UseR4DetMgr", false,
                                     "Switch between the legacy and the new geometry"};

    /// only needed to retrieve information on number of tubes etc. (no alignment needed)

    Gaudi::Property<bool> m_checkTubes{this, "checkTubes", true,"If true the number of tubes must agree between the conditions DB & geometry"};
    // new conditions format 2020
    Gaudi::Property<bool> m_newFormat2020{this, "NewFormat2020", false, "Use the new calibration data format "};

    // like MdtCalibrationDbSvc
    // for corData in loadRt
    Gaudi::Property<bool> m_create_b_field_function{this, "CreateBFieldFunctions", false,
        "If set to true, the B-field correction functions are initialized for each rt-relation that is loaded."};

    Gaudi::Property<bool> m_createSlewingFunction{this, "CreateSlewingFunctions", false,
        "If set to true, the slewing correction functions are initialized for each rt-relation that is loaded."};


    // if m_TimeSlewingCorrection is set to true then it is assumed that the
    // time slewing correction is applied. If false not. If this flag does
    // not match the bit in the creation parameters, the rt-relation and t0
    // will be corrected.
    // NOTE: This was a preliminary solution for 17.2. In principle each
    // MdtDriftCircleOnTrackCreator could decide individually if it wants to
    // have TS-correction. In the default reco-jobs however, this is
    // configured by one muonRecFlag, that will be used to set this job-option.
    Gaudi::Property<bool> m_TimeSlewingCorrection{this, "TimeSlewingCorrection", false};
    Gaudi::Property<bool> m_UseMLRt{this, "UseMLRt", false, "Enable use of ML-RTs from COOL"};

    Gaudi::Property<std::vector<float>> m_MeanCorrectionVsR{this, "MeanCorrectionVsR", {}};

    Gaudi::Property<double> m_TsCorrectionT0{this, "TimeSlewCorrectionT0", 0.};

    Gaudi::Property<double> m_defaultT0{this, "defaultT0", 40., "default T0 value to be used in absence of DB information"};
    Gaudi::Property<double> m_t0Shift{this, "T0Shift", 0., "for simulation: common shift of all T0s, in ns"};
    Gaudi::Property<double> m_t0Spread{this, "T0Spread", 0., "for simulation: sigma for random smeraing of T0s, in ns"};

    Gaudi::Property<double> m_rtShift{this, "RTShift", 0., "for simulations: maximum RT distortion, in mm"};
    Gaudi::Property<double> m_rtScale{this, "RTScale", 1., "for simulations: a muliplicitive scale to the drift r"};
    Gaudi::Property<double> m_prop_beta{this, "PropagationSpeedBeta", 1., "Speed of the signal propagation"};

    ServiceHandle<IAthRNGSvc> m_AthRNGSvc{this, "AthRNGSvc", "AthRNGSvc"};
    StringProperty m_randomStream{this, "RandomStream", "MDTCALIBDBALG"};
    ATHRNG::RNGWrapper* m_RNGWrapper{nullptr};

    StringProperty m_RTfileName{this, "RT_InputFile", "MuonCondAlg/Muon_RT_default.data",
                                 "single input ascii file for default RT to be applied in absence of DB information"};  // temporary!!!

    static std::unique_ptr<MuonCalib::RtResolutionLookUp> getRtResolutionInterpolation(const std::vector<MuonCalib::SamplePoint>& sample_points);

    SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyRt{this, "ReadKeyRt", "/MDT/RTBLOB", "DB folder containing the RT calibrations"};
    SG::ReadCondHandleKey<CondAttrListCollection> m_readKeyTube{this, "ReadKeyTube", "/MDT/T0BLOB", "DB folder containing the tube constants"};
    SG::WriteCondHandleKey<MuonCalib::MdtCalibDataContainer> m_writeKey{this, "WriteKey",  "MdtCalibConstants",
                                                                       "Conditions object containing the calibrations"};
    
    SG::ReadCondHandleKey<MdtCondDbData> m_readKeyDCS{this, "ReadKeyDCS", "MdtCondDbData", "Key of the input DCS data"};
};
}
#endif
