/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// EnergyLossValidation.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKEXALGS_ENERGYLOSSEXTRAPOLATIONVALIDATION_H
#define TRKEXALGS_ENERGYLOSSEXTRAPOLATIONVALIDATION_H

// Gaudi includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/SystemOfUnits.h"
#include <string>
#include "AthContainers/DataVector.h"
// CylinderSurfaces
#include "TrkSurfaces/CylinderSurface.h"
#include "TrkSurfaces/DiscSurface.h"
//
#include <memory>
class TTree;

#ifndef TRKEXALGS_MAXPARAMETERS
#define TRKEXALGS_MAXPARAMETERS 10
#endif


namespace Trk 
{

  class IExtrapolator;
  class Surface;
  class TrackingGeometry;
  class TrackingVolume;

  /** @class EnergyLossExtrapolationValidation

     The EnergyLossExtrapolationValidation Algorithm runs a number of n test
     extrapolations from randomly distributed Track Parameters to specific
     reference surfaces within the ATLAS Detector (user defined cylinder tubes
     which can be placed e.g. around Beam Pipe, Pixel, SCT, TRT, Calorimeter,
     Muon System, or have completely arbitrary dimensions).

     In the jobOptions one can toggle these surfaces for

     - a) the Inner Detector via DetFlags.ID_setOn()   / DetFlags.ID_setOff()
     - b) the Calorimeter    via DetFlags.Calo_setOn() / DetFlags.Calo_setOff()
     - c) the Muon System    via DetFlags.Muon_setOn() / DetFlags.Calo_setOff()

	 The code of ExtrapolationValidation (by Andreas Salzburger) has been
	 reused and adjusted for this algorithm.

      @author  Wolfgang Lukas     <Wolfgang.Lukas -at- cern.ch>
  */  

  class EnergyLossExtrapolationValidation : public AthAlgorithm
    {
    public:

       /** Standard Athena-Algorithm Constructor */
       EnergyLossExtrapolationValidation(const std::string& name, ISvcLocator* pSvcLocator);
       /** Default Destructor */
       ~EnergyLossExtrapolationValidation();

       /** standard Athena-Algorithm method */
       StatusCode          initialize();
       /** standard Athena-Algorithm method */
       StatusCode          execute();
       /** standard Athena-Algorithm method */
       StatusCode          finalize();

    private:
      /** private helper method to create a Transform */
      static std::unique_ptr<Amg::Transform3D> createTransform(double x,
                                                        double y,
                                                        double z,
                                                        double phi = 0.,
                                                        double theta = 0.,
                                                        double alphaZ = 0.);

      /** the highest volume */
      const TrackingVolume* m_highestVolume = nullptr;

      /** The Extrapolator to be retrieved */
      ToolHandle<IExtrapolator> m_extrapolator
	{this, "Extrapolator", "Trk::Extrapolator/AtlasExtrapolator"};

      /** Random Number setup */
      Rndm::Numbers* m_gaussDist = nullptr;
      Rndm::Numbers* m_flatDist = nullptr;

      BooleanProperty m_materialCollectionValidation
	{this, "UseMaterialCollection", false,
	 "use the material collection (extrapolateM)"};

      TTree* m_validationTree = nullptr;            //!< Root Validation Tree
      TTree* m_validationRunTree = nullptr;         //!< Root Run Stats Tree

      StringProperty m_validationTreeFolder
	{this, "ValidationTreeFolder", "/val/EventTreeTG",
	 "stream/folder to for the TTree to be written out"};
      StringProperty m_validationTreeName
	{this, "ValidationTreeName", "EventTreeTG",
	 "validation tree name - to be accessed by this from root"};
      StringProperty m_validationTreeDescription
	{this, "ValidationTreeDescription",
	 "Event output of the EnergyLossExtrapolationValidation Algorithm",
	 "validation tree description - second argument in TTree"};
      StringProperty m_validationRunTreeFolder
	{this, "ValidationRunTreeFolder", "/val/RunTreeTG",
	 "stream/folder to for the second TTree to be written out"};
      StringProperty m_validationRunTreeName
	{this, "ValidationRunTreeName", "RunTreeTG",
	 "run stats tree name - to be accessed by this from root"};
      StringProperty m_validationRunTreeDescription
	{this, "ValidationRunTreeDescription",
	 "Run stats of the EnergyLossExtrapolationValidation Algorithm",
	 "run stats tree description - second argument in TTree"};

      UnsignedIntegerProperty m_cylinders{this, "ValidationCylinders", 6,
	"number of cylinder layers"};
      BooleanProperty m_onion{this, "StrictOnionMode", true,
	"strictly hierarchical ordering (onion-like)"};
      FloatProperty m_momentum{this, "StartPerigeeMomentum",
	10.*Gaudi::Units::GeV};
      BooleanProperty m_usePt{this, "StartPerigeeUsePt", true,
	"use pt instead of p"};
      FloatProperty m_minEta{this, "StartPerigeeMinEta", -3.};
      FloatProperty m_maxEta{this, "StartPerigeeMaxEta", 3.};

      size_t  m_events = 0;         //!< total number of recorded events
      int     m_totalRecordedLayers = 0; //!< total number of recorded layers
      float   m_avgRecordedLayers = 0;   //!< average recorded layers per event

      int     m_pdg = 0;            //!< PDG code corresponding to m_particleType
      IntegerProperty m_particleType{this, "ParticleType", 2,
	"the particle type for the extrapolation"};

      // ---- variables for the ROOT event tree
      size_t m_entries = 0;         //!< effective number of used entries (recorded layers) in this event
      float m_energy[TRKEXALGS_MAXPARAMETERS]{};           //!< energy
      float m_energyLoss[TRKEXALGS_MAXPARAMETERS]{};       //!< energy loss
      float m_parameterX0[TRKEXALGS_MAXPARAMETERS]{};      //!< thickness in X0
      float m_radius[TRKEXALGS_MAXPARAMETERS]{};           //!< position radius
      float m_positionX[TRKEXALGS_MAXPARAMETERS]{};        //!< position X
      float m_positionY[TRKEXALGS_MAXPARAMETERS]{};        //!< position Y
      float m_positionZ[TRKEXALGS_MAXPARAMETERS]{};        //!< position Z
      float m_parameterPhi[TRKEXALGS_MAXPARAMETERS]{};     //!< phi
      float m_parameterEta[TRKEXALGS_MAXPARAMETERS]{};     //!< eta
      float m_parameterTheta[TRKEXALGS_MAXPARAMETERS]{};   //!< theta
      float m_parameterQoverP[TRKEXALGS_MAXPARAMETERS]{};  //!< qOverP
      float m_parameterP[TRKEXALGS_MAXPARAMETERS]{};       //!< P
      size_t m_layer[TRKEXALGS_MAXPARAMETERS]{};            //!< layer id

      // ---- output statistics
      size_t m_triesForward = 0;      //!< extrapolation events forward
      size_t m_breaksForward = 0;     //!< extrapolation breaks forward
      size_t m_triesBack = 0;         //!< extrapolation events backward
      size_t m_breaksBack = 0;        //!< extrapolation breaks backward

      size_t m_collectedLayerForward = 0;   //!< collected material layers forward
      size_t m_collectedLayerBack = 0;    //!< collected material layers backward


      // ---- cylinders and discs
      float m_cylinderR[TRKEXALGS_MAXPARAMETERS]{};   //!< radius of cylinder layers (for ROOT tree)
      float m_cylinderZ[TRKEXALGS_MAXPARAMETERS]{};   //!< length of cylinder layers (for ROOT tree)
      DataVector<const Trk::CylinderSurface>* m_theCylinders = nullptr;
      DataVector<const Trk::DiscSurface>* m_theDiscs1 = nullptr;
      DataVector<const Trk::DiscSurface>* m_theDiscs2 = nullptr;
      FloatArrayProperty m_cylinderVR{this, "ValidationCylinderR", {},
	"radius of cylinder layers"};
      FloatArrayProperty m_cylinderVZ{this, "ValidationCylinderZ", {},
	"length of cylinder layers"};
    }; 
} // end of namespace

#endif
