// Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
//
// Derived from the NVIDIA Triton backend examples, licensed under
// BSD-3-Clause under the following notice:
//
// Copyright 2021, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/**
 * @file  Trigger/EFTracking/TracccTritonBackend/src/TracccTritonBackend.cxx
 * @author Miles Cochran-Branson
 * @date September 2026
 * @brief Implementation of the Triton traccc-aaS backend
 */

#include <cuda_runtime_api.h>

#include <array>
#include <chrono>
#include <cstring>
#include <set>

#include "TracccTritonInitializer.h"
#include "TracccTritonRunner.h"
#include "TracccTritonClient/TracccEdmSerialization.h"
#include "GaudiKernel/EventIDBase.h"
#include "triton/backend/backend_common.h"
#include "triton/backend/backend_input_collector.h"
#include "triton/backend/backend_model.h"
#include "triton/backend/backend_model_instance.h"
#include "triton/backend/backend_output_responder.h"
#include "triton/core/tritonbackend.h"

namespace triton { namespace backend { namespace traccc {

//
// Backend serving the Traccc GPU track-reconstruction chain: one UINT8
// CELLS input tensor and the INT64 EVENT_IOV of the event in, the traccc EDM
// collections out as UINT8 tensors. Every UINT8 tensor is one collection
// serialized with TracccEdmSerialization.h.
//

const std::string iovInputName = "EVENT_IOV";
constexpr std::size_t iovSize = 5;

const std::array<std::string, 4> outputNames = {
    "MEASUREMENTS", "CLUSTERS", "TRACKS", "TRACK_STATES"};

/////////////

extern "C" {

// Triton calls TRITONBACKEND_Initialize when a backend is loaded into
// Triton to allow the backend to create and initialize any state that
// is intended to be shared across all models and model instances that
// use the backend. The backend should also verify version
// compatibility with Triton in this function.
//
TRITONSERVER_Error*
TRITONBACKEND_Initialize(TRITONBACKEND_Backend* backend)
{
  const char* cname;
  RETURN_IF_ERROR(TRITONBACKEND_BackendName(backend, &cname));
  std::string name(cname);

  LOG_MESSAGE(
      TRITONSERVER_LOG_INFO,
      (std::string("TRITONBACKEND_Initialize: ") + name).c_str());

  // Check the backend API version that Triton supports vs. what this
  // backend was compiled against. Make sure that the Triton major
  // version is the same and the minor version is >= what this backend
  // uses.
  uint32_t api_version_major, api_version_minor;
  RETURN_IF_ERROR(
      TRITONBACKEND_ApiVersion(&api_version_major, &api_version_minor));

  LOG_MESSAGE(
      TRITONSERVER_LOG_INFO,
      (std::string("Triton TRITONBACKEND API version: ") +
       std::to_string(api_version_major) + "." +
       std::to_string(api_version_minor))
          .c_str());
  LOG_MESSAGE(
      TRITONSERVER_LOG_INFO,
      (std::string("'") + name + "' TRITONBACKEND API version: " +
       std::to_string(TRITONBACKEND_API_VERSION_MAJOR) + "." +
       std::to_string(TRITONBACKEND_API_VERSION_MINOR))
          .c_str());

  if ((api_version_major != TRITONBACKEND_API_VERSION_MAJOR) ||
      (api_version_minor < TRITONBACKEND_API_VERSION_MINOR)) {
    return TRITONSERVER_ErrorNew(
        TRITONSERVER_ERROR_UNSUPPORTED,
        "triton backend API version does not support this backend");
  }

  // The backend configuration may contain information needed by the
  // backend, such as tritonserver command-line arguments. This
  // backend doesn't use any such configuration but for this example
  // print whatever is available.
  TRITONSERVER_Message* backend_config_message;
  RETURN_IF_ERROR(
      TRITONBACKEND_BackendConfig(backend, &backend_config_message));

  const char* buffer;
  size_t byte_size;
  RETURN_IF_ERROR(TRITONSERVER_MessageSerializeToJson(
      backend_config_message, &buffer, &byte_size));
  LOG_MESSAGE(
      TRITONSERVER_LOG_INFO,
      (std::string("backend configuration:\n") + buffer).c_str());

  // This backend does not require any "global" state but as an
  // example create a string to demonstrate.
  std::string* state = new std::string("backend state");
  RETURN_IF_ERROR(
      TRITONBACKEND_BackendSetState(backend, reinterpret_cast<void*>(state)));

  return nullptr;  // success
}

// Triton calls TRITONBACKEND_Finalize when a backend is no longer
// needed.
//
TRITONSERVER_Error*
TRITONBACKEND_Finalize(TRITONBACKEND_Backend* backend)
{
    // Delete the "global" state associated with the backend.
    void* vstate;
    RETURN_IF_ERROR(TRITONBACKEND_BackendState(backend, &vstate));
    std::string* state = reinterpret_cast<std::string*>(vstate);

    LOG_MESSAGE(
        TRITONSERVER_LOG_INFO,
        (std::string("TRITONBACKEND_Finalize: state is '") + *state + "'")
            .c_str());

    delete state;

    return nullptr;  // success
}

}  // extern "C"

/////////////

//
// ModelState
//
// State associated with a model that is using this backend. An object
// of this class is created and associated with each
// TRITONBACKEND_Model. ModelState is derived from BackendModel class
// provided in the backend utilities that provides many common
// functions.
class ModelState : public BackendModel {
 public:
  static TRITONSERVER_Error* Create(
      TRITONBACKEND_Model* triton_model, ModelState** state);
  virtual ~ModelState() = default;

