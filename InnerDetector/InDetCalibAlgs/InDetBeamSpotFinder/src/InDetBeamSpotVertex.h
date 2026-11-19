/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDET_INDETBEAMSPOTVERTEX_H
#define INDET_INDETBEAMSPOTVERTEX_H

////////////////////////////////////////////////////////////
// Author jwalder@cern.ch
// Concrete implementation of the IInDetBeamSpotTool class
// for Vertex methods of Beamspot determination.
////////////////////////////////////////////////////////////

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetBeamSpotFinder/IInDetBeamSpotTool.h"
#include "TrkEventPrimitives/VertexType.h"
#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Matrix/SymMatrix.h"
#include "TMinuit.h"
#include "TMath.h"
#include "TTree.h"
#include <atomic>
#include <string>
#include <vector>
#include <map>

namespace InDet {
  /** A concrete implementation of IInDetBeamSpotTool, using primary vertex information to 
      determine the position of the beamspot using a chi2 and/or log-likelihood method.
      @copydoc InDet::IInDetBeamSpotTool
  */
  class InDetBeamSpotVertex : public extends<AthAlgTool, IInDetBeamSpotTool> {
    public:
    // Constructor
    using base_class::base_class;

    //Copy Constructor
    InDetBeamSpotVertex( const InDetBeamSpotVertex& rhs );
    

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    virtual FitStatus fit(std::vector< BeamSpot::VrtHolder >&) override;
    virtual FitID getFitID() const override { return (m_getLLres ? vertexLL : vertexChi2  );}
    virtual IInDetBeamSpotTool *Clone() override {return new InDetBeamSpotVertex(*this);}

    double  getX( double  z) const {
      return (m_getLLres ? m_pLL(1) + m_pLL(3)*z  : m_p(1) + m_p(2)*z);}
    double  getY(double z) const {
      return (m_getLLres ? m_pLL(2) + m_pLL(4)*z  : m_p(3) + m_p(4)*z);}

    double  getZ() const { return (m_getLLres ? m_pLL(9):m_zSolved);}
    double  getErrX( double  z) const {
      return (m_getLLres ? sqrt(m_VLL(1,1) + 2*z*m_VLL(3,1) + z*z*m_VLL(3,3)):
	      sqrt(m_V(1,1) + 2*z*m_V(2,1) + z*z*m_V(2,2)));
    }
    double  getErrY( double  z) const{
	return  (m_getLLres ? sqrt(m_VLL(2,2) + 2*z*m_VLL(4,2) + z*z*m_VLL(4,4)):
		 sqrt(m_V(3,3) + 2*z*m_V(4,3) + z*z*m_V(4,4)) );
    }
    double  getErrZ() const {
      return (m_getLLres ?  (m_fixInputK ? sqrt(m_VLL(8,8)) : sqrt(m_VLL(9,9))):m_zErrSolved);} // defined as centroid of beam
    
    double  getSigmaX(double) const { 
      return (m_getLLres ? m_pLL(5) : m_def_sx.value());}
    double  getSigmaY(double) const  { 
      return (m_getLLres ? m_pLL(6) : m_def_sy.value());}
    double  getSigmaZ() const { 
      return (m_getLLres ? m_pLL(10) : m_def_sz.value());}
    double  getErrSigmaX(double) const {
      return (m_getLLres ? sqrt(m_VLL(5,5)) :0.);}
    double  getErrSigmaY(double) const {
      return (m_getLLres ? sqrt(m_VLL(6,6)) :0.);}
    double  getErrSigmaZ() const  {
      return (m_getLLres ?  (m_fixInputK ? sqrt(m_VLL(9,9)) : sqrt(m_VLL(10,10))) :0.);}

    double  getTiltX() const {
      return (m_getLLres ? m_pLL(3) :m_p(2));}
    double  getTiltY() const {
      return (m_getLLres ? m_pLL(4) :m_p(4));}
    double  getErrTiltX() const { 
      return (m_getLLres ? sqrt(m_VLL(3,3)):sqrt( m_V(2,2)));}
    double  getErrTiltY() const { 
      return (m_getLLres ? sqrt(m_VLL(4,4)):sqrt( m_V(4,4)));}

    //specific for Vertex
    double getRhoXY() const { return (m_getLLres ? m_pLL(7) : 0.);}
    double getK() const     { return (m_getLLres ? m_pLL(8) : 0.);}
    double getErrRhoXY() const { return  (m_getLLres ? sqrt(m_VLL(7,7)) : 0.);}
    double getErrK() const { return  (m_getLLres ?  (m_fixInputK ? 0. : sqrt(m_VLL(8,8)) ) : 0.);}

