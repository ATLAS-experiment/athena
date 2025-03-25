/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef NSWCalibTool_h
#define NSWCalibTool_h

#include "NSWCalibTools/INSWCalibTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"

#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonPrepRawData/MMPrepData.h"
#include "MuonRDO/MM_RawData.h"
#include "MuonRDO/STGC_RawData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MuonCondData/NswCalibDbTimeChargeData.h"
#include "MuonCondData/NswT0Data.h"
#include "MuonCondData/mmCTPClusterCalibData.h"

#include "TRandom3.h"
#include "TTree.h"
#include "TF1.h"

#include <string>
#include <vector>

namespace Muon {

  class NSWCalibTool : virtual public INSWCalibTool, public AthAlgTool {

  public:

    NSWCalibTool(const std::string&, const std::string&, const IInterface*);

    virtual ~NSWCalibTool() = default;
    
    using TimeCalibType = NswCalibDbTimeChargeData::CalibDataType;
    using TimeCalibConst = NswCalibDbTimeChargeData::CalibConstants;
    
    StatusCode calibrateClus(const EventContext& ctx, const Muon::MMPrepData* prepData, const Amg::Vector3D& globalPos, std::vector<NSWCalib::CalibratedStrip>& calibClus) const override;
    StatusCode distToTime(const EventContext& ctx, const Muon::MMPrepData* prepData, const Amg::Vector3D& globalPos, const std::vector<double>& driftDistances, std::vector<double>& driftTimes) const override;
    StatusCode calibrateStrip(const EventContext& ctx, const Identifier& id, const double time, const double charge, const double theta, const double lorentzAngle, NSWCalib::CalibratedStrip& calibStrip) const override;
    StatusCode calibrateStrip(const EventContext& ctx, const Muon::MM_RawData* mmRawData, NSWCalib::CalibratedStrip& calibStrip) const override;
    StatusCode calibrateStrip(const EventContext& ctx, const Muon::STGC_RawData* sTGCRawData, NSWCalib::CalibratedStrip& calibStrip) const override;
    
    
    bool tdoToTime  (const EventContext& ctx, const bool inCounts, const int tdo, const Identifier& chnlId, float& time, const int relBCID) const override;
    bool timeToTdo  (const EventContext& ctx, const float time, const Identifier& chnlId, int& tdo, int& relBCID) const override;
    bool chargeToPdo(const EventContext& ctx, const float charge, const Identifier& chnlId, int& pdo) const override;
    bool pdoToCharge(const EventContext& ctx, const bool inCounts, const int pdo, const Identifier& chnlId, float& charge) const override;

    virtual StatusCode initialize() override;  

    NSWCalib::MicroMegaGas mmGasProperties() const override;

    inline float mmPeakTime  () const override {return m_mmPeakTime;  }
    inline float stgcPeakTime() const override {return m_stgcPeakTime;}

  private:
    const NswCalibDbTimeChargeData* getCalibData(const EventContext& ctx) const;
    bool loadMagneticField(const EventContext& ctx, MagField::AtlasFieldCache& fieldCache ) const;
    const Muon::mmCTPClusterCalibData* getCTPClusterCalibData(const EventContext& ctx) const;

    bool timeToTdoMM(const NswCalibDbTimeChargeData* tdoPdoData, const float time, const Identifier& chnlId, int& tdo, int& relBCID) const;
    bool timeToTdoSTGC(const NswCalibDbTimeChargeData* tdoPdoData, const float time, const Identifier& chnlId, int& tdo, int& relBCID) const;

    float applyT0Calibration(const EventContext& ctx, const Identifier& id, float time) const;

    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
    SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCondObjInputKey {this, "AtlasFieldCacheCondObj", "fieldCondObj"};
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_muDetMgrKey {this, "DetectorManagerKey", "MuonDetectorManager", "Key of input MuonDetectorManager condition data"}; 
    SG::ReadCondHandleKey<NswCalibDbTimeChargeData> m_condTdoPdoKey {this, "condTdoPdoKey", "NswCalibDbTimeChargeData", "Key of NswCalibDbTimeChargeData object containing calibration data (TDO and PDO)"};
    SG::ReadCondHandleKey<NswT0Data> m_condT0Key{this, "condT0Key", "NswT0Data", "Key of NswT0Data containing the t0 calibration data"};

    Gaudi::Property<bool> m_isData{this, "isData", false, "Processing data"};

    Gaudi::Property<bool> m_CalibDriftVelocityFromData{this, "calibrateDriftVelocityFromData", false, "calibrateDriftVelocityFromData"};
    SG::ReadCondHandleKey<Muon::mmCTPClusterCalibData> m_ctpClusterCalibKey{this, "CTPClusterCalibKey", "mmCTPClusterCalibData", "Key of the CTP cluster calibration corrections"};

    StatusCode initializeGasProperties();
    
    Gaudi::Property<double> m_vDrift{this, "DriftVelocity",0.047, "Drift velocity"};
    Gaudi::Property<double> m_timeRes{this, "TimeResolution", 25., "Time resolution"};
    Gaudi::Property<double> m_longDiff{this, "longDiff", 0.019}; // mm/ mm
    Gaudi::Property<double> m_transDiff{this, "transDiff", 0.036};
    Gaudi::Property<double> m_ionUncertainty{this,"ionUncertainty", 4.0}; //ns
    Gaudi::Property<float> m_mmPeakTime{this, "mmPeakTime", 200.}; //ns
    Gaudi::Property<float> m_stgcPeakTime{this, "sTgcPeakTime", 0.}; // ns
    Gaudi::Property<std::string> m_gasMixture{this, "GasMixture",  "ArCo2_937"};

    // these values should go into the conditions database, but introduce them as properties for now
    // they describe the shift of the time to get it into the VMM time window
    Gaudi::Property<float> m_mmLatencyMC{this,"mmLatencyMC",25};
    Gaudi::Property<float> m_mmLatencyData{this,"mmLatencyData",0}; //tuned with 2023 data, should not change anymore. 
    
    Gaudi::Property<float> m_stgcLatencyMC{this,"stgcLatencyMC",-50};
    Gaudi::Property<float> m_stgcLatencyData{this,"stgcLatencyData",-50}; //this is temporary, need to align with what we find in data


    Gaudi::Property<bool> m_applyMmT0Calib {this, "applyMmT0Calib", false, "apply the MM t0 calibration"};
    Gaudi::Property<bool> m_applysTgcT0Calib {this, "applysTgcT0Calib", false, "apply the sTGC t0 calibration"};

    Gaudi::Property<bool> m_applyMmBFieldCalib {this, "applyMmBFieldCalib", true, "apply the MM BField correction in reconstruction"};


	  Gaudi::Property<float> m_mmT0TargetValue {this,  "mmT0TargetValue", 50.0, "target mean value for the MM t0 calibration"};
	  Gaudi::Property<float> m_stgcT0TargetValue {this,"stgcT0TargetValue", 0.0 ,"target mean value for the  sTGC t0 calibration"};


    double m_interactionDensitySigma{0.0F};
    double m_interactionDensityMean{0.0F};

    using angleFunction = NSWCalib::MicroMegaGas::angleFunction;
    angleFunction m_lorentzAngleFunction{NSWCalib::MicroMegaGas::dummy_func()};
   
    bool localStripPosition(const Identifier& id, Amg::Vector2D &locPos) const;

  };


}  // namespace Muon

#endif
