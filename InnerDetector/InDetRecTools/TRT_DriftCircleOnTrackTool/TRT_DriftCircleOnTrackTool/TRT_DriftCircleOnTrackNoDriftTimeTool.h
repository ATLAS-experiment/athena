/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//  Header file for class  TRT_DriftCircleOnTrackNoDriftTimeTool
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Interface for TRT_DriftCircleOnTrack production
///////////////////////////////////////////////////////////////////
// Version 1.0 18/02/2003 I.Gavrilenko
///////////////////////////////////////////////////////////////////

#ifndef TRT_DriftCircleOnTrackNoDriftTimeTool_H
#define TRT_DriftCircleOnTrackNoDriftTimeTool_H

#include "GaudiKernel/ThreadLocalContext.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"
#include "InDetRIO_OnTrack/TRT_DriftCircleOnTrack.h"

namespace InDet {

  /** @class TRT_DriftCircleOnTrackNoDriftTimeTool
      This tool creates TRT_DriftCircleOnTrack objects using a given
      track hypothesis. See doxygen to Trk::RIO_OnTrackCreator for details.
  */

  class TRT_DriftCircleOnTrackNoDriftTimeTool final : 
    virtual public Trk::IRIO_OnTrackCreator,public AthAlgTool
{
  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////

public:

  TRT_DriftCircleOnTrackNoDriftTimeTool(const std::string&,const std::string&,const IInterface*);
  virtual ~TRT_DriftCircleOnTrackNoDriftTimeTool ();
  virtual StatusCode initialize() override;
  virtual StatusCode finalize  () override;
  virtual InDet::TRT_DriftCircleOnTrack* correct
    (const Trk::PrepRawData&,const Trk::TrackParameters&, const EventContext& /*ctx*/) const override; 

  ///////////////////////////////////////////////////////////////////
  // Private methods:
  ///////////////////////////////////////////////////////////////////
  
 private:

  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////
  
};

} // end of namespace InDet

#endif // TRT_DriftCircleOnTrackNoDriftTimeTool_H
