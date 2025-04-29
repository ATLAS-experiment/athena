
#include <vector>
#include <map>
#include <Eigen/Dense>
#include <onnxruntime_cxx_api.h>
#include <TString.h>

using NetworkBatchInput =
    Eigen::Matrix<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

// General class that sets up the ONNX runtime framework for loading a ML model
// and using it for inference.
class OnnxRuntimeBase {
    public:        
        // int m_totalInputs;

        TString m_fileName;
        // int n_hits;
        // std::string m_trackType;
        // std::string m_order;
        // bool m_scaled;

        OnnxRuntimeBase(TString fileName);
        OnnxRuntimeBase();
        ~OnnxRuntimeBase(){}
        void initialize(TString);

        std::vector<float> runONNXInference(std::vector<float>& inputTensorValues) const;
        std::vector<std::vector<float>> runONNXInference(NetworkBatchInput& inputTensorValues) const;
        std::map<int, Eigen::MatrixXf> runONNXInferenceMultilayerOutput(NetworkBatchInput& inputTensorValues) const;

        const std::vector<int64_t>& getInputNodesDims(){return m_inputNodeDims;};
        const std::vector<int64_t>& getOutputNodesDims(){return m_outputNodeDims;};

    private:
        /// ONNX runtime session / model properties
        std::unique_ptr<Ort::Session> m_session;

        std::vector<const char*> m_inputNodeNames;
        std::vector<int64_t> m_inputNodeDims;
        std::vector<const char*> m_outputNodeNames;
        std::vector<int64_t> m_outputNodeDims;

        std::unique_ptr< Ort::Env > m_env;

};

