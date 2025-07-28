// Dear emacs, this is -*- c++ -*-
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
#ifndef BOOSTEDJETTAGGERS_JSSMLTOOL_H
#define BOOSTEDJETTAGGERS_JSSMLTOOL_H

#include "IJSSMLTool.h"
#include "AsgTools/AsgTool.h"

// ONNX Runtime include(s).
#include <onnxruntime_cxx_api.h>

// xAOD
#include "xAODJet/JetContainer.h"
#include "xAODPFlow/TrackCaloClusterContainer.h"
// System include(s).
#include <memory> //unique_ptr
#include <string>
#include <map>
#include <vector>
#include <cstdint>

class TH2D; 


namespace AthONNX {

   /// Tool using the ONNX Runtime C++ API 
   /// to retrive constituents based model for boson jet tagging
   ///
   /// this is inspired from the general athena example here:
   /// https://gitlab.cern.ch/atlas/athena/-/blob/21.2/Control/AthenaExamples/AthExOnnxRuntime/AthExOnnxRuntime/CxxApiAlgorithm.h
   ///
   /// this is implementation is an extension from the one done in rel.21
   /// https://gitlab.cern.ch/atlas/athena/-/tree/21.2/Reconstruction/Jet/AthOnnxRuntimeBJT
   /// as the plan is to move to use the central ONNX interface
   /// the tool has been merged with the BJT
   ///
   /// monitoring jira ticket:
   /// https://its.cern.ch/jira/browse/ATLJETMET-1893
   /// 

   ///
   /// @author Antonio Giannini <antonio.giannini@cern.ch>
   ///
  
class JSSMLTool 
  : public asg::AsgTool, 
    virtual public IJSSMLTool {
  ASG_TOOL_CLASS(JSSMLTool, IJSSMLTool)

  public:
    JSSMLTool (const std::string& name);

    /// Function initialising the tool
    virtual StatusCode initialize() override;
    /// Function executing the tool for a single event
    virtual double retrieveConstituentsScore(std::vector<TH2D> Images) const override;
    virtual double retrieveConstituentsScore(std::vector<std::vector<float>> constituents) const override;
    virtual double retrieveConstituentsScore(std::vector<std::vector<float>> constituents, std::vector<std::vector<std::vector<float>>> interactions) const override;
    virtual double retrieveConstituentsScore(std::vector<std::vector<float>> constituents, std::vector<std::vector<std::vector<float>>> interactions, std::vector<std::vector<float>> mask) const override;
    virtual double retrieveHighLevelScore(std::map<std::string, double> JSSVars) const override;

    // basic tool functions
    std::vector<float> ReadJetImagePixels( std::vector<TH2D> Images ) const;
    std::vector<float> ReadJSSInputs(std::map<std::string, double> JSSVars) const;
    std::vector<int> ReadOutputLabels() const;

    // extra methods
    StatusCode SetScaler(std::map<std::string, std::vector<double>> scaler) override;

      /// @}
    std::unique_ptr< Ort::Session > m_session;
    std::unique_ptr< Ort::Env > m_env;

    std::map<std::string, std::vector<double>> m_scaler;
    std::map<int, std::string> m_JSSInputMap;

   private:

    /// Name of the model file to load
      std::string m_modelFileName;
      std::string m_pixelFileName;
      std::string m_labelFileName;

     // input node info
     std::vector<int64_t> m_input_node_dims;
     size_t m_num_input_nodes{};
     std::vector<const char*> m_input_node_names;

     // output node info
     std::vector<int64_t> m_output_node_dims;
     size_t m_num_output_nodes{};
     std::vector<const char*> m_output_node_names;

     // some configs
      int m_nPixelsX{}, m_nPixelsY{}, m_nPixelsZ{};

     int m_nvars{};

   }; // class JSSMLTool

} // namespace AthONNX

#endif // BOOSTEDJETTAGGERS_JSSMLTOOL_H
