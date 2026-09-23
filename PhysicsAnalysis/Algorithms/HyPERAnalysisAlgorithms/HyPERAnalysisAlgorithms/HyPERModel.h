/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef HYPERANALYSISALGORITHMS_HYPERMODEL_H
#define HYPERANALYSISALGORITHMS_HYPERMODEL_H

#include <AsgTools/MessageCheckAsgTools.h>  // To access ANA_MSG
#include <math.h>

#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "AthOnnxInterfaces/IOnnxRuntimeInferenceTool.h"
#include "AthOnnxUtils/OnnxUtils.h"

namespace EventReco {
// @brief The enum class that holds the different topologies that the HyPER
// model can make predictions on.
enum class HyPERTopology {
  NotSelected = 0,
  TtbarLJets,
  TtbarAllHadronic,
  TtbarDiLepton,
  NumberOfTopologies
};

/**
 * @brief Which of the graph's dynamic dimensions an output tensor scales with.
 * The HyPER models declare every output with exactly one symbolic dimension,
 * tied either to the number of hyperedges or to the number of edges.
 */
enum class HyPEROutputDim { HyperEdges, Edges, Single };

/**
 * @struct HyPEROutputNode
 * @brief Description of a single output node of the ONNX graph.
 * Every output must be declared, including the ones HyPER does not consume,
 * because ONNX Runtime binds outputs by position against the model's own
 * node list.
 */
struct HyPEROutputNode {
  std::string name;
  bool isFloat;        ///< false means int64
  HyPEROutputDim dim;  ///< which dynamic dimension the tensor scales with
  bool trailingOne;    ///< shape is {N, 1} rather than {N}
};

// @class HyPERModel
// @brief Adapter around the Athena ONNX inference tools (AthOnnx) for the
// HyPER models. Two tools are held, one per cross-validation fold, and the
// caller picks between them per event. Inputs are bound by node name and
// handed to ONNX Runtime in the order the graph declares them, which is what
// the tensor-level AthOnnx interface requires.
class HyPERModel {
 public:
  HyPERModel(const AthOnnx::IOnnxRuntimeInferenceTool* trainedOnEven,
             const AthOnnx::IOnnxRuntimeInferenceTool* trainedOnOdd,
             HyPERTopology topology)
      : m_topology(topology),
        m_toolTrainedOnEven(trainedOnEven),
        m_toolTrainedOnOdd(trainedOnOdd) {}

  virtual ~HyPERModel() = default;

