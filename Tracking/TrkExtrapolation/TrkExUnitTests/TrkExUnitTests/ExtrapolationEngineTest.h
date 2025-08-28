/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// ExtrapolationEngineTest.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEXUNITTESTS_EXTRAPOLATIONENGINETEST_H
#define TRKEXUNITTESTS_EXTRAPOLATIONENGINETEST_H

// Athena & Gaudi includes
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "CxxUtils/checker_macros.h"
// Trk includes
#include "TrkExUnitTests/TrkExUnitTestBase.h"
#include "TrkExInterfaces/IExtrapolationEngine.h"
#include "TString.h"
#include "TRandom.h"

class TTree;

class PixelID;
class SCT_ID;
class HGTD_ID;
class AtlasDetectorID;

namespace Trk {
  class IExtrapolationEngine;
  class IPositionMomentumWriter;

  /** @class ExtrapolationEngineTest

      Test Algorithm to run test extrapolations with the new IExtrapolationEngine

      @author Andreas.Salzburger@cern.ch, Noemi.Calace@cern.ch
   */

  class ATLAS_NOT_THREAD_SAFE ExtrapolationEngineTest: public TrkExUnitTestBase  {
  //    ^ ExtrapolationEngine itself is not thread-safe
  public:
    /** Standard Athena-Algorithm Constructor */
    using TrkExUnitTestBase::TrkExUnitTestBase;

    /* finalize */
    StatusCode finalize();

    /* specify the test here */
    StatusCode runTest();

    /* specify the scan here */
    StatusCode runScan();

    /** initialize the test, i.e. retrieve the TrackingGeometry Svc */
    StatusCode initializeTest();

    /* book the TTree branches */
    StatusCode bookTree();
  private:
       template <class T, class P> StatusCode runTestT(); 
         
       template <class T, class P> StatusCode fillStepInformationT(ExtrapolationCell<T>& eCell, int fwbw, std::vector<const Trk::Surface*>& stepSurfaces);
         
       /** retrieve it */
       ToolHandle<IExtrapolationEngine> m_extrapolationEngine{
	 this, "ExtrapolationEngine", ""};
       
       const AtlasDetectorID*                       m_idHelper = nullptr;
       const PixelID*                               m_pixel_ID = nullptr;
       const SCT_ID*                                m_sct_ID = nullptr;
       const HGTD_ID*                               m_hgtd_ID = nullptr;
       BooleanProperty m_useHGTD{this, "UseHGTD", false};
              
       BooleanProperty m_parametersMode{this, "ParametersMode", 1,
	 "0 - neutral, 1 - charged, 2 - multi"};
       IntegerProperty m_particleHypothesis{this, "ParticleHypothesis", 2};

       BooleanProperty m_smearProductionVertex{this, "SmearOrigin", false};
       BooleanProperty m_smearFlatOriginT{this, "SmearFlatOriginD0", false};
       BooleanProperty m_smearFlatOriginZ{this, "SmearFlatOriginZ0", false};
       DoubleProperty m_sigmaOriginT{this, "SimgaOriginD0", 0.};
       DoubleProperty m_sigmaOriginZ{this, "SimgaOriginZ0", 0.};
       DoubleProperty m_d0Min{this, "D0Min", 0.};
       DoubleProperty m_d0Max{this, "D0Max", 0.};
       DoubleProperty m_z0Min{this, "Z0Min", 0.};
       DoubleProperty m_z0Max{this, "Z0Max", 0.};
     
       DoubleProperty m_etaMin{this, "EtaMin", -3.};
       DoubleProperty m_etaMax{this, "EtaMax", 3.};
       DoubleProperty m_phiMin{this, "PhiMin", -M_PI};
       DoubleProperty m_phiMax{this, "PhiMax", M_PI};
       DoubleProperty m_ptMin{this, "PtMin", 100.};
       DoubleProperty m_ptMax{this, "PtMax", 100000.};
       
