/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//****************************************************************************
/// Filename : TileTBAANtuple.h
/// Author   : Luca Fiorini (based on TileTBNtuple)
/// Created  : Mar, 2007
///
/// DESCRIPTION
/// 
///    To create TileCal Ntuple file with all digits and rawChannels 
///
/// Properties (JobOption Parameters):
///
///    TileDigitsContainer     string   key value of Digits in TDS 
///    TileBeamElemContainer   string   key of BeamElems in TDS
///    TileRawChannelContainerFlat      key of flat filtered RawChannels in TDS
///    TileRawChannelContainerFit       key of fit filtered RawChannels in TDS
///    TileRawChannelContainerFitCool   key of fit filtered RawChannels in TDS
///    CalibrateEnergy         bool     If calibration should be applied to energy
///    CalibMode               bool     If data is in calibration mode
///    NtupleLoc               string   pathname of ntuple file
///    NtupleID                int      ID of ntuple
///    BC1X1                   float    TDC Conv param
///    BC1X2                   float    TDC Conv param
///    BC1Y1                   float    TDC Conv param
///    BC1Y2                   float    TDC Conv param
///    BC1Z                    float    TDC Conv param
///    BC2X1                   float    TDC Conv param
///    BC2X2                   float    TDC Conv param
///    BC2Y1                   float    TDC Conv param
///    BC2Y2                   float    TDC Conv param
///    BC2Z                    float    TDC Conv param
///
/// TODO: Use 7 samples in bigain runs. Now all array variables are stored in Ntuple as 9 elements long. 
///       Make the code store informations about modules even if they appear during the run.
///       Some change is necessary for the new feature of the reconstruction that can mask empty fragments. 
/// History:
///  
///  
//****************************************************************************
#ifndef TileTBAANtuple_H
#define TileTBAANtuple_H

// Gaudi includes
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ITHistSvc.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

// Athena includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "CaloEvent/CaloCellContainer.h"

// Tile includes
#include "TileConditions/TileInfo.h"
#include "TileEvent/TileLaserObject.h"
#include "TileEvent/TileDigitsContainer.h"
#include "TileEvent/TileBeamElemContainer.h"
#include "TileEvent/TileRawChannelContainer.h"
#include "TileEvent/TileHitContainer.h"
#include "TileSimEvent/TileHitVector.h"
#include "TileConditions/TileSamplingFraction.h"
#include "TileConditions/TileCablingService.h"
#include "TileConditions/TileCondToolEmscale.h"
#include "TileIdentifier/TileRawChannelUnit.h"
#include "TileRecUtils/TileRawChannelBuilderFlatFilter.h"
#include "TileEvent/TileDQstatus.h"

#include "TFile.h"
#include "TMatrixT.h"
#include "TTree.h"
#include <string>
#include <vector>
#include <map>
#include <stdint.h>

class ITHistSvc;
class TileID;
class TileHWID;
//class TileCablingSvc;
class TileBeamElemContByteStreamCnv;
class TileLaserObject;
class TileHit;

class TileTBAANtuple: public AthAlgorithm {
  public:
    //Constructor
    TileTBAANtuple(const std::string& name, ISvcLocator* pSvcLocator);

    //Destructor 
    virtual ~TileTBAANtuple() = default;
    StatusCode ntuple_initialize(const EventContext& ctx);
    StatusCode ntuple_clear();

    //Gaudi Hooks
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;

  private:

    enum {NOT_SETUP = -9999};
    enum {MAX_MINIDRAWER = 4, MAX_CHAN = 48, MAX_DMU = 16};

    /**
     * @brief Name of TileSamplingFraction in condition store
     */
    SG::ReadCondHandleKey<TileSamplingFraction> m_samplingFractionKey{this,
        "TileSamplingFraction", "TileSamplingFraction", "Input Tile sampling fraction"};

    SG::ReadHandleKey<TileDigitsContainer> m_digitsContainerKey{this,
        "TileDigitsContainer", "TileDigitsCnt", "Input Tile digits container"};

