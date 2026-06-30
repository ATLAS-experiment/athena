/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//  Header file for class  StripClusterOnTrackTool
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Interface for StripClusterOnTrack production
///////////////////////////////////////////////////////////////////

#ifndef ITkStripClusterOnTrackTool_H
#define ITkStripClusterOnTrackTool_H

#include "GaudiKernel/ToolHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"

#include "InDetRIO_OnTrack/SCT_ClusterOnTrack.h"
#include "InDetRIO_OnTrack/SCTRIO_OnTrackErrorScaling.h"
#include "TrkParameters/TrackParameters.h"

namespace ITk {

/** @brief creates SCT_ClusterOnTrack objects allowing to
     calibrate cluster position and error using a given track hypothesis. 

    See doxygen of Trk::RIO_OnTrackCreator for details.
    Different strategies to calibrate the cluster error can be chosen
    by job Option. Also the handle to the general hit-error scaling
    is implemented.
*/
  class StripClusterOnTrackTool final: 
    public AthAlgTool,virtual public Trk::IRIO_OnTrackCreator
{
  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////

public:

  //! AlgTool constructor 
  StripClusterOnTrackTool(const std::string&,const std::string&,const IInterface*);
  virtual ~StripClusterOnTrackTool () = default;
  //! AlgTool initialisation
  virtual StatusCode initialize() override;

  
  /** @brief produces an SCT_ClusterOnTrack for ITk strip clusters
   * using the measured SCT_Cluster and the track prediction.
   * This method is a factory, so the client has to take care
   * of management/deletion of the SCT_ClusterOnTrack.
  */
  virtual InDet::SCT_ClusterOnTrack* correct(
      const Trk::PrepRawData&, const Trk::TrackParameters&, const EventContext&) const override;

 private:

  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////

  //! toolhandle for central error scaling
  SG::ReadCondHandleKey<RIO_OnTrackErrorScaling> m_stripErrorScalingKey
    {this,"ErrorScalingKey", "/Indet/TrkErrorScalingITkStrip", "Key for ITkStrip error scaling conditions data."};
  IntegerProperty m_option_errorStrategy{this, "ErrorStrategy", -1, "if ErrorStrategy < 0, keep previous errors else recompute"};
  IntegerProperty m_option_correctionStrategy{this, "CorrectionStrategy", -1, "if CorrectionStrategy >= 0, apply correction to position"};

};

} // end of namespace ITk

#endif // ITkStripClusterOnTrackTool_H
