/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCAFJOBS_LARSHAPEDUMPERTOOL_H
#define LARCAFJOBS_LARSHAPEDUMPERTOOL_H

#include "LArCafJobs/ILArShapeDumperTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "GaudiKernel/ToolHandle.h"

#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/LArHEC_ID.h"
#include "CaloIdentifier/LArFCAL_ID.h"

class ILArShape;
class LArOnlineID_Base;

class ATLAS_NOT_THREAD_SAFE LArShapeDumperTool : public AthAlgTool, public virtual ILArShapeDumperTool
{
 public:
 
  LArShapeDumperTool(const std::string& type, const std::string& name,
                     const IInterface* parent);
 
  virtual ~LArShapeDumperTool();

  StatusCode initialize() override;
  StatusCode finalize() override;
  
  virtual LArSamples::CellInfo* makeCellInfo(const HWIdentifier& channelID, const Identifier& id, const CaloDetDescrElement* caloDetElement = 0) const override;
  virtual LArSamples::ShapeInfo* retrieveShape(const HWIdentifier& channelID, CaloGain::CaloGain gain) const override;

  virtual bool doShape() const override { return m_doShape; }
  
 private:
  
  bool m_doShape, m_doAllShapes, m_isSC;
  std::string m_shapeKey;
   
  const LArOnlineID_Base* m_onlineHelper{nullptr};
  const LArEM_Base_ID* m_emId{nullptr};
  const LArHEC_Base_ID* m_hecId{nullptr};
  const LArFCAL_Base_ID* m_fcalId{nullptr};
};
#endif