    SG::ReadHandleKey<TileDigitsContainer> m_digitsContainerFlxKey{this,
        "TileDigitsContainerFlx", "", "Input Tile FELIX digits container"};

    SG::ReadHandleKey<TileBeamElemContainer> m_beamElemContainerKey{this,
        "TileBeamElemContainer", "TileBeamElemCnt", "Input Tile beam elements container"};

    SG::ReadHandleKey<TileRawChannelContainer> m_flatRawChannelContainerKey{this,
        "TileRawChannelContainerFlat", "", "Input Tile raw channel container reconstructed with Flat method"};

    SG::ReadHandleKey<TileRawChannelContainer> m_fitRawChannelContainerKey{this,
        "TileRawChannelContainerFit", "", "Input Tile raw channel container reconstructed with Fit method"};

    SG::ReadHandleKey<TileRawChannelContainer> m_optRawChannelContainerKey{this,
        "TileRawChannelContainerOpt", "TileRawChannelOpt2", "Input Tile raw channel container reconstructed with Opt2 method"};

    SG::ReadHandleKey<TileRawChannelContainer> m_dspRawChannelContainerKey{this,
        "TileRawChannelContainerDsp", "", "Input Tile DSP raw channel container"};

    SG::ReadHandleKey<TileRawChannelContainer> m_fitcRawChannelContainerKey{this,
        "TileRawChannelContainerFitCool", "", "Input Tile raw channel container reconstructed with Fit COOL method"};

    SG::ReadHandleKey<TileRawChannelContainer> m_flxFitRawChannelContainerKey{this,
        "TileRawChannelContainerFitFlx", "", "Input Tile FELIX raw channel container reconstructed with Fit method"};

    SG::ReadHandleKey<TileRawChannelContainer> m_flxOptRawChannelContainerKey{this,
        "TileRawChannelContainerOptFlx", "", "Input Tile FELIX raw channel container reconstructed with Opt2 method"};

    SG::ReadHandleKey<TileLaserObject> m_laserObjectKey{this,
        "TileLaserObj", "", "Input Tile laser object"};

    SG::ReadHandleKey<TileHitContainer> m_hitContainerKey{this,
        "TileHitContainer", "TileHitCnt", "Input Tile hit container"};

    SG::ReadHandleKey<TileHitVector> m_hitVectorKey{this,
        "TileHitVector", "TileHitVec", "Input Tile hit vector"};

    SG::ReadHandleKey<CaloCellContainer> m_cellContainerKey{this,
        "CaloCellContainer", "AllCalo", "Input Calo cell container"};

    ToolHandle<TileCondToolEmscale> m_tileToolEmscale{this,
        "TileCondToolEmscale", "TileCondToolEmscale", "Tile EMS conditions tool"};

    ToolHandle<TileRawChannelBuilderFlatFilter> m_adderFilterAlgTool{this,
        "TileRawChannelBuilderFlatFilter", "TileRawChannelBuilderFlatFilter", "Tile raw channel builder tool"};

    SG::ReadHandleKey<TileDQstatus> m_dqStatusKey{ this,
       "TileDQstatus", "TileDQstatus", "TileDQstatus key" };

