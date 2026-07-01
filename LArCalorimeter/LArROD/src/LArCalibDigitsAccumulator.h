/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @class LArCalibDigitsAccumulator
 * @version \$Id: LArCalibDigitsAccumulator.h,v 1.8 2008-06-12 14:51:35 lafaye Exp $
 * @author Sandrine Laplace <laplace@lapp.in2p3.fr>
 * @date 22-January-2006
 * @brief Emulator of DSP code in accumulation calibration mode
 *
 * Input to this algorithm are LArCalibDigits. These LArCalibDigits are accumulated
 * (sum and squared sum)over a number of triggers given by the calibration run settings. 
 * Intermediate sums are also computed if the JO property "StepOfTriggers" is set to 
 * a larger number than one. The output of the algorithm is a LArAccumulatedCalibDigitContainer.
 *   */


#ifndef LARCALIBDIGITSACCUMULATOR
#define LARCALIBDIGITSACCUMULATOR
#include <sstream>

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "LArRawEvent/LArCalibDigitContainer.h"
#include "LArRawEvent/LArAccumulatedCalibDigitContainer.h"
#include "LArIdentifier/LArOnlineID_Base.h"
#include "StoreGate/StoreGateSvc.h"
#include "LArRawConditions/LArCalibParams.h"
#include "LArRecConditions/LArCalibLineMapping.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "CaloDetDescr/ICaloSuperCellIDTool.h"
class LArCalibDigitsAccumulator : public AthAlgorithm
{

public:
  LArCalibDigitsAccumulator (const std::string& name, ISvcLocator* pSvcLocator);
  StatusCode initialize();
  StatusCode execute(const EventContext& ctx);
  StatusCode finalize();

 /** 
   * @brief Class of intermediate accumulations.
   *
   * The first index of the vector of vector is the step of intermediate
   * accumulation (0 is for total accumulation, i>0 for intermediate 
   * accumulations), while the second index is for the sample.  */
  class LArAccumulated{
  public:
    unsigned int m_ntrigger;
    int m_nused;
    unsigned int m_onlineId;
    std::vector< uint64_t > m_sum;
    std::vector< uint64_t > m_sum2;
    LArAccumulated() : m_ntrigger(0), m_nused(0), m_onlineId(0) {};
  };

private:

  static std::string getPatternName(const std::string& gain, bool isPulsed, int delay, int dac);

  ToolHandle<ICaloSuperCellIDTool> m_sc2ccMappingTool;

  SG::ReadCondHandleKey<LArCalibLineMapping> m_calibMapKey{this,"CalibCablingKey","LArCalibLineMap","SG Key of LArCalibLineMapping object"};


  SG::ReadCondHandleKey<LArCalibLineMapping> m_calibMapSCKey{this,"CalibMapSCKey","LArCalibIdMapSC","SG Key of calib line mapping object"};


  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this, "OnOffMap", "LArOnOffIdMap", "SG key for mapping object"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKeySC{this,"ScCablingKey","LArOnOffIdMapSC","SG Key of SC LArOnOffIdMapping object"};

  const LArOnlineID_Base* m_onlineHelper;

 /** 
   * @brief Store delay.
   * */
  int m_delay;

 /** 
   * @brief LArAccumulatedCalibDigitContainer name.
   * */
  std::string m_calibAccuDigitContainerName;  // do we need this?

 /** 
   * @brief list of key for input digit container (=gain)
   * */
  std::vector<std::string> m_keylist;

 /** 
   * @brief Number of intermediate accumulations (JO property)
   * */
  unsigned int m_nStepTrigger;

 /** 
   * @brief Set delay scale.
   * */
  double m_delayScale;

 /** 
   * @brief Tells you whether you keep only pulsed cells or all cells
   * */
  bool m_keepPulsed;

 /** 
   * @brief Tells you whether you run on SuperCells or Cells
   * */
  bool m_isSC;

 /** 
   * @brief Tells you whether you keep only fully pulsed supercells or all supercells
   * */
  bool m_keepFullyPulsedSC;

 /** 
   * @brief Percentage of the used triggers that we will skip over at the end, in order ot ensure that the accumulation is done, even if there are lots of missing events from SC
   * */
  double m_DropPercentTrig;
  
  /** 
   * @brief Vector (index=hash ID) of accumulation quantities
   * */
  std::map<std::string, std::vector<LArAccumulated> > m_Accumulated_map;
  
  /** 
   * @brief Event counter
   * */
  unsigned int m_event_counter;
  unsigned int m_eventNb = 0U;

  /** 
   * @brief Samples to shift by, usually used in the case of SCs
   * */
  int m_sampleShift;
};

#endif
