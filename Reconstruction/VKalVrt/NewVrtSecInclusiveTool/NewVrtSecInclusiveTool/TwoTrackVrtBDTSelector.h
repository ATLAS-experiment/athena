/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

//
// TwoTrackVrtBDTSelector.h - Description
//
/*
   Tool for selection of good two track vertex for inclusive secondary vertex reconstruction

    Author: Vadim Kostyukhin
    e-mail: vadim.kostyukhin@cern.ch
-----------------------------------------------------------------------------*/

#ifndef _VKalVrt_TwoTrackVrtBDTSelector_H
#define _VKalVrt_TwoTrackVrtBDTSelector_H
// Normal STL and physical vectors
#include <vector>
// Gaudi includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
//
#include "NewVrtSecInclusiveTool/ITwoTrackVertexSelector.h"
#include "TrkVKalVrtFitter/TrkVKalVrtFitter.h"


namespace Trk{ class TrkVKalVrtFitter; }
 
namespace MVAUtils{ class BDT; }

 
//------------------------------------------------------------------------
namespace Rec{

  class TwoTrackVrtBDTSelector : public AthAlgTool, virtual public ITwoTrackVertexSelector
  {

    public:
       /* Constructor */
      TwoTrackVrtBDTSelector(const std::string& type, const std::string& name, const IInterface* parent);
       /* Destructor */
      virtual ~TwoTrackVrtBDTSelector();

      StatusCode initialize();
      StatusCode finalize();

     bool isgood( const std::pair<const xAOD::TrackParticle*,const xAOD::TrackParticle*> tracks,
		                      const xAOD::Vertex & candV,
                                std::pair<ROOT::Math::XYZTVector,ROOT::Math::XYZTVector> moms,
		                      const xAOD::Vertex & tPV) const final;
     bool isgood( const std::pair<const xAOD::TrackParticle*,const xAOD::TrackParticle*> tracks,
		                      const xAOD::Vertex & candV,
                                std::pair<ROOT::Math::XYZTVector,ROOT::Math::XYZTVector> moms,
		                      const xAOD::Vertex & tPV,
                          float & quality) const final;

//------------------------------------------------------------------------------------------------------------------
// Private data and functions
//
    private:

      //-- 2-track vertex acceptance and cleaning cuts
      Gaudi::Property<float> m_vrt2TrMassLimit{this,"Vrt2TrMassLimit",4000., "Maximal allowed mass for 2-track vertices" };
      Gaudi::Property<float> m_vrt2TrPtMin{this,    "Vrt2TrPtMin",    1000., "Minimal allowed Pt for 2-track vertices." };
      Gaudi::Property<float> m_vrt2TrPtMax{this,    "Vrt2TrPtMax",     5.e5, "Maximal allowed Pt for 2-track vertices. Calibration limit" };
      Gaudi::Property<float> m_sel2VrtProbCut{this, "Sel2VrtProbCut",  0.02, "Cut on probability of 2-track vertex for initial selection"  };
      Gaudi::Property<float> m_maxSVRadiusCut{this, "MaxSVRadiusCut",  140., "Cut on maximal radius of SV (def = Pixel detector size)"  };
      Gaudi::Property<float> m_cosSVPVCut{this,     "cosSVPVCut",        0., "Cut on cos of angle between SV-PV and full vertex momentum"  };
      //
      //-- Pixel geometry based 2-track vertex cleaning cuts
      Gaudi::Property<bool> m_do2TrkIBLChecks  {this, "do2TrkIBLChecks",   true, "IBL and B-layer hit requrirements based on the position of 2-track DV." };
      Gaudi::Property<bool> m_useVertexCleaning{this, "useVertexCleaning", true, "Clean vertices by requiring pixel hit presence according to vertex position" };
      Gaudi::Property<float> m_firstPixelLayerR{this, "FirstPixelLayerR",	  32.0,"Radius of the first Pixel layer" };
      //
      //--- BDT weight for 2-track vertex selection
      Gaudi::Property<float> m_v2tBDTCut{this, "v2tBDTCut",      -0.5,  "BDT cut to select  2-track vertices"  };
      Gaudi::Property<std::string> m_calibFileName{this, "CalibFileName", "Fake2TrVertexReject.MVA.v02.root", " MVA calibration file for 2-track fake vertices removal" };
      
      std::unique_ptr<MVAUtils::BDT> m_SV2T_BDT;

      ToolHandle<Trk::TrkVKalVrtFitter>  m_fitSvc{this, "VertexFitterTool", "Trk::TrkVKalVrtFitter/VertexFitterTool", "Vertex Fitter tool for 2-track selector"};

      static double vrtRadiusError(const Amg::Vector3D & secVrt, const std::vector<float>  & vrtErr);
      static int getIBLHit(const xAOD::TrackParticle* Part);
      static int getBLHit(const xAOD::TrackParticle* Part);

      double m_massPi{};
      double m_massP{};
      double m_massE{};
      std::string m_instanceName{};

//=====================
  };

}  //end namespace

#endif
