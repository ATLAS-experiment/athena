/*
  Copyright (C) 2002-2026 for the benefit of the ATLAS collaboration
*/

// Pipeline to install an Example Autoencoder as new a dqm algorithm
// Author: Cary David Randazzo, November 2025, Louisiana Tech University

// Demonstration Note:
// The following is required by all new algorithms.
// The following two lines must be changed to MyAlgorithm_H
#ifndef AutoencoderExampleAlgorithm_H
#define AutoencoderExampleAlgorithm_H
#include <dqm_core/Algorithm.h>

// Required if using onnx framework for your model.
#include <onnxruntime_cxx_api.h>

// Algorithm specific:
// Add the includes necessary for your algorithm.
#include <string>
#include <vector>
#include <iosfwd>
#include <memory>


// Demonstration Note:
//  The method below for extracting data is specific to the algorithm that was created for the example.
//  What data you will work with and how its extracted and manipulated is up to you in the code.
class TH2;
class TObject;

// Demonstration Note:
// The following is required by all new algorithms.
namespace dqm_algorithms
{
    std::vector<std::vector<float>> extract_histogram_data(const TH2* hist);

    struct AutoencoderExampleAlgorithm : public dqm_core::Algorithm
    {
        AutoencoderExampleAlgorithm();

        virtual ~AutoencoderExampleAlgorithm() override = default;

        virtual AutoencoderExampleAlgorithm* clone() override;

        virtual dqm_core::Result * execute( const std::string & , const TObject & , const dqm_core::AlgorithmConfig & ) override;

        using dqm_core::Algorithm::printDescription;
        void  printDescription(std::ostream& out);


        Ort::Env env;
        Ort::SessionOptions session_options;
        std::unique_ptr<Ort::Session> session;
        std::vector<std::string> input_names;
        std::vector<std::string> output_names;
    };

}

// Demonstration Note:
// The following is required by all new algorithms. (The comment shows what the condition is closing, can be removed.)
#endif // AutoencoderExampleAlgorithm_H
