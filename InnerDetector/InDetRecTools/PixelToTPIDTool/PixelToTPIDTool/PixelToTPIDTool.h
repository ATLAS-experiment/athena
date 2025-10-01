/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/** 
 * @file PixelToTPIDTool/PixelToTPIDTool.h
 * @author Thijs Cornelissen <thijs.cornelissen@cern.ch>
 * @date November, 2019
 * @brief Return pixel dEdx.
 */

#ifndef INDETPIXELTOTPIDTOOL_H
#define INDETPIXELTOTPIDTOOL_H

#include "TrackingAnalysisAlgorithms/PixelDEdxUtils.h"

#include "AthenaBaseComps/AthAlgTool.h"    

#include "TrkToolInterfaces/IPixelToTPIDTool.h"

#include "PixelConditionsData/PixelChargeCalibCondData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "PixelGeoModel/IIBLParameterSvc.h"
#include "TrkTrack/Track.h"
#include "TrkTrack/TrackStateOnSurface.h"
#include "TrkTrack/TrackInfo.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkSurfaces/Surface.h"
#include "InDetRIO_OnTrack/PixelClusterOnTrack.h"
#include "InDetIdentifier/PixelID.h"

class PixelID;
class IIBLParameterSvc;

namespace Trk {
  class Track;
}

namespace InDet {
  class PixelToTPIDTool : virtual public Trk::IPixelToTPIDTool, public AthAlgTool {
    public:
      PixelToTPIDTool(const std::string&,const std::string&,const IInterface*);

      virtual ~PixelToTPIDTool ();
      virtual StatusCode initialize() override;
      virtual StatusCode finalize  () override;

      virtual float dEdx(const EventContext& ctx,
                         const Trk::Track& track,
                         int& nUsedHits,
                         int& nIBLOverflowHits) const override final;

    private:
      ServiceHandle<IIBLParameterSvc> m_IBLParameterSvc;
      const PixelID* m_pixelid;
      bool m_isMC = false;

      SG::ReadCondHandleKey<PixelChargeCalibCondData> m_moduleDataKey
      {this, "PixelChargeCalibCondData", "PixelChargeCalibCondData", "ChargeCalibration data, for ToT overflow setting"};

      /// Equalize the cluster-level dE/dx measurements before taking the truncated mean.
      /// Not yet implemented.  See ATLIDTRKCP-579 for progress.
      /// Will eventually read run- and module-specific SFs from the conditions database.
      /// SFs account for radiation damage & varying operation conditions (bias voltages, thresholds, etc.).
      /// See PixelDEdxEqualizationAlg for applying these SFs to special (D)xAODs with pixel clusters & MSOSs. 
      Gaudi::Property<bool> m_equalizeClusterMeasurements
      { this, "EqualizeClusterMeasurements", false, "Equalize cluster dE/dx before truncated mean"};

      /// Apply tight cluster cleaning requirements (e.g. cluster size/shape cuts).
      Gaudi::Property<bool> m_tightClusterCleaning
      { this, "TightClusterCleaning", false, ""};    
  }; 
} // end of namespace

#endif 
