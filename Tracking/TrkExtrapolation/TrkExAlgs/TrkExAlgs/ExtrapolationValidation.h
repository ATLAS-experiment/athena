/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// ExtrapolationValidation.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEXALGS_EXTRAPOLATIONVALIDATION_H
#define TRKEXALGS_EXTRAPOLATIONVALIDATION_H

// Gaudi includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "EventPrimitives/EventPrimitives.h"
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
#include <string>

#include "TrkExInterfaces/IExtrapolator.h"

class TTree;

#ifndef TRKEXALGS_MAXPARAMETERS
#define TRKEXALGS_MAXPARAMETERS 10
#endif

namespace Trk 
{

  class Surface;
  class TrackingGeometry;
  class TrackingVolume;

  /** @class ExtrapolationValidation

     The ExtrapolationValidation Algorithm runs a number of n test extrapolations
     from randomly distributed Track Parameters to randombly distributed
     reference surfcas within
    
     - a) the Inner Detector if DetFlags.ID_On()
     - b) the Calorimeter if DetFlags.Calo_On()
     - c) the Muon System if DetFlags.Muon_On()

     and backwards.

     It is the TestAlgorithm for 'the' Extrapolator instance provided 
     to client algorithms.

      @author  Andreas Salzburger <Andreas.Salzburger@cern.ch>
  */  

  class ExtrapolationValidation : public AthAlgorithm
    {
    public:

       /** Standard Athena-Algorithm Constructor */
       ExtrapolationValidation(const std::string& name, ISvcLocator* pSvcLocator);
       /** Default Destructor */
       ~ExtrapolationValidation();

       /** standard Athena-Algorithm method */
       StatusCode          initialize();
       /** standard Athena-Algorithm method */
       StatusCode          execute();
       /** standard Athena-Algorithm method */
       StatusCode          finalize();

    private:
      /** private helper method to create a HepTransform */
      static Amg::Transform3D createTransform(double x, double y, double z, double phi=0., double theta=0., double alphaZ=0.);      

      /** the highest volume */
      const TrackingVolume* m_highestVolume = nullptr;

      /** The Extrapolator to be retrieved */
      ToolHandle<IExtrapolator> m_extrapolator
	{this, "Extrapolator", "Trk::Extrapolator/AtlasExtrapolator"};

      /** Random Number setup */
      Rndm::Numbers* m_gaussDist = nullptr;
      Rndm::Numbers* m_flatDist = nullptr;

      BooleanProperty m_materialCollectionValidation
	{this, "ValidateMaterialCollection", true};

      BooleanProperty m_direct{this, "ExtrapolateDirectly", false,
	"extrapolate directly"};

      TTree* m_validationTree = nullptr;            //!< Root Validation Tree

      StringProperty m_validationTreeName
	{this, "ValidationTreeName", "ExtrapolationValidation",
	 "validation tree name - to be acessed by this from root"};
      StringProperty m_validationTreeDescription
	{this, "ValidationTreeDescription",
	 "Output of the ExtrapolationValidation Algorithm",
	 "validation tree description - second argument in TTree"};
      StringProperty m_validationTreeFolder
	{this, "ValidationTreeFolder", "/val/ExtrapolationValidation",
	 "stream/folder to for the TTree to be written out"};

      double m_maximumR = 0.;                  //!< maximum R of the highest
      double m_maximumZ = 0.;                  //!< maximum halfZ of the highest tracking volume

      DoubleProperty m_sigmaLoc{this, "StartPerigeeSigmaLoc",
	10.*Gaudi::Units::micrometer, "local sigma of start value"};
      DoubleProperty m_sigmaR{this, "StartPerigeeSigmaR",
	17.*Gaudi::Units::micrometer, "r sigma of start value"};
      DoubleProperty m_sigmaZ{this, "StartPerigeeSigmaZ",
	50.*Gaudi::Units::millimeter, "Z sigma of start value"};
      DoubleProperty m_minEta{this, "StartPerigeeMinEta", -3.};
      DoubleProperty m_maxEta{this, "StartPerigeeMaxEta", 3.};
      DoubleProperty m_minP{this, "StartPerigeeMinP", 0.5*Gaudi::Units::GeV};
      DoubleProperty m_maxP{this, "StartPerigeeMaxP", 100.*Gaudi::Units::GeV};
 
      IntegerProperty m_particleType{this, "ParticleType", 2,
	"the particle type for the extrap."};

      int m_parameters = 0;          //!< maximum 3 : start - destination - backward
      float m_parameterLoc1[TRKEXALGS_MAXPARAMETERS]{};    //!< start local 1
      float m_parameterLoc2[TRKEXALGS_MAXPARAMETERS]{};    //!< start local 2
      float m_parameterPhi[TRKEXALGS_MAXPARAMETERS]{};     //!< start phi
      float m_parameterTheta[TRKEXALGS_MAXPARAMETERS]{};   //!< start theta
      float m_parameterEta[TRKEXALGS_MAXPARAMETERS]{};     //!< start eta
      float m_parameterQoverP[TRKEXALGS_MAXPARAMETERS]{};  //!< start qOverP

      float m_covarianceLoc1[TRKEXALGS_MAXPARAMETERS]{};    //!< start local 1
      float m_covarianceLoc2[TRKEXALGS_MAXPARAMETERS]{};    //!< start local 2
      float m_covariancePhi[TRKEXALGS_MAXPARAMETERS]{};     //!< start phi
      float m_covarianceTheta[TRKEXALGS_MAXPARAMETERS]{};   //!< start theta
      float m_covarianceQoverP[TRKEXALGS_MAXPARAMETERS]{};  //!< start qOverP
      float m_covarianceDeterminant[TRKEXALGS_MAXPARAMETERS]{};  //!< start qOverP

      int m_destinationSurfaceType = 0;  //!< destination surface type
      float m_startX = 0.;      //!< startX
      float m_startY = 0.;      //!< startX
      float m_startR = 0.;      //!< startX
      float m_startZ = 0.;      //!< startX
      float m_startP = 0.;      //!< startP
 
      float m_estimationX = 0.;      //!< estimation in X
      float m_estimationY = 0.;      //!< estimation in Y
      float m_estimationR = 0.;      //!< estimation in R
      float m_estimationZ = 0.;      //!< estimation in Z
 
      float m_destinationX = 0.;      //!< destination in X
      float m_destinationY = 0.;      //!< destination in Y
      float m_destinationR = 0.;      //!< destination in R
      float m_destinationZ = 0.;      //!< destination in Z
      
      // ---- output statistics
      unsigned int m_triesFront = 0;    //!< events front
      unsigned int m_breaksFront = 0;  //!< breaks front
      unsigned int m_triesBack = 0;    //!< events back
      unsigned int m_breaksBack = 0;   //!< breaks

      unsigned int m_collectedLayerFront = 0;    //!< collected material layers forward
      unsigned int m_collectedLayerBack = 0;    //!< collected material layers backwards
      
    }; 
} // end of namespace

#endif