    Gaudi::Property<bool> m_calibrateEnergy{this, "CalibrateEnergy", true, "Calibrate energy"};
    Gaudi::Property<bool> m_useDspUnits{this, "UseDspUnits", false, "Use DSP untis"};
    Gaudi::Property<int> m_finalUnit{this, "OfflineUnits", TileRawChannelUnit::MegaElectronVolts, "Calibrate everything to this level"};
    Gaudi::Property<bool> m_calibMode{this, "CalibMode", false, "If data should be put in calib mode"};
    Gaudi::Property<bool> m_unpackAdder{this, "UnpackAdder", false, "Unpack adder"};
    Gaudi::Property<bool> m_completeNtuple{this, "CompleteNtuple", true, "Complete the ntuple"};
    Gaudi::Property<bool> m_bsInput{this, "BSInput", true, "Bytestream input"};
    Gaudi::Property<bool> m_pmtOrder{this, "PMTOrder", true, "Change channel ordering to pmt ordering in the ntuple"};
    Gaudi::Property<int> m_nSamples{this, "NSamples", NOT_SETUP, "Number of samples"};
    Gaudi::Property<int> m_nSamplesFlx{this, "NSamplesFelix", NOT_SETUP, "Number of samples for FELIX"};
    Gaudi::Property<unsigned int> m_nDrawers{this, "NDrawers", 6, "Number of drawers"};
    Gaudi::Property<unsigned int> m_nDrawersFlx{this, "NDrawersFelix", 0, "Number of drawers for FELIX"};
    Gaudi::Property<int> m_TBperiod{this, "TBperiod", 2016, "Tuned for 2016 testbeam by default"};
    Gaudi::Property<int> m_eventsPerFile{this, "EventsPerFile", 200000, "Number of events per file"};
    Gaudi::Property<Long64_t> m_treeSize{this, "TreeSize", 16000000000LL, "Size of tree"};
    Gaudi::Property<std::string> m_streamName{this, "StreamName", "AANT", "Name of the output stream"};
    Gaudi::Property<std::string> m_ntupleID{this, "NTupleID", "h1000", "Name of the ntuple ID"};

    Gaudi::Property<std::vector<std::string>> m_rosName{this, "rosName", {"B", "A", "C", "D", "E"}, "Name of arrays in ntuple for different ROSes"};
    Gaudi::Property<std::vector<std::string>> m_drawerList{this, "drawerList", {"-1"}, "List of frag IDs in correct order; Setup drawer list from data"};
    Gaudi::Property<std::vector<int>> m_drawerType{this, "drawerType", {}, "Type of every drawer 1-4: B+, B-, EB+, EB-; Take drawer type from Frag ID (doesn't work for 2003)"};
    Gaudi::Property<std::vector<std::string>> m_beamFragList{this, "beamFragList", {}, "List of beam frag IDs to store in the ntuple"};

    Gaudi::Property<float> m_beamBN2X1{this, "BN2X1", 0.0, "Params for Beam TDC: Beam chamber: -2"};
    Gaudi::Property<float> m_beamBN2X2{this, "BN2X2", 0.2, "Params for Beam TDC: Beam chamber: -2"};
    Gaudi::Property<float> m_beamBN2Y1{this, "BN2Y1", 0.0, "Params for Beam TDC: Beam chamber: -2"};
    Gaudi::Property<float> m_beamBN2Y2{this, "BN2Y2", 0.2, "Params for Beam TDC: Beam chamber: -2"};

    Gaudi::Property<float> m_beamBN1X1{this, "BN1X1", 0.0, "Params for Beam TDC: Beam chamber: -1"};
    Gaudi::Property<float> m_beamBN1X2{this, "BN1X2", 0.2, "Params for Beam TDC: Beam chamber: -1"};
    Gaudi::Property<float> m_beamBN1Y1{this, "BN1Y1", 0.0, "Params for Beam TDC: Beam chamber: -1"};
    Gaudi::Property<float> m_beamBN1Y2{this, "BN1Y2", 0.2, "Params for Beam TDC: Beam chamber: -1"};

    Gaudi::Property<float> m_beamBC0X1{this, "BC0X1", 0.0, "Params for Beam TDC: Beam chamber: 0"};
    Gaudi::Property<float> m_beamBC0X2{this, "BC0X2", 0.2, "Params for Beam TDC: Beam chamber: 0"};
    Gaudi::Property<float> m_beamBC0Y1{this, "BC0Y1", 0.0, "Params for Beam TDC: Beam chamber: 0"};
    Gaudi::Property<float> m_beamBC0Y2{this, "BC0Y2", 0.2, "Params for Beam TDC: Beam chamber: 0"};
    Gaudi::Property<float> m_beamBC0Z{this, "BC0Z", 17138.0, "Params for Beam TDC: Beam chamber: 0"};

