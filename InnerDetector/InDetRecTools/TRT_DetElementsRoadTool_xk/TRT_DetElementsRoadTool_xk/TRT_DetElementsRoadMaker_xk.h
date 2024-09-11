/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/


/////////////////////////////////////////////////////////////////////////////////
//  Header file for class TRT_DetElementsRoadMaker_xk
/////////////////////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
/////////////////////////////////////////////////////////////////////////////////
// Class for  InDetDD::TRT_BaseElement* collection production
/////////////////////////////////////////////////////////////////////////////////
// Version 1.0 3/10/2004 I.Gavrilenko
/////////////////////////////////////////////////////////////////////////////////

#ifndef TRT_DetElementsRoadMaker_xk_H
#define TRT_DetElementsRoadMaker_xk_H



#include "AthenaKernel/SlotSpecificObj.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/AlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/MsgStream.h"

#include "StoreGate/ReadCondHandleKey.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetRecToolInterfaces/ITRT_DetElementsRoadMaker.h"

#include "TrkGeometry/MagneticFieldProperties.h"
#include "TrkSurfaces/CylinderBounds.h"

#include "TRT_DetElementsRoadTool_xk/TRT_DetElementsLayer_xk.h"
#include "TRT_DetElementsRoadTool_xk/TRT_DetElementsRoadData_xk.h"

#include "MagFieldConditions/AtlasFieldCacheCondObj.h"
#include "MagFieldElements/AtlasFieldCache.h"


#include <atomic>
#include <mutex>
#include <list>
#include <vector>
#include <iosfwd>

class MsgStream;

namespace Trk {

  class IPropagator       ;
}

namespace InDet {

  /**
  @class TRT_DetElementsRoadMaker_xk
  InDet::TRT_DetElementsRoadMaker_xk is algorithm which produce list of
  InDetDD::TRT_BaseElement* sorted in propagation order.
  @author Igor.Gavrilenko@cern.ch
  */


class TRT_DetElementsRoadMaker_xk final:
  virtual public ITRT_DetElementsRoadMaker, public AthAlgTool
    {
      ///////////////////////////////////////////////////////////////////
      // Public methods:
      ///////////////////////////////////////////////////////////////////

    public:

      ///////////////////////////////////////////////////////////////////
      // Standard tool methods
      ///////////////////////////////////////////////////////////////////

     TRT_DetElementsRoadMaker_xk(const std::string&, const std::string&,
                                 const IInterface*);
     virtual ~TRT_DetElementsRoadMaker_xk();
     virtual StatusCode initialize() override;
     virtual StatusCode finalize() override;

     ///////////////////////////////////////////////////////////////////
     // Main methods for road builder
     ///////////////////////////////////////////////////////////////////

     virtual std::vector<const InDetDD::TRT_BaseElement*> detElementsRoad(
         const EventContext& ctx, MagField::AtlasFieldCache& fieldCache,
         const Trk::TrackParameters& Tp, Trk::PropDirection D,
         InDet::TRT_DetElementLink_xk::TRT_DetElemUsedMap& used) const override;

     ///////////////////////////////////////////////////////////////////
     // Print internal tool parameters and status
     ///////////////////////////////////////////////////////////////////

     virtual MsgStream& dump(MsgStream& out) const override;
     virtual std::ostream& dump(std::ostream& out) const override;


    private :

      SG::ReadCondHandleKey<TRT_DetElementsRoadData_xk> m_roadDataKey{this, "RoadDataKey",
        "TRT_DetElementsRoadData_xk", "Key of TRT_DetElementsRoadData_xk"};

      SG::ReadCondHandleKey<AtlasFieldCacheCondObj> m_fieldCacheCondObjInputKey {this,"AtlasFieldCacheCondObj", "fieldCondObj",
        "Name of the Magnetic Field conditions object key"}; // Necessary only for dumpConditions method

      ToolHandle<Trk::IPropagator>          m_proptool ;  // Propagator     tool

      float                                m_width{}    ;  // Width of the roadInnerDetector/InDetRecTools/
      double                               m_step{}     ;  // Max step allowed

      std::string                          m_fieldmode;  // Mode of magnetic field
      Trk::MagneticFieldMode               m_fieldModeEnum{Trk::FullField};

      ///////////////////////////////////////////////////////////////////
      // Methods
      ///////////////////////////////////////////////////////////////////

      void detElementsRoadATL(
          std::deque<Amg::Vector3D>&,
          std::vector<const InDetDD::TRT_BaseElement*>&,
          InDet::TRT_DetElementLink_xk::TRT_DetElemUsedMap& used,
          const EventContext& ctx) const;
      void detElementsRoadCTB(
          std::deque<Amg::Vector3D>&,
          std::vector<const InDetDD::TRT_BaseElement*>&,
          InDet::TRT_DetElementLink_xk::TRT_DetElemUsedMap& used,
          const EventContext& ctx) const;
      static double stepToDetElement(const InDetDD::TRT_BaseElement*&,
                                     Amg::Vector3D&, Amg::Vector3D&);

      Trk::CylinderBounds getBound(MagField::AtlasFieldCache& fieldCache, const Trk::TrackParameters&, const EventContext& ctx) const;

      MsgStream&    dumpConditions(MsgStream   & out) const;

      static MsgStream&    dumpEvent     (MsgStream   & out, int size_road) ;

      inline
      const TRT_DetElementsLayerVectors_xk *getLayers(const EventContext& ctx) const {
         SG::ReadCondHandle<TRT_DetElementsRoadData_xk> roadData(m_roadDataKey, ctx);
         if (not roadData.isValid()) {
            ATH_MSG_FATAL("Failed to get " << m_roadDataKey.key());
         }
         return roadData->getLayers();
      }

      inline
        const Trk::CylinderBounds get_bounds(const EventContext& ctx) const{
        SG::ReadCondHandle<TRT_DetElementsRoadData_xk> roadData(m_roadDataKey, ctx);
        if (not roadData.isValid()) {
          ATH_MSG_FATAL("Failed to get " << m_roadDataKey.key());
        }
        return roadData->getBounds();
      }

      inline
        double getTRTMinR(const EventContext& ctx) const{
        SG::ReadCondHandle<TRT_DetElementsRoadData_xk> roadData(m_roadDataKey, ctx);
        if (not roadData.isValid()) {
          ATH_MSG_FATAL("Failed to get " << m_roadDataKey.key());
        }
        double rmintrt = roadData->getTRTMinR();
        return rmintrt;
      }

  };

} // end of name space

#endif // TRT_DetElementsRoadMaker_xk_H
