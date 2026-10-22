/**
 **  @file RoiSelector.cxx
 **  @author marco aparo
 **
 **  NB: the ONLY reason to have things like the roiKey as class
 **      variables is so you don't have to pass them as parameters
 **      during the retrieve call, and the ONLY reason to make it
 **      a python configurable tool is so you can configure these
 **      parameters so if you ALWAYS need to pass the (only) THREE
 **      parameters in the retrieve, then you don't need the class
 **      variables, and then you don't need it to be python
 **      configurable, and you don't need to make it a tool, it can
 **      just be a simple class
 **
 **  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 **
 **/

#include "RoiSelector.h"


///------------------------
///----- retrieveRois -----
///------------------------
std::vector<IDTPM2::RoiSelector::roi_info_t> IDTPM2::RoiSelector::retrieveRois( const std::string& chainName,
										const std::string& roiKey,
										const int& chainLeg ) const {

  /// if ( m_trigDecTool==0 ) need to throw an exception here really ...
  
  unsigned decisionType = TrigDefs::Physics; // TrigDefs::includeFailedDecisions;

  unsigned featureType = ( roiKey.empty() ) ? TrigDefs::lastFeatureOfType : TrigDefs::allFeaturesOfType;

  std::string roi_key = roiKey; 
  
  std::vector<IDTPM2::RoiSelector::roi_info_t> rois =
    (*m_trigDecTool)->template features<TrigRoiDescriptorCollection >( Trig::FeatureRequestDescriptor( chainName,
												       decisionType,
												       roi_key,
												       featureType,
												       TrigCompositeUtils::roiString(),
												       chainLeg ) );
  
  /// maybe it is a tag and probe roi with _probe appended to the end
  if ( rois.empty() ) {
    roi_key += "_probe";
    rois = (*m_trigDecTool)->template features<TrigRoiDescriptorCollection >( Trig::FeatureRequestDescriptor( chainName,
													      decisionType,
													      roi_key,
													      featureType,
													      TrigCompositeUtils::roiString(),
													      chainLeg ) );
  }
  
  
  std::vector<IDTPM2::RoiSelector::roi_info_t> selectedRois;

  /// loop to select RoIs
  for ( size_t ir=0 ; ir<rois.size() ; ir++ ) {

    /// Don't extract any additional RoIs if a superRoi is requested: 
    /// In this case, the superRoi would be shared between the different chains 

    if( roiKey == "SuperRoi" && ir>0 ) continue; 
    if( roiKey.find("Super")!=std::string::npos && ir>0 ) continue; 

    const ElementLink<TrigRoiDescriptorCollection> roiLink = rois[ir].link;

    /// check this is ! a spurious TDT match
    if( !roiKey.empty() && roiLink.dataID() != roi_key ) continue;

    const TrigRoiDescriptor* const* roi = roiLink.cptr();

    /// remove roi with corrupted link
    if( roi == 0 ) continue;

    //    MY_MSG_DEBUG <<  "Retrieved RoI descriptor for chain " << chainName <<" " <<**roi  << endmsg;

    selectedRois.push_back( rois[ir] );

  } // close rois loop

  return selectedRois;
}