    Gaudi::Property<float> m_beamBC1X1{this, "BC1X1", NOT_SETUP - 1, "Params for Beam TDC: Beam chamber: 1"};
    Gaudi::Property<float> m_beamBC1X2{this, "BC1X2", NOT_SETUP - 1, "Params for Beam TDC: Beam chamber: 1"};
    Gaudi::Property<float> m_beamBC1Y1{this, "BC1Y1", NOT_SETUP - 1, "Params for Beam TDC: Beam chamber: 1"};
    Gaudi::Property<float> m_beamBC1Y2{this, "BC1Y2", NOT_SETUP - 1, "Params for Beam TDC: Beam chamber: 1"};
    Gaudi::Property<float> m_beamBC1Z{this, "BC1Z", NOT_SETUP - 1, "Params for Beam TDC: Beam chamber: 1"};
    Gaudi::Property<float> m_beamBC1Z_0{this, "BC1Z_0", NOT_SETUP - 1, "Params for Beam TDC: Beam chamber: 1"};
    Gaudi::Property<float> m_beamBC1Z_90{this, "BC1Z_90", NOT_SETUP - 1, "Params for Beam TDC: Beam chamber: 1"};
    Gaudi::Property<float> m_beamBC1Z_min90{this, "BC1Z_min90", NOT_SETUP - 1, "Params for Beam TDC: Beam chamber: 1"};

    Gaudi::Property<float> m_beamBC2X1{this, "BC2X1", NOT_SETUP - 1, "Params for Beam TDC: Beam chamber: 2"};
    Gaudi::Property<float> m_beamBC2X2{this, "BC2X2", NOT_SETUP - 1, "Params for Beam TDC: Beam chamber: 2"};
    Gaudi::Property<float> m_beamBC2Y1{this, "BC2Y1", NOT_SETUP - 1, "Params for Beam TDC: Beam chamber: 2"};
    Gaudi::Property<float> m_beamBC2Y2{this, "BC2Y2", NOT_SETUP - 1, "Params for Beam TDC: Beam chamber: 2"};
    Gaudi::Property<float> m_beamBC2Z{this, "BC2Z", NOT_SETUP - 1, "Params for Beam TDC: Beam chamber: 2"};
    Gaudi::Property<float> m_beamBC2Z_0{this, "BC2Z_0", NOT_SETUP - 1, "Params for Beam TDC: Beam chamber: 2"};
    Gaudi::Property<float> m_beamBC2Z_90{this, "BC2Z_90", NOT_SETUP - 1, "Params for Beam TDC: Beam chamber: 2"};
    Gaudi::Property<float> m_beamBC2Z_min90{this, "BC2Z_min90", NOT_SETUP - 1, "Params for Beam TDC: Beam chamber: 2"};

    Gaudi::Property<float> m_radius{this, "Radius", 2280.0, "Inner radius of calo, for CTB 2004 only"};
    Gaudi::Property<std::string> m_etaFileName{this, "EtaFileName", "TileEtaCTB.txt", "File name with ETA, for CTB 2004 only"};

    StatusCode storeRawChannels(const EventContext& ctx
                                , const SG::ReadHandleKey<TileRawChannelContainer>& containerKey
                                , bool calibMode
                                , std::vector<std::array<float, MAX_CHAN>>* eneVec
                                , std::vector<std::array<float, MAX_CHAN>>* timeVec
                                , std::vector<std::array<float, MAX_CHAN>>* chi2Vec
                                , std::vector<std::array<float, MAX_CHAN>>* pedVec
                                , bool saveDQstatus = false);

    StatusCode storeDigits(const EventContext& ctx, const SG::ReadHandleKey<TileDigitsContainer>& containerKey, const TileDQstatus* dqStatus=nullptr);
    StatusCode storeDigitsFlx(const EventContext& ctx, const SG::ReadHandleKey<TileDigitsContainer>& containerKey);
    StatusCode storeBeamElements(const EventContext& ctx);
    StatusCode storeCells(const EventContext& ctx);
    StatusCode storeLaser(const EventContext& ctx);
    StatusCode storeHitVector(const EventContext& ctx);
    StatusCode storeHitContainer(const EventContext& ctx);
    void storeHit(const TileHit *hit, int fragType, int fragId,
                  std::array<float, MAX_CHAN>& ehitVec,
                  std::array<float, MAX_CHAN>& thitVec,
                  const TileSamplingFraction* samplingFraction);

