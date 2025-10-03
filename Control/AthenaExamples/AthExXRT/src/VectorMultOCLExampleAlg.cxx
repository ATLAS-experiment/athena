//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "VectorMultOCLExampleAlg.h"

namespace AthExXRT {

StatusCode VectorMultOCLExampleAlg::initialize_global() {

  ATH_MSG_INFO("initialize_global()");

  ATH_CHECK(m_DeviceMgmtSvc.retrieve());

  // Retrieve the list of OpencCL Handle(s) providing the kernel.
  m_handles = m_DeviceMgmtSvc->get_opencl_handles_by_kernel_name(s_krnl_name);

  if (m_handles.empty()) {
    ATH_MSG_ERROR("No OpenCL context provides kernel '" << s_krnl_name << "'");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

StatusCode VectorMultOCLExampleAlg::initialize_worker() {

  ATH_MSG_INFO("initialize_worker()");

  cl_int err = CL_SUCCESS;

  // Allocate slot specific resources.
  std::size_t slotIdx = 0;
  for (SlotData& slot : m_slots) {
    ATH_MSG_DEBUG("Allocating resources for slot " << slotIdx);

    if (m_handles.size() > 1) {
      ATH_MSG_WARNING("More than one OpenCL context provides a '"
                      << s_krnl_name << "' kernel (" << m_handles.size()
                      << "), using the first one");
    }
    slot.m_context = m_handles[0].context;
    slot.m_program = m_handles[0].program;

    // Create kernel objects.
    slot.m_kernel =
        std::make_unique<cl::Kernel>(*slot.m_program, s_krnl_name, &err);
    if (err != 0) {
      ATH_MSG_ERROR(
          "Could not create OpenCL kernel '"
          << s_krnl_name
          << "', check that correct XCLBIN is programmed by AthXRT service");
      return StatusCode::FAILURE;
    }

    // Create command queue.
    slot.m_queue = std::make_unique<cl::CommandQueue>(
        *slot.m_context, slot.m_context->getInfo<CL_CONTEXT_DEVICES>()[0], 0,
        &err);
    ATH_CHECK(err == CL_SUCCESS);

    const std::size_t size_in_bytes = s_element_count * sizeof(uint32_t);

    // Create buffer objects.
    // This create aligned buffer object both on host and device.
    slot.m_dev_buf_in1 = std::make_unique<cl::Buffer>(
        *slot.m_context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
        size_in_bytes, nullptr, &err);
    ATH_CHECK(err == CL_SUCCESS);

    slot.m_dev_buf_in2 = std::make_unique<cl::Buffer>(
        *slot.m_context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
        size_in_bytes, nullptr, &err);
    ATH_CHECK(err == CL_SUCCESS);

    slot.m_dev_buf_out = std::make_unique<cl::Buffer>(
        *slot.m_context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
        size_in_bytes, nullptr, &err);
    ATH_CHECK(err == CL_SUCCESS);

    slot.m_host_buf_in1 = (uint32_t*)slot.m_queue->enqueueMapBuffer(
        *slot.m_dev_buf_in1, CL_TRUE, CL_MAP_WRITE, 0, size_in_bytes, nullptr,
        nullptr, &err);
    ATH_CHECK(err == CL_SUCCESS);

    slot.m_host_buf_in2 = (uint32_t*)slot.m_queue->enqueueMapBuffer(
        *slot.m_dev_buf_in2, CL_TRUE, CL_MAP_WRITE, 0, size_in_bytes, nullptr,
        nullptr, &err);
    ATH_CHECK(err == CL_SUCCESS);

    slot.m_host_buf_out = (uint32_t*)slot.m_queue->enqueueMapBuffer(
        *slot.m_dev_buf_out, CL_TRUE, CL_MAP_READ, 0, size_in_bytes, nullptr,
        nullptr, &err);
    ATH_CHECK(err == CL_SUCCESS);

    // Set kernel arguments.
    ATH_CHECK(slot.m_kernel->setArg(s_krnl_param_in1, *slot.m_dev_buf_in1) ==
              CL_SUCCESS);
    ATH_CHECK(slot.m_kernel->setArg(s_krnl_param_in2, *slot.m_dev_buf_in2) ==
              CL_SUCCESS);
    ATH_CHECK(slot.m_kernel->setArg(s_krnl_param_out, *slot.m_dev_buf_out) ==
              CL_SUCCESS);
    ATH_CHECK(slot.m_kernel->setArg(s_krnl_param_size, s_element_count) ==
              CL_SUCCESS);

    ++slotIdx;
  }

  return StatusCode::SUCCESS;
}

StatusCode VectorMultOCLExampleAlg::stop_worker() {

  ATH_MSG_INFO("stop_worker(): Cleaning OCL environment");
  // Unmap buffer objects.
  for (SlotData& slot : m_slots) {
    ATH_CHECK(slot.m_queue->enqueueUnmapMemObject(
                  *slot.m_dev_buf_in1, slot.m_host_buf_in1) == CL_SUCCESS);
    ATH_CHECK(slot.m_queue->enqueueUnmapMemObject(
                  *slot.m_dev_buf_in2, slot.m_host_buf_in2) == CL_SUCCESS);
    ATH_CHECK(slot.m_queue->enqueueUnmapMemObject(
                  *slot.m_dev_buf_out, slot.m_host_buf_out) == CL_SUCCESS);
    ATH_CHECK(slot.m_queue->finish() == CL_SUCCESS);
  }

  return StatusCode::SUCCESS;
}

StatusCode VectorMultOCLExampleAlg::execute(const EventContext& ctx) const {

  // Get the slot (thread) specific data.
  const SlotData& slot = *m_slots.get(ctx);

  // Initialize the buffers with random data.
  for (std::size_t i = 0; i < s_element_count; ++i) {
    slot.m_host_buf_in1[i] = rand() % s_element_count;
    slot.m_host_buf_in2[i] = rand() % s_element_count;
  }

  ATH_MSG_DEBUG("Transfer data buffer to device");
  std::vector<cl::Memory> mems_vector = {*slot.m_dev_buf_in1,
                                         *slot.m_dev_buf_in2};
  ATH_CHECK(slot.m_queue->enqueueMigrateMemObjects(mems_vector, 0, nullptr,
                                                   nullptr) == CL_SUCCESS);

  // Schedule the kernel.
  ATH_MSG_DEBUG("Running kernel");
  ATH_CHECK(slot.m_queue->enqueueTask(*slot.m_kernel, nullptr, nullptr) ==
            CL_SUCCESS);

  // Migrate data back to host.
  ATH_MSG_DEBUG("Transfer data back to host");
  std::vector<cl::Memory> mems_out_vector = {*slot.m_dev_buf_out};
  ATH_CHECK(slot.m_queue->enqueueMigrateMemObjects(
                mems_out_vector, CL_MIGRATE_MEM_OBJECT_HOST, nullptr,
                nullptr) == CL_SUCCESS);
  ATH_CHECK(slot.m_queue->finish() == CL_SUCCESS);

  // Check that kernel results are correct.
  bool correct = true;
  for (std::size_t i = 0; i < s_element_count; ++i) {
    uint32_t cpu_result = slot.m_host_buf_in1[i] * slot.m_host_buf_in2[i];
    if (slot.m_host_buf_out[i] != cpu_result) {
      ATH_MSG_ERROR("Error: Result mismatch: i = "
                    << i << ": CPU result = " << cpu_result
                    << " Device result = " << slot.m_host_buf_out[i]);
      correct = false;
      break;
    }
  }
  if (correct) {
    ATH_MSG_INFO("OpenCL vector multiplication test PASSED!");
  } else {
    ATH_MSG_ERROR("OpenCL vector multiplication test FAILED!");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

}  // namespace AthExXRT
