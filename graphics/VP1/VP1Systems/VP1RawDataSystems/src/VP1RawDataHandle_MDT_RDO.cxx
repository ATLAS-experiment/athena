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
  const Identifier id = m_data->identify();
  QStringList ll;
  ll << " ===> MDT Digit data";
  ll << "   Identifier: " + QString::fromStdString(id.getString());
  ll << "   adc: " + QString::number(m_data->adc());
  ll << "   tdc: " + QString::number(m_data->tdc());
  ll << "   isMasked: " + QString(isMasked() ? "true" : "false");
  if (verbose) {
    const MdtIdHelper* idhelper = VP1DetInfo::mdtIDHelper();
    const bool valid = idhelper && idhelper->valid(id);
    if (valid) {
      ll << "   Station name: " + QString::fromStdString(idhelper->stationNameString(id));
      ll << "   Station eta index: " + QString::number(idhelper->stationEta(id));
      ll << "   Station phi index: " + QString::number(idhelper->stationPhi(id));
      ll << "   Multilayer: " + QString::number(idhelper->multilayer(id));
      ll << "   Tube layer: " + QString::number(idhelper->tubeLayer(id));
      ll << "   Tube: " + QString::number(idhelper->tube(id));
    } else {
      ll << "   Decoded identifier unavailable";
    }
    const MuonGM::MdtReadoutElement* elem = valid ? element() : nullptr;
    if (elem) {
      ll << "   Is Barrel: " + QString::number(elem->barrel());
      ll << "   Is Endcap: " + QString::number(elem->endcap());
      const Amg::Vector3D position = elem->tubePos(id);
      ll << QString("   Global position [mm] (x, y, z): (%1, %2, %3)")
                .arg(position.x() / Gaudi::Units::mm, 0, 'g', 10)
                .arg(position.y() / Gaudi::Units::mm, 0, 'g', 10)
                .arg(position.z() / Gaudi::Units::mm, 0, 'g', 10);
    } else {
      ll << "   Global position unavailable";
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

double VP1RawDataHandle_MDT_RDO::amplitude() const
{
  return m_data->adc();
}

double VP1RawDataHandle_MDT_RDO::time() const
{
  return m_data->tdc();
}

bool VP1RawDataHandle_MDT_RDO::isMasked() const
{
  return m_data->isMasked();
}
