/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <onnxruntime_cxx_api.h>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <cstdint>

void pretty_print_table(
  const std::vector<std::string>& names,
  const std::vector<ONNXTensorElementDataType>& types,
  const std::vector<std::vector<int64_t>>& shapes
){
  size_t max_length_name = 0;
  for (const auto& name : names) {
    max_length_name = std::max(max_length_name, name.length());
  }

  // Define the mapping of enum values to string representations
  static const std::unordered_map<ONNXTensorElementDataType, std::string> typeMap = {
      {ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT, "float"},
      {ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_DOUBLE, "double"},
      {ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT8, "int8"},
      {ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT16, "int16"},
      {ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT32, "int32"},
      {ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_INT64, "int64"},
      {ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT8, "uint8"},
      {ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT16, "uint16"},
      {ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT32, "uint32"},
      {ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_UINT64, "uint64"},
      {ONNXTensorElementDataType::ONNX_TENSOR_ELEMENT_DATA_TYPE_BOOL, "bool"},
  };

  // format the shape as a string
  size_t max_length_shape = 0;
  std::vector<std::string> shape_strs;
  for (const auto& shape: shapes) {
    std::string shape_str = "[";
    for (size_t j = 0; j < shape.size(); ++j) {
      shape_str += std::to_string(shape[j]);
      if (j != shape.size() - 1) {
        shape_str += ", ";
      }
    }
    shape_str += "]";
    size_t l = shape_str.length();
    shape_strs.push_back(std::move(shape_str));
    max_length_shape = std::max(max_length_shape, l);
  }

  int line_length = max_length_name + 4 + 10 + 3 + max_length_shape;
  std::string h_line(line_length, '-');
  std::cout << h_line << std::endl;

  // header
  std::ios_base::fmtflags f( std::cout.flags() ); //save cout format flags
  std::cout << std::left << std::setw(max_length_name + 4) << " name";
  std::cout << std::setw(10) << "type";
  std::cout << "shape" << std::endl;

  std::cout << h_line << std::endl;

  for (size_t i = 0; i < names.size(); i++) {
    std::cout << std::left << std::setw(max_length_name + 4) << " " + names.at(i);
    std::cout << std::setw(10) << typeMap.at(types.at(i));
    std::cout << shape_strs.at(i) << std::endl;
  }
  std::cout << h_line << std::endl;
  std::cout.flags( f );//restore format
}



int main(int narg, char* argv[]) {
  if (narg != 3 && narg != 2) {
    std::cout << "usage: " << argv[0] << " <onnx_file> [key]" << std::endl;
    return 1;
  }

  //load the onnx model to memory using the path
  auto env = std::make_unique< Ort::Env >(ORT_LOGGING_LEVEL_ERROR, "");

  // initialize session options if needed
  Ort::SessionOptions session_options;
  session_options.SetIntraOpNumThreads(1);
  session_options.SetGraphOptimizationLevel(
    GraphOptimizationLevel::ORT_DISABLE_ALL);

  // create session and load model into memory
  auto session = std::make_unique< Ort::Session >(*env, argv[1],
                                                session_options);

  // cout the input nodes
  size_t num_input_nodes = session->GetInputCount();
  std::vector<std::string> names;
  std::vector<ONNXTensorElementDataType> types;
  std::vector<std::vector<int64_t>> shapes;

  for (std::size_t i = 0; i < num_input_nodes; i++) {
    char* input_name = session->GetInputNameAllocated(i, Ort::AllocatorWithDefaultOptions()).release();
    names.push_back(input_name);

    // get input type and shape
    Ort::TypeInfo type_info = session->GetInputTypeInfo(i);
    auto tensor_info = type_info.GetTensorTypeAndShapeInfo();

    types.push_back(tensor_info.GetElementType());
    shapes.push_back(tensor_info.GetShape());
  }

  std::cout << std::endl << "input nodes: " << std::endl;
  pretty_print_table(names, types, shapes);


  // cout the output nodes
  size_t num_output_nodes = session->GetOutputCount();
  names.clear(); types.clear(); shapes.clear();

  for (std::size_t i = 0; i < num_output_nodes; i++) {
    char* output_name = session->GetOutputNameAllocated(i, Ort::AllocatorWithDefaultOptions()).release();
    names.push_back(output_name);

    // get input type and shape
    Ort::TypeInfo type_info = session->GetOutputTypeInfo(i);
    auto tensor_info = type_info.GetTensorTypeAndShapeInfo();

    types.push_back(tensor_info.GetElementType());
    shapes.push_back(tensor_info.GetShape());
  }

  std::cout << std::endl << "output nodes: " << std::endl;
  pretty_print_table(names, types, shapes);
  std::cout << std::endl;


  // get metadata
  Ort::AllocatorWithDefaultOptions allocator;
  Ort::ModelMetadata metadata = session->GetModelMetadata();
  if (narg == 2) {
    std::cout << "available metadata keys: ";
    auto keys = metadata.GetCustomMetadataMapKeysAllocated(allocator);
    for (uint64_t i = 0; i < keys.size(); i++) {
      std::cout << keys[i].get();
      if (i+1 < keys.size()) std::cout << ", ";
    }
    std::cout << std::endl;
    return 2;
  }
  auto val = metadata.LookupCustomMetadataMapAllocated(argv[2], allocator);
  std::cout << val.get() << std::endl;

  return 0;
}
