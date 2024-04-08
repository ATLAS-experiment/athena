/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// test interface for a wrapper module that can open various kinds of neural
// network graph file

// include things to test
#include "ISF_FastCaloSimEvent/TFCSGANLWTNNHandler.h"
#include "ISF_FastCaloSimEvent/TFCSSimpleLWTNNHandler.h"
#include "ISF_FastCaloSimEvent/TFCSONNXHandler.h"

// include things to allow us to check the streamers
#include "TFile.h"

// include generic utilities
#include "ISF_FastCaloSimEvent/MLogging.h"
#include <iostream>
#include <fstream>
#include <vector>

using ISF_FCS::MLogging;

// Very crude way to make a file for input
// writes a fake lwtnn GAN to the disk at the
// specified file name
void setup_fastCaloGAN(const std::string& outputFile) {
  std::ofstream output;
  output.open(outputFile);

  const char *text = "{\n"
                     "  \"defaults\": {},\n"
                     "  \"inputs\": [\n"
                     "    {\n"
                     "      \"name\": \"node_1\",\n"
                     "      \"offset\": 0,\n"
                     "      \"scale\": 1\n"
                     "    },\n"
                     "    {\n"
                     "      \"name\": \"node_2\",\n"
                     "      \"offset\": 0,\n"
                     "      \"scale\": 1\n"
                     "    }\n"
                     "  ],\n"
                     "  \"layers\": [\n"
                     "    {\n"
                     "      \"activation\": \"rectified\",\n"
                     "      \"architecture\": \"dense\",\n"
                     "      \"bias\": [\n"
                     "        1.0,\n"
                     "        0.0\n"
                     "      ],\n"
                     "      \"weights\": [\n"
                     "        1.0,\n"
                     "        0.5,\n"
                     "        0.5,\n"
                     "        0.0\n"
                     "      ]\n"
                     "    }\n"
                     "  ],\n"
                     "  \"outputs\": [\n"
                     "    \"EXTRAPWEIGHT_0\",\n"
                     "    \"EXTRAPWEIGHT_1\"\n"
                     "  ]\n"
                     "}\n";
  output << text;

  output.close();
}

// ditto for Sim
void setup_fastCaloSim(const std::string& outputFile) {
  std::ofstream output;
  output.open(outputFile);

  const char *text = "{\n"
                     "  \"input_sequences\": [],\n"
                     "  \"inputs\": [\n"
                     "    {\n"
                     "      \"name\": \"node_0\",\n"
                     "      \"variables\": [\n"
                     "        {\n"
                     "          \"name\": \"0\",\n"
                     "          \"offset\": 0,\n"
                     "          \"scale\": 1\n"
                     "        }\n"
                     "      ]\n"
                     "    },\n"
                     "    {\n"
                     "      \"name\": \"node_1\",\n"
                     "      \"variables\": [\n"
                     "        {\n"
                     "          \"name\": \"0\",\n"
                     "          \"offset\": 0,\n"
                     "          \"scale\": 1\n"
                     "        },\n"
                     "        {\n"
                     "          \"name\": \"1\",\n"
                     "          \"offset\": 0,\n"
                     "          \"scale\": 1\n"
                     "        }\n"
                     "      ]\n"
                     "    }\n"
                     "  ],\n"
                     "  \"layers\": [\n"
                     "    {\n"
                     "      \"activation\": \"rectified\",\n"
                     "      \"architecture\": \"dense\",\n"
                     "      \"bias\": [\n"
                     "        0.1,\n"
                     "        0.2,\n"
                     "        0.3\n"
                     "      ],\n"
                     "      \"weights\": [\n"
                     "        0.1,\n"
                     "        0.2,\n"
                     "        0.3,\n"
                     "        0.4,\n"
                     "        0.5,\n"
                     "        0.6,\n"
                     "        0.7,\n"
                     "        0.8,\n"
                     "        0.9\n"
                     "      ]\n"
                     "    }\n"
                     "  ],\n"
                     "  \"nodes\": [\n"
                     "    {\n"
                     "      \"size\": 1,\n"
                     "      \"sources\": [\n"
                     "        0\n"
                     "      ],\n"
                     "      \"type\": \"input\"\n"
                     "    },\n"
                     "    {\n"
                     "      \"size\": 2,\n"
                     "      \"sources\": [\n"
                     "        1\n"
                     "      ],\n"
                     "      \"type\": \"input\"\n"
                     "    },\n"
                     "    {\n"
                     "      \"sources\": [\n"
                     "        0,\n"
                     "        1\n"
                     "      ],\n"
                     "      \"type\": \"concatenate\"\n"
                     "    },\n"
                     "    {\n"
                     "      \"layer_index\": 0,\n"
                     "      \"sources\": [\n"
                     "        2\n"
                     "      ],\n"
                     "      \"type\": \"feed_forward\"\n"
                     "    }\n"
                     "  ],\n"
                     "  \"outputs\": {\n"
                     "    \"output_layer\": {\n"
                     "      \"labels\": [\n"
                     "        \"out_0\",\n"
                     "        \"out_1\",\n"
                     "        \"out_2\"\n"
                     "      ],\n"
                     "      \"node_index\": 3\n"
                     "    }\n"
                     "  }\n"
                     "}\n";
  output << text;

  output.close();
}

