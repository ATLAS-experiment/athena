/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRK_SHIFTINGDERIVCALCTOOL_H
#define TRK_SHIFTINGDERIVCALCTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"

#include "TrkEventPrimitives/ParamDefs.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"

#include "TrkFitterInterfaces/IGlobalTrackFitter.h"
#include "TrkAlignInterfaces/IDerivCalcTool.h"
#include "TrkAlignInterfaces/IAlignResidualCalculator.h"
#include "TrkAlignInterfaces/IAlignModuleTool.h"


#include <vector>
#include <map>

/** @file ShiftingDerivCalcTool.h
    @class ShiftingDerivCalcTool

    @brief Tool used to calculate total derivatives of residuals w.r.t. 
    alignment parameters by shifting chambers in various directions, 
    refitting tracks, and determining chi2 vs. chamber positions.  This 
    is used to calculate first derivatives.
    
    @author Robert Harrington <roberth@bu.edu>
    @date 1/5/08
*/

class TGraph;

namespace Trk {

  class TrackStateOnSurface;
  class Track;
  class AlignModule;
  class AlignTSOS;
  class AlignTrack;
  class AlignPar;

  class ShiftingDerivCalcTool : virtual public IDerivCalcTool, public AthAlgTool {

  public:
    ShiftingDerivCalcTool(const std::string& type, const std::string& name,
                           const IInterface* parent);
    virtual ~ShiftingDerivCalcTool();

    StatusCode initialize();
    StatusCode finalize();

    /** sets derivatives of residuals w.r.t. alignment parameters for hits on track.*/
    bool setDerivatives(AlignTrack* track);

    void showStatistics() {}

    /** sets residual covariance matrix */
    bool setResidualCovMatrix(AlignTrack* alignTrack) const;

  protected:

    // protected typedefs
    typedef std::vector<Amg::VectorX> HitDerivative;
    typedef std::map<const TrackStateOnSurface*,HitDerivative*> DerivativeMap;
    typedef DerivativeMap::value_type DerivativePair;
    
    // protected methods
    Amg::VectorX getDerivatives(AlignTrack* alignTrack, 
                                int ipar, AlignPar* alignPar,
                                Amg::VectorX& derivativeErr, bool& resetIPar,
                                double& actualSecondDerivative);

    void   setChi2VAlignParam(const AlignTrack* alignTrack, 
                              const AlignModule* module,
                              int nshifts=0);
    void   deleteChi2VAlignParam();

    double shiftSize(const AlignPar* alignPar) const;
    bool   setUnshiftedResiduals(AlignTrack* alignTrack);

  private:
    // private methods
    const Trk::Track* bestPerigeeTrack(const Track* track) const;
    bool scanShifts(const AlignTrack* alignTrack, 
    const std::vector<AlignModule*>& alignModules);

    bool getAllDerivatives(AlignTrack* alignTrack, const AlignModule* alignModule,
                           std::vector<Amg::VectorX>& deriv_vec,
                           std::vector<Amg::VectorX>& derivErr_vec,
                           std::vector<double>& actualsecderiv_vec,
                           bool& resetIPar);

    // private variables
    ToolHandle<IGlobalTrackFitter> m_trackFitterTool
      {this, "TrackFitterTool", "Trk::GlobalChi2Fitter/MCTBFitter"};
    ToolHandle<IGlobalTrackFitter> m_SLTrackFitterTool
      {this, "SLTrackFitterTool", "Trk::GlobalChi2Fitter/MCTBSLFitter"};
    ToolHandle<IGlobalTrackFitter> m_fitter;
    
    ToolHandle<IAlignResidualCalculator> m_residualCalculator
      {this, "ResidualCalculator", "Trk::AlignResidualCalculator/ResidualCalculator"};
    ToolHandle<IAlignModuleTool> m_alignModuleTool
      {this, "AlignModuleTool", "Trk::AlignModuleTool/AlignModuleTool"};

    Gaudi::Property<double> m_traSize{this, "TranslationSize", .1};
    Gaudi::Property<double> m_rotSize{this, "RotationSize", .1};

    Gaudi::Property<bool> m_runOutlierRemoval
      {this, "RunOutlierRemoval", false};
    ParticleHypothesis m_particleHypothesis = Trk::muon;

    Gaudi::Property<int> m_particleNumber{this, "ParticleNumber", 2};

    DerivativeMap m_derivative_map;
  
    Gaudi::Property<bool> m_doFits{this, "doResidualFits", true};
    Gaudi::Property<int> m_nFits{this, "NumberOfShifts", 5};
    Gaudi::Property<bool> m_doChi2VAlignParamMeasType
      {this, "doChi2VChamberShiftsMeasType", false};
    Gaudi::Property<bool> m_doResidualPlots{this, "doResidualPlots", false};
    int m_nIterations = 0;

    Amg::VectorX* m_unshiftedResiduals = nullptr;
    Amg::VectorX* m_unshiftedResErrors = nullptr;

    // stores double** for each module that track passes through
    std::vector<double**> m_chi2VAlignParamVec;  //!< track chi2[idof][ichambershift]
    std::vector<double**> m_chi2VAlignParamXVec; //!< chamber shift[idof][ichambershift]

    double** m_tmpChi2VAlignParam = nullptr;
    double** m_tmpChi2VAlignParamX = nullptr;
    double*** m_tmpChi2VAlignParamMeasType = nullptr;

    // stores double*** for each module that track passes through 
    // (one double** for each TrackState::MeasurementType)
    std::vector<double***> m_chi2VAlignParamVecMeasType;  //!< track chi2[idof][imeastype][ichambershift]

    double  m_unshiftedTrackChi2{};
    std::unique_ptr<double[]> m_unshiftedTrackChi2MeasType;

    //!< cut on value of track alignment parameter, determined from fit of chi2 vs. align parameters to a quadratic
    Gaudi::Property<double> m_trackAlignParamCut{this, "TrackAlignParamCut", 1e6};

    //!< fit track with AlignModules shifted up and down in each extreme, find the number of iterations fitter uses to converge.  Set this number for all subsequent track refits.
    Gaudi::Property<bool> m_setMinIterations{this, "SetMinIterations", false};

    //!< reject track if exceed maximum number of iterations
    Gaudi::Property<int> m_maxIter{this, "MaxIterations", 50};

    //!< set minimum number of iterations for first track fits
    Gaudi::Property<int> m_minIter{this, "MinIterations", 10};

    //!< flag to remove scattering before refitting track
    Gaudi::Property<bool> m_removeScatteringBeforeRefit
      {this, "RemoveScatteringBeforeRefit", false};

    int m_ntracksProcessed = 0;           //!< number tracks processed
    int m_ntracksPassInitScan = 0;        //!< number tracks pass initial scan
    int m_ntracksPassSetUnshiftedRes = 0; //!< number tracks pass setting unshifted residuals
    int m_ntracksPassDerivatives = 0;     //!< number tracks pass setting derivatives
    int m_ntracksPassGetDeriv = 0;        //!< number tracks pass getting derivatives
    int m_ntracksPassGetDerivSecPass = 0; //!< number tracks pass 2nd pass of getting derivatives
    int m_ntracksPassGetDerivLastPass = 0; //!< number tracks pass 2nd pass of getting derivatives
    int m_ntracksFailMaxIter = 0;
    int m_ntracksFailTrackRefit = 0;
    int m_ntracksFailAlignParamCut = 0;
    int m_ntracksFailFinalAttempt = 0;

    bool m_secPass{};

  }; // end class


} // end namespace

#endif // TRK_SHIFTINGDERIVCALCTOOL_H
