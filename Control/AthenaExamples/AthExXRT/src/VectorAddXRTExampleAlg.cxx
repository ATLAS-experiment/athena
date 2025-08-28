//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//

// Local include(s).
#include "VectorAddXRTExampleAlg.h"

namespace AthExXRT {

StatusCode VectorAddXRTExampleAlg::initialize_global() {

  ATH_MSG_INFO("initialize_global()");

  ATH_CHECK(m_DeviceMgmtSvc.retrieve());

  // Retrieve the list of device(s) providing the kernel.
  m_devices = m_DeviceMgmtSvc->get_xrt_devices_by_kernel_name(s_krnl_name);
  if (m_devices.empty()) {
    ATH_MSG_ERROR("No XRT device provides kernel '" << s_krnl_name << "'");
    return StatusCode::FAILURE;
  }
  ATH_MSG_INFO("Retrieved " << m_devices.size()<<" devices running "<< s_krnl_name);

  return StatusCode::SUCCESS;

}

StatusCode VectorAddXRTExampleAlg::initialize_worker() {

  ATH_MSG_INFO("initialize_worker()");

  // Allocate slot specific resources.
  std::size_t slotIdx = 0;
  for (SlotData& slot : m_slots) {
    ATH_MSG_DEBUG("Allocating resources for slot " << slotIdx);

    // If multiple device are available, we can select one based on the slot
    // number in a round-robin fashion. This is just an example, and more
    // complex logic could be implemented here to take advantage of multiple
    // devices.
    const std::size_t device_idx = slotIdx % m_devices.size();
    ATH_MSG_DEBUG("Using device " << device_idx << " for slot " << slotIdx);
    slot.m_device = m_devices[device_idx];

    // Create kernel objects.
    try {
      slot.m_kernel = std::make_unique<xrt::kernel>(
          *slot.m_device, slot.m_device->get_xclbin_uuid(), s_krnl_name);
    } catch (...) {
      std::exception_ptr p = std::current_exception();
      ATH_MSG_ERROR(
          "Could not create XRT kernel '"
          << s_krnl_name
          << "', check that correct XCLBIN is programmed by AthXRT service");
      return StatusCode::FAILURE;
    }

    // Get memory bank groups for device buffers.
    xrtMemoryGroup bank_grp_in1 = slot.m_kernel->group_id(s_krnl_param_in1);
    xrtMemoryGroup bank_grp_in2 = slot.m_kernel->group_id(s_krnl_param_in2);
    xrtMemoryGroup bank_grp_out = slot.m_kernel->group_id(s_krnl_param_out);

    std::size_t size_in_bytes = s_element_count * sizeof(uint32_t);

    // Create buffer objects.
    // This create aligned buffer object both on host and device.
    slot.m_bo_in1 = std::make_unique<xrt::bo>(
        *slot.m_device, size_in_bytes, xrt::bo::flags::normal, bank_grp_in1);
    slot.m_bo_in2 = std::make_unique<xrt::bo>(
        *slot.m_device, size_in_bytes, xrt::bo::flags::normal, bank_grp_in2);
    slot.m_bo_out = std::make_unique<xrt::bo>(
        *slot.m_device, size_in_bytes, xrt::bo::flags::normal, bank_grp_out);

    // Create run object and set arguments for subsequent executions.
    slot.m_run = std::make_unique<xrt::run>(*slot.m_kernel);
    slot.m_run->set_arg(s_krnl_param_in1, *slot.m_bo_in1);
    slot.m_run->set_arg(s_krnl_param_in2, *slot.m_bo_in2);
    slot.m_run->set_arg(s_krnl_param_out, *slot.m_bo_out);
    slot.m_run->set_arg(s_krnl_param_size, s_element_count);

    ++slotIdx;
  }

  return StatusCode::SUCCESS;
}

StatusCode VectorAddXRTExampleAlg::execute(const EventContext& ctx) const {

  // Get the slot (thread) specific data.
  const SlotData& slot = *m_slots.get(ctx);

  // Map buffer objects to host pointers.
  uint32_t* buffer_in1 = slot.m_bo_in1->map<uint32_t*>();
  uint32_t* buffer_in2 = slot.m_bo_in2->map<uint32_t*>();
  uint32_t* buffer_out = slot.m_bo_out->map<uint32_t*>();

  // Initialize the buffers with random data.
  for (int i = 0; i < s_element_count; ++i) {
    buffer_in1[i] = rand() % s_element_count;
    buffer_in2[i] = rand() % s_element_count;
  }

  ATH_MSG_DEBUG("Transfer data buffer to device");
  slot.m_bo_in1->sync(XCL_BO_SYNC_BO_TO_DEVICE);
  slot.m_bo_in2->sync(XCL_BO_SYNC_BO_TO_DEVICE);

  ATH_MSG_DEBUG("Running kernel");
  slot.m_run->start();
  slot.m_run->wait();

  ATH_MSG_DEBUG("Transfer data back to host");
  slot.m_bo_out->sync(XCL_BO_SYNC_BO_FROM_DEVICE);

  // Check that kernel results are correct.
  bool correct = true;
  for (int i = 0; i < s_element_count; ++i) {
    uint32_t cpu_result = buffer_in1[i] + buffer_in2[i];
    if (buffer_out[i] != cpu_result) {
      ATH_MSG_ERROR("Error: Result mismatch: i = "
                    << i << ": CPU result = " << cpu_result
                    << " Device result = " << buffer_out[i]);
      correct = false;
      break;
    }
  }
  if (correct) {
    ATH_MSG_INFO("XRT vector addition test PASSED!");
  } else {
    ATH_MSG_ERROR("XRT vector addition test FAILED!");
    return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}

}  // namespace AthExXRT
