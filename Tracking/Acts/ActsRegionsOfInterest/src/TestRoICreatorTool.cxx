/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/TestRoICreatorTool.h"

namespace ActsTrk {

TestRoICreatorTool::TestRoICreatorTool(const std::string& type,
				       const std::string& name,
				       const IInterface* parent)
  : base_class(type, name, parent) 
{}

StatusCode TestRoICreatorTool::initialize()
{
  ATH_MSG_DEBUG("Inizializing " << name() << " ..." );

  // Check consistency
  // We need at least one entry
  if (m_eta_center_rois.size() == 0) {
    ATH_MSG_ERROR("No RoI definition defined, cannot create the test RoI.");
    return StatusCode::FAILURE;
  }

  // eta and phi vectors MUST have the same size. z vectors can have 0 size OR the same size as the eta/phi vectors
  if (m_eta_center_rois.size() != m_phi_center_rois.size() or 
      m_eta_center_rois.size() != m_half_eta_width_rois.size() or
      m_eta_center_rois.size() != m_half_phi_width_rois.size()) {
    ATH_MSG_ERROR("Inconsistent definitions for eta/phi vectors. Check their definitions!");
    return StatusCode::FAILURE;
  }

  // Now check the z vectors
  if (m_z_center_rois.size() != m_half_z_width_rois.size()) {
    ATH_MSG_ERROR("Inconsistent definitions for the z vectors. Check their definitions!");
    return StatusCode::FAILURE;
  }
  
  if (m_z_center_rois.size() != 0 and m_z_center_rois.size() != m_eta_center_rois.size()) {
    ATH_MSG_ERROR("Z vector is not empty but it is not consistent with the phi/eta counterpart. Check their definitions!");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode TestRoICreatorTool::defineRegionsOfInterest(const EventContext& /*ctx*/,
						       TrigRoiDescriptorCollection& collectionRoI) const
{
  bool isComposite = m_eta_center_rois.size() > 1;
  // Add a composite RoI
  if (isComposite) {
    collectionRoI.push_back( new TrigRoiDescriptor() );
    collectionRoI.back()->setComposite(true);
  }

  bool useZconstraint = m_z_center_rois.size() != 0;

  for (std::size_t i(0); i<m_eta_center_rois.size(); ++i) {
    double eta = m_eta_center_rois.value().at(i);
    double phi = m_phi_center_rois.value().at(i);
    double eta_half_width = m_half_eta_width_rois.value().at(i);
    double phi_half_width = m_half_phi_width_rois.value().at(i);

    TrigRoiDescriptor *toAdd = nullptr;
    if (not useZconstraint) {
      toAdd = new TrigRoiDescriptor(eta, eta - eta_half_width, eta + eta_half_width,
				    phi, phi - phi_half_width, phi + phi_half_width);
    } else {
      double z = m_z_center_rois.value().at(i);
      double z_half_width = m_half_z_width_rois.value().at(i);
      toAdd = new TrigRoiDescriptor(eta, eta - eta_half_width, eta + eta_half_width,
      				    phi, phi - phi_half_width, phi + phi_half_width,
      				    z, z - z_half_width, z + z_half_width);
    }

    // if composite roi, add the consituent to it (already in the collection)
    // if not, the collection is empty and we have to add this one to it
    if (isComposite) {
      collectionRoI.back()->push_back( std::move(toAdd) );
    } else {
      collectionRoI.push_back( std::move(toAdd) );
    }
  }

  ATH_MSG_DEBUG("RoI collection size: " << collectionRoI.size());
  ATH_MSG_DEBUG("Created a test RoI");
  if (collectionRoI.back()->composite()) {
    ATH_MSG_DEBUG("This is a composite RoI made from " << collectionRoI.back()->size() << " constituents");
  }

  return StatusCode::SUCCESS;
}

}