void test_fastCaloGAN(const std::string& inputFile, const ISF_FCS::MLogging& logger) {
  ATH_MSG_NOCLASS(logger, "Testing fastCaloGAN format.");
  TFCSSimpleLWTNNHandler my_net(inputFile);

  // Fake inputs
  std::map<std::string, double> input_nodes;
  for (int node = 1; node < 3; node++) {
    input_nodes["node_" + std::to_string(node)] = node;
  }
  decltype(my_net)::NetworkInputs inputs;
  inputs["inputs"] = input_nodes;
  ATH_MSG_NOCLASS(logger, VNetworkBase::representNetworkInputs(inputs, 100));

  // run the net
  decltype(my_net)::NetworkOutputs outputs = my_net.compute(inputs);

  ATH_MSG_NOCLASS(logger, VNetworkBase::representNetworkOutputs(outputs, 100));

  // Save the net to root file
  std::string output_root_file = "with_lwtnn_network.root";
  ATH_MSG_NOCLASS(logger, "Writing to a root file; " << output_root_file);
  my_net.writeNetToTTree(output_root_file);

  ATH_MSG_NOCLASS(logger, "Reading copy written to root");
  TFCSSimpleLWTNNHandler copy_net(output_root_file);
  ATH_MSG_NOCLASS(logger, "Running copy from root file");
  decltype(copy_net)::NetworkOutputs other_out = copy_net.compute(inputs);
  ATH_MSG_NOCLASS(logger,
                  VNetworkBase::representNetworkOutputs(other_out, 100));

  ATH_MSG_NOCLASS(logger,
                  "Outputs should before and after writing shoud be identical");

  // Finally, save the network using a streamer.
  ATH_MSG_NOCLASS(logger, "Writing with a streamer to; " << output_root_file);
  TFile test_stream_write(output_root_file.c_str(), "RECREATE");
  test_stream_write.WriteObjectAny(&my_net, "TFCSSimpleLWTNNHandler",
                                   "test_net");
  test_stream_write.Close();

  ATH_MSG_NOCLASS(logger, "Reading streamer copy written to root");
  TFile test_stream_read(output_root_file.c_str(), "READ");
  TFCSSimpleLWTNNHandler *streamed_net =
      test_stream_read.Get<TFCSSimpleLWTNNHandler>("test_net");

  ATH_MSG_NOCLASS(logger, "Running copy streamed from root file");
  TFCSSimpleLWTNNHandler::NetworkOutputs streamed_out =
      streamed_net->compute(inputs);
  ATH_MSG_NOCLASS(logger,
                  VNetworkBase::representNetworkOutputs(streamed_out, 100));
  ATH_MSG_NOCLASS(logger,
                  "Outputs should before and after writing shoud be identical");
}

void test_fastCaloSim(const std::string& inputFile, const ISF_FCS::MLogging& logger) {
  ATH_MSG_NOCLASS(logger, "Testing fastCaloSim format.");
  TFCSGANLWTNNHandler my_net(inputFile);
  ATH_MSG_NOCLASS(logger, "Made the net.");

  // Fake inputs
  decltype(my_net)::NetworkInputs inputs;
  for (int node = 0; node < 2; node++) {
    std::map<std::string, double> node_input;
    for (int i = 0; i <= node; i++) {
      node_input[std::to_string(i)] = i;
    }
    inputs["node_" + std::to_string(node)] = node_input;
  }
  ATH_MSG_NOCLASS(logger, "Made the inputs.");
  ATH_MSG_NOCLASS(logger, VNetworkBase::representNetworkInputs(inputs, 100));

  // run the net
  decltype(my_net)::NetworkOutputs outputs = my_net.compute(inputs);

  ATH_MSG_NOCLASS(logger, VNetworkBase::representNetworkOutputs(outputs, 100));
  // Save the net to root file
  std::string output_root_file = "with_lwtnn_graph.root";
  ATH_MSG_NOCLASS(logger, "Writing to a root file; " << output_root_file);
  my_net.writeNetToTTree(output_root_file);

  ATH_MSG_NOCLASS(logger, "Reading copy written to root");
  TFCSGANLWTNNHandler copy_net(output_root_file);
  ATH_MSG_NOCLASS(logger, "Running copy from root file");
  decltype(copy_net)::NetworkOutputs other_out = copy_net.compute(inputs);
  ATH_MSG_NOCLASS(logger,
                  VNetworkBase::representNetworkOutputs(other_out, 100));
  ATH_MSG_NOCLASS(logger,
                  "Outputs should before and after writing shoud be identical");

  // Finally, save the network using a streamer.
  ATH_MSG_NOCLASS(logger, "Writing with a streamer to; " << output_root_file);
  TFile test_stream_write(output_root_file.c_str(), "RECREATE");
  test_stream_write.WriteObjectAny(&my_net, "TFCSGANLWTNNHandler", "test_net");
  test_stream_write.Close();

  ATH_MSG_NOCLASS(logger, "Reading streamer copy written to root");
  TFile test_stream_read(output_root_file.c_str(), "READ");
  TFCSGANLWTNNHandler *streamed_net =
      test_stream_read.Get<TFCSGANLWTNNHandler>("test_net");

  ATH_MSG_NOCLASS(logger, "Running copy streamed from root file");
  TFCSGANLWTNNHandler::NetworkOutputs streamed_out =
      streamed_net->compute(inputs);
  ATH_MSG_NOCLASS(logger,
                  VNetworkBase::representNetworkOutputs(streamed_out, 100));
  ATH_MSG_NOCLASS(logger,
                  "Outputs should before and after writing shoud be identical");
}

