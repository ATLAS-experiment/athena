
//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCALIBUTILS_LAROFCALGORITHM_H
#define LARCALIBUTILS_LAROFCALGORITHM_H
 
#include <vector>
#include <string>
 
#include "LArRawConditions/LArWaveCumul.h"

#include "GaudiKernel/ToolHandle.h"
#include "LArElecCalib/ILArAutoCorrDecoderTool.h"

#include "CaloIdentifier/CaloGain.h"
#include "LArRawConditions/LArCaliWaveContainer.h"
#include "LArRawConditions/LArPhysWaveContainer.h"

#include "LArRawConditions/LArOFCComplete.h"
#include "LArRawConditions/LArOFCBinComplete.h"
#include "LArRawConditions/LArShapeComplete.h"
#include "LArCOOLConditions/LArDSPConfig.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "AthenaBaseComps/AthAlgorithm.h"

#include <Eigen/Dense>

#include "tbb/blocked_range.h"
#include "tbb/global_control.h"

#include <memory>

#include "CxxUtils/checker_macros.h"

class LArOnlineID_Base; 
class CaloDetDescrManager_Base; 


class ATLAS_NOT_THREAD_SAFE LArOFCAlg:public AthAlgorithm {
  //Acutally this algo can do internal multi-threading at finalize 
  //but not the way regular athenaMT works, so the thread-safety checker complains 
public:
 
