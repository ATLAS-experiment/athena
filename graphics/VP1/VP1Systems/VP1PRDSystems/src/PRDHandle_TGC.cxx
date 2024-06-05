/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "VP1PRDSystems/PRDHandle_TGC.h"
#include "VP1Utils/HitsSoNodeManager.h"
#include "VP1Utils/VP1DetInfo.h"
#include "VP1Base/VP1Msg.h"
#include "VP1Utils/VP1LinAlgUtils.h"
#include "VP1HEPVis/nodes/SoTransparency.h"

#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/nodes/SoTranslation.h>
#include <Inventor/nodes/SoTransform.h>

#include "MuonReadoutGeometry/MuonStation.h"

#include "GeoPrimitives/GeoPrimitives.h"
#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "GeoPrimitives/EulerAnglesHelpers.h"


//____________________________________________________________________
PRDHandle_TGC::PRDHandle_TGC(PRDCollHandle_TGC*collhandle, const Muon::TgcPrepData*tgc)
  : PRDHandleBase(static_cast<PRDCollHandleBase*>(collhandle)), m_tgc(tgc)
{
  SoTransparency::initClass();
}

//____________________________________________________________________
SoTransform * PRDHandle_TGC::createTransform() const
{
  const TgcIdHelper * idhelper = VP1DetInfo::tgcIDHelper();
  const Muon::TgcPrepData * prd = tgc();
  if (!prd || !idhelper) {
    return new SoTransform;
  }

  const Trk::Surface& theSurface = prd->detectorElement()->surface(prd->identify());
  SoTransform * theHitTransform =0;
   if (idhelper->isStrip( prd->identify() )){
    
    int stripNo = idhelper->channel(prd->identify()); 
    int gasGap  = idhelper->gasGap(prd->identify()); 
    
    const MuonGM::TgcReadoutElement* detEl = prd->detectorElement(); 
    
    // calculate two points along the tgc phi strip in the local tgc reference frame 
    const Amg::Vector3D lposTGC = detEl->stripCenterLocX(gasGap, stripNo, 0.) * Amg::Vector3D::UnitX();
    const double shift = detEl->getStationEta() > 0 ? 1.*Gaudi::Units::cm : - 1.*Gaudi::Units::cm;
    const  Amg::Vector3D lposTgcShifted = detEl->stripCenterLocX(gasGap, stripNo, shift) * Amg::Vector3D::UnitX()+
                                           shift * Amg::Vector3D::UnitY();
    const double angle = (lposTGC - lposTgcShifted).phi();
     // for phi strips, use sinstereo to get correct orientation
     Amg::RotationMatrix3D localRot{Amg::RotationMatrix3D::Identity()};


     // std::ostream os;
     // std::cout<<localRot.print(os)<<std::endl;
     //double angle  = M_PI/2.0 - asin(prd->detectorElement()->sinStereo(prd->identify())) ;
     // double angle  = asin(prd->detectorElement()->sinStereo(prd->identify())) ;
     // trying by trial and error to set right component of matrix!
     // setting theta=PI/4 rotated strips 45deg around z axis, but also a bit around x and y too! So clearly not working as expected.
      
    // localRot.setPhi (M_PI/4); 
    
     if (0 < detEl->getStationEta()) { 
       Amg::setPhi (localRot, angle);
     } else {
       Amg::setPhi (localRot, -angle);
     }
     // std::cout<<localRot.print(os)<<std::endl;
       // localRot.setPhi (angle); 
     
     // VP1Msg::message(QString::number(prd->identify().get_compact())+": angle="+QString::number(angle));
     localRot*=theSurface.transform().rotation ();

     Amg::Transform3D trans (localRot, theSurface.transform().translation());
     // Trk::GlobalDirection difPosGlobal = gpos_shift-gpos;
     // HepTransform3D toLocal = trans.inverse();
     // Trk::GlobalDirection difPosLocal = toLocal*difPosGlobal;
     // Trk::GlobalDirection difPosLocal2 = trans*difPosGlobal;
     
     // std::cout << " global strip direction " << difPosGlobal << " local " << difPosLocal << " local2 " << difPosLocal2 << std::endl;
     theHitTransform = VP1LinAlgUtils::toSoTransform(trans);
   } else {
     // eta strips just use surface orientiation
     theHitTransform = VP1LinAlgUtils::toSoTransform(theSurface.transform());
   }

   Amg::Vector3D theHitGPos= theSurface.localToGlobal(prd->localPosition());
   theHitTransform->translation.setValue((theHitGPos)[0], (theHitGPos)[1], (theHitGPos)[2]);
   return theHitTransform;
}