    StatusCode initList(const EventContext& ctx);
    StatusCode initListFlx(const EventContext& ctx);
    StatusCode initNTuple(void);
    //StatusCode connectFile(void);

    void getEta(void);

    void TRIGGER_addBranch(void);
    void MUON_addBranch(void);
    void ECAL_addBranch(void);
    void QDC_addBranch(void);
    void LASER_addBranch(void);
    void ADDER_addBranch(void);
    void CISPAR_addBranch(void);
    void BEAM_addBranch(void);
    void DIGI_addBranch(void);
    void FELIX_addBranch(void);
    //void RAW_addBranch(void);
    void HIT_addBranch(void);
    void ENETOTAL_addBranch(void);
    void COINCBOARD_addBranch(void);
    void LASEROBJ_addBranch(void);

    void TRIGGER_clearBranch(void);
    void MUON_clearBranch(void);
    void ECAL_clearBranch(void);
    void QDC_clearBranch(void);
    void LASER_clearBranch(void);
    void ADDER_clearBranch(void);
    void CISPAR_clearBranch(void);
    void BEAM_clearBranch(void);
    void DIGI_clearBranch(void);
    void FELIX_clearBranch(void);
    //void RAW_clearBranch(void);
    void HIT_clearBranch(void);
    void ENETOTAL_clearBranch(void);
    void COINCBOARD_clearBranch(void);
    void LASEROBJ_clearBranch(void);

    template<typename T>
    void clear_init_minus1(std::vector<T>& vec);

    template<typename T, size_t N>
    void clear_init_minus1(std::vector<std::array<T,N>>& vec);

    template<typename T, size_t N>
    void clear_init_zero(std::vector<std::array<T,N>>& vec);

    void clear_samples(std::vector<std::unique_ptr<int []>>& vec, const std::vector<int>& nsamples, int nchan=MAX_CHAN);
    
    inline int digiChannel2PMT(int fragType, int chan) {
      return (abs(m_cabling->channel2hole(fragType, chan)) - 1);
    }

    /// bit_31 of the DMU header must be 1 and
    /// bit_17 of the DMU header must be 0
    inline short CheckDMUFormat(uint32_t header) {
      if (((header >> 31 & 0x1) == 1) && ((header >> 17 & 0x1) == 0))
        return 0; // no error
      else
        return 1; //error
    }

    /// Parity of the DMU header should be odd

    inline short CheckDMUParity(uint32_t header) {
      uint32_t parity(0);
      for (int i = 0; i < 32; ++i)
        parity += header >> i & 0x1;

      if ((parity % 2) == 1)
        return 0; //no error
      else
        return 1; //error
    }

    void checkIsPropertySetup(float property, const std::string& name) {
      if (property < NOT_SETUP) {
        ATH_MSG_ERROR("The following property should be set up via JO: " << name);
      }
    }

    void setupPropertyDefaultValue(float property, float defaultValue, const std::string& name) {
      if (property < NOT_SETUP) {
        property = defaultValue;
        ATH_MSG_INFO("The following property is not set up via JO, using default value: " << name << "=" << defaultValue);
      }
    }

    void setupBeamChambersBeforeTB2015(void);
    void setupBeamChambersTB2015(void);
    void setupBeamChambersTB2016_2020(void);

    //

    //handle to THistSvc
    ServiceHandle<ITHistSvc> m_thistSvc;

    // The ntuple
    TTree* m_ntuplePtr{nullptr};
    bool m_ntupleCreated{false};

    // event number
    int m_evtNr{-1};

    // Trigger items
    int m_evTime{};
    int m_run{};
    int m_evt{};
    int m_trigType{};
    int m_dspFlags{};

    // 0 - Beam, 1 neg eta, 2 pos eta
    std::vector<int> m_l1ID;
    std::vector<int> m_l1Type;
    std::vector<int> m_evBCID;
    std::vector<int> m_evType;
    std::vector<int> m_frBCID;

    // Muon items
    float m_muBackHit{};
    float m_muBackSum{};
    std::array<float,14> m_muBack{}; // MUON/MuBack
    std::array<float,2> m_muCalib{}; // MUON/MuCalib

