/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDET_INDETBEAMSPOTROOFIT_H
#define INDET_INDETBEAMSPOTROOFIT_H

////////////////////////////////////////////////////////////
// Author jwalder@cern.ch
// Concrete implementation of the IInDetBeamSpotTool class
// for RooFit method of Beamspot determination.
////////////////////////////////////////////////////////////

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetBeamSpotFinder/IInDetBeamSpotTool.h"
#include "TrkEventPrimitives/VertexType.h"
#include "CLHEP/Matrix/Vector.h"
#include "TMinuit.h"
#include "TMath.h"
#include "TTree.h"
#include <string>
#include <vector>
#include <RooAbsReal.h>
#include <RooRealVar.h>
#include "BeamSpotPdf.h"
#include "RooFitResult.h"

namespace InDet {
  /** A concrete implementation of IInDetBeamSpotTool, using primary vertex information to 
      determine the position of the beamspot using a chi2 and/or log-likelihood method.
      @copydoc InDet::IInDetBeamSpotTool
  */
  class InDetBeamSpotRooFit : public extends<AthAlgTool, IInDetBeamSpotTool> {
    public:
    // Constructor
    using base_class::base_class;

    //Copy Constructor
    InDetBeamSpotRooFit( const InDetBeamSpotRooFit& rhs );
    
 
    //These are the member functions to keep:
    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;
    virtual std::map<std::string,double> getCovMap() const override;
    virtual std::map<std::string,double> getParamMap() const override;
    virtual FitID getFitID() const override { return RooFitLL; }
    virtual FitStatus fit(std::vector< BeamSpot::VrtHolder >&) override;
    virtual IInDetBeamSpotTool *Clone() override { return new InDetBeamSpotRooFit(*this); }
    //std::string combineCuts( RooDataSet & );

    private:
    //m_cov is the covariance matrix from the fit
    TMatrixDSym m_cov{10};
    //Fit parameter results
    double m_ax{0};
    double m_ay{0};
    double m_k{0};
    double m_mx{0};
    double m_my{0};
    double m_mz{0};
    double m_rho{0};
    double m_sx{0};
    double m_sy{0};
    double m_sz{0};
    int m_nUsed{0};
    std::vector< BeamSpot::VrtHolder > m_vertexData;
    long m_vertexCount{0};
    FitStatus m_fitStatus{unsolved};
    std::string m_vtxCutString;
    Gaudi::Property<double> m_vtxResCut{this, "vtxResCut", 0.05};
    Gaudi::Property<double> m_kStart{this, "InitialKFactor", 1.0};
    Gaudi::Property<double> m_rmsCutNum{this, "RMSCut", 16};
    Gaudi::Property<bool> m_kConst{this, "ConstantKFactor", false};
  };
}



#endif
