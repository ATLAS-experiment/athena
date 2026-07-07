/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Demonstration to install an Example Autoencoder as new a dqm algorithm
// Author: Cary David Randazzo, November 2025, Louisiana Tech University


// Demonstration Note:
// The following is required by all new algorithms.
#include <dqm_algorithms/AutoencoderExampleAlgorithm.h> // Your new algorithm's header
#include <dqm_core/AlgorithmManager.h> // the manager

// Algorithm specific
// This should change depending on your algorithm's implementation.
#include <iostream>
#include <TH2.h>
#include <TClass.h> // Required for line using object.IsA()->InheritsFrom
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <ers/ers.h>
#include "PathResolver/PathResolver.h"



//////////////////////////////////
// DEFINE FUNCTIONS and CLASSES //
//////////////////////////////////
static dqm_algorithms::AutoencoderExampleAlgorithm myInstance;

// Algorithm specific functions
// (Will depend on what you want to do, this extracts histogram data directly from a TH2)

std::vector<std::vector<float>> dqm_algorithms::extract_histogram_data(const TH2* hist) {
    std::vector<std::vector<float>> data;
    int n_bins_x = hist->GetNbinsX();
    int n_bins_y = hist->GetNbinsY();

    data.reserve(n_bins_x * n_bins_y);

    for (int x = 1; x <= n_bins_x; ++x) {
        for (int y = 1; y <= n_bins_y; ++y) {
            data.push_back({static_cast<float>(x-1), static_cast<float>(y-1), static_cast<float>(hist->GetBinContent(x, y))});
        }
    }
    return data;
}


/////////////////////
// Algorithm Setup //
/////////////////////

// Demonstration Note:
// The following is required for all new algorithms.

// Initialize ONNX Runtime in the constructor (efficient).
dqm_algorithms::AutoencoderExampleAlgorithm::AutoencoderExampleAlgorithm() 
: env(ORT_LOGGING_LEVEL_WARNING, "test"), 
session_options()
{

  dqm_core::AlgorithmManager::instance().registerAlgorithm("AutoencoderExampleAlgorithm", this);

  // Demonstration Note:
  // The following code and some code in the execute method utilize the onnx cxx api.
  // Please refer to that documentation as you prepare cxx code for your algorithm.

  // Configure session options before use.
  session_options.SetIntraOpNumThreads(1);

  // Load the model.
  // eos does not have guaranteed availability, so we use PathResolver to find asg-calib/dev for non-production algorithms.
  const std::string model_path = PathResolverFindCalibFile("dev/ONNXfiles/autoencoder_model.onnx");
  session = std::make_unique<Ort::Session>(env, model_path.c_str(), session_options);

  // Prepare and, if desired, print the input node names.
  size_t num_input_nodes = session->GetInputCount();
  Ort::AllocatorWithDefaultOptions allocator;
  for (size_t i = 0; i < num_input_nodes; ++i)
  {
      std::unique_ptr<char, Ort::detail::AllocatedFree> input_name_ptr = session->GetInputNameAllocated(i, allocator);
      char* input_name = input_name_ptr.get();
      std::cout << "Input " << i << ": name=" << input_name << std::endl;
      input_names.push_back(input_name);
  }

  // Prepare and, if desired, print the output node names.
  size_t num_output_nodes = session->GetOutputCount();
  for (size_t i = 0; i < num_output_nodes; ++i)
  {
      std::unique_ptr<char, Ort::detail::AllocatedFree> output_name_ptr = session->GetOutputNameAllocated(i, allocator);
      char* output_name = output_name_ptr.get();
      std::cout << "Output " << i << ": name=" << output_name << std::endl;
      output_names.push_back(output_name);
  }

}


dqm_algorithms::AutoencoderExampleAlgorithm*
dqm_algorithms::AutoencoderExampleAlgorithm::clone()
{
  return new AutoencoderExampleAlgorithm();
}


//////////////////
// Main/Execute //
//////////////////