    // Ecal
    std::array<float,8> m_ecal{};

    // QDC
    std::array<uint32_t,33> m_qdc{};

    // laser items
    int m_las_BCID{};

    int m_las_Filt{};
    double m_las_ReqAmp{};
    double m_las_MeasAmp{};

    int m_las_D1_ADC{};
    int m_las_D2_ADC{};
    int m_las_D3_ADC{};
    int m_las_D4_ADC{};

    double m_las_D1_Ped{};
    double m_las_D2_Ped{};
    double m_las_D3_Ped{};
    double m_las_D4_Ped{};

    double m_las_D1_Ped_RMS{};
    double m_las_D2_Ped_RMS{};
    double m_las_D3_Ped_RMS{};
    double m_las_D4_Ped_RMS{};

    double m_las_D1_Alpha{};
    double m_las_D2_Alpha{};
    double m_las_D3_Alpha{};
    double m_las_D4_Alpha{};

    double m_las_D1_Alpha_RMS{};
    double m_las_D2_Alpha_RMS{};
    double m_las_D3_Alpha_RMS{};
    double m_las_D4_Alpha_RMS{};

    double m_las_D1_AlphaPed{};
    double m_las_D2_AlphaPed{};
    double m_las_D3_AlphaPed{};
    double m_las_D4_AlphaPed{};

    double m_las_D1_AlphaPed_RMS{};
    double m_las_D2_AlphaPed_RMS{};
    double m_las_D3_AlphaPed_RMS{};
    double m_las_D4_AlphaPed_RMS{};

    int m_las_PMT1_ADC{};
    int m_las_PMT2_ADC{};

    int m_las_PMT1_TDC{};
    int m_las_PMT2_TDC{};

    double m_las_PMT1_Ped{};
    double m_las_PMT2_Ped{};

    double m_las_PMT1_Ped_RMS{};
    double m_las_PMT2_Ped_RMS{};

    double m_las_Temperature{};

    int m_lasFlag{};
    float m_las0{};
    float m_las1{};
    float m_las2{};
    float m_las3{};
    std::array<float, 4> m_lasExtra{};

    // pattern Unit in common beam crate
    int m_commonPU{};

    // Adder items
    int** m_adder{};
    std::array<float, 16> m_eneAdd{};
    std::array<float, 16> m_timeAdd{};

    // Cispar
    int m_cispar[16]{};

    // TDC/BEAM Items
    uint32_t m_s1cou{};
    uint32_t m_s2cou{};
    uint32_t m_s3cou{};
    uint32_t m_cher1{};
    uint32_t m_cher2{};
    uint32_t m_cher3{};
    uint32_t m_muTag{};
    uint32_t m_muHalo{};
    uint32_t m_muVeto{};

    int m_s2extra{};
    int m_s3extra{};

    int m_sc1{};
    int m_sc2{};

    std::array<int, 16> m_tof{};
    std::array<int, 16> m_btdc1{};
    std::array<int, 16> m_btdc2{};
    std::array<int, 16> m_scaler{};
    std::vector<std::vector<int> > *m_btdc{};
    int m_tjitter{};
    int m_tscTOF{};
    int m_btdcNhit[16]{};
    int m_btdcNchMultiHit[2]{};

    float m_xChN2{};
    float m_yChN2{};
    float m_xChN1{};
    float m_yChN1{};
    float m_xCha0{};
    float m_yCha0{};

    float m_xCha1{};
    float m_yCha1{};
    float m_xCha2{};
    float m_yCha2{};
    float m_xCha1_0{};
    float m_yCha1_0{};
    float m_xCha2_0{};
    float m_yCha2_0{};
    float m_xImp{};
    float m_yImp{};

    float m_xImp_0{};
    float m_yImp_0{};
    float m_xImp_90{};
    float m_yImp_90{};
    float m_xImp_min90{};
    float m_yImp_min90{};
    // Digi/Energy items
    std::vector<int> m_evtVec;
    std::vector<short> m_rodBCIDVec;
    std::vector<short> m_sizeVec;