//____________________________________________________________________
void PRDHandle_TGC::buildShapes(SoNode*&shape_simple, SoNode*&shape_detailed)
{
  const TgcIdHelper * idhelper = VP1DetInfo::tgcIDHelper();
  if (!idhelper)
    return;

  Identifier id = m_tgc->identify();

  int plane = idhelper->gasGap( id );
  int strip = idhelper->channel( id );
  int isStrip = idhelper->isStrip( id );

  double striplength =0.0, stripWidth = 0.0;

  if (isStrip){
    striplength = m_tgc->detectorElement()->stripLength();
    stripWidth = m_tgc->detectorElement()->stripWidth(plane, strip);
  } else {    
    striplength = m_tgc->detectorElement()->gangShortWidth(plane, strip);
    stripWidth = m_tgc->detectorElement()->gangRadialLength(plane, strip);
  }

  if (static_cast<PRDCollHandle_TGC*>(collHandle())->project())
    striplength += 300.0;//Fixme: Rough extension for now

  shape_simple = common()->nodeManager()->getShapeNode_Strip(striplength);

  const bool settingsShowRDOs = true; //FIXME: get from controller
  SoSeparator * errDetailed  = new SoSeparator;
  const std::vector<Identifier> rdolist = m_tgc->rdoList();
  if (rdolist.size() == 1 || !settingsShowRDOs)
  {
    errDetailed->addChild(common()->nodeManager()->getShapeNode_Strip(striplength,
      std::max(10.0,stripWidth), //strip width -> fixme: std::max hack for now since stripWidth returns 0.0
      3*0.8)); //strip thickness - hardcoded to be ~= the gas gap
    } else
    {
      VP1Msg::message("Warning: TGC has additional elements in rdoList: THIS HAS NEVER BEEN TESTED");
      SoSeparator * rdos = new SoSeparator;

      const Amg::Vector3D& globalposHIT = m_tgc->detectorElement()->channelPos( id );
    // get local position on wire plane, here we have to use a tolarance as the wire plane is located 2.5 CLHEP::mm
    // from the strip plane
      double tolerance = 3.;
      std::optional<Amg::Vector2D>localposHIT = m_tgc->detectorElement()->surface( id ).Trk::Surface::globalToLocal(globalposHIT,tolerance);
      if( !localposHIT )
      {
        localposHIT.emplace();
        localposHIT->setZero();
        VP1Msg::message("Warning: Local wire position is NULL");
      }

      rdos->addChild(common()->nodeManager()->getShapeNode_Strip(striplength,
        std::max(10.0,stripWidth), //strip width -> fixme: std::max hack for now since stripWidth returns 0.0
        3*0.8)); //strip thickness - hardcoded to be ~= the gas gap

      SoTransparency * transparent = new SoTransparency;
      transparent->transparency.setValue(0.5);
      rdos->addChild( transparent );
      Amg::Vector2D localposOLD = *localposHIT;
      for (const Identifier& rdo_id : rdolist)
      {
        if (rdo_id == id )
          continue;
        const Amg::Vector3D& globalposRDO = m_tgc->detectorElement()->channelPos( rdo_id );
        std::optional<Amg::Vector2D> localposRDO = m_tgc->detectorElement()->surface( rdo_id ).Trk::Surface::globalToLocal(globalposRDO,tolerance);
        if (!localposRDO)
        {
          VP1Msg::message("Warning: Local wire position is NULL");
          continue;
        }

        SoTranslation * localtrans = new SoTranslation;
        localtrans->translation.setValue((*localposRDO)[Trk::locX]-localposOLD[Trk::locX],(*localposRDO)[Trk::locY]-localposOLD[Trk::locY],0);
        rdos->addChild(localtrans);

        rdos->addChild(common()->nodeManager()->getShapeNode_Strip(striplength,
          std::max(10.0,stripWidth), //strip width -> fixme: std::max hack for now since stripWidth returns 0.0
          3*0.8)); //strip thickness - hardcoded to be ~= the gas gap

        localposOLD = *localposRDO;
      }
      errDetailed->addChild(rdos);
    }
    shape_detailed = errDetailed;
  }

//____________________________________________________________________
  int PRDHandle_TGC::regionIndex()
  {
  //Unique for each station.
    const MuonGM::MuonStation* station = m_tgc->detectorElement()->parentMuonStation();
    return station->getPhiIndex()-99999*station->getEtaIndex();//hopefully unique.
  }
