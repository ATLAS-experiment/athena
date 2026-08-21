/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////
// MappingTest.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef TRKDETDESCRUNITTESTS_MAPPINGTEST_H
#define TRKDETDESCRUNITTESTS_MAPPINGTEST_H

// Athena & Gaudi includes
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
// Trk includes
#include "TrkDetDescrUnitTests/TrkDetDescrUnitTestBase.h"
#include "TrkDetDescrInterfaces/ITrackingGeometrySvc.h"

class TTree;

namespace Trk {
     
    class TrackingGeometry;

        
    /** @class MappingTest
       
        Test to build the TrackingGeometry and try to map all points to layers.
        
        @author Andreas.Salzburger@cern.ch       
      */
      
    class MappingTest : public TrkDetDescrUnitTestBase  {
     public:

       /** Standard Athena-Algorithm Constructor */
       using Trk::TrkDetDescrUnitTestBase::TrkDetDescrUnitTestBase;
       
       /* specify the test here */
       StatusCode runTest();
       
       /* initialize the test, i.e. retrieve the TrackingGeometry Svc */
       StatusCode initializeTest();

       /* book the TTree branches */
       StatusCode bookTree();
                                                  
     private:          
       bool m_executed = false; //!< Make sure it only runs once
                                                        
      ServiceHandle<Trk::ITrackingGeometrySvc> m_trackingGeometrySvc
	{this, "TrackingGeometrySvc", "TrackingGeometrySvc/AtlasTrackingGeometrySvc",
	 "Service handle for retrieving the TrackingGeometry"};
       const TrackingGeometry* m_trackingGeometry = nullptr; //!< The TrackingGeometry to be retrieved
       std::string m_trackingGeometryName = "AtlasTrackingGeometry"; //!< The Name of the TrackingGeometry
       
       Gaudi::Property<double> m_etaCutOff
	 {this, "EtaCutOff", 6.0, "do not map beyond this point"};
       Gaudi::Property<std::string> m_mappingVolumeName
	 {this, "HighestVolume", "InDet::Detectors::Pixel::Barrel",
	  "only map within this volume"};
       
       Gaudi::Property<std::string> m_mappingTreeName
	 {this, "MappingTreeName", "LayerMappingTest"};
       Gaudi::Property<std::string> m_mappingTreeDescription
	 {this, "MappingTreeDescription", "Test Algorithm for Layer - 3D point association"};
       TTree* m_mappingTree = nullptr;
                                                    
       float m_mappingPositionX{};
       float m_mappingPositionY{};
       float m_mappingPositionZ{};
       float m_mappingPositionR{};
                                                    
       float m_assignedPositionX{};
       float m_assignedPositionY{};
       float m_assignedPositionZ{};
       float m_assignedPositionR{};
       float m_assignedCorrection{};
       int m_assignedLayerIndex{};
       float m_assignmentDistance{};
                                                    
       TTree* m_unmappedTree = nullptr;
       float m_unmappedPositionX{};
       float m_unmappedPositionY{};
       float m_unmappedPositionZ{};
       float m_unmappedPositionR{};
                                    
   };
}

#endif