    double getSigmaXY(double z) const {return getRhoXY()*getSigmaX(z)*getSigmaY(z);}
    
    double getErrSigmaXY(double z) const {
      // Error proporagtion multiplied thorughout by sigmaXY to remove denominator ->  safer against zeros

      double rhoXY = getRhoXY();
      double sigmaX = getSigmaX(z);
      double sigmaY = getSigmaY(z);

      double errRhoXY = getErrRhoXY();
      double errSigmaX = getErrSigmaX(z);
      double errSigmaY = getErrSigmaY(z);

      double errSigmaXY = sqrt((errRhoXY*errRhoXY) * (sigmaX*sigmaX) * (sigmaY*sigmaY) +
			       (rhoXY*rhoXY) * (errSigmaX*errSigmaX) * (sigmaY*sigmaY) +
			       (rhoXY*rhoXY) * (sigmaX*sigmaX) * (errSigmaY*errSigmaY));
      return errSigmaXY;
    }
    CLHEP::HepSymMatrix getCov(double z) const; //x(z),y(z),tiltx,tilty

    const CLHEP::HepVector & getLLpos() const { return m_pLL;}
    const CLHEP::HepSymMatrix & getLLcov() const { return m_VLL;}
    virtual std::map<std::string,double> getCovMap() const override;
    virtual std::map<std::string,double> getParamMap() const override;
    void clear();
    private:

    //updated per vertex
    CLHEP::HepVector m_x{4};
    CLHEP::HepSymMatrix m_cov{4,0};
    double m_z{0.}, m_zErr{0.};

    //solved matrices
    CLHEP::HepVector m_p{4};
    CLHEP::HepSymMatrix m_V{4,0};
    double m_zSolved{0.}, m_zErrSolved{0.};

    //for LL output
    const int m_NPARS{10};
    CLHEP::HepVector m_pLL{m_NPARS,0};
    CLHEP::HepSymMatrix m_VLL{m_NPARS,0};

    long m_vertexCount{0};

    //    std::string m_containerName;

    Gaudi::Property<bool> m_useLL{this, "UseLikelihood", true, "true if using the full log-likelihood method"};
    bool m_getLLres{false}; //use LL results, or fallback on Chi2
    std::vector< BeamSpot::VrtHolder > m_vertexData;

    
    ///////////////////////////////////
    // Initial fit values: set in Job options.
    // Note, it using Chi2, these may be overriding by the
    // results of the Chi2 fit.
    
    Gaudi::Property<double> m_init_x{this, "InitParX", 0.0};
    Gaudi::Property<double> m_init_y{this, "InitParY", 0.0};
    Gaudi::Property<double> m_init_z{this, "InitParZ", 0.0};
    Gaudi::Property<double> m_init_sx{this, "InitParSigmaX", 0.01};
    Gaudi::Property<double> m_init_sy{this, "InitParSigmaY", 0.01};
    Gaudi::Property<double> m_init_sz{this, "InitParSigmaZ", 56.0};
    Gaudi::Property<double> m_init_ax{this, "InitParAx", 0.0};
    Gaudi::Property<double> m_init_ay{this, "InitParAy", 0.0};
    Gaudi::Property<double> m_init_k{this, "InitParK", 1.0};
    Gaudi::Property<double> m_init_rhoxy{this, "InitParRhoXY", 0.0};

    Gaudi::Property<double> m_init_min_x{this, "InitParMinX", 0.0};
    Gaudi::Property<double> m_init_min_y{this, "InitParMinY", 0.0};
    Gaudi::Property<double> m_init_min_z{this, "InitParMinZ", 0.0};
    Gaudi::Property<double> m_init_min_sx{this, "InitParMinSigmaX", 0.0001};
    Gaudi::Property<double> m_init_min_sy{this, "InitParMinSigmaY", 0.0001};
    Gaudi::Property<double> m_init_min_sz{this, "InitParMinSigmaZ", 0.0};
    Gaudi::Property<double> m_init_min_ax{this, "InitParMinAx", 0.0};
    Gaudi::Property<double> m_init_min_ay{this, "InitParMinAy", 0.0};
    Gaudi::Property<double> m_init_min_k{this, "InitParMinK", 0};
    Gaudi::Property<double> m_init_min_rhoxy{this, "InitParMinRhoXY", -1.0};

