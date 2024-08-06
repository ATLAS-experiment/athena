/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PhotonVertexSelection_PhotonVertexSelectionTool_h
#define PhotonVertexSelection_PhotonVertexSelectionTool_h

// Framework includes
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include "AsgDataHandles/ReadHandleKey.h"

// EDM includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/VertexContainer.h"
#include "xAODEgamma/PhotonContainer.h"

// Local includes
#include "PhotonVertexSelection/IPhotonVertexSelectionTool.h"

// ONNX Runtime include(s).
#include <onnxruntime_cxx_api.h>

// Forward declarations
namespace TMVA { class Reader; }

namespace CP {

  /// Implementation for the photon vertex selection tool
  ///
  /// Takes a list of photons (for example, to two leading photons) and
  /// the most likely primary vertex, based on an MVA.
  ///
  /// @author Christopher Meyer <chris.meyer@cern.ch>
  /// @author Bruno Lenzi <bruno.lenzi@cern.ch>
  ///
  class PhotonVertexSelectionTool : public virtual IPhotonVertexSelectionTool,
                                    public asg::AsgTool {

    /// Create a proper constructor for Athena
    ASG_TOOL_CLASS(PhotonVertexSelectionTool, CP::IPhotonVertexSelectionTool)

  private:
    /// Configuration variables
    int         m_nVars;   // number of input variables
    float       m_convPtCut;
    bool        m_doSkipByZSigma;
    std::string m_vertexContainerName;
    std::string m_derivationPrefix;

    /// Container declarations
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EventInfoContName", "EventInfo", "event info key"};
    SG::ReadHandleKey<xAOD::VertexContainer> m_vertexContainer {this, "VertexContainer", "PrimaryVertices", "Vertex container name"};

    /// TMVA
    /// ==================================================
    bool        m_isTMVA;  // boolean to use TMVA, if false assume to be ONNX-based
    std::string m_TMVAModelFilePath1; //TMVA config file, converted case
    std::string m_TMVAModelFilePath2; //TMVA config file, unconverted case

    // MVA readers
    // Ideally these would be const but the main method called, EvaluateMVA, is non const.
    std::unique_ptr<TMVA::Reader> m_mva1;
    std::unique_ptr<TMVA::Reader> m_mva2;

    // ONNX
    // ==================================================
    // Name of the ONNX model file to load
    std::string m_ONNXModelFilePath1; //converted case
    std::string m_ONNXModelFilePath2; //unconverted case 

    // declare node vars
    // converted
    std::vector<int64_t> m_input_node_dims1, m_output_node_dims1;
    std::vector<const char*> m_input_node_names1, m_output_node_names1;   
    // unconverted
    std::vector<int64_t> m_input_node_dims2, m_output_node_dims2;
    std::vector<const char*> m_input_node_names2, m_output_node_names2;   

    // The ONNX session handlers
    std::shared_ptr<Ort::Session> m_sessionHandle1; //converted case     
    std::shared_ptr<Ort::Session> m_sessionHandle2; //unconverted case     
    // The ONNX memory allocators, for looping
    Ort::AllocatorWithDefaultOptions m_allocator1; //converted case     
    Ort::AllocatorWithDefaultOptions m_allocator2; //unconverted case  

    // ONNX Methods
    // create ONNX session and return both the allocator and session handler from user-defined onnx env and model
    std::tuple<std::shared_ptr<Ort::Session>, Ort::AllocatorWithDefaultOptions> setONNXSession(Ort::Env& env, const std::string& modelFilePath);
    // get the input nodes info from the onnx model file (using session and allocator)
    std::tuple<std::vector<int64_t>, std::vector<const char*>> getInputNodes( const std::shared_ptr<Ort::Session> sessionHandle, Ort::AllocatorWithDefaultOptions& allocator);
    // get the output nodes info from the onnx model file (using session and allocator)
    std::tuple<std::vector<int64_t>, std::vector<const char*>> getOutputNodes(const std::shared_ptr<Ort::Session> sessionHandle, Ort::AllocatorWithDefaultOptions& allocator);
    // wrapper for getting the NN score from onnx model file (passed as onnx session)
    float getScore(int nVars, const std::vector<std::vector<float>>& input_data, const std::shared_ptr<Ort::Session> sessionHandle, std::vector<int64_t> input_node_dims, std::vector<const char*> input_node_names, std::vector<const char*> output_node_names) const;
    // ==================================================

  private:
    /// Get combined 4-vector of photon container
    TLorentzVector getEgammaVector(const xAOD::EgammaContainer *egammas, FailType& failType) const;

    /// Sort MLP results
    static bool sortMLP(const std::pair<const xAOD::Vertex*, float> &a, const std::pair<const xAOD::Vertex*, float> &b);


    /// Given a list of photons, return the MLPs of all vertices in the event
    StatusCode getVertexImp(const xAOD::EgammaContainer &egammas, const xAOD::Vertex* &vertex, bool ignoreConv, bool noDecorate, std::vector<std::pair<const xAOD::Vertex*, float> >&, yyVtxType& , FailType& ) const;


  public:
    PhotonVertexSelectionTool(const std::string &name);
    virtual ~PhotonVertexSelectionTool();

    /// @name Function(s) implementing the asg::IAsgTool interface
    /// @{

    /// Function initialising the tool
    virtual StatusCode initialize();

    /// @}

    /// @name Function(s) implementing the IPhotonVertexSelectionTool interface
    /// @{

    /// Given a list of photons, decorate vertex container with MVA variables
    StatusCode decorateInputs(const xAOD::EgammaContainer &egammas, FailType* failType = nullptr) const;

    /// Given a list of photons, return the most likely vertex based on MVA likelihood
    StatusCode getVertex(const xAOD::EgammaContainer &egammas, const xAOD::Vertex* &vertex, bool ignoreConv = false) const;

    /// Given a list of photons, return the MLPs of all vertices in the event
    std::vector<std::pair<const xAOD::Vertex*, float> > getVertex(const xAOD::EgammaContainer &egammas, bool ignoreConv = false, bool noDecorate = false, yyVtxType* vtxCase = nullptr, FailType* failType = nullptr) const;

    /// Return the last case treated:
    //  Deprecated no longer use this function
    int getCase() const { return -1; }

    /// Get possible vertex directly associated with photon conversions
    const xAOD::Vertex* getPrimaryVertexFromConv(const xAOD::PhotonContainer *photons) const;

    /// @}

  }; // class PhotonVertexSelectionTool

} // namespace CP


#endif // PhotonVertexSelection_PhotonVertexSelectionTool_h