// Demonstration Note:
// The following is required by all new algorithms.
// "execute" function can be thought of similar main() in your program - implement algorithm logic here.
dqm_core::Result *
dqm_algorithms::AutoencoderExampleAlgorithm::execute(const std::string& name, 
                                               const TObject& object, 
                                               const dqm_core::AlgorithmConfig& )
{
    // Using the TObject object, check if it is of TH2 type, throw error if not the right type.
    // Demonstration Note:
    //  The TObject object can vary depending on what kind of TObject you wish to utilize for your algorithm.
    //  The remainder of the execute method assumes the goals of this particular new algorithm. Modify as needed.
    const TH2* histogram;
    if (object.IsA()->InheritsFrom("TH2")) 
    {
        histogram = dynamic_cast<const TH2*>(&object);
        if (histogram->GetDimension() < 2)
        {
            throw dqm_core::BadConfig( ERS_HERE, name, "dimension of histogram < 2");
        }
    }
    else
    {
        throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TH2" );
    }

    // Extract data from histogram.
    std::vector<std::vector<float>> hist_data = extract_histogram_data(histogram);

    // Run inference for the data using the model.
    std::vector<std::vector<float>> output_data;
    std::vector<int64_t> input_dims = {1, 3}; // Input here is as a single data point with 3 features

    Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
    const char* input_names_cstr[] = { input_names[0].c_str() };
    const char* output_names_cstr[] = { output_names[0].c_str() };
    for (auto& input_data : hist_data) {
        // Create the input tensor.
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(memory_info, input_data.data(), input_data.size(), input_dims.data(), input_dims.size());

        // Run the inference.
        auto output_tensors = session->Run(Ort::RunOptions{nullptr}, input_names_cstr, &input_tensor, 1, output_names_cstr, 1);

        // Check if the std::vector or its output_tensors do or do not have valid values for existence.
        if (output_tensors.empty() || output_tensors[0].HasValue())
        {
            throw dqm_core::BadConfig(ERS_HERE, name, "Output tensor does not have values or does not exist");
        }

        // Check Tensor Size.
        const size_t tensor_size = output_tensors[0].GetTensorTypeAndShapeInfo().GetElementCount();

        if (tensor_size < 3)
        {
             throw dqm_core::BadConfig(ERS_HERE, name, "Output tensor < 3 elements");
        }

        // Prepare the output_data.
        float* raw_output = output_tensors[0].GetTensorMutableData<float>();
        std::vector<float> output_point(raw_output, raw_output + 3);
        output_data.push_back(output_point);
    }

    // Compute the reconstruction error.
    //  (Algorithm specific logic, post inference)
    std::vector<float> reconstruction_errors;
    for (size_t i = 0; i < hist_data.size(); ++i) {
        float error = 0.0;
        for (size_t j = 0; j < 3; ++j) {
            error += std::pow(hist_data[i][j] - output_data[i][j], 2);
        }
        reconstruction_errors.push_back(std::sqrt(error));
    }

    // Calculate threshold based on the 99th percentile of reconstruction errors
    //  (Algorithm specific logic, post inference)
    std::nth_element(reconstruction_errors.begin(), reconstruction_errors.begin() + std::roundl(reconstruction_errors.size() * (99. / 100.)), reconstruction_errors.end());
    float threshold = reconstruction_errors[reconstruction_errors.size() * 99 / 100];

    // Identify anomalies based on threshold vs reconstruction_error.
    //  (Algorithm specific logic, post inference)
    std::vector<bool> anomalies;
    for (const auto& error : reconstruction_errors) {
        anomalies.push_back(error > threshold);
    }

    // Demonstration Note:
    //  There are a lot of variations on how the result of the dqm_algorithm including how it interacts with the test display.
    //  I suggest looking at least at other algorithms for inspiration.
    //  The documentation and examples of that is outside of scope of this template.
    //  However, this template demonstrates at least the minimum required to setup the new algorithm and on the test display
    //  with perhaps some information of interest to this particular new algorithm.

    // Prepare the resulthisto
    TH2* resulthisto;
    if (histogram->InheritsFrom("TH2"))
    {
        resulthisto=static_cast<TH2*>(histogram->Clone());
    }
    else
    {
        throw dqm_core::BadConfig( ERS_HERE, name, "does not inherit from TH2" );
    }
    resulthisto->Reset();

    // Loop through anomalies to set result, print, etc.
    std::cout << "Anomalies:" << std::endl;
    for (size_t i = 0; i < anomalies.size(); ++i) {
        if (anomalies[i])
        {
            // Set the resulting histogram to the bin content
            resulthisto->SetBinContent(hist_data[i][0], hist_data[i][1], hist_data[i][2]);
            std::cout << "Input (" << hist_data[i][0] << ", " << hist_data[i][1] << ", " <<  hist_data[i][2] << "): ";
            std::cout << "Output (" << output_data[i][0] << ", " << output_data[i][1] << ", " << output_data[i][2] << ")";
            std::cout << " Reconstruction Error: " << reconstruction_errors[i] << std::endl;
        }
    }

    // Prepare the result of the main/execute method
    dqm_core::Result* result = new dqm_core::Result();

    // For the result, set NBins to the number of anomalies
    result->tags_["NBins"] = anomalies.size();

    // For the result, set the resulting histogram to the bin content we SetBinContent above with
    result->object_ =  boost::shared_ptr<TObject>(resulthisto);

    // Set the thresholds
    double gthreshold = 5000;
    double rthreshold = 10000;

    // Determine if the overall histogram is green, yellow, or red and send that to the result
    if (anomalies.size() <= gthreshold)
    {
        result->status_ = dqm_core::Result::Green;
    }
    else if (anomalies.size() < rthreshold)
    {
        result->status_ = dqm_core::Result::Yellow;
    }
    else
    {
        result->status_ = dqm_core::Result::Red;
    }

    // For debugging, remove when done
    //dqm_core::Result *result = new dqm_core::Result(dqm_core::Result::Undefined);
    return result;
}

// Warning:
//  This algorithm is not production ready for anomaly detection or autoencoder specific work.
//  The errors are related to the results and calculations relative to the chosen histogram
//  and are irrelevant for the pipeline itself.
//  This example serves as an "example only" (AutoencoderExampleALG) for installing a new algorithm.

// Demonstration Note:
//  The following is required for all new algorithms.
void
dqm_algorithms::AutoencoderExampleAlgorithm::printDescription(std::ostream& out)
{
    out<<"AutoencoderExampleAlgorithm: Load and process an onnx autoencoder model given the AutoencoderExampleAlgorithm.cxx "<<std::endl;
}
