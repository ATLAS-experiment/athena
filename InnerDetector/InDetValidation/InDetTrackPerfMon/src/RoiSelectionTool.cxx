/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file RoiSelectionTool.cxx
 * @author marco aparo
 **/

#include "InDetTrackPerfMon/RoiSelectionTool.h"


///----------------------------------------
///------- parametrized constructor -------
///----------------------------------------
IDTPM::RoiSelectionTool::RoiSelectionTool(
    const std::string& name ) :
  asg::AsgTool( name ) { }


///--------------------------
///------- Initialize -------
///--------------------------
StatusCode IDTPM::RoiSelectionTool::initialize() {

  ATH_CHECK( asg::AsgTool::initialize() );

  ATH_MSG_DEBUG( "Initializing " << name() );

  ATH_CHECK( m_trigDecTool.retrieve() );

  return StatusCode::SUCCESS;
}


///------------------------
///----- retrieveRois -----
///------------------------
std::vector< IDTPM::roiCollection_t > IDTPM::RoiSelectionTool::retrieveRois(
    const std::string& chainName,
    const std::string& roiKey,
    const int& chainLeg ) const { 

  unsigned decisionType = TrigDefs::Physics; // TrigDefs::includeFailedDecisions;

  unsigned featureType = ( roiKey.empty() ) ?
                         TrigDefs::lastFeatureOfType :
                         TrigDefs::allFeaturesOfType;

  std::vector< roiCollection_t > rois = 
      m_trigDecTool->features< TrigRoiDescriptorCollection >( 
          Trig::FeatureRequestDescriptor(
              chainName,
              decisionType,
              roiKey,
              featureType,
              TrigCompositeUtils::roiString(),
              chainLeg ) );

  ATH_MSG_DEBUG( "Retrieved " << rois.size() <<
                 " RoIs for chain " << chainName <<
                 " , RoI key : " << roiKey <<
                 " , leg  = " << chainLeg );

  std::vector< roiCollection_t > selectedRois;

  /// loop to select RoIs
  for( size_t ir=0 ; ir<rois.size() ; ir++ ) {

    /// Don't extract any additional RoIs if a superRoi is requested: 
    /// In this case, the superRoi would be shared between the different chains 
    if( roiKey == "SuperRoi" && ir > 0 ) continue; 

    const ElementLink< TrigRoiDescriptorCollection > thisRoiLink = rois[ir].link;

    /// check this is ! a spurious TDT match
    if( !roiKey.empty() && thisRoiLink.dataID() != roiKey ) continue;

    const TrigRoiDescriptor* const* thisRoi = thisRoiLink.cptr();

    /// remove roi with corrupted link
    if( thisRoi == 0 ) continue;

    ATH_MSG_DEBUG( "Retrieved RoI descriptor for chain " <<
                   chainName << " " << **thisRoi );

    selectedRois.push_back( rois[ir] );

  } // close rois loop

  return selectedRois;
}


///---------------------------
///----- getRoisStandard -----
///---------------------------
std::vector< IDTPM::roiCollection_t > IDTPM::RoiSelectionTool::getRoisStandard( 
    const std::string& chainName ) const {

  return retrieveRois( chainName,
                       m_roiKey.value(),
                       m_chainLeg.value() );
}


///----------------------
///----- getRoisTnP -----
///----------------------
std::vector< IDTPM::roiCollection_t > IDTPM::RoiSelectionTool::getRoisTnP( 
    const std::string& chainName ) const {

  /// retrieving tag rois
  std::vector< roiCollection_t > selectedRoisTag =
      retrieveRois( chainName,
                    m_roiKeyTag.value(),
                    m_chainLegTag.value() );

  /// retrieving probe rois
  std::vector< roiCollection_t > selectedRoisProbe =
      retrieveRois( chainName,
                    m_roiKeyProbe.value(),
                    m_chainLegProbe.value() );

  /// TODO - add TnP selection
  //std::vector< roiCollection_t > selectedRois =
  //    m_TnPselectionTool( selectedRoisTag, selectedRoisProbe, tracks );
  std::vector< roiCollection_t > selectedRois = selectedRoisProbe;

  return selectedRois;
}


///-------------------
///----- getRois -----
///-------------------
std::vector< IDTPM::roiCollection_t > IDTPM::RoiSelectionTool::getRois(
    const std::string& chainName ) const { 

  return ( m_doTnP.value() ) ?
         getRoisTnP( chainName ) :
         getRoisStandard( chainName );
}
