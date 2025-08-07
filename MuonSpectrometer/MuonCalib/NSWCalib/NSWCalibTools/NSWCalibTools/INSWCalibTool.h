/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
#ifndef INSWCalibTool_h
#define INSWCalibTool_h

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "xAODMuonPrepData/MMCluster.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"



#include <cmath>
#include <vector>
#include "float.h"



namespace NSWCalib { 

  struct CalibratedStrip {
    double charge{0};
    double time{0};
    double resTime{0};
    double distDrift{0};
    double resTransDistDrift{0};
    double resLongDistDrift{0};
    double dx{0};      
    Amg::Vector2D locPos{Amg::Vector2D::Zero()};
    Identifier identifier{0};
  };

  struct MicroMegaGas{
        /** //0.050 drift velocity in [mm/ns], driftGap=5 mm +0.128 mm (the amplification gap) */
      float driftVelocity{0.};
       /** // 0.350/10 diffusSigma=transverse diffusion (350 microm per 1cm ) for 93:7 @ 600 V/cm, according to garfield  */
      float longitudinalDiffusionSigma{0.};
      float transverseDiffusionSigma{0.};
      float interactionDensityMean{0.};
      float interactionDensitySigma{0.};
      using angleFunction = std::function<double(double)>;
      /// Dummy function to be used for the initialization
      static angleFunction dummy_func() {
        return [](float){   
           throw std::runtime_error("Please do not use the dummy lorentz function");
           return 0.;
        };
      }
      angleFunction lorentzAngleFunction{dummy_func()};    
  };

}

namespace Muon {

  class MM_RawData;
  class MMPrepData;
  class STGC_RawData;

  class INSWCalibTool : virtual public IAlgTool {
    
  public:  // static methods

    static const InterfaceID& interfaceID()  {
        static const InterfaceID IID_INSWCalibTool("Muon::INSWCalibTool",1,0);
        return IID_INSWCalibTool;
    }

  public:  // interface methods
 
    virtual StatusCode calibrateClus(const EventContext& ctx, const Muon::MMPrepData* prepRawData, const Amg::Vector3D& globalPos, std::vector<NSWCalib::CalibratedStrip>& calibClus) const = 0;
    
    virtual StatusCode calibrateClus(const EventContext& ctx, const ActsGeometryContext& gctx, const xAOD::MMCluster* prepRawData, const Amg::Vector3D& globalPos, std::vector<NSWCalib::CalibratedStrip>& calibClus) const = 0;

    virtual StatusCode calibrateStrip(const EventContext& ctx ,const Identifier& id,  const double time, const double charge, const double theta, const double lorentzAngle, NSWCalib::CalibratedStrip&calibStrip) const = 0;



    
    virtual StatusCode calibrateStrip(const EventContext& ctx, const Muon::MM_RawData* mmRawData, NSWCalib::CalibratedStrip& calibStrip) const = 0;
    virtual StatusCode calibrateStrip(const EventContext& ctx, const Muon::STGC_RawData* sTGCRawData, NSWCalib::CalibratedStrip& calibStrip) const = 0;

    virtual bool tdoToTime  (const EventContext& ctx, const bool inCounts, const int tdo, const Identifier& chnlId, float& time, const int relBCID) const = 0;
    virtual bool timeToTdo  (const EventContext& ctx, const float time, const Identifier& chnlId, int& tdo, int& relBCID) const = 0;
    virtual bool chargeToPdo(const EventContext& ctx, const float charge, const Identifier& chnlId, int& pdo) const = 0;
    virtual bool pdoToCharge(const EventContext& ctx, const bool inCounts, const int pdo, const Identifier& chnlId, float& charge) const = 0;

    virtual StatusCode distToTime(const EventContext& ctx, const Muon::MMPrepData* prepData, const Amg::Vector3D& globalPos,const std::vector<double>& driftDistances, std::vector<double>& driftTimes) const = 0;

    virtual NSWCalib::MicroMegaGas mmGasProperties() const = 0;
    virtual float mmPeakTime() const = 0;
    virtual float stgcPeakTime() const = 0;
  };
  
}


#endif
