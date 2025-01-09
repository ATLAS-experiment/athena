/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// CETmaterial.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEXALGS_CETMATERIAL_H
#define TRKEXALGS_CETMATERIAL_H

// Gaudi includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "TrkParameters/TrackParameters.h"
#include <string>

#include "TrkExInterfaces/IExtrapolator.h"

namespace Trk 
{

  class Surface;
  class TrackingVolume;
  class TrackingGeometry;

  /** @class CETmaterial

     The ExtrapolatorTest Algorithm runs a number of n test extrapolations
     from randomly distributed Track Parameters to geometry outer boundary and back to perigee

      @author Sarka Todorova <sarka.todorova@cern.ch>
  */  

  class CETmaterial : public AthAlgorithm
    {
    public:

       /** Standard Athena-Algorithm Constructor */
       CETmaterial(const std::string& name, ISvcLocator* pSvcLocator);
       /** Default Destructor */
       ~CETmaterial();

       /** standard Athena-Algorithm method */
       StatusCode          initialize();
       /** standard Athena-Algorithm method */
       StatusCode          execute();
       /** standard Athena-Algorithm method */
       StatusCode          finalize();

    private:
       void printMat(double th, double ph, double mat,double dtheta=0.,double dphi=0.) const;
       void printMatScan(double theta, double phi, double r, double z, double mat, const std::string& name) const;
       void printMatPrec(double theta, double phi, const Trk::TrackParameters*, const Trk::TrackParameters*, 
			 double mat, int id, const std::string& name);
       void printMatComp(double theta, double phi, const Trk::TrackParameters* currPar, const std::string& name, double mat, double matApp,double dx, double dy) const;
            
      /** The Extrapolator(s) to be retrieved */
      ToolHandle<IExtrapolator> m_extrapolator
        {this, "Extrapolator", "Trk::Extrapolator/AtlasExtrapolator"};
      ToolHandle<IExtrapolator> m_extraprec
        {this, "ExtraExtrapolator", "Trk::Extrapolator/MuonExtrapolator"};

      DoubleProperty m_minZ0{this, "StartPerigeeMinZ0", 0.};
      DoubleProperty m_maxZ0{this, "StartPerigeeMaxZ0", 0.};
      DoubleProperty m_minTheta{this, "StartPerigeeMinTheta", 0.};
      DoubleProperty m_maxTheta{this, "StartPerigeeMaxTheta", M_PI};
      DoubleProperty m_minP{this, "StartPerigeeMinP", 50000*Gaudi::Units::GeV};
      DoubleProperty m_maxP{this, "StartPerigeeMaxP", 50000*Gaudi::Units::GeV};
      DoubleProperty m_charge{this, "StartPerigeeCharge", 1.};
      UnsignedIntegerProperty m_numScan{this, "NumberOfScanTracks", 10};
      BooleanProperty m_checkStepWise{this, "CheckActiveLayers", false};
      BooleanProperty m_printMaterial{this, "PrintMaterial", false};
      BooleanProperty m_printActive{this, "PrintActivePos", false};

      const char* m_matTotFile = "material.txt";
      const char* m_matScanFile = "material_scan.txt";
      const char* m_matActiveFile = "mat_active.txt";
      const char* m_matCompFile = "material_comp.txt";

      BooleanProperty m_backward{this, "CheckBackward", false};
      BooleanProperty m_domsentry{this, "CheckMSentry", false};
      BooleanProperty m_doprecision{this, "CheckPrecision", false};

      double m_th = 0.;
      double m_ph = 0.;
      int    m_id = 0;
      double m_matSaved = 0.;
      Trk::TrackParameters* m_next = nullptr;
      Amg::MatrixX* m_err = nullptr;

      const Trk::Surface*          m_outerBoundary = nullptr;
      const Trk::TrackingGeometry* m_trackingGeometry = nullptr;
      const Trk::TrackingVolume*   m_msentry = nullptr;

      IntegerProperty m_particleType{this, "ParticleType", Trk::muon,
	"the particle type for the extrap."};

      typedef ServiceHandle<IChronoStatSvc> IChronoStatSvc_t;
      IChronoStatSvc_t              m_chronoStatSvc;      
     
    }; 
} // end of namespace

#endif