    std::vector<int> m_evtflxVec;
    std::vector<short> m_rodBCIDflxVec;
    std::vector<short> m_sizeflxVec;

    std::vector<std::array<int, MAX_CHAN>> m_gainflxVec;
    std::vector<std::unique_ptr<int []>> m_sampleflxVec;

    std::vector<std::array<int, MAX_DMU>> m_bcidVec;
    std::vector<std::array<uint32_t, MAX_DMU>> m_DMUheaderVec;
    std::vector<std::array<short, MAX_DMU>> m_DMUformatErrVec;
    std::vector<std::array<short, MAX_DMU>> m_DMUparityErrVec;
    std::vector<std::array<short, MAX_DMU>> m_DMUmemoryErrVec;
    std::vector<std::array<short, MAX_DMU>> m_DMUDstrobeErrVec;
    std::vector<std::array<short, MAX_DMU>> m_DMUSstrobeErrVec;
    std::vector<std::array<short, MAX_DMU>> m_DMUHeadparityErrVec;
    std::vector<std::array<short, MAX_DMU>> m_DMUDataparityErrVec;
    std::vector<std::array<int, 2>> m_dmuMaskVec;
    std::vector<std::array<int, 2>> m_slinkCRCVec;
    std::vector<std::array<int, MAX_CHAN>> m_gainVec;
    std::vector<std::unique_ptr<int []>> m_sampleVec;
    std::vector<std::array<int, MAX_DMU>> m_feCRCVec; //we use int, because vector<bool> and shorts are bugged
    std::vector<std::array<int, MAX_DMU>> m_rodCRCVec;

    std::vector<std::array<float, MAX_CHAN>> m_eneVec;
    std::vector<std::array<float, MAX_CHAN>> m_timeVec;
    std::vector<std::array<float, MAX_CHAN>> m_pedFlatVec;
    std::vector<std::array<float, MAX_CHAN>> m_chi2FlatVec;
    std::vector<std::array<float, MAX_CHAN>> m_efitVec;
    std::vector<std::array<float, MAX_CHAN>> m_tfitVec;
    std::vector<std::array<float, MAX_CHAN>> m_pedfitVec;
    std::vector<std::array<float, MAX_CHAN>> m_chi2Vec;
    std::vector<std::array<float, MAX_CHAN>> m_efitcVec;
    std::vector<std::array<float, MAX_CHAN>> m_tfitcVec;
    std::vector<std::array<float, MAX_CHAN>> m_pedfitcVec;
    std::vector<std::array<float, MAX_CHAN>> m_chi2cVec;
    std::vector<std::array<float, MAX_CHAN>> m_eOptVec;
    std::vector<std::array<float, MAX_CHAN>> m_tOptVec;
    std::vector<std::array<float, MAX_CHAN>> m_pedOptVec;
    std::vector<std::array<float, MAX_CHAN>> m_chi2OptVec;
    std::vector<std::array<float, MAX_CHAN>> m_eDspVec;
    std::vector<std::array<float, MAX_CHAN>> m_tDspVec;
    std::vector<std::array<float, MAX_CHAN>> m_chi2DspVec;

    std::vector<std::array<float, MAX_CHAN>> m_eflxfitVec;
    std::vector<std::array<float, MAX_CHAN>> m_tflxfitVec;
    std::vector<std::array<float, MAX_CHAN>> m_chi2flxfitVec;
    std::vector<std::array<float, MAX_CHAN>> m_pedflxfitVec;
    std::vector<std::array<float, MAX_CHAN>> m_eflxoptVec;
    std::vector<std::array<float, MAX_CHAN>> m_tflxoptVec;
    std::vector<std::array<float, MAX_CHAN>> m_chi2flxoptVec;
    std::vector<std::array<float, MAX_CHAN>> m_pedflxoptVec;