  LArOFCAlg (const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize();
  StatusCode execute() {return StatusCode::SUCCESS;}
  virtual StatusCode stop();
  StatusCode finalize(){return StatusCode::SUCCESS;}

private:

  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKeySC{this,"ScCablingKey","LArOnOffIdMapSC","SG Key of SC LArOnOffIdMapping object"};

  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey { this
      , "CaloDetDescrManager"
      , "CaloDetDescrManager"
      , "SG Key for CaloDetDescrManager in the Condition Store" };

  SG::ReadCondHandleKey<CaloSuperCellDetDescrManager> m_caloSuperCellMgrKey { this
      , "CaloSuperCellDetDescrManager"
      , "CaloSuperCellDetDescrManager"
      , "SG Key for CaloSuperCellDetDescrManager in the Condition Store" };

  struct perChannelData_t {
    //Input:
    const LArWaveCumul* inputWave;
    HWIdentifier chid;
    unsigned gain;

    //Output:
    std::vector<std::vector<float> > ofc_a;
    std::vector<std::vector<float> > ofc_b;

    std::vector<std::vector<float> > ofcV2_a;
    std::vector<std::vector<float> > ofcV2_b;

    std::vector<std::vector<float> >shape;
    std::vector<std::vector<float> >shapeDer;
    
    float tstart;
    float timeBinWidthOFC;
    unsigned phasewMaxAt3;
    bool faultyOFC;
    bool shortWave;


    perChannelData_t(const LArWaveCumul* wave, const HWIdentifier hi, const unsigned g) : 
      inputWave(wave), chid(hi), gain(g),tstart(0), timeBinWidthOFC(25./24), phasewMaxAt3(0), faultyOFC(false), shortWave(false) {};

  };


  std::vector<perChannelData_t> m_allChannelData;

  static void           optFilt(const std::vector<float> &gWave_in, const std::vector<float>  &gDerivWave_in, const Eigen::MatrixXd& autoCorrInv, //input variables
			 std::vector<float>& OFCa, std::vector<float>& OFCb // Output variables;
			 ) ; 

  static void           optFiltDelta(const std::vector<float> &gWave_in, const std::vector<float>  &gDerivWave_in, const Eigen::MatrixXd& autoCorrInv, 
			      const Eigen::VectorXd& delta, //input variables
			      std::vector<float>& vecOFCa, std::vector<float>& vecOFCb // Output variables;
			      ) ; 

  static void           optFiltPed(const std::vector<float> &gWave_in, const std::vector<float>  &gDerivWave_in, const Eigen::MatrixXd& autoCorrInv, //input variables
			 std::vector<float>& OFCa, std::vector<float>& OFCb // Output variables;
			 ) ; 

  void process(perChannelData_t&, const LArOnOffIdMapping* cabling) const;


  bool verify(const HWIdentifier chid, const std::vector<float>& OFCa, const std::vector<float>& OFCb, 
	      const std::vector<float>& Shape, const char* ofcversion, const unsigned phase) const;

  static void printOFCVec(const std::vector<float>& vec, MsgStream& mLog) ;

  
  StatusCode     initPhysWaveContainer(const LArOnOffIdMapping* cabling);
  StatusCode     initCaliWaveContainer();

  unsigned int  m_nPoints;
    
  StringProperty           m_dumpOFCfile{this, "DumpOFCfile", ""};
  StringArrayProperty      m_keylist{this, "KeyList", {}, "List of keys to process"};
  BooleanProperty          m_verify{this, "Verify", true, "Verufy OFCs after computation"}; 
  BooleanProperty          m_normalize{this, "Normalize", false, "Normalize input wave"};
  BooleanProperty          m_timeShift{this, "TimeShift", false, "Shifting input wave"};
  IntegerProperty          m_timeShiftByIndex{this, "TimeShiftByIndex", -1, "shifting by n bins input wave"} ;


  LArCaliWaveContainer*    m_waveCnt_nc=nullptr;

  UnsignedIntegerProperty  m_nSamples{this, "Nsample", 5, "How many sample to compute"};
  UnsignedIntegerProperty  m_nPhases{this, "Nphase", 50, "How many sphases to compute"};
  UnsignedIntegerProperty  m_dPhases{this, "Dphase", 1, "Number of samples between two neighboring phases (OFC sets)"};
  UnsignedIntegerProperty  m_nDelays{this, "Ndelay", 24, "Number of delays in one clock"};
  FloatProperty            m_addOffset{this, "AddTimeOffset", 0., "Time offset to add"} ;

  ToolHandle<ILArAutoCorrDecoderTool> m_AutoCorrDecoder{this,"DecoderTool",{} };
  ToolHandle<ILArAutoCorrDecoderTool> m_AutoCorrDecoderV2{this,"DecoderToolV2", {} };

  const CaloDetDescrManager_Base* m_calo_dd_man;
  const LArOnlineID_Base*  m_onlineID; 
  const LArOFCBinComplete* m_larPhysWaveBin;

  DoubleProperty m_errAmpl{this, "ErrAmplitude", 0.01, "Allowed amplitude difference in check"};
  DoubleProperty m_errTime{this, "ErrTime",      0.01, "Allowed time difference in check"};

  BooleanProperty          m_readCaliWave{this, "ReadCaliWave",  true,       "If false PhysWave is input"};
  BooleanProperty          m_fillShape{this,    "FillShape",     false,      "Fill also shape object"};
  StringProperty           m_ofcKey{this,       "KeyOFC",        "LArOFC",   "Output key non-pileup OFCs"}; 
  StringProperty           m_ofcKeyV2{this,     "KeyOFCV2",      "LArOFCV2", "Output key pileup OFCs"}; 
  StringProperty           m_shapeKey{this,     "KeyShape",      "LArShape", "Output key Shape object"}; 
  BooleanProperty          m_storeMaxPhase{this,"StoreMaxPhase", false,      "Store phase of input wave max.?"};
  StringProperty           m_ofcBinKey{this,    "LArOFCBinKey",  "LArOFCPhase","Key for storing OFCBin object for MAx phase"};

  StringProperty           m_groupingType{this,  "GroupingType",  "SubDetector","Which grouping type to use"};
  StringProperty           m_larPhysWaveBinKey{this,"LArPhysWaveBinKey", "",  "Key for object to choose bin"};

  IntegerProperty          m_useDelta{this,      "UseDelta",      0,          "0= not use Delta, 1=only EMECIW/HEC/FCAL, 2=all , 3 = only EMECIW/HEC/FCAL1+high eta FCAL2-3"};
  IntegerProperty          m_useDeltaV2{this,    "UseDeltaV2",    0,          "Same af before for Delta"};
  BooleanProperty          m_computeV2{this,     "ComputeOFCV2",  false,      "Compute pileup OFCs?"};
  BooleanProperty          m_computePed{this,    "ComputeOFCPed", false,      "Compute OFCs with additional constraint to pedestal?"};
  IntegerProperty          m_nThreads{this,      "nThreads",      -1,         "-1: No TBB, 0: Let TBB decide, >0 number of threads"};

  BooleanProperty          m_readDSPConfig{this, "ReadDSPConfig", false,      "Read DSPConfig object ?"};
  StringProperty           m_DSPConfigFolder{this,"DSPConfigFolder","/LAR/Configuration/DSPConfiguration", "Folder for DSPConfig object"};
  std::unique_ptr<LArDSPConfig>  m_DSPConfig;

  BooleanProperty          m_forceShift{this,     "ForceShift",   false,       "Forcing shift of input wave ?"};

  BooleanProperty m_isSC{this, "isSC", false, "Running on cells or supercells?"};

  Eigen::VectorXd getDelta(std::vector<float>& samples, const HWIdentifier chid, unsigned nSamples) const;
 


  bool useDelta(const HWIdentifier chid, const int jobOFlag, const LArOnOffIdMapping* cabling) const;

  static const float m_fcal3Delta[5];
  static const float m_fcal2Delta[5];
  static const float m_fcal1Delta[5];


  //Functor for processing with TBB
  class  ATLAS_NOT_THREAD_SAFE Looper {
    //The way this class gets used is actually thread-safe
  public:
    Looper(std::vector<perChannelData_t>* p, const LArOnOffIdMapping* cabling, const LArOFCAlg* a) : m_perChanData(p), m_cabling(cabling), m_ofcAlg(a) {};
    void operator() (tbb::blocked_range<size_t>& r) const {
      for (size_t i=r.begin();i!=r.end();++i) {
	m_ofcAlg->process(m_perChanData->at(i),m_cabling);
      }
    }
  private:
    std::vector<perChannelData_t>* m_perChanData;
    const LArOnOffIdMapping* m_cabling;
    const LArOFCAlg* m_ofcAlg;
  };
};


#endif