  /**
   * @brief Resolve the node name -> node index maps. Must be called once
   * before the first evaluate(). This cannot live in the constructor because
   * it relies on the topology-specific virtual methods below.
   */
  StatusCode initialize() {
    using namespace asg::msgUserCode;
    if (!m_toolTrainedOnEven || !m_toolTrainedOnOdd) {
      ANA_MSG_ERROR("HyPERModel: ONNX inference tools have not been set");
      return StatusCode::FAILURE;
    }

    const std::vector<std::string> inputNames = getInputNames();
    m_inputIndex.clear();
    for (std::size_t i = 0; i < inputNames.size(); ++i) {
      m_inputIndex[inputNames[i]] = i;
    }
    m_boundInputs.assign(inputNames.size(), BoundInput{});

    m_outputNodes = getModelOutputs();
    m_outputIndex.clear();
    for (std::size_t i = 0; i < m_outputNodes.size(); ++i) {
      m_outputIndex[m_outputNodes[i].name] = i;
    }
    m_outputsFloat.assign(m_outputNodes.size(), std::vector<float>{});
    m_outputsInt64.assign(m_outputNodes.size(), std::vector<int64_t>{});

    // The dynamic output dimensions are read back off these two inputs.
    if (m_inputIndex.find(s_edgeIndexName) == m_inputIndex.end() ||
        m_inputIndex.find(s_hyperEdgeIndexName) == m_inputIndex.end()) {
      ANA_MSG_ERROR("HyPERModel: the input names must contain '"
                    << s_edgeIndexName << "' and '" << s_hyperEdgeIndexName
                    << "'");
      return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
  }

  void setTopology(HyPERTopology topology) =
      delete;  // Not able to change the topology after construction
  HyPERTopology getTopology() const { return m_topology; }

  /**
   * @brief Input node names in the order the ONNX graph declares them.
   * The order matters: ONNX Runtime is handed the tensors positionally.
   */
  virtual std::vector<std::string> getInputNames() const = 0;

  /**
   * @brief Every output node of the ONNX graph, in declaration order.
   */
  virtual std::vector<HyPEROutputNode> getModelOutputs() const = 0;

  /**
   * @brief Model-specific output names consumed by the parser.
   * Output names vary according to the number of message-passing layers etc.
   * This must be defined for each and every topology.
   * @return std::vector<std::string> The output names of the model
   */
  virtual std::vector<std::string> getOutputNames() const = 0;

  /**
   * @brief Bind an input by node name. The tensor aliases @p values, so the
   * caller must keep it alive and unmodified until after evaluate().
   */
  template <typename T>
  void setInputs(const std::string& node, std::vector<T>& values,
                 const std::vector<int64_t>& shape) {
    using namespace asg::msgUserCode;
    static_assert(std::is_same_v<T, float> || std::is_same_v<T, int64_t>,
                  "HyPER ONNX inputs must be float or int64_t");
    const auto it = m_inputIndex.find(node);
    if (it == m_inputIndex.end()) {
      ANA_MSG_ERROR("HyPERModel: unknown input node '" << node << "'");
      return;
    }
    BoundInput& bound = m_boundInputs[it->second];
    bound.data = &values;
    bound.shape = shape;
    bound.set = true;
  }

  /**
   * @brief Run the model of the requested fold over the bound inputs.
   * @param sessionIndex 0 for the model trained on even events, 1 for odd.
   */
  StatusCode evaluate(unsigned sessionIndex) {
    using namespace asg::msgUserCode;

    const AthOnnx::IOnnxRuntimeInferenceTool* tool =
        (sessionIndex == 0) ? m_toolTrainedOnEven : m_toolTrainedOnOdd;

    // Inputs, in the order the ONNX graph declares them.
    std::vector<Ort::Value> inputTensors;
    inputTensors.reserve(m_boundInputs.size());
    for (std::size_t i = 0; i < m_boundInputs.size(); ++i) {
      BoundInput& bound = m_boundInputs[i];
      if (!bound.set) {
        ANA_MSG_ERROR("HyPERModel: input node " << i << " was never set");
        return StatusCode::FAILURE;
      }
      if (std::holds_alternative<std::vector<float>*>(bound.data)) {
        inputTensors.push_back(AthOnnxUtils::createTensor(
            *std::get<std::vector<float>*>(bound.data), bound.shape));
      } else {
        inputTensors.push_back(AthOnnxUtils::createTensor(
            *std::get<std::vector<int64_t>*>(bound.data), bound.shape));
      }
    }

    // The graph's dynamic dimensions, taken from the shapes of the index
    // tensors: edge_index is {2, nEdges} and edge_index_h is {order, nHyper}.
    const int64_t nEdges =
        m_boundInputs[m_inputIndex.at(s_edgeIndexName)].shape.back();
    const int64_t nHyperEdges =
        m_boundInputs[m_inputIndex.at(s_hyperEdgeIndexName)].shape.back();

    // Outputs, also in declaration order. ONNX Runtime writes straight into
    // the buffers owned here, which getOutputs() then hands to the parser.
    std::vector<Ort::Value> outputTensors;
    outputTensors.reserve(m_outputNodes.size());
    for (std::size_t i = 0; i < m_outputNodes.size(); ++i) {
      const HyPEROutputNode& node = m_outputNodes[i];
      int64_t leading = 1;
      switch (node.dim) {
        case HyPEROutputDim::HyperEdges: leading = nHyperEdges; break;
        case HyPEROutputDim::Edges:      leading = nEdges;      break;
        case HyPEROutputDim::Single:     leading = 1;           break;
      }
      std::vector<int64_t> shape{leading};
      if (node.trailingOne) shape.push_back(1);

      const int64_t size = AthOnnxUtils::getTensorSize(shape);
      if (node.isFloat) {
        m_outputsFloat[i].assign(static_cast<std::size_t>(size), 0.f);
        outputTensors.push_back(
            AthOnnxUtils::createTensor(m_outputsFloat[i], shape));
      } else {
        m_outputsInt64[i].assign(static_cast<std::size_t>(size), 0);
        outputTensors.push_back(
            AthOnnxUtils::createTensor(m_outputsInt64[i], shape));
      }
    }

    return tool->inference(inputTensors, outputTensors);
  }

  /**
   * @brief Pointer to the buffer of the named output. Valid until the next
   * clearOutputs() or evaluate().
   */
  template <typename T>
  T* getOutputs(const std::string& node) {
    using namespace asg::msgUserCode;
    static_assert(std::is_same_v<T, float> || std::is_same_v<T, int64_t>,
                  "HyPER ONNX outputs must be float or int64_t");
    const auto it = m_outputIndex.find(node);
    if (it == m_outputIndex.end()) {
      ANA_MSG_ERROR("HyPERModel: unknown output node '" << node << "'");
      return nullptr;
    }
    if constexpr (std::is_same_v<T, float>) {
      return m_outputsFloat[it->second].data();
    } else {
      return m_outputsInt64[it->second].data();
    }
  }

  void clearInputs() {
    for (BoundInput& bound : m_boundInputs) bound = BoundInput{};
  }

  void clearOutputs() {
    for (std::vector<float>& out : m_outputsFloat) out.clear();
    for (std::vector<int64_t>& out : m_outputsInt64) out.clear();
  }

  void printInputInfo(bool printContent = false) const {
    using namespace asg::msgUserCode;
    const std::vector<std::string> names = getInputNames();
    for (std::size_t i = 0; i < m_boundInputs.size(); ++i) {
      const BoundInput& bound = m_boundInputs[i];
      ANA_MSG_INFO("  input " << i << " '" << names[i]
                              << "' shape=" << shapeToString(bound.shape));
      if (!printContent || !bound.set) continue;
      if (std::holds_alternative<std::vector<float>*>(bound.data)) {
        ANA_MSG_INFO("    " << contentToString(
                         *std::get<std::vector<float>*>(bound.data)));
      } else {
        ANA_MSG_INFO("    " << contentToString(
                         *std::get<std::vector<int64_t>*>(bound.data)));
      }
    }
  }

  void printOutputInfo(bool printContent = false) const {
    using namespace asg::msgUserCode;
    for (std::size_t i = 0; i < m_outputNodes.size(); ++i) {
      ANA_MSG_INFO("  output " << i << " '" << m_outputNodes[i].name << "'");
      if (!printContent) continue;
      if (m_outputNodes[i].isFloat) {
        ANA_MSG_INFO("    " << contentToString(m_outputsFloat[i]));
      } else {
        ANA_MSG_INFO("    " << contentToString(m_outputsInt64[i]));
      }
    }
  }

 protected:
  HyPERTopology m_topology{HyPERTopology::NotSelected};

 private:
  /// Input nodes whose shapes carry the graph's dynamic dimensions. These are
  /// named identically in every HyPER topology.
  static constexpr const char* s_edgeIndexName = "edge_index";
  static constexpr const char* s_hyperEdgeIndexName = "edge_index_h";

  using InputData = std::variant<std::vector<float>*, std::vector<int64_t>*>;
  struct BoundInput {
    InputData data{static_cast<std::vector<float>*>(nullptr)};
    std::vector<int64_t> shape{};
    bool set{false};
  };

  template <typename T>
  static std::string contentToString(const std::vector<T>& values) {
    std::string row = "[";
    for (const T& value : values) row += std::to_string(value) + ", ";
    return row + "]";
  }

  static std::string shapeToString(const std::vector<int64_t>& shape) {
    std::string row = "(";
    for (const int64_t dim : shape) row += std::to_string(dim) + ", ";
    return row + ")";
  }

  const AthOnnx::IOnnxRuntimeInferenceTool* m_toolTrainedOnEven{nullptr};
  const AthOnnx::IOnnxRuntimeInferenceTool* m_toolTrainedOnOdd{nullptr};

  std::map<std::string, std::size_t> m_inputIndex{};
  std::vector<BoundInput> m_boundInputs{};

  std::vector<HyPEROutputNode> m_outputNodes{};
  std::map<std::string, std::size_t> m_outputIndex{};
  std::vector<std::vector<float>> m_outputsFloat{};
  std::vector<std::vector<int64_t>> m_outputsInt64{};
};
}  // namespace EventReco

#endif  // HYPERANALYSISALGORITHMS_HYPERMODEL_H