    std::vector<short> m_ROD_GlobalCRCVec;
    std::vector<std::array<short, MAX_DMU>> m_ROD_DMUBCIDVec;
    std::vector<std::array<short, MAX_DMU>> m_ROD_DMUmemoryErrVec;
    std::vector<std::array<short, MAX_DMU>> m_ROD_DMUSstrobeErrVec;
    std::vector<std::array<short, MAX_DMU>> m_ROD_DMUDstrobeErrVec;
    std::vector<std::array<short, MAX_DMU>> m_ROD_DMUHeadformatErrVec;
    std::vector<std::array<short, MAX_DMU>> m_ROD_DMUDataformatErrVec;
    std::vector<std::array<short, 2>> m_ROD_DMUMaskVec;

    std::vector<std::array<int, MAX_MINIDRAWER>> m_mdL1idflxVec;
    std::vector<std::array<int, MAX_MINIDRAWER>> m_mdBcidflxVec;
    std::vector<std::array<int, MAX_MINIDRAWER>> m_mdModuleflxVec;
    std::vector<std::array<int, MAX_MINIDRAWER>> m_mdRunTypeflxVec;
    std::vector<std::array<int, MAX_MINIDRAWER>> m_mdPedLoflxVec;
    std::vector<std::array<int, MAX_MINIDRAWER>> m_mdPedHiflxVec;
    std::vector<std::array<int, MAX_MINIDRAWER>> m_mdRunflxVec;
    std::vector<std::array<int, MAX_MINIDRAWER>> m_mdChargeflxVec;
    std::vector<std::array<int, MAX_MINIDRAWER>> m_mdChargeTimeflxVec;
    std::vector<std::array<int, MAX_MINIDRAWER>> m_mdCapacitorflxVec;

    std::array<float, 4> m_LarEne{};
    std::array<float, 3> m_BarEne{};
    std::array<float, 3> m_ExtEne{};
    std::array<float, 3> m_GapEne{};

    std::array<unsigned int, 96> m_coincTrig1{};
    std::array<unsigned int, 96> m_coincTrig2{};
    std::array<unsigned int, 96> m_coincTrig3{};
    std::array<unsigned int, 96> m_coincTrig4{};
    std::array<unsigned int, 96> m_coincTrig5{};
    std::array<unsigned int, 96> m_coincTrig6{};
    std::array<unsigned int, 96> m_coincTrig7{};
    std::array<unsigned int, 96> m_coincTrig8{};

    int m_coincFlag1{};
    int m_coincFlag2{};
    int m_coincFlag3{};
    int m_coincFlag4{};
    int m_coincFlag5{};
    int m_coincFlag6{};
    int m_coincFlag7{};
    int m_coincFlag8{};

    std::map<unsigned int, unsigned int, std::less<unsigned int> > m_drawerMap; // map for frag IDs -> index
    std::map<unsigned int, unsigned int, std::less<unsigned int> > m_drawerFlxMap; // map for frag IDs -> index for FELIX
    typedef std::map<unsigned int, unsigned int, std::less<unsigned int> >::iterator drawerMap_iterator;

    bool m_beamIdList[32]; // list of beam frag IDs to store in the ntuple

    //run number
    int m_runNumber{};
    float m_eta{};
    float m_theta{};

    //MC truth info
    std::vector<std::array<float, MAX_CHAN>> m_ehitVec;
    std::vector<std::array<float, MAX_CHAN>> m_thitVec;
    std::vector<std::array<float, MAX_CHAN>> m_ehitCnt;
    std::vector<std::array<float, MAX_CHAN>> m_thitCnt;

    bool m_calibrateEnergyThisEvent{false};

    TileRawChannelUnit::UNIT m_rchUnit;  //!< Unit for TileRawChannels (ADC, pCb, MeV)
    TileRawChannelUnit::UNIT m_dspUnit;  //!< Unit for TileRawChannels in DSP

    // Identifiers
    const TileID* m_tileID{nullptr};
    const TileHWID* m_tileHWID{nullptr};
    const TileCablingService* m_cabling{nullptr};

    std::map<int, int> m_nSamplesInDrawerMap;
    std::map<int, int> m_nSamplesFlxInDrawerMap;
    std::vector<int> m_nSamplesInDrawer;
    std::vector<int> m_nSamplesFlxInDrawer;
    bool m_saveFelixData{false};



};

#endif
