/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKVERTEXWEIGHTCALCULATORS_BDTVERTEXWEIGHTCALCULATOR_H
#define TRKVERTEXWEIGHTCALCULATORS_BDTVERTEXWEIGHTCALCULATOR_H

#include <AsgDataHandles/ReadDecorHandleKey.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgTools/AsgTool.h>
#include <TrkVertexFitterInterfaces/IVertexWeightCalculator.h>
#include <xAODTracking/VertexContainer.h>

#include <memory>
#include <string>

#include "AsgTools/PropertyWrapper.h"

namespace MVAUtils {
class BDT;
}

/**
 * @brief A tool to select the primary vertex associated to the hard-scatter
 * using a BDT
 *
 * This tool uses a BDT to select the hard-scatter vertex from a collection of
 * primary vertices.
 * It reads the pointing information from the pointing vertex created by
 *  @c BuildVertexPointingAlg.
 *
 */
class BDTVertexWeightCalculator final
    : public asg::AsgTool,
      virtual public Trk::IVertexWeightCalculator {
  ASG_TOOL_CLASS(BDTVertexWeightCalculator, IVertexWeightCalculator)
 public:
  BDTVertexWeightCalculator(const std::string& name);
  virtual ~BDTVertexWeightCalculator() override;
  virtual StatusCode initialize() override;
  /**
   * @brief Estimate the compatibility of the vertex with a hard scatter vertex,
   * with respect to pileup vertices
   */
  virtual double estimateSignalCompatibility(
      const xAOD::Vertex& vertex) const override;
  const xAOD::Vertex* getVertex(const xAOD::VertexContainer& vertices) const;

 private:
  SG::ReadHandleKey<xAOD::VertexContainer> m_pointingVertexContainerKey{
      this, "PointingVertexContainerKey", "PhotonPointingVertices",
      "The container with the vertex build with (photon) pointing"};
  SG::ReadDecorHandleKey<xAOD::VertexContainer> m_nphotons_good_key{
      this, "NGoodPhotonsKey", m_pointingVertexContainerKey, "nphotons_good"};
  SG::ReadDecorHandleKey<xAOD::VertexContainer> m_photons_px_key{
      this, "PhotonsPxKey", m_pointingVertexContainerKey, "photons_px"};
  SG::ReadDecorHandleKey<xAOD::VertexContainer> m_photons_py_key{
      this, "PhotonsPyKey", m_pointingVertexContainerKey, "photons_py"};
  SG::ReadDecorHandleKey<xAOD::VertexContainer> m_photons_pz_key{
      this, "PhotonsPzKey", m_pointingVertexContainerKey, "photons_pz"};
  Gaudi::Property<std::string> m_bdt_file{this, "BDTFile", "",
                                          "BDT ROOT filename"};
  Gaudi::Property<std::string> m_bdt_name{this, "BDTName", "BDT",
                                          "BDT tree name"};

  std::unique_ptr<MVAUtils::BDT> m_bdt;

  std::vector<float> get_input_values(const xAOD::Vertex& vertex) const;
  StatusCode initialize_BDT();
};

#endif  // TRKVERTEXWEIGHTCALCULATORS_BDTVERTEXWEIGHTCALCULATOR_H