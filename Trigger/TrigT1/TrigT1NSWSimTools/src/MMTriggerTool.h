/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MMTRIGGERTOOL_H
#define MMTRIGGERTOOL_H

//basic includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ConcurrencyFlags.h"
#include "CxxUtils/checker_macros.h"
#include "Gaudi/Property.h"
#include "GaudiKernel/ITHistSvc.h"
#include "MuonIdHelpers/MmIdHelper.h"

//local includes
#include "TrigT1NSWSimTools/IMMTriggerTool.h"
#include "MMLoadVariables.h"
#include "MMT_Diamond.h"

#include "MuonDigitContainer/MmDigitContainer.h"
#include "MuonDigitContainer/MmDigit.h"
#include "GeneratorObjects/McEventCollection.h"
#include "TrackRecord/TrackRecordCollection.h"


// namespace for the NSW LVL1 related classes
namespace NSWL1 {

  class MMTriggerTool : public extends<AthAlgTool, IMMTriggerTool> {

  public:

    MMTriggerTool(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~MMTriggerTool() override = default;

    virtual StatusCode initialize() override;
    StatusCode attachBranches(MuonVal::MuonTesterTree &tree) override;
    StatusCode runTrigger(const EventContext& ctx, Muon::NSW_TrigRawDataContainer* rdo, const bool do_MMDiamonds) const override;

  private:
    SG::ReadHandleKey<McEventCollection> m_keyMcEventCollection{this,"McEventCollection","TruthEvent","Location of TruthEvent"};
    SG::ReadHandleKey<TrackRecordCollection> m_keyMuonEntryLayer{this,"MuonEntryLayer","MuonEntryLayer","Location of MuonEntryLayer"};
    SG::ReadHandleKey<MmDigitContainer> m_keyMmDigitContainer{this,"MmDigitContainer","MM_DIGITS","Location of MmDigitContainer"};
    SG::ReadCondHandleKey<MuonGM::MuonDetectorManager> m_detectorManagerKey {this, "DetectorManagerKey", "MuonDetectorManager", "Key of MuonDetectorManager condition data"};
    ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc {this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

    Gaudi::Property<bool>  m_isMC            {this, "IsMC",         true, "This is MC"};
    Gaudi::Property<bool>  m_doTruth         {this, "DoTruth",     false, "Process truth information. Disabled by default"};

    // Parameters for Diamond Road algorithms
    Gaudi::Property<int>   m_diamRoadSize         {this, "DiamondRoadSize",          8,    "Number of strips to create a road"};
    Gaudi::Property<bool>  m_uv                   {this, "DiamondUV",                true, "Include Stereo planes for tracking"};
    Gaudi::Property<int>   m_diamXthreshold       {this, "DiamondEtaThreshold",      3,    "Number of Eta planes for coincidences"};
    Gaudi::Property<int>   m_diamUVthreshold      {this, "DiamondStereoThreshold",   3,    "Number of Stereo planes for coincidences"};
    Gaudi::Property<int>   m_diamOverlapEtaUp     {this, "DiamondEtaUpOverlap",      4,    "Number of Eta strips for upper road overlap"};
    Gaudi::Property<int>   m_diamOverlapEtaDown   {this, "DiamondEtaDownOverlap",    0,    "Number of Eta strips for lower road overlap"};
    Gaudi::Property<int>   m_diamOverlapStereoUp  {this, "DiamondStereoUpOverlap",   4,    "Number of Stereo strips for upper road overlap"};
    Gaudi::Property<int>   m_diamOverlapStereoDown{this, "DiamondStereoDownOverlap", 0,    "Number of Stereo strips for lower road overlap"};

    // Parameters for RDO encoding
    Gaudi::Property<std::string>  m_mmDigitContainer{this, "MM_DigitContainerName", "MM_DIGITS", "Name of the MM digit container"};
    Gaudi::Property<bool>         m_doNtuple        {this, "DoNtuple",   false,          "Input the MMStrip branches into the analysis ntuple"};
    Gaudi::Property<float>        m_phiMin          {this, "PhiMin",    -16.*M_PI/180.0, "Minimum Phi"};
    Gaudi::Property<float>        m_phiMax          {this, "PhiMax",     16.*M_PI/180.0, "Maximum Phi"};
    Gaudi::Property<int>          m_phiBits         {this, "PhiBits",    6,              "Number of Phi bits"};
    Gaudi::Property<float>        m_rMin            {this, "RMin",       900.0,          "Minimum R [mm]"};
    Gaudi::Property<float>        m_rMax            {this, "RMax",       5000.0,         "Maximum R [mm]"};
    Gaudi::Property<int>          m_rBits           {this, "RBits",      8,              "Number of R bits"};
    Gaudi::Property<float>        m_dThetaMin       {this, "DThetaMin", -0.015,          "Minimum dTheta [rad]"};
    Gaudi::Property<float>        m_dThetaMax       {this, "DThetaMax",  0.015,          "Maximum dTheta [rad]"};
    Gaudi::Property<int>          m_dThetaBits      {this, "DThetaBits", 5,              "Number of dTheta bits"};

    std::unique_ptr<MMT_Diamond> m_diamond;

    std::shared_ptr<MuonVal::VectorBranch<unsigned int> > m_trigger_diamond_ntrig ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<int> > m_trigger_diamond_bc ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<char> > m_trigger_diamond_sector ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<int> > m_trigger_diamond_sectorPhi ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<unsigned int> > m_trigger_diamond_totalCount ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<unsigned int> > m_trigger_diamond_realCount ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<int> > m_trigger_diamond_iX ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<int> > m_trigger_diamond_iU ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<int> > m_trigger_diamond_iV ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<unsigned int> > m_trigger_diamond_XbkgCount ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<unsigned int> > m_trigger_diamond_UVbkgCount ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<unsigned int> > m_trigger_diamond_XmuonCount ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<unsigned int> > m_trigger_diamond_UVmuonCount ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<int> > m_trigger_diamond_age ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_diamond_mx ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_diamond_my ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_diamond_Uavg ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_diamond_Vavg ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_diamond_mxl ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_diamond_theta ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_diamond_eta ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_diamond_dtheta ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_diamond_phi ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_diamond_phiShf ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<uint8_t> > m_trigger_diamond_TP_phi_id ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<uint8_t> > m_trigger_diamond_TP_R_id ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<uint8_t> > m_trigger_diamond_TP_dTheta_id ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_RZslopes ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_trueEtaRange ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_truePtRange ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<int> > m_trigger_VMM ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<int> > m_trigger_plane ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<int> > m_trigger_station ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<int> > m_trigger_strip ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_slope ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_trueThe ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_truePhi ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_trueDth ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_trueEtaEnt ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_trueTheEnt ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_truePhiEnt ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_trueEtaPos ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_trueThePos ATLAS_THREAD_SAFE {};
    std::shared_ptr<MuonVal::VectorBranch<double> > m_trigger_truePhiPos ATLAS_THREAD_SAFE {};
  };  // end of MMTriggerTool class
} // namespace NSWL1
#endif