       DoubleProperty m_pathLimit{this, "PathLimit", 10e10};
       
       BooleanProperty m_collectSensitive{this, "CollectSensitive", false};
       BooleanProperty m_collectPassive{this, "CollectPassive", false};
       BooleanProperty m_collectBoundary{this, "CollectBoundary", false};
       BooleanProperty m_collectMaterial{this, "CollectMaterial", false};
       
       BooleanProperty m_backExtrapolation{this, "BackExtrapolation", false};
       BooleanProperty m_stepwiseExtrapolation{this, "StepwiseExtrapolation", false};
       
       /** scanning parameters */
       IntegerProperty m_stepsPhi{this, "PhiSteps", 1};
       int m_currentPhiStep = 0;
       FloatArrayProperty m_etaScans{this, "EtaScans", {}};
       double m_currentEta = 0.;
       FloatArrayProperty m_phiScans{this, "PhiScans", {}};
       double m_currentPhi = 0.;
       BooleanProperty m_splitCharge{this, "SplitCharge", false};

       BooleanProperty m_writeTTree{this, "WriteTTree", true};
       ToolHandle<IPositionMomentumWriter> m_posmomWriter{
	 this, "PositionMomentumWriter", ""};

       StringProperty m_treeName{this, "TreeName", "ExtrapolationEngineTest"};
       StringProperty m_treeFolder{this, "TreeFolder", "/val/"};
       StringProperty m_treeDescription{this, "TreeDescription",
	 "ExtrapolationEngine test setup"};
       TTree*                                       m_tree = nullptr;
       TRandom                                      m_tRandom;
                                                    
       float                                        m_startPositionX = 0.0F;
       float                                        m_startPositionY = 0.0F;
       float                                        m_startPositionZ = 0.0F;
       float                                        m_startPositionR = 0.0F;
       float                                        m_startPhi = 0.0F;
       float                                        m_startTheta = 0.0F;
       float                                        m_startEta = 0.0F;
       float                                        m_startP = 0.0F;
       float                                        m_startPt = 0.0F;
       float                                        m_charge = -1.;
        
       int                                          m_endSuccessful = 0;
       float                                        m_endPositionX = 0.0F;
       float                                        m_endPositionY = 0.0F;
       float                                        m_endPositionZ = 0.0F;
       float                                        m_endPositionR = 0.0F;
       float                                        m_endPhi = 0.0F;
       float                                        m_endTheta = 0.0F;                                                    
       float                                        m_endEta = 0.0F;                                                    
       float                                        m_endP = 0.0F;                                                    
       float                                        m_endPt = 0.0F;        
       float                                        m_endPathLength = 0.0F;
       
       int                                          m_backSuccessful = 0;
       float                                        m_backPositionX = 0.0F;
       float                                        m_backPositionY = 0.0F;
       float                                        m_backPositionZ = 0.0F;
       float                                        m_backPositionR = 0.0F;
       float                                        m_backPhi = 0.;
       float                                        m_backTheta = 0.;
       float                                        m_backEta = 0.;
       float                                        m_backP = 0.;
       float                                        m_backPt = 0.;
       
       std::vector<TString>                         m_parameterNames;                                                  
       std::vector< std::vector< float >* >         m_pPositionX;
       std::vector< std::vector< float >* >         m_pPositionY;
       std::vector< std::vector< float >* >         m_pPositionZ;
       std::vector< std::vector< float >* >         m_pPositionR;
       std::vector< std::vector< float >* >         m_pPhi;
       std::vector< std::vector< float >* >         m_pTheta;                                                
       std::vector< std::vector< float >* >         m_pEta;                                                    
       std::vector< std::vector< float >* >         m_pP;                                              
       std::vector< std::vector< float >* >         m_pPt;

