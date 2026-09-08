/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// GeometryTTreeDumper.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

// Trk include
#include "TrkDetDescrTestTools/GeometryTTreeDumper.h"
#include "TrkValidationUtils/SurfaceNtupleBranch.h"
#include "TrkGeometry/TrackingGeometry.h"
#include "TrkGeometry/Layer.h"
#include "TrkSurfaces/Surface.h"
// Root
#include "TTree.h"
#include "TString.h"
#include <sstream>

// Gaudi
#include "GaudiKernel/ITHistSvc.h"


StatusCode Trk::GeometryTTreeDumper::processNode(const Trk::TrackingVolume& tvol, size_t) const
{
   ATH_MSG_VERBOSE("Dumping information for TrackingVolume.");
   // clean up from the previous event
   if (m_currentTree) m_currentTree->Fill();
   
   // create the Tree for this TrackingVolume
   m_currentTree = new TTree(tvol.volumeName().c_str(),"Geometrical information");
   SmartIF<ITHistSvc> tHistSvc{service("THistSvc")};
   if (!tHistSvc) {
       ATH_MSG_FATAL( "initialize() Could not find Hist Service! Aborting." );
       delete m_currentTree;
       return StatusCode::FAILURE;
   }
   // create the name under which the tree is registered
   std::string fullTreeName = m_treeFolder+tvol.volumeName();
   if (tHistSvc->regTree(fullTreeName.c_str(), m_currentTree).isFailure()) {
       ATH_MSG_ERROR( "initialize() Could not register the validation Tree!Aborting." );
       delete m_currentTree;
       return StatusCode::FAILURE;
   }
   
   // make the boundary surface branch
   Trk::SurfaceNtupleBranch boundarySurfaceBranch;
   
   const auto& bSurfaces = tvol.boundarySurfaces();
   for (size_t ib = 0 ; ib < bSurfaces.size(); ++ib ){
        TString bSurfId  = "bsurf_";
                bSurfId += ib;
        // initialize 
        boundarySurfaceBranch.initForWrite(*m_currentTree,bSurfId.Data());
        boundarySurfaceBranch.fill(bSurfaces[ib]->surfaceRepresentation());
   }
   
   return StatusCode::SUCCESS;
}


StatusCode Trk::GeometryTTreeDumper::processNode(const Trk::Layer& lay, size_t) const
{
    
    ATH_MSG_VERBOSE("Dumping information for Layer.");

    // make the layer branch
    Trk::SurfaceNtupleBranch layerBranch;
    TString layerId  = "lay_";
            layerId += lay.layerIndex().value();
    // initialize 
    layerBranch.initForWrite(*m_currentTree,layerId.Data());
    layerBranch.fill(lay.surfaceRepresentation());
    
    return StatusCode::SUCCESS;    
}


StatusCode Trk::GeometryTTreeDumper::processNode(const Trk::Surface& sf, size_t) const
{
    ATH_MSG_VERBOSE("Dumping information for Surfaces.");

    // make the surface branch
    Trk::SurfaceNtupleBranch surfaceBranch;
    TString surfaceId  = "surf";
    // initialize 
    surfaceBranch.initForWrite(*m_currentTree,surfaceId.Data());
    surfaceBranch.fill(sf);

    return StatusCode::SUCCESS;    
}





