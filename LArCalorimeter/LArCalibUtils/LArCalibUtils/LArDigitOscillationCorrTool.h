/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * AlgoTool to correct for the oscillating noise in the HEC at the 
 * Combined EMEC-HEC-FCAl test beam 2004
 *
 * @author T. Barillari
 * @version  \$Id: LArDigitOscillationCorrTool.h,v 1.3 2007-10-04 16:37:59 mdelmast Exp $
 */
#ifndef LARCALIBUTILS_LARDIGITOSCILLATIONCORRTOOL_H
#define LARCALIBUTILS_LARDIGITOSCILLATIONCORRTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/ToolHandle.h"

#include "LArElecCalib/ILArDigitOscillationCorrTool.h"
#include "LArElecCalib/ILArH6Oscillation.h"
#include "LArElecCalib/ILArRamp.h"

#include "LArCabling/LArOnOffIdMapping.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "LArIdentifier/LArOnlineID.h"

class LArDigitContainer;
class LArDigit;

class LArDigitOscillationCorrTool: public extends<AthAlgTool,ILArDigitOscillationCorrTool,IIncidentListener>
{
public:
  // constructor
  LArDigitOscillationCorrTool(const std::string& type,
			      const std::string& name,
			      const IInterface* parent);

  // destructor 
  ~LArDigitOscillationCorrTool() = default;
 
  // initialize and finalize methods
  virtual StatusCode initialize() override;
  virtual void handle(const Incident&) override;
 
  StatusCode calculateEventPhase(const LArDigitContainer &theDC) override;
  StatusCode correctLArDigits(LArDigitContainer &theDC) override;
 
private:
 
  StatusCode retrieveDB();

  int m_priority;
  double m_nSigma;
  double m_eventPhase;
  double m_omega;

  const LArEM_ID*    m_emId{};
  const LArFCAL_ID*  m_fcalId{};
  const LArHEC_ID*   m_hecId{};
  const LArOnlineID* m_lar_on_id{};
 
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this, "OnOffMap", "LArOnOffIdMap", "SG key for mapping object"};
};

#endif