       std::vector< int >*                          m_sensitiveSurfaceType = nullptr;
       std::vector< int >*                          m_sensitiveLayerIndex = nullptr;
       std::vector< float >*                        m_sensitiveLocalPosX = nullptr;
       std::vector< float >*                        m_sensitiveLocalPosY = nullptr;
       std::vector< float >*                        m_sensitiveCenterPosX = nullptr;
       std::vector< float >*                        m_sensitiveCenterPosY = nullptr;
       std::vector< float >*                        m_sensitiveCenterPosZ = nullptr;
       std::vector< float >*                        m_sensitiveCenterPosR = nullptr;
       std::vector< float >*                        m_sensitiveCenterPosPhi = nullptr;
       std::vector< float >*                        m_sensitiveLocalPosR = nullptr;
       std::vector< float >*                        m_sensitiveLocalPosPhi = nullptr;
       std::vector< int >*                          m_sensitiveDetector = nullptr;
       std::vector< int >*                          m_sensitiveIsInnermost = nullptr;
       std::vector< int >*                          m_sensitiveIsNextToInnermost = nullptr;
       std::vector< int >*                          m_sensitiveBarrelEndcap = nullptr;
       std::vector< int >*                          m_sensitiveLayerDisc = nullptr;
       std::vector< int >*                          m_sensitiveEtaModule = nullptr;
       std::vector< int >*                          m_sensitivePhiModule = nullptr;
       std::vector< int >*                          m_sensitiveSide = nullptr;
       std::vector< int >*                          m_sensitiveNearBondGap = nullptr;
       std::vector< int >*                          m_sensitiveisInside = nullptr;
       std::vector< int >*                          m_sensitiveisInsideBound = nullptr;
       std::vector< float >*                        m_materialtInX0AccumulatedUpTo = nullptr;
       
       float                                        m_materialThicknessInX0 = 0.;
       float                                        m_materialThicknessInL0 = 0.;
       float                                        m_materialThicknessZARho = 0.;
       float                                        m_materialEmulatedIonizationLoss = 0.;

       float                                        m_materialThicknessInX0Bwd = 0.;
       float                                        m_materialThicknessInL0Bwd = 0.;

       float                                        m_materialThicknessInX0Sensitive = 0.;
       float                                        m_materialThicknessInX0Passive = 0.;
       float                                        m_materialThicknessInX0Boundary = 0.;
       
       float                                        m_materialThicknessInX0Cylinder = 0.;
       float                                        m_materialThicknessInX0Disc = 0.;
       float                                        m_materialThicknessInX0Plane = 0.;
       
       std::vector< float >*                        m_materialThicknessInX0Accumulated = nullptr;
       std::vector< float >*                        m_materialThicknessInX0Steps = nullptr;
       std::vector< float >*                        m_materialThicknessInL0Steps = nullptr;
       std::vector< float >*                        m_materialPositionX = nullptr;
       std::vector< float >*                        m_materialPositionY = nullptr;
       std::vector< float >*                        m_materialPositionZ = nullptr;
       std::vector< float >*                        m_materialPositionR = nullptr;
       std::vector< float >*                        m_materialPositionP = nullptr;
       std::vector< float >*                        m_materialPositionPt = nullptr;
       std::vector< float >*                        m_materialScaling = nullptr;
       std::vector< int   >*                        m_stepDirection = nullptr;
       
       int                                          m_endStepSuccessful = 0;
       float                                        m_endStepPositionX = 0.;
       float                                        m_endStepPositionY = 0.;
       float                                        m_endStepPositionZ = 0.;
       float                                        m_endStepPositionR = 0.;
       float                                        m_endStepPhi = 0.;
       float                                        m_endStepTheta = 0.;
       float                                        m_endStepEta = 0.;
       float                                        m_endStepP = 0.;
       float                                        m_endStepPt = 0.;
       float                                        m_endStepPathLength = 0.;
       float                                        m_endStepThicknessInX0 = 0.;

   };
}

// include the templated function
#include "ExtrapolationEngineTest.icc"

#endif