    // Name of the input tensor
    const std::string &InputCellsTensorName() const { return m_inputCellsName; }

    // Datatype of the input tensor
    TRITONSERVER_DataType InputCellsTensorDataType() const { return m_inputCellsDatatype; }

    // Shape of the input and output tensor as given in the model
    // configuration file. This shape will not include the batch
    // dimension (if the model has one).
    // const std::vector<int64_t>& TensorNonBatchShape() const { return nb_shape_;
    // }

    // Shape of the input and output tensor, including the batch
    // dimension (if the model has one). This method cannot be called
    // until the model is completely loaded and initialized, including
    // all instances of the model. In practice, this means that backend
    // should only call it in TRITONBACKEND_ModelInstanceExecute.
    const std::vector<int64_t> &InputCellsTensorNonBatchShape() const
    {
        return m_inputCellsNbShape;
    }

    size_t TotalInstanceCount() const { return m_totalInstanceCount; }

    // Validate that this model is supported by this backend.
    TRITONSERVER_Error *ValidateModelConfig();

    //   std::string model_path;
    int64_t cellFeatures;

 private:
  ModelState(TRITONBACKEND_Model* triton_model);

    std::string m_inputCellsName;

    TRITONSERVER_DataType m_inputCellsDatatype;

    std::vector<int64_t> m_inputCellsNbShape;