    Gaudi::Property<double> m_init_max_x{this, "InitParMaxX", 0.0};
    Gaudi::Property<double> m_init_max_y{this, "InitParMaxY", 0.0};
    Gaudi::Property<double> m_init_max_z{this, "InitParMaxZ", 0.0};
    Gaudi::Property<double> m_init_max_sx{this, "InitParMaxSigmaX", 2.0};
    Gaudi::Property<double> m_init_max_sy{this, "InitParMaxSigmaY", 2.0};
    Gaudi::Property<double> m_init_max_sz{this, "InitParMaxSigmaZ", 3000.};
    Gaudi::Property<double> m_init_max_ax{this, "InitParMaxAx", 0.0};
    Gaudi::Property<double> m_init_max_ay{this, "InitParMaxAy", 0.0};
    Gaudi::Property<double> m_init_max_k{this, "InitParMaxK", 10.};
    Gaudi::Property<double> m_init_max_rhoxy{this, "InitParMaxRhoXY", 1.0};

    Gaudi::Property<int> m_minuit_maxIter{this, "MinuitMaxIterations", 2000, "max iterations for Minuit to perform"};

    //default values: controled by jobOptions
    Gaudi::Property<double> m_def_x0{this, "DefaultX0", 0.0};
    Gaudi::Property<double> m_def_y0{this, "DefaultY0", 0.0};
    Gaudi::Property<double> m_def_z{this, "DefaultZ", 0.0};
    Gaudi::Property<double> m_def_ax{this, "DefaultTiltX", 0.0};
    Gaudi::Property<double> m_def_ay{this, "DefaultTiltY", 0.0};
    //these are used when returning a chi2 value
    Gaudi::Property<double> m_def_sx{this, "DefaultSigmaX", 0.0};
    Gaudi::Property<double> m_def_sy{this, "DefaultSigmaY", 0.0};
    Gaudi::Property<double> m_def_sz{this, "DefaultSigmaZ", 0.0};

    // selections based on chi2-estimated position of beamspot
    Gaudi::Property<double> m_sigTr{this, "MaxSigmaTr", 20., "number of sigmas from total beam-spread width"};
    Gaudi::Property<double> m_maxVtxErTr{this, "MaxVtxErrTr", 100., "max uncertainty in vertex error in transverse direction (in mm)"};
    Gaudi::Property<double> m_widthFail{this, "OutlierWidthFail", 5.1e-3, "sigma value to 'not trust LL fit' (in mm)"};
    Gaudi::Property<double> m_rhoFail{this, "OutlierRhoFail", 0.8, "rho value to 'not trust LL fit'"};
    Gaudi::Property<int> m_singleIterationMax{this, "OutlierMaxRejection", 30, "max allowed vertices to reject in a single iteration (-1 => all)"};
    Gaudi::Property<bool> m_truncatedRMS{this, "TruncatedRMS", true};
    Gaudi::Property<float> m_fractionRMS{this, "RMSFraction", 0.95};
    Gaudi::Property<bool> m_setInitialRMS{this, "SetInitialRMS", false};

    FitStatus m_fitStatus{unsolved};
    bool solveChi2();
    bool solveLL(bool printOut = false);
    bool applyOutlierRemoval();
    Gaudi::Property<double> m_outlierChi2Tr{this, "OutlierChi2Tr", 20.};
    Gaudi::Property<int> m_maxOutlierLoops{this, "MaxOutlierLoops", 30, "maximum number of resursive loops in outlier removal to perform"};
    //LL setup
    int setInitialPars( TMinuit * minuit);
    int setParsFromChi2( TMinuit * minuit);

    bool setOutput( TMinuit * minuit);

    bool successfulFit( TMinuit * , 
			std::pair<int, std::string> & );
    void doFit2(TMinuit * , bool printOut = false); 

    Gaudi::Property<bool> m_doFitSanityCheck{this, "DoFitSanityCheck", true, "determine that fit is really converged"};
    Gaudi::Property<bool> m_doChi2OutlierRemoval{this, "DoChi2OnlyOutlierRemoval", false, "for chi2 only fit, run outlier removal as well"};
    Gaudi::Property<double> m_kMaxFail{this, "OutlierKFailMax", 9.9, "max sanity value for k"};
    Gaudi::Property<double> m_kMinFail{this, "OutlierKFailMin", 0, "min sanity value for k"};
    double m_minVtxProb{}; // probability cut on chi2/ndf
    
    Gaudi::Property<bool> m_fixInputK{this, "FixParK", false, "fix in LL fit the K factor to it's initial value"};
    Gaudi::Property<bool> m_useLLNorm{this, "UseLLNorm", false, "use the normalisation mode of the pdf"};
    Gaudi::Property<bool> m_fixWidth{this, "FixWidth", false};
    
    int m_nUsed{0};

    mutable std::atomic<int> m_rCount{0}; // counter used for recussive mode
  };
}
#endif
