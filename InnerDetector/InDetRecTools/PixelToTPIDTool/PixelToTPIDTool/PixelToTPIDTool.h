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

#include "PixelToTPIDTool/PixelDEdxUtils.h"

//#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"    

#include "TrkToolInterfaces/IPixelToTPIDTool.h"
//#include "TrkEventPrimitives/ParticleHypothesis.h"

#include "PixelConditionsData/PixelChargeCalibCondData.h"
//#include "PixelConditionsData/PixeldEdxData.h"
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
//#include "Identifier/Identifier.h" // needed?
#include "InDetIdentifier/PixelID.h"

#include "xAODEventInfo/EventInfo.h"

class AtlasDetectorID;
class Identifier;
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
                         int& nUsedIBLOverflowHits) const override final;

    /*
      virtual std::vector<float> getLikelihoods(
        const EventContext& ctx,
        double dedx,
        double p,
        int nGoodPixels) const override final;

      virtual float getMass(const EventContext& ctx,
                            double dedx,
                            double p,
                            int nGoodPixels) const override final;
    */
    private:
      ServiceHandle<IIBLParameterSvc> m_IBLParameterSvc;
      const PixelID* m_pixelid;
      bool m_isMC = false;

      SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey
      {this, "EventInfoContName", "EventInfo", "Event info key"};

      SG::ReadCondHandleKey<PixelChargeCalibCondData> m_moduleDataKey
      {this, "PixelChargeCalibCondData", "PixelChargeCalibCondData", "ChargeCalibration data, for ToT overflow setting"};

      /// Equalize the cluster-level dE/dx measuremented before the taking the truncated mean.
      /// For ESD EDM, always have access to pixel clusters.
      /// For xAOD EDM, requires special datasets with pixel clusters.
      Gaudi::Property<bool> m_equalizeClusterMeasurements
      //{ this, "EqualizeClusterMeasurements", false, "Equalize cluster dE/dx before truncated mean"};
      { this, "EqualizeClusterMeasurements", true, "Equalize cluster dE/dx before truncated mean"}; // on-off test

      /// Apply tight cluster cleaning requirements (e.g. cluster size/shape cuts).
      Gaudi::Property<bool> m_tightClusterCleaning
      { this, "TightClusterCleaning", false, ""};
    
    
    //SG::ReadCondHandleKey<PixeldEdxData> m_dedxKey
    //{this, "PixeldEdxData", "PixeldEdxData", "Output key of pixel dEdx"};
  }; 
} // end of namespace

#endif 