void test_ONNX(const ISF_FCS::MLogging& logger) {
  // Curiously, there is no easy way to generate an ONNX
  // model from c++. It is expected you will convert an
  // existing model, so creating one here would require
  // additional imports.
  std::string this_file = __FILE__;
  std::string parent_dir = this_file.substr(0, this_file.find("/test/"));
  std::string inputFile = parent_dir + "/share/toy_network.onnx";
  // Only proceed if that file can be read.
  std::ifstream onnx_file(inputFile);
  if (onnx_file.good()) {
    // Read form regular onnx file
    ATH_MSG_NOCLASS(logger, "Testing ONNX format.");
    TFCSONNXHandler my_net(inputFile);

    // Fake inputs
    decltype(my_net)::NetworkInputs inputs;
    std::map<std::string, double> node_input;
    for (int i = 0; i < 2; i++) {
      node_input[std::to_string(i)] = i + 1;
    }
    inputs["inputs"] = node_input;
    ATH_MSG_NOCLASS(logger, VNetworkBase::representNetworkInputs(inputs, 100));

    // run the net
    decltype(my_net)::NetworkOutputs outputs = my_net.compute(inputs);
    ATH_MSG_NOCLASS(logger,
                    VNetworkBase::representNetworkOutputs(outputs, 100));

    // Write a copy to a root file
    std::string output_root_file = "with_serialized_network.root";
    ATH_MSG_NOCLASS(logger, "Writing to a root file; " << output_root_file);
    my_net.writeNetToTTree(output_root_file);

    ATH_MSG_NOCLASS(logger, "Reading copy written to root");
    TFCSONNXHandler copy_net(output_root_file);

    ATH_MSG_NOCLASS(logger, "Running copy from root file");
    decltype(copy_net)::NetworkOutputs other_out = copy_net.compute(inputs);
    ATH_MSG_NOCLASS(logger,
                    VNetworkBase::representNetworkOutputs(other_out, 100));

    ATH_MSG_NOCLASS(
        logger, "Outputs should before and after writing shoud be identical");

    // Finally, save the network using a streamer.
    ATH_MSG_NOCLASS(logger, "Writing with a streamer to; " << output_root_file);
    TFile test_stream_write(output_root_file.c_str(), "RECREATE");
    test_stream_write.WriteObjectAny(&my_net, "TFCSONNXHandler", "test_net");
    test_stream_write.Close();

    ATH_MSG_NOCLASS(logger, "Reading streamer copy written to root");
    TFile test_stream_read(output_root_file.c_str(), "READ");
    TFCSONNXHandler *streamed_net =
        test_stream_read.Get<TFCSONNXHandler>("test_net");

    ATH_MSG_NOCLASS(logger, "Running copy streamed from root file");
    TFCSONNXHandler::NetworkOutputs streamed_out =
        streamed_net->compute(inputs);
    ATH_MSG_NOCLASS(logger,
                    VNetworkBase::representNetworkOutputs(streamed_out));
    ATH_MSG_NOCLASS(
        logger, "Outputs should before and after writing shoud be identical");

  } else {
    ATH_MSG_NOCLASS(logger, "Couldn't read file "
                                << inputFile << "\n Will skip ONNX tests.");
  }
}

int main() {
  ISF_FCS::MLogging logger;
  std::string gan_data_example("example_data_gan.json");
  setup_fastCaloGAN(gan_data_example);
  test_fastCaloGAN(gan_data_example, logger);

  std::string sim_data_example("example_data_sim.json");
  setup_fastCaloSim(sim_data_example);
  test_fastCaloSim(sim_data_example, logger);
  
  // For some reason this freezes...
  //test_ONNX(logger);
  ATH_MSG_NOCLASS(logger, "Program ends");
  return 0;
}
