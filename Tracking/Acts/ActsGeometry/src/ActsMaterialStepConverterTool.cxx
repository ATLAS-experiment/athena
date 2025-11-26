/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ActsGeometry/ActsMaterialStepConverterTool.h"

// ATHENA
#include "GaudiKernel/EventContext.h"

// PACKAGE
#include "ActsGeometryInterfaces/ITrackingGeometrySvc.h"

// Tracking
#include "TrkGeometry/MaterialStep.h"

// ACTS
#include "Acts/Material/MaterialSlab.hpp"

// STL
#include <iostream>
#include <memory>

ActsMaterialStepConverterTool::ActsMaterialStepConverterTool(const std::string& type, const std::string& name,
    const IInterface* parent)
  : base_class(type, name, parent)
{
}

StatusCode
ActsMaterialStepConverterTool::initialize()
{
  ATH_MSG_INFO(name() << " initializing");

  return StatusCode::SUCCESS;
}


const Acts::RecordedMaterialTrack
ActsMaterialStepConverterTool::convertToMaterialTrack(const Trk::MaterialStepCollection &colStep) const
{
  Acts::RecordedMaterialTrack mTrack;
  std::vector<Acts::MaterialInteraction> nStep;
  Acts::RecordedMaterial recorded;
  double sum_X0 = 0;
  double sum_L0 = 0;

  // check you are working with the correct dataset, otherwise write out a warning
  // for mapping geantinos must be produced from (0, 0, 0), hence expecting z being very small!
  double deltaZ = colStep.back()->hitZ() - colStep.front()->hitZ();
  double deltaR = colStep.back()->hitR() - colStep.front()->hitR();
  double vertexZ = colStep.front()->hitZ() - (deltaZ/deltaR)*colStep.front()->hitR();
  if (std::abs(vertexZ)>s_tolerance)
    ATH_MSG_WARNING("z-vertex position larger than expected for material mapping. Check the beamspot config used for production of material steps!");

  Acts::Vector3 v_pos{0, 0, 0};
  Acts::Vector3 v_imp{colStep.back()->hitX(), colStep.back()->hitY(), colStep.back()->hitZ()};
  v_imp = v_imp.normalized();

  for(auto const step: colStep) {
    Acts::Vector3 pos{step->hitX(), step->hitY(), step->hitZ()};
    Acts::MaterialSlab matProp(Acts::Material::fromMassDensity(step->x0(), step->l0(), step->A(), step->Z(), (step->rho() * Acts::UnitConstants::g) ),step->steplength());
    Acts::MaterialInteraction interaction;
    interaction.position = pos;
    interaction.direction = Acts::Vector3(pos.x(), pos.y(), pos.z());
    interaction.direction = interaction.direction.normalized();
    interaction.materialSlab = matProp;
    sum_X0 += step->steplengthInX0();
    sum_L0 += step->steplengthInL0();
    nStep.push_back(interaction);
  }

  recorded.materialInX0 = sum_X0;
  recorded.materialInL0 = sum_L0;
  recorded.materialInteractions = std::move(nStep);

  mTrack = std::make_pair(std::make_pair(v_pos, v_imp), recorded);

  return mTrack;

}
