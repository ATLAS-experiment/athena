/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRK_ANALYTICALDERIVCALCTOOL_H
#define TRK_ANALYTICALDERIVCALCTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "EventPrimitives/EventPrimitives.h"
#include "GeoPrimitives/GeoPrimitives.h"

#include "TrkAlignInterfaces/IDerivCalcTool.h"
#include "TrkAlignInterfaces/IAlignModuleTool.h"

#include "TrkAlignEvent/AlignResidualType.h"

#include <vector>


/**
   @file AnalyticalDerivCalcTool.h 
   @class AnalyticalDerivCalcTool

   @brief This class is the tool used with global chi2 aligment to calculate analytical 
   derivatives based on the track covariance matrix.

   @author Robert Harrington <roberth@bu.edu>
   @author Daniel Kollar <daniel.kollar@cern.ch>
   @author Shih-Chieh Hsu <Shih-Chieh.Hsu@cern.ch>
*/

class AtlasDetectorID;

namespace Trk
{
  class AlignModule;
  class AlignTrack;
  class MeasurementTypeID;

  class AnalyticalDerivCalcTool : virtual public IDerivCalcTool, public AthAlgTool
  {

  public:
    AnalyticalDerivCalcTool(const std::string & type, const std::string & name, const IInterface * parent);

    StatusCode initialize() override;
    StatusCode finalize() override;

    /** sets analytical partial derivatives of residuals w.r.t alignment parameters for TSOS on alignTrack. */ 
    bool setDerivatives(AlignTrack * alignTrack) override;

    /** not used yet */
    void showStatistics() override {}

    /** sets residual covariance matrix */
    bool setResidualCovMatrix(AlignTrack * alignTrack) const override;

  private:

    PublicToolHandle<IAlignModuleTool> m_alignModuleTool{
      this, "AlignModuleTool", "InDet::InDetAlignModuleTool/InDetAlignModuleTool"};

    const AtlasDetectorID * m_idHelper = nullptr;
    MeasurementTypeID * m_measTypeIdHelper = nullptr;

    bool getMeasErrorMatrix(const AlignTrack * alignTrack, Amg::MatrixX & V) const;

    bool getTrkParamCovMatrix(const AlignTrack * alignTrack, Amg::MatrixX & HCH) const;

    bool checkValidity(const Amg::MatrixX & R) const;

    std::vector<Amg::VectorX> getDerivatives(AlignTrack * alignTrack,  const AlignModule * module);

    void checkResidualType(const AlignTrack * alignTrack);

    std::vector<std::pair<const AlignModule *, std::vector<Amg::VectorX> > > m_derivatives;

    BooleanProperty m_useLocalSetting{this, "UseLocalSetting", false,
      "use local setup for the covariance matrix of the track"};

    // Use constant errors for each sub-detector such that it's equivalent
    // to minimize residual distance instead of minizing residual pull.
    // This is only applied if m_useLocalSetting==true
    BooleanProperty m_useIntrinsicPixelErrors{
      this, "UseIntrinsicPixelError", false, "use intrinsic errors for Pixel"};
    BooleanProperty m_useIntrinsicSCTErrors{
      this, "UseIntrinsicSCTError", false, "use intrinsic errors for SCT"};
    BooleanProperty m_useIntrinsicTRTErrors{
      this, "UseIntrinsicTRTError", false, "use intrinsic errors for TRT"};

    int m_residualType = Trk::AlignResidualType::HitOnly; //!< residual type to be used in the calculations
    bool m_residualTypeSet = false;   //!< do we have the residual type set?

    BooleanProperty m_storeDerivatives{this, "StoreDerivatives", false,
      "store derivatives dr/da on AlignTSOS to be filled into ntuple"};

  }; // end class

} // end namespace

#endif // TRK_ANALYTICALDERIVCALCTOOL_H
