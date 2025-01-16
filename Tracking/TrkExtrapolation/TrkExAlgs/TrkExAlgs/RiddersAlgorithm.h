/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// RiddersAlgorithm.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKRIDDERSALGORITHM_H
#define TRKRIDDERSALGORITHM_H

// Gaudi includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include <string>

#include "TrkExInterfaces/IPropagator.h"

class AtlasDetectorID;
class Identifier;
class TTree;

#ifndef RIDDLERSSTEPS
#define RIDDLERSSTEPS 10
#endif

namespace Trk 
{

  class MagneticFieldProperties;

  /** @class RiddersAlgorithm

      This is a test algorithm for evaluating the jacobians for
      the transport of Track Parameters

      @author  Andreas Salzburger <Andreas.Salzburger@cern.ch>
  */  

  class RiddersAlgorithm : public AthAlgorithm
    {
    public:

       /** Standard Athena-Algorithm Constructor */
       RiddersAlgorithm(const std::string& name, ISvcLocator* pSvcLocator);
       /** Default Destructor */
       ~RiddersAlgorithm();

       /** standard Athena-Algorithm method */
       StatusCode          initialize();
       /** standard Athena-Algorithm method */
       StatusCode          execute();
       /** standard Athena-Algorithm method */
       StatusCode          finalize();

    private:
      /** private helper method to create a HepTransform */
      static Amg::Transform3D createTransform(double x, double y, double z, double phi=0., double theta=0., double alphaZ=0.);      
     
      /** Langrange-parabolic interpolation */
      static double parabolicInterpolation(double y0, double y1, double y2,
                                           double x0, double x1, double x2);

      /** member variables for algorithm properties: */
      PublicToolHandle<IPropagator> m_propagator
	{this, "Propagator", "Trk::RungeKuttaPropagator/RungeKuttaPropagator"};
      BooleanProperty m_useCustomField{this, "UseCustomMagneticField", true};
      BooleanProperty m_useAlignedSurfaces{this, "UseAlignedSurfaces", true};
      DoubleProperty m_fieldValue
	{this, "CustomFieldValue", 2.*Gaudi::Units::tesla};
      MagneticFieldProperties* m_magFieldProperties = nullptr;

      /** The smearing */
      DoubleProperty m_sigmaLoc
	{this, "StartPerigeeSigmaLoc", 100.*Gaudi::Units::micrometer};
      DoubleProperty m_sigmaR
	{this, "StartPerigeeSigmaR", 0.};
      DoubleProperty m_minPhi{this, "StartPerigeeMinPhi", -M_PI};
      DoubleProperty m_maxPhi{this, "StartPerigeeMaxPhi", M_PI};
      DoubleProperty m_minEta{this, "StartPerigeeMinEta", -2.5};
      DoubleProperty m_maxEta{this, "StartPerigeeMaxEta", 2.5};
      DoubleProperty m_minP
	{this, "StartPerigeeMinP", 0.5*Gaudi::Units::GeV};
      DoubleProperty m_maxP
	{this, "StartPerigeeMaxP", 50000*Gaudi::Units::GeV};

      /** To create the first extimations */
      DoubleProperty m_minimumR{this, "TargetSurfaceMinR", 10.};
      DoubleProperty m_maximumR{this, "TargetSurfaceMaxR", 1000.};

      /** variations */
      DoubleArrayProperty m_localVariations{this, "LocalVariations", {}};
      DoubleArrayProperty m_angularVariations{this, "AngularVariations", {}};
      DoubleArrayProperty m_qOpVariations{this, "QopVariations", {}};

      TTree* m_validationTree = nullptr;            //!< Root Validation Tree

      StringProperty m_validationTreeName
	{this, "ValidationTreeName", "RiddersTree",
	 "validation tree name - to be acessed by this from root"};
      StringProperty m_validationTreeDescription
	{this, "ValidationTreeDescription", "Output of the RiddersAlgorithm",
	 "validation tree description - second argument in TTree"};
      StringProperty m_validationTreeFolder
	{this, "ValidationTreeFolder", "/val/RiddersAlgorithm",
	 "stream/folder to for the TTree to be written out"};

      int m_steps = 0;
      float m_loc1loc1[RIDDLERSSTEPS]{};
      float m_loc1loc2[RIDDLERSSTEPS]{};
      float m_loc1phi[RIDDLERSSTEPS]{};
      float m_loc1theta[RIDDLERSSTEPS]{};
      float m_loc1qop[RIDDLERSSTEPS]{};
      float m_loc1steps[RIDDLERSSTEPS]{};

      float m_loc2loc1[RIDDLERSSTEPS]{};
      float m_loc2loc2[RIDDLERSSTEPS]{};
      float m_loc2phi[RIDDLERSSTEPS]{};
      float m_loc2theta[RIDDLERSSTEPS]{};
      float m_loc2qop[RIDDLERSSTEPS]{};
      float m_loc2steps[RIDDLERSSTEPS]{};

      float m_philoc1[RIDDLERSSTEPS]{};
      float m_philoc2[RIDDLERSSTEPS]{};
      float m_phiphi[RIDDLERSSTEPS]{};
      float m_phitheta[RIDDLERSSTEPS]{};
      float m_phiqop[RIDDLERSSTEPS]{};
      float m_phisteps[RIDDLERSSTEPS]{};

      float m_thetaloc1[RIDDLERSSTEPS]{};
      float m_thetaloc2[RIDDLERSSTEPS]{};
      float m_thetaphi[RIDDLERSSTEPS]{};
      float m_thetatheta[RIDDLERSSTEPS]{};
      float m_thetaqop[RIDDLERSSTEPS]{};
      float m_thetasteps[RIDDLERSSTEPS]{};

      float m_qoploc1[RIDDLERSSTEPS]{};
      float m_qoploc2[RIDDLERSSTEPS]{};
      float m_qopphi[RIDDLERSSTEPS]{};
      float m_qoptheta[RIDDLERSSTEPS]{};
      float m_qopqop[RIDDLERSSTEPS]{};
      float m_qopsteps[RIDDLERSSTEPS]{};


      /** Random Number setup */
      Rndm::Numbers* m_gaussDist = nullptr;
      Rndm::Numbers* m_flatDist = nullptr;

    }; 
} // end of namespace

#endif 
