/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//  Header file for class  TRT_DriftCircleOnTrackRecalibrateTool
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////


#ifndef TRT_DriftCircleOnTrackRecalibrateTool_H
#define TRT_DriftCircleOnTrackRecalibrateTool_H

#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"
#include "TRT_DriftFunctionTool/ITRT_DriftFunctionTool.h"
#include "InDetRIO_OnTrack/TRTRIO_OnTrackErrorScaling.h"

#include "LumiBlockData/LuminosityCondData.h"
#include "StoreGate/ReadCondHandleKey.h"


namespace InDet {

  /** @class TRT_DriftCircleOnTrackRecalibrateTool
      This tool creates TRT_DriftCircleOnTrack objects using a given
      track hypothesis. See doxygen to Trk::RIO_OnTrackCreator for details.
  */

  class TRT_DriftCircleOnTrackRecalibrateTool final: 
    virtual public Trk::IRIO_OnTrackCreator, public AthAlgTool
{
  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////

public:

  TRT_DriftCircleOnTrackRecalibrateTool
    (const std::string&,const std::string&,const IInterface*);
  virtual ~TRT_DriftCircleOnTrackRecalibrateTool ();
  virtual StatusCode initialize() override;
  virtual StatusCode finalize  () override;
  virtual Trk::RIO_OnTrack* correct
    (const Trk::PrepRawData&,const Trk::TrackParameters&,const EventContext& ctx = Gaudi::Hive::currentContext()) const override; 

  ///////////////////////////////////////////////////////////////////
  // Private methods:
  ///////////////////////////////////////////////////////////////////
  
 private:

  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////

  ToolHandle<Trk::IRIO_OnTrackCreator> m_riontrackTube{this, "RIOonTrackToolTube",
    "InDet::TRT_DriftCircleOnTrackNoDriftTimeTool/TRT_DriftCircleOnTrackNoDriftTimeTool"};
  ToolHandle<ITRT_DriftFunctionTool> m_drifttool{this, "DriftFunctionTool",
    "TRT_DriftFunctionTool"};

  SG::ReadCondHandleKey<LuminosityCondData>     m_lumiDataKey
      {this, "LumiDataKey", "", "SG key for luminosity data"};

  //  SG::ReadCondHandleKey<TRTRIO_OnTrackErrorScaling> m_trtErrorScalingKey
  SG::ReadCondHandleKey<RIO_OnTrackErrorScaling> m_trtErrorScalingKey
      {this,"TRTErrorScalingKey", "/Indet/TrkErrorScalingTRT", "Key for TRT error scaling conditions data."};

  BooleanProperty m_useToTCorrection{this, "useDriftTimeToTCorrection", false,
    "Shall the Time over Threshold correction be used?"};
  DoubleProperty m_scalefactor{this, "ScaleHitUncertainty", 2.,
    "scale factor for hit uncertainty"};

};

} // end of namespace InDet

#endif // TRT_DriftCircleOnTrackRecalibrateTool_H
