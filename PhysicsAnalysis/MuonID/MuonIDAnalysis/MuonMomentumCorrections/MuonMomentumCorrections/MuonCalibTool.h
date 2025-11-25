/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MCAST_MUONCALIBTOOL_H
#define MCAST_MUONCALIBTOOL_H

// Framework include(s):
#include "AsgDataHandles/ReadHandleKey.h"
#include "AsgTools/AnaToolHandle.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "MuonAnalysisInterfaces/IMuonCalibrationAndSmearingTool.h"
#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "PATInterfaces/SystematicsCache.h"
#include "xAODEventInfo/EventInfo.h"

#include "ColumnarCore/ColumnarTool.h"
#include "ColumnarCore/ColumnAccessor.h"
#include "ColumnarCore/LinkColumn.h"
#include "ColumnarEventInfo/EventInfoHelpers.h"
#include "ColumnarMuon/MuonDef.h"
#include "ColumnarMuon/MuonTrackHelpers.h"
#include <ColumnarCore/ObjectColumn.h>
#include "ColumnarTracking/TrackHelpers.h"
#include "ColumnarVariant/VariantAccessor.h"
#include "ColumnarVariant/VariantDef.h"
#include "ColumnarVariant/VariantLinkColumn.h"

#include "MuonMomentumCorrections/MuonObj.h"
#include "MuonMomentumCorrections/IMuonCalibIntTool.h"


namespace CP {

