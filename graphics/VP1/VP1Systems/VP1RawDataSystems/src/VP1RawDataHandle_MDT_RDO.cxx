/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////
//                                                            //
//  Implementation of class VP1RawDataHandle_MDT_RDO          //
//                                                            //
//  Author: Riccardo Maria BIANCHI <riccardo.maria.bianchi@cern.ch> 
//  Initial version: January 2024    
//                                                            //
////////////////////////////////////////////////////////////////

#include "VP1RawDataSystems/VP1RawDataHandle_MDT_RDO.h"
#include "VP1RawDataSystems/VP1RawDataColl_MDT_RDO.h"

#include "VP1RawDataSystems/VP1RawDataCommonData.h"
#include "VP1RawDataSystems/VP1RawDataSysController.h"

#include "VP1Utils/HitsSoNodeManager.h"
#include "VP1Utils/VP1DetInfo.h"
#include "VP1Utils/VP1LinAlgUtils.h"

#include "MuonReadoutGeometry/MuonDetectorManager.h"

#include <Inventor/nodes/SoTransform.h>
#include <Inventor/nodes/SoGroup.h>
#include <Inventor/nodes/SoMaterial.h>

#include "GaudiKernel/SystemOfUnits.h"

//____________________________________________________________________
VP1RawDataHandle_MDT_RDO::VP1RawDataHandle_MDT_RDO(VP1RawDataCollBase* coll,const MdtDigit*data)
  : VP1RawDataHandleBase(coll), m_data(data)
{
}

//____________________________________________________________________
VP1RawDataHandle_MDT_RDO::~VP1RawDataHandle_MDT_RDO()
{
}

//____________________________________________________________________
QStringList VP1RawDataHandle_MDT_RDO::clicked(bool verbose) const
{
  static const MdtIdHelper * idhelper = VP1DetInfo::mdtIDHelper();
  const MuonGM::MdtReadoutElement * elem = element();

  QStringList ll;
  ll << " ===> MDT Digit data";
  // ll << "   is a valid digit: "+ m_data->is_valid(idhelper);
  ll << "   adc: " + QString::number(m_data->adc());
  ll << "   tdc: " + QString::number(m_data->tdc());
  if (verbose)
  {
    if (idhelper)
    {
      // Fixme: We should have common identify -> QStringList method in VP1DetInfo!!
      Identifier id(m_data->identify());

      ll << "Is Barrel: " << QString::number(elem->barrel());
      /// Returns whether the chamber is in the endcap
      ll << "Is Endcap: " << QString::number(elem->endcap());
      // ll << "Tube position :" << elem->tubePos(id).position[Amg::x];
      std::cout << "tubePos x: " << elem->tubePos(id)[Amg::x] << std::endl;
    }
  }
  return ll;
}

//____________________________________________________________________
SoNode * VP1RawDataHandle_MDT_RDO::buildShape()
{
  const MuonGM::MdtReadoutElement * elem = element();
  Identifier id(m_data->identify());
  double tubeLength = elem->tubeLength(id);
  // double strawlength = elem ? elem->strawLength() : 200.0;

  SoNode * node = common()->nodeManager()->getShapeNode_DriftTube(tubeLength/2, elem->innerTubeRadius());
  return node;
}

//____________________________________________________________________
SoTransform * VP1RawDataHandle_MDT_RDO::buildTransform()
{
  const MuonGM::MdtReadoutElement * elem = element();
  if (!elem)
    return new SoTransform;//fixme
  Identifier id(m_data->identify());
  return VP1LinAlgUtils::toSoTransform(elem->localToGlobalTransf(id));
}

//____________________________________________________________________
const MuonGM::MdtReadoutElement * VP1RawDataHandle_MDT_RDO::element() const
{
  
  const MuonGM::MuonDetectorManager * detmgr = VP1DetInfo::muonDetMgr();

  if (!detmgr)
    return 0;
  return detmgr->getMdtReadoutElement(m_data->identify());
}