    size_t m_totalInstanceCount = 1;
};

//! Also possible problems in this function!
ModelState::ModelState(TRITONBACKEND_Model* triton_model)
    : BackendModel(triton_model)
{
  // Validate that the model's configuration matches what is supported
  // by this backend.
  THROW_IF_BACKEND_MODEL_ERROR(ValidateModelConfig());
}

TRITONSERVER_Error*
ModelState::Create(TRITONBACKEND_Model* triton_model, ModelState** state)
{
  try {
    *state = new ModelState(triton_model);
  }
  catch (const BackendModelException& ex) {
    RETURN_ERROR_IF_TRUE(
        ex.err_ == nullptr, TRITONSERVER_ERROR_INTERNAL,
        std::string("unexpected nullptr in BackendModelException"));
    RETURN_IF_ERROR(ex.err_);
  }

  return nullptr;  // success
}

TRITONSERVER_Error*
ModelState::ValidateModelConfig()
{
    // If verbose logging is enabled, dump the model's configuration as
    // JSON into the console output.
    if (TRITONSERVER_LogIsEnabled(TRITONSERVER_LOG_VERBOSE)) {
    common::TritonJson::WriteBuffer buffer;
    RETURN_IF_ERROR(ModelConfig().PrettyWrite(&buffer));
    LOG_MESSAGE(
        TRITONSERVER_LOG_VERBOSE,
        (std::string("model configuration:\n") + buffer.Contents()).c_str());
    }

    // ModelConfig is the model configuration as a TritonJson
    // object. Use the TritonJson utilities to parse the JSON and
    // determine if the configuration is supported by this backend.
    common::TritonJson::Value inputs, outputs;
    RETURN_IF_ERROR(ModelConfig().MemberAsArray("input", &inputs));
    RETURN_IF_ERROR(ModelConfig().MemberAsArray("output", &outputs));

    // The model must have exactly the 2 inputs (CELLS, EVENT_IOV) and the 4
    // outputs the client reads back
    RETURN_ERROR_IF_FALSE(
        inputs.ArraySize() == 2, TRITONSERVER_ERROR_INVALID_ARG,
        std::string("model configuration must have 2 inputs"));
    RETURN_ERROR_IF_FALSE(
        outputs.ArraySize() == outputNames.size(), TRITONSERVER_ERROR_INVALID_ARG,
        std::string("model configuration must have ") +
            std::to_string(outputNames.size()) + " outputs");

    // CELLS comes first; EVENT_IOV is looked up by its fixed name, and
    // Triton checks its type and shape against this configuration
    common::TritonJson::Value input_cells;
    RETURN_IF_ERROR(inputs.IndexAsObject(0, &input_cells));

    // Record the input name in the model state.
    const char *input_cells_name;
    size_t input_cells_len;
    RETURN_IF_ERROR(input_cells.MemberAsString("name", &input_cells_name, &input_cells_len));
    // MemberAsString hands back a pointer into the parsed JSON plus its
    // length; that buffer is not guaranteed to be NUL-terminated, so the
    // length must be used. Building the name with the const char* overload
    // instead can append trailing garbage, which then makes
    // TRITONBACKEND_RequestInput fail to find the tensor at execute time.
    m_inputCellsName = std::string(input_cells_name, input_cells_len);

    LOG_MESSAGE(TRITONSERVER_LOG_INFO,
                (std::string("'traccc' backend: input tensor is '") +
                 m_inputCellsName + "' (" +
                 std::to_string(m_inputCellsName.size()) + " chars)").c_str());

    std::string input_cells_dtype;
    RETURN_IF_ERROR(input_cells.MemberAsString("data_type", &input_cells_dtype));
    m_inputCellsDatatype = ModelConfigDataTypeToTritonServerDataType(input_cells_dtype);

    RETURN_IF_ERROR(backend::ParseShape(input_cells, "dims", &m_inputCellsNbShape));

    // The output tensors are written by fixed name, so the configured names
    // must be exactly the expected set (order is irrelevant).
    std::set<std::string> configured_outputs;
    for (size_t i = 0; i < outputs.ArraySize(); ++i) {
        common::TritonJson::Value output;
        RETURN_IF_ERROR(outputs.IndexAsObject(i, &output));
        const char *output_name;
        size_t output_name_len;
        RETURN_IF_ERROR(
            output.MemberAsString("name", &output_name, &output_name_len));
        configured_outputs.emplace(output_name, output_name_len);
    }
    for (const auto &expected : outputNames) {
        RETURN_ERROR_IF_FALSE(
            configured_outputs.count(expected) == 1, TRITONSERVER_ERROR_INVALID_ARG,
            std::string("model configuration is missing output '") + expected + "'");
    }

    // Count the instances Triton is going to create, so the embedded Athena
    // application can be brought up with one event-store slot per instance.
    m_totalInstanceCount = 0;
    common::TritonJson::Value instance_groups;
    if (ModelConfig().Find("instance_group", &instance_groups)) {
        for (size_t i = 0; i < instance_groups.ArraySize(); ++i) {
            common::TritonJson::Value group;
            RETURN_IF_ERROR(instance_groups.IndexAsObject(i, &group));

            int64_t count = 1;
            if (group.Find("count")) {
                RETURN_IF_ERROR(group.MemberAsInt("count", &count));
            }
            RETURN_ERROR_IF_FALSE(
                count >= 0, TRITONSERVER_ERROR_INVALID_ARG,
                std::string("instance_group count must not be negative"));

            size_t devices = 1;
            common::TritonJson::Value gpus;
            if (group.Find("gpus", &gpus) && gpus.ArraySize() > 0) {
                devices = gpus.ArraySize();
            }
            m_totalInstanceCount += static_cast<size_t>(count) * devices;
        }
    }
    // No instance_group at all, or one that resolves to nothing, still means
    // Triton creates a single instance.
    if (m_totalInstanceCount == 0) {
        m_totalInstanceCount = 1;
    }

    LOG_MESSAGE(TRITONSERVER_LOG_INFO,
                (std::string("'traccc' backend: model configured with ") +
                 std::to_string(m_totalInstanceCount) +
                 " instance(s), one event slot each").c_str());

    return nullptr; // success
}

extern "C" {

// Triton calls TRITONBACKEND_ModelInitialize when a model is loaded
// to allow the backend to create any state associated with the model,
// and to also examine the model configuration to determine if the
// configuration is suitable for the backend. Any errors reported by
// this function will prevent the model from loading.
//
TRITONSERVER_Error*
TRITONBACKEND_ModelInitialize(TRITONBACKEND_Model* model)
{
  // Create a ModelState object and associate it with the
  // TRITONBACKEND_Model. If anything goes wrong with initialization
  // of the model state then an error is returned and Triton will fail
  // to load the model.
  ModelState* model_state;
  RETURN_IF_ERROR(ModelState::Create(model, &model_state));
  RETURN_IF_ERROR(
      TRITONBACKEND_ModelSetState(model, reinterpret_cast<void*>(model_state)));

  return nullptr;  // success
}

// Triton calls TRITONBACKEND_ModelFinalize when a model is no longer
// needed. The backend should cleanup any state associated with the
// model. This function will not be called until all model instances
// of the model have been finalized.
//
TRITONSERVER_Error*
TRITONBACKEND_ModelFinalize(TRITONBACKEND_Model* model)
{
  void* vstate;
  RETURN_IF_ERROR(TRITONBACKEND_ModelState(model, &vstate));
  ModelState* model_state = reinterpret_cast<ModelState*>(vstate);
  delete model_state;

  return nullptr;  // success
}

}  // extern "C"

/////////////

//
// ModelInstanceState
//
// State associated with a model instance. An object of this class is
// created and associated with each
// TRITONBACKEND_ModelInstance. ModelInstanceState is derived from
// BackendModelInstance class provided in the backend utilities that
// provides many common functions.
//
class ModelInstanceState : public BackendModelInstance 
{
private:

    ModelState* m_modelState;

    ModelInstanceState(
        ModelState* model_state,
        TRITONBACKEND_ModelInstance* triton_model_instance)
        : BackendModelInstance(model_state, triton_model_instance),
            m_modelState(model_state)
    {
    }

public:
    static TRITONSERVER_Error* Create(
        ModelState* model_state,
        TRITONBACKEND_ModelInstance* triton_model_instance,
        ModelInstanceState** state);
    virtual ~ModelInstanceState() = default;

    // Get the state of the model that corresponds to this instance.
    ModelState* StateForModel() { return m_modelState; }

    // Drives the device reconstruction chain for each request
    std::unique_ptr<TracccTritonRunner> traccc_triton_runner_;
};

TRITONSERVER_Error*
ModelInstanceState::Create(
    ModelState* model_state, TRITONBACKEND_ModelInstance* triton_model_instance,
    ModelInstanceState** state)
{
  try {
    *state = new ModelInstanceState(model_state, triton_model_instance);
  }
  catch (const BackendModelInstanceException& ex) {
    RETURN_ERROR_IF_TRUE(
        ex.err_ == nullptr, TRITONSERVER_ERROR_INTERNAL,
        std::string("unexpected nullptr in BackendModelInstanceException"));
    RETURN_IF_ERROR(ex.err_);
  }

  return nullptr;  // success
}

extern "C" {

// Triton calls TRITONBACKEND_ModelInstanceInitialize when a model
// instance is created to allow the backend to initialize any state
// associated with the instance.
//
TRITONSERVER_Error*
TRITONBACKEND_ModelInstanceInitialize(TRITONBACKEND_ModelInstance* instance)
{
    // Get the model state associated with this instance's model.
    TRITONBACKEND_Model* model;
    RETURN_IF_ERROR(TRITONBACKEND_ModelInstanceModel(instance, &model));

    void* vmodelstate;
    RETURN_IF_ERROR(TRITONBACKEND_ModelState(model, &vmodelstate));
    ModelState* model_state = reinterpret_cast<ModelState*>(vmodelstate);

    // Create a ModelInstanceState object and associate it with the
    // TRITONBACKEND_ModelInstance.
    ModelInstanceState* instance_state;
    RETURN_IF_ERROR(
        ModelInstanceState::Create(model_state, instance, &instance_state));
    RETURN_IF_ERROR(TRITONBACKEND_ModelInstanceSetState(
        instance, reinterpret_cast<void*>(instance_state)));

    // Set the CUDA device for this thread
    cudaError_t err = cudaSetDevice(instance_state->DeviceId());
    if (err != cudaSuccess)
    {
        return TRITONSERVER_ErrorNew(
            TRITONSERVER_ERROR_INTERNAL,
            ("Failed to set CUDA device: " + std::string(cudaGetErrorString(err))).c_str());
    }

    // Bring up the embedded Athena application (process-wide singleton,
    // idempotent) and construct the per-instance runner that drives the
    // device chain.
    try
    {
        TracccTritonInitializer::Config config;
        config.deviceId = instance_state->DeviceId();
        config.nSlots = model_state->TotalInstanceCount();

        TracccTritonInitializer::instance().initialize(config);

        const size_t slot = TracccTritonInitializer::instance().acquireSlot();
        instance_state->traccc_triton_runner_ =
            std::make_unique<TracccTritonRunner>(
                TracccTritonInitializer::instance(), slot);

        LOG_MESSAGE(TRITONSERVER_LOG_INFO,
                    (std::string("'traccc' backend: instance '") +
                     instance_state->Name() + "' bound to event slot " +
                     std::to_string(slot)).c_str());
    }
    catch (const std::exception& e)
    {
        return TRITONSERVER_ErrorNew(
            TRITONSERVER_ERROR_INTERNAL,
            (std::string("Failed to initialize Traccc device chain: ") +
                e.what()).c_str());
    }
    return nullptr;  // success
}

// Triton calls TRITONBACKEND_ModelInstanceFinalize when a model
// instance is no longer needed. The backend should cleanup any state
// associated with the model instance.
//
TRITONSERVER_Error*
TRITONBACKEND_ModelInstanceFinalize(TRITONBACKEND_ModelInstance* instance)
{
  void* vstate;
  RETURN_IF_ERROR(TRITONBACKEND_ModelInstanceState(instance, &vstate));
  ModelInstanceState* instance_state =
      reinterpret_cast<ModelInstanceState*>(vstate);
  delete instance_state;

  return nullptr;  // success
}

}  // extern "C"

/////////////

extern "C" {

// When Triton calls TRITONBACKEND_ModelInstanceExecute it is required
// that a backend create a response for each request in the batch. A
// response may be the output tensors required for that request or may
// be an error that is returned in the response.
//
TRITONSERVER_Error*
TRITONBACKEND_ModelInstanceExecute(
    TRITONBACKEND_ModelInstance* instance, TRITONBACKEND_Request** requests,
    const uint32_t request_count)
{
    // Collect various timestamps during the execution of this batch or
    // requests. These values are reported below before returning from
    // the function.

    uint64_t exec_start_ns = 0;
    SET_TIMESTAMP(exec_start_ns);

    // Triton will not call this function simultaneously for the same
    // 'instance'. But since this backend could be used by multiple
    // instances from multiple models the implementation needs to handle
    // multiple calls to this function at the same time (with different
    // 'instance' objects). Best practice for a high-performance
    // implementation is to avoid introducing mutex/lock and instead use
    // only function-local and model-instance-specific state.
    ModelInstanceState* instance_state;
    RETURN_IF_ERROR(TRITONBACKEND_ModelInstanceState(
        instance, reinterpret_cast<void**>(&instance_state)));
    ModelState* model_state = instance_state->StateForModel();

    // Set the CUDA device for this thread
    // Seems that this is necessary to set the device for each request
    // Without leads to out-of-bounds memory access error
    cudaError_t err = cudaSetDevice(instance_state->DeviceId());
    if (err != cudaSuccess)
    {
        return TRITONSERVER_ErrorNew(
            TRITONSERVER_ERROR_INTERNAL,
            ("Failed to set CUDA device: " + std::string(cudaGetErrorString(err))).c_str());
    }

    // 'responses' is initialized as a parallel array to 'requests',
    // with one TRITONBACKEND_Response object for each
    // TRITONBACKEND_Request object. If something goes wrong while
    // creating these response objects, the backend simply returns an
    // error from TRITONBACKEND_ModelInstanceExecute, indicating to
    // Triton that this backend did not create or send any responses and
    // so it is up to Triton to create and send an appropriate error
    // response for each request. RETURN_IF_ERROR is one of several
    // useful macros for error handling that can be found in
    // backend_common.h.

    std::vector<TRITONBACKEND_Response*> responses;
    responses.reserve(request_count);
    for (uint32_t r = 0; r < request_count; ++r) {
        TRITONBACKEND_Request* request = requests[r];
        TRITONBACKEND_Response* response;
        RETURN_IF_ERROR(TRITONBACKEND_ResponseNew(&response, request));
        responses.push_back(response);
    }

    // At this point, the backend takes ownership of 'requests', which
    // means that it is responsible for sending a response for every
    // request. From here, even if something goes wrong in processing,
    // the backend must return 'nullptr' from this function to indicate
    // success. Any errors and failures must be communicated via the
    // response objects.
    //
    // To simplify error handling, the backend utilities manage
    // 'responses' in a specific way and it is recommended that backends
    // follow this same pattern. When an error is detected in the
    // processing of a request, an appropriate error response is sent
    // and the corresponding TRITONBACKEND_Response object within
    // 'responses' is set to nullptr to indicate that the
    // request/response has already been handled and no further processing
    // should be performed for that request. Even if all responses fail,
    // the backend still allows execution to flow to the end of the
    // function so that statistics are correctly reported by the calls
    // to TRITONBACKEND_ModelInstanceReportStatistics and
    // TRITONBACKEND_ModelInstanceReportBatchStatistics.
    // RESPOND_AND_SET_NULL_IF_ERROR, and
    // RESPOND_ALL_AND_SET_NULL_IF_ERROR are macros from
    // backend_common.h that assist in this management of response
    // objects.

    // The backend could iterate over the 'requests' and process each
    // one separately. But for performance reasons it is usually
    // preferred to create batched input tensors that are processed
    // simultaneously. This is especially true for devices like GPUs
    // that are capable of exploiting the large amount parallelism
    // exposed by larger data sets.
    //
    // The backend utilities provide a "collector" to facilitate this
    // batching process. The 'collector's ProcessTensor function will
    // combine a tensor's value from each request in the batch into a
    // single contiguous buffer. The buffer can be provided by the
    // backend or 'collector' can create and manage it. In this backend,
    // there is not a specific buffer into which the batch should be
    // created, so use ProcessTensor arguments that cause collector to
    // manage it. ProcessTensor does NOT support TRITONSERVER_TYPE_BYTES
    // data type.

    // BackendInputCollector::ProcessTensor leaves its TRITONBACKEND_Input
    // handle uninitialized when TRITONBACKEND_RequestInput fails, and then
    // dereferences it anyway -- one request that does not carry the input
    // would take the whole server down. Check every request up front and skip
    // the collector entirely unless they all have it, so such a request comes
    // back as a normal error response.
    bool all_inputs_present = true;
    for (uint32_t r = 0; r < request_count; ++r) {
        for (const std::string& name :
             {model_state->InputCellsTensorName(), iovInputName}) {
            TRITONBACKEND_Input* input = nullptr;
            TRITONSERVER_Error* input_err =
                TRITONBACKEND_RequestInput(requests[r], name.c_str(), &input);
            if (input_err != nullptr) {
                all_inputs_present = false;
                RESPOND_AND_SET_NULL_IF_ERROR(&responses[r], input_err);
                break;
            }
        }
    }

    BackendInputCollector collector(
        requests, request_count, &responses, model_state->TritonMemoryManager(),
        false /* pinned_enabled */, nullptr /* stream*/);

    // To instruct ProcessTensor to "gather" the entire batch of input
    // tensors into a single contiguous buffer in CPU memory, set the
    // "allowed input types" to be the CPU ones (see tritonserver.h in
    // the triton-inference-server/core repo for allowed memory types).
    std::vector<std::pair<TRITONSERVER_MemoryType, int64_t>> allowed_input_types =
        {{TRITONSERVER_MEMORY_CPU_PINNED, 0}, {TRITONSERVER_MEMORY_CPU, 0}};

    const char *input_cells_buffer = nullptr;
    size_t input_cells_buffer_byte_size = 0;
    TRITONSERVER_MemoryType input_cells_buffer_memory_type;
    int64_t input_cells_buffer_memory_type_id;

    const char *input_iov_buffer = nullptr;
    size_t input_iov_buffer_byte_size = 0;
    TRITONSERVER_MemoryType input_iov_buffer_memory_type;
    int64_t input_iov_buffer_memory_type_id;

    if (all_inputs_present)
    {
        RESPOND_ALL_AND_SET_NULL_IF_ERROR(
            responses, request_count,
            collector.ProcessTensor(
                model_state->InputCellsTensorName().c_str(), nullptr /* existing_buffer */,
                0 /* existing_buffer_byte_size */, allowed_input_types, &input_cells_buffer,
                &input_cells_buffer_byte_size, &input_cells_buffer_memory_type,
                &input_cells_buffer_memory_type_id));
        RESPOND_ALL_AND_SET_NULL_IF_ERROR(
            responses, request_count,
            collector.ProcessTensor(
                iovInputName.c_str(), nullptr /* existing_buffer */,
                0 /* existing_buffer_byte_size */, allowed_input_types, &input_iov_buffer,
                &input_iov_buffer_byte_size, &input_iov_buffer_memory_type,
                &input_iov_buffer_memory_type_id));

        // Finalize the collector. If 'true' is returned, 'input_buffer'
        // will not be valid until the backend synchronizes the CUDA
        // stream or event that was used when creating the collector. For
        // this backend, GPU is not supported and so no CUDA sync should
        // be needed; so if 'true' is returned simply log an error.
        const bool need_cuda_input_sync = collector.Finalize();
        if (need_cuda_input_sync)
        {
            LOG_MESSAGE(
                TRITONSERVER_LOG_ERROR,
                "'Traccc' backend: unexpected CUDA sync required by collector");
        }
    }

    bool print_stats = true;

    // run the reco chain: deserialize CELLS, drive the device algorithms
    // (clusterization -> spacepoint formation -> seeding -> track parameter
    // estimation -> track finding) and copy the fitted tracks and their
    // measurements back to the host. All of that lives in TracccTritonRunner.
    uint64_t compute_start_ns = 0;
    SET_TIMESTAMP(compute_start_ns);

    TracccTritonRunner::Output traccc_result;
    bool run_succeeded = false;
    try
    {
        if (!all_inputs_present) {
            throw std::runtime_error(
                "request is missing the '" +
                model_state->InputCellsTensorName() + "' or '" +
                iovInputName + "' input tensor");
        }
        // max_batch_size is 0, so there is one request and one IOV
        std::array<int64_t, iovSize> iov;
        if (input_iov_buffer_byte_size != sizeof(iov)) {
            throw std::runtime_error("'" + iovInputName + "' has the wrong size");
        }
        std::memcpy(iov.data(), input_iov_buffer, sizeof(iov));
        using number_type = EventIDBase::number_type;
        const EventIDBase eventID(
            static_cast<number_type>(iov[0]), EventIDBase::UNDEFEVT,
            static_cast<number_type>(iov[2]), static_cast<number_type>(iov[3]),
            static_cast<number_type>(iov[1]));
        instance_state->traccc_triton_runner_->run(
            reinterpret_cast<const uint8_t*>(input_cells_buffer),
            input_cells_buffer_byte_size, eventID,
            static_cast<number_type>(iov[4]), print_stats, traccc_result);
        run_succeeded = true;
    }
    catch (const std::exception& e)
    {
        RESPOND_ALL_AND_SET_NULL_IF_ERROR(
            responses, request_count,
            TRITONSERVER_ErrorNew(TRITONSERVER_ERROR_INTERNAL, e.what()));
    }

    uint64_t compute_end_ns = 0;
    SET_TIMESTAMP(compute_end_ns);

    bool supports_first_dim_batching;
    RESPOND_ALL_AND_SET_NULL_IF_ERROR(
        responses, request_count,
        model_state->SupportsFirstDimBatching(&supports_first_dim_batching));

    // Because the output tensor values are concatenated into a single
    // contiguous 'output_buffer', the backend must "scatter" them out
    // to the individual response output tensors.  The backend utilities
    // provide a "responder" to facilitate this scattering process.
    // BackendOutputResponder does NOT support TRITONSERVER_TYPE_BYTES
    // data type.

    // Initialize the responder
    BackendOutputResponder responder(
        requests, request_count, &responses, model_state->TritonMemoryManager(),
        supports_first_dim_batching, false /* pinned_enabled */,
        nullptr /* stream*/);

    auto output_proc_start = std::chrono::high_resolution_clock::now();

    // Serialized output tensors
    std::vector<std::vector<std::uint8_t>> output_bytes;
    output_bytes.reserve(outputNames.size());

    // Process the outputs. Skipped when the chain threw: every response has
    // already been failed above and there is no result to unpack.
    if (run_succeeded)
    {
        // Unlike BackendInputCollector::ProcessTensor, the responder's
        // ProcessTensor returns void: errors are recorded directly into the
        // per-request response objects rather than returned.
        const auto sendCollection = [&](const std::string& name,
                                        const auto& collection) {
            std::vector<std::uint8_t>& bytes = output_bytes.emplace_back();
            TracccTriton::serialize(collection, bytes);
            std::vector<int64_t> shape = {
                static_cast<int64_t>(bytes.size())};
            responder.ProcessTensor(
                name.c_str(), TRITONSERVER_TYPE_UINT8, shape,
                reinterpret_cast<const char*>(bytes.data()),
                TRITONSERVER_MEMORY_CPU, 0);
        };
        sendCollection("MEASUREMENTS", traccc_result.measurements);
        sendCollection("CLUSTERS", traccc_result.clusters);
        sendCollection("TRACKS", traccc_result.tracks.tracks);
        sendCollection("TRACK_STATES", traccc_result.tracks.states);
    }

    if (print_stats)
    {
        auto output_proc_end = std::chrono::high_resolution_clock::now();
        std::cout << "[TIMING] Output processing: "
                << std::chrono::duration_cast<std::chrono::milliseconds>(
                        output_proc_end - output_proc_start).count()
                << " ms" << std::endl;
    }

    // Finalize the responder. If 'true' is returned, the output
    // tensors' data will not be valid until the backend synchronizes
    // the CUDA stream or event that was used when creating the
    // responder. For this backend, GPU is not supported and so no CUDA
    // sync should be needed; so if 'true' is returned simply log an
    // error.
    const bool need_cuda_output_sync = responder.Finalize();
    if (need_cuda_output_sync) {
    LOG_MESSAGE(
        TRITONSERVER_LOG_ERROR,
        "'traccc' backend: unexpected CUDA sync required by responder");
    }

    // Send all the responses that haven't already been sent because of
    // an earlier error.
    for (auto& response : responses) {
    if (response != nullptr) {
        LOG_IF_ERROR(
            TRITONBACKEND_ResponseSend(
                response, TRITONSERVER_RESPONSE_COMPLETE_FINAL, nullptr),
            "failed to send response");
    }
    }

    uint64_t exec_end_ns = 0;
    SET_TIMESTAMP(exec_end_ns);

#ifdef TRITON_ENABLE_STATS
    // For batch statistics need to know the total batch size of the
    // requests. This is not necessarily just the number of requests,
    // because if the model supports batching then any request can be a
    // batched request itself.
    size_t total_batch_size = 0;
    if (!supports_first_dim_batching) {
    total_batch_size = request_count;
    } else {
        for (uint32_t r = 0; r < request_count; ++r) 
        {
            auto& request = requests[r];
            TRITONBACKEND_Input* input = nullptr;
            LOG_IF_ERROR(
                TRITONBACKEND_RequestInputByIndex(request, 0 /* index */, &input),
                "failed getting request input");
            if (input != nullptr) 
            {
                const int64_t* shape = nullptr;
                LOG_IF_ERROR(
                    TRITONBACKEND_InputProperties(
                        input, nullptr, nullptr, &shape, nullptr, nullptr, nullptr),
                    "failed getting input properties");
                if (shape != nullptr) 
                {
                    total_batch_size += shape[0];
                }
            }
        }
    }
#else
    (void)exec_start_ns;
    (void)exec_end_ns;
    (void)compute_start_ns;
    (void)compute_end_ns;
#endif  // TRITON_ENABLE_STATS

// Report statistics for each request, and then release the request.
for (uint32_t r = 0; r < request_count; ++r) 
{
    auto& request = requests[r];

    #ifdef TRITON_ENABLE_STATS
        LOG_IF_ERROR(
            TRITONBACKEND_ModelInstanceReportStatistics(
                instance_state->TritonModelInstance(), request,
                (responses[r] != nullptr) /* success */, exec_start_ns,
                compute_start_ns, compute_end_ns, exec_end_ns),
            "failed reporting request statistics");
    #endif  // TRITON_ENABLE_STATS

        LOG_IF_ERROR(
            TRITONBACKEND_RequestRelease(request, TRITONSERVER_REQUEST_RELEASE_ALL),
            "failed releasing request");
}

#ifdef TRITON_ENABLE_STATS
    // Report batch statistics.
    LOG_IF_ERROR(
        TRITONBACKEND_ModelInstanceReportBatchStatistics(
            instance_state->TritonModelInstance(), total_batch_size,
            exec_start_ns, compute_start_ns, compute_end_ns, exec_end_ns),
        "failed reporting batch request statistics");
#endif  // TRITON_ENABLE_STATS

return nullptr;  // success
}

}  // extern "C"

}}}  // namespace triton::backend::traccc