    class MuonCalibTool : public virtual IMuonCalibrationAndSmearingTool,
                                           public asg::AsgTool, public columnar::ColumnarTool<> {
        // Create a proper constructor for Athena
        ASG_TOOL_CLASS3(MuonCalibTool, CP::IMuonCalibrationAndSmearingTool, CP::ISystematicsTool,
                        CP::IReentrantSystematicsTool)
                        
    public:
        enum CalibMode {
	  //this should be sync with PhysicsAnalysis/Algorithms/MuonAnalysisAlgorithms/python/MuonAnalysisConfig.py
	    noOption = -1,  // <--- default
            correctData_CB = 0,
            correctData_IDMS = 1,
            notCorrectData_IDMS = 2,
            notCorrectData_CB = 3,
            userDefined = 99,
        };

        // Interface methods that must be defined
        // Interface - Apply the correction on a modifyable object
        virtual CorrectionCode applyCorrection(xAOD::Muon& mu) const override;
        CorrectionCode applyCorrection(columnar::MuonId mu, columnar::EventInfoId evtInfo) const;
        // Interface - Create a corrected copy from a constant muon
        virtual CorrectionCode correctedCopy(const xAOD::Muon& input, xAOD::Muon*& output) const override;
        // Interface - Is the tool affected by a specific systematic?
        virtual bool isAffectedBySystematic(const SystematicVariation& systematic) const override;
        // Interface - Which systematics have an effect on the tool's behaviour?
        virtual SystematicSet affectingSystematics() const override;
        // Interface - Systematics to be used for physics analysis
        virtual SystematicSet recommendedSystematics() const override;
        // Interface - Use specific systematic
        virtual StatusCode applySystematicVariation(const SystematicSet& systConfig) override;
        // Interface - get the expected resolution of the muon
        virtual double expectedResolution(const std::string& DetType, const xAOD::Muon& mu, const bool addMCCorrectionSmearing) const override;
        // Interface - get the expected resolution of the muon
        virtual double expectedResolution(const int& DetType, const xAOD::Muon& mu, const bool addMCCorrectionSmearing) const override;
        double expectedResolution(const int& DetType, columnar::MuonId mu, columnar::EventInfoId evtInfo, const bool addMCCorrectionSmearing) const;
        // Interface - Expert method to apply the MC correction on a modifyable trackParticle for ID- or MS-only corrections
        virtual CorrectionCode applyCorrectionTrkOnly(xAOD::TrackParticle& inTrk, const int DetType) const override;

    public:
        // Constructor
        MuonCalibTool(const std::string& name);

        // Destructor
        virtual ~MuonCalibTool() = default;

        virtual StatusCode initialize() override;

      
   protected:
        // Event info
        SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EventInfoContName", "EventInfo", "event info key"};

        Gaudi::Property<bool> m_isRun3{this, "IsRun3Geo", false, "Needed for MuonSelectionTool"}; 

        // Properties of the tool
        Gaudi::Property<std::string> m_release{this, "release", "Recs2025_03_26_Run2Run3",     "Release"};
        // Systematics scheme 
        Gaudi::Property<std::string> m_sysScheme{this, "systematicScheme", "Corr_Scale",     "Systematic scheme to be configured"};

        Gaudi::Property<bool> m_validationMode{this, "expert_validationMode", false, "Expert only option. Puts the tool in the validation mode setup"};
        Gaudi::Property<bool> m_expertMode_isData{this, "expertMode_isData", false, "Expert only option. Overwrites the isData Flag"};
        Gaudi::Property<int> m_expertMode_RunNumber{this, "expertMode_RunNumber", 0, "Expert only option. Overwrites RunNumber"};
        Gaudi::Property<unsigned long long> m_expertMode_EvtNumber{this, "expertMode_EvtNumber", 0, "Expert only option. Overwrites EventNumber"};
        Gaudi::Property<bool> m_useRndRun{this, "useRandomRunNumber", false, "To use the random run number for deciding which calibration to apply"};

        Gaudi::Property<int> m_calibMode{this, "calibMode", -1, "Calib mode"};

	//it should be used ONLY by releases>=24.2.24 and p-tag < p5834
	//more infos here https://atlas-mcp.docs.cern.ch/guidelines/muonselectiontool/index.html
	Gaudi::Property<bool> m_excludeNSWFromPrecisionLayers{this, "ExcludeNSWFromPrecisionLayers", false, "Cut on the number of precision layers ignores the NSW"};

        // Scale and Smear options
        // Do direct CB calibration 
        Gaudi::Property<bool> m_doDirectCBCalib{this, "doDirectCBCalib", true,     "Do direct calibration of CB tracks, otherwise, do ID+MS"};


        // SagittaBias properties
        Gaudi::Property<bool> m_doEtaSagittaSys{this, "doEtaSagittaSys",   false, "Do Eta dependant systematic system"};
        Gaudi::Property<bool> m_applyCorrectionOnData{this, "applyCorrectionOnData",   true, "If to apply sagitta corrections on data, or take the full effect as systematic"};


        // High pT correction options
        // For more info: https://twiki.cern.ch/twiki/bin/view/Atlas/MuonSelectionToolR21#Usage_of_the_HighPt_selection
        Gaudi::Property<bool>  m_2stations_highpt_smearing{this, "do2StationsHighPt", false, "Extra smearing to be applied if high pT WP is used"};
        // For more info: https://twiki.cern.ch/twiki/bin/view/Atlas/MuonSelectionToolR21#Usage_of_the_HighPt_selection
        Gaudi::Property<bool>  m_extra_highpt_smearing{this, "doExtraSmearing", false, "Flag provided to test if analysis are sensitive to high pT calibration. NOT TO BE USED FOR RESULTS. Please see twiki"};
        Gaudi::Property<float> m_HighPtSystThreshold{this, "HighPtSystThr", 300.0, "Thershold for high pT smearing in GeV"};      

        asg::AnaToolHandle<CP::IMuonSelectionTool> m_MuonSelectionTool{""};

        asg::AnaToolHandle<CP::IMuonCalibIntTool> m_MuonIntSagittaTool{""};
        asg::AnaToolHandle<CP::IMuonCalibIntScaleSmearTool> m_MuonIntScaleSmearTool{""};

        asg::AnaToolHandle<CP::IMuonCalibIntTool> m_MuonIntHighTSmearTool{""};


        // internal tool function
        // Converts xAOD object to an internal MuonObj for easier transfer of information
        MCP::MuonObj convertToMuonObj(columnar::MuonId mu, columnar::EventInfoId evtInfo) const;
        MCP::MuonObj convertToMuonObj(const xAOD::TrackParticle& inTrk, const int DetType) const;
        /// Decorate all information that's needed to ensure reproducibility of the smearing
        void initializeRandNumbers(MCP::MuonObj& obj, columnar::EventInfoId evtInfo) const;


        MCP::DataYear getPeriod(bool isData, columnar::EventInfoId evtInfo) const; 

   private:

        bool m_MuonIntHighTSmearToolInitialized{false};

    public:

        Gaudi::Property<bool> m_skipResolutionCategory{this, "skipResolutionCategory", false, "whether to skip the resolution category variable"};

        std::unique_ptr<MCP::MuonCalibToolAccessors> m_acc {std::make_unique<MCP::MuonCalibToolAccessors>(*this)};

        void callSingleEvent (columnar::MuonRange muons, columnar::EventInfoId event) const;
        void callEvents (columnar::EventContextRange events) const override;

    };  // class MuonCalibTool

}  // namespace CP

#endif
