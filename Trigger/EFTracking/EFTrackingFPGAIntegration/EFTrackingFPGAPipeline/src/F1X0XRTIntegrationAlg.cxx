/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "EFTrackingFPGAPipeline/F1X0XRTIntegrationAlg.h"
#include "AthenaKernel/Chrono.h"
#include "AthenaKernel/SlotSpecificObj.h"

#include <xrt/xrt_bo.h>
#include <xrt/xrt_device.h>
#include <xrt/xrt_kernel.h>
#include <xrt/xrt_uuid.h>

#include <algorithm>
#include <chrono>
#include <cmath>

namespace EFTrackingFPGAIntegration
{

// small helper for ns accounting
static inline uint64_t ns_between(const std::chrono::steady_clock::time_point& a,
                                  const std::chrono::steady_clock::time_point& b)
{
  return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(b - a).count());
}

StatusCode F1X0XRTIntegrationAlg::initialize()
{
  ATH_MSG_INFO("Running on the FPGA accelerator (XRT native)");

  ATH_CHECK(IntegrationBase::precheck({m_xclbin}));
  ATH_CHECK(m_chronoSvc.retrieve());

  // Open device and load xclbin
  {
    Athena::Chrono chrono("XRT::device open", m_chronoSvc.get());
    // TODO: expose as configurable if you need a different board index
    m_xrtDevice = xrt::device(0);
  }
  {
    Athena::Chrono chrono("XRT::load_xclbin", m_chronoSvc.get());
    xrt::xclbin xb(m_xclbin.value());
    m_xrtUuid = m_xrtDevice.load_xclbin(xb);
  }
  ATH_MSG_INFO("loading " << m_xclbin);

  ATH_CHECK(m_FPGAStripRDO.initialize());
  ATH_CHECK(m_FPGAPixelRDO.initialize());
  ATH_CHECK(m_FPGAStripOutput.initialize());
  ATH_CHECK(m_FPGAPixelOutput.initialize());
  ATH_CHECK(m_FPGAPixelRDOSize.initialize());
  ATH_CHECK(m_FPGAStripRDOSize.initialize());
  
  // Enumerate CUs
  std::vector<std::string> listofCUs;
  getListofCUs(listofCUs);

  // Create kernels (per CU)
  for (const auto& cuName : listofCUs) {
    try {
      if (cuName.find(m_pixelClusterKernelName.value()) != std::string::npos)
        m_pixelClusteringKernels.emplace_back(xrt::kernel{m_xrtDevice, m_xrtUuid, cuName.c_str()});
      else if (cuName.find(m_stripClusterKernelName.value()) != std::string::npos)
        m_stripClusteringKernels.emplace_back(xrt::kernel{m_xrtDevice, m_xrtUuid, cuName.c_str()});
      else if (!m_doF110 && cuName.find(m_pixelL2GKernelName.value()) != std::string::npos)
        m_pixelL2GKernels.emplace_back(xrt::kernel{m_xrtDevice, m_xrtUuid, cuName.c_str()});
      else if (cuName.find(m_stripL2GKernelName.value()) != std::string::npos)
        m_stripL2GKernels.emplace_back(xrt::kernel{m_xrtDevice, m_xrtUuid, cuName.c_str()});
      else if (cuName.find(m_pixelEdmKernelName.value()) != std::string::npos)
        m_pixelEdmPrepKernels.emplace_back(xrt::kernel{m_xrtDevice, m_xrtUuid, cuName.c_str()});
      else if (cuName.find(m_stripEdmKernelName.value()) != std::string::npos)
        m_stripEdmPrepKernels.emplace_back(xrt::kernel{m_xrtDevice, m_xrtUuid, cuName.c_str()});
      else
        ATH_MSG_WARNING("Do not recognize kernel name: " << cuName);
    } catch (const std::exception& e) {
      ATH_MSG_ERROR("Failed to create kernel for CU '" << cuName << "': " << e.what());
      return StatusCode::FAILURE;
    }
  }

  ATH_MSG_INFO(m_pixelClusterKernelName.value() << " size: " << m_pixelClusteringKernels.size());
  ATH_MSG_INFO(m_stripClusterKernelName.value() << " size: " << m_stripClusteringKernels.size());
  ATH_MSG_INFO(m_pixelL2GKernelName.value()   << " size: " << m_pixelL2GKernels.size());
  ATH_MSG_INFO(m_stripL2GKernelName.value()   << " size: " << m_stripL2GKernels.size());
  ATH_MSG_INFO(m_pixelEdmKernelName.value()   << " size: " << m_pixelEdmPrepKernels.size());
  ATH_MSG_INFO(m_stripEdmKernelName.value()   << " size: " << m_stripEdmPrepKernels.size());

  // ---------------------------------------------------------------------------
  // Allocate BOs per "thread" (slot). Bind each BO to the bank inferred from
  // the kernel's group_id(argument_index). If kernel vector is empty, fall back to bank 0.
  // ---------------------------------------------------------------------------
  unsigned int nthreads = (m_FPGAThreads.value() < 1) ? SG::getNSlots() : m_FPGAThreads.value();

  auto choose = [](const std::vector<xrt::kernel>& ks) -> const xrt::kernel* {
    return ks.empty() ? nullptr : &ks.front();
  };

  const xrt::kernel* kPC   = choose(m_pixelClusteringKernels);
  const xrt::kernel* kSC   = choose(m_stripClusteringKernels);
  const xrt::kernel* kPL2G = choose(m_pixelL2GKernels);
  const xrt::kernel* kSL2G = choose(m_stripL2GKernels);
  const xrt::kernel* kPEDM = choose(m_pixelEdmPrepKernels);
  const xrt::kernel* kSEDM = choose(m_stripEdmPrepKernels);

  auto gid = [](const xrt::kernel* k, unsigned arg_index)->unsigned {
    return k ? k->group_id(arg_index) : 0;
  };

  for (unsigned i = 0; i < nthreads; ++i) {
    // Inputs
    m_pixelClusterInputBOList.emplace_back(xrt::bo{m_xrtDevice, EFTrackingTransient::PIXEL_CONTAINER_INPUT_BUF_SIZE * sizeof(uint64_t), xrt::bo::flags::normal, gid(kPC, 0)});
    m_stripClusterInputBOList.emplace_back(xrt::bo{m_xrtDevice, EFTrackingTransient::STRIP_CONTAINER_INPUT_BUF_SIZE * sizeof(uint64_t), xrt::bo::flags::normal, gid(kSC, 0)});

   // Clustering outputs
    if (!m_doF110) m_pixelClusterOutputBOList.emplace_back(xrt::bo{m_xrtDevice, EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t),xrt::bo::flags::normal, gid(kPC, 1)});

    m_stripClusterOutputBOList.emplace_back(xrt::bo{m_xrtDevice, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), xrt::bo::flags::normal, gid(kSC, 1)});

    // Pixel clustering EDM output: arg index depends on F110 usage
    m_pixelClusterEDMOutputBOList.emplace_back(xrt::bo{m_xrtDevice, EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), xrt::bo::flags::normal, gid(kPC, m_doF110 ? 1u : 2u)});

    m_stripClusterEDMOutputBOList.emplace_back(xrt::bo{m_xrtDevice, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), xrt::bo::flags::normal, gid(kSC, 2)});

    // L2G outputs
    if (!m_doF110) {
      m_pixelL2GOutputBOList.emplace_back(xrt::bo{m_xrtDevice, EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t), xrt::bo::flags::normal, gid(kPL2G, 2)});
      m_pixelL2GEDMOutputBOList.emplace_back(xrt::bo{m_xrtDevice, EFTrackingTransient::PIXEL_BLOCK_BUF_SIZE * sizeof(uint64_t),xrt::bo::flags::normal, gid(kPL2G, 3)});
    }
    m_stripL2GOutputBOList.emplace_back(xrt::bo{m_xrtDevice, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), xrt::bo::flags::normal, gid(kSL2G, 2)});
    m_stripL2GEDMOutputBOList.emplace_back(xrt::bo{m_xrtDevice, EFTrackingTransient::STRIP_BLOCK_BUF_SIZE * sizeof(uint64_t), xrt::bo::flags::normal, gid(kSL2G, 3)});

    // Final EDM containers (outputs)
    // PixelEDM(arg1) and StripEDM(arg1) are the output BOs
    m_edmPixelOutputBOList.emplace_back(xrt::bo{m_xrtDevice, EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE * sizeof(uint32_t), xrt::bo::flags::normal, gid(kPEDM, 1)});
    m_edmStripOutputBOList.emplace_back(xrt::bo{m_xrtDevice, EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE * sizeof(uint32_t), xrt::bo::flags::normal, gid(kSEDM, 1)});
  }

  return StatusCode::SUCCESS;
}

StatusCode F1X0XRTIntegrationAlg::execute(const EventContext &ctx) const
{
  ATH_MSG_DEBUG("Executing F1X0XRTIntegrationAlg (XRT)");
  m_numEvents++;

  // Inputs
  const std::vector<uint64_t>* pixelInput{nullptr};
  const std::vector<uint64_t>* stripInput{nullptr};
  ATH_CHECK(SG::get(pixelInput, m_FPGAPixelRDO, ctx));
  ATH_CHECK(SG::get(stripInput, m_FPGAStripRDO, ctx));

  const int* pixelInputSize{nullptr}, *stripInputSize{nullptr};
  ATH_CHECK(SG::get(pixelInputSize, m_FPGAPixelRDOSize, ctx));
  ATH_CHECK(SG::get(stripInputSize, m_FPGAStripRDOSize, ctx));


  // Thread/buffer index
  unsigned int nthreads = (m_FPGAThreads.value() < 1) ? SG::getNSlots() : m_FPGAThreads.value();
  const size_t bufferIndex = ctx.slot() % nthreads;

  // Kernel indices
  const size_t pixelClusterIndex = ctx.slot() % m_pixelClusteringKernels.size();
  const size_t stripClusterIndex = ctx.slot() % m_stripClusteringKernels.size();
  const size_t stripL2GIndex     = ctx.slot() % m_stripL2GKernels.size();
  const size_t pixelL2GIndex     = m_pixelL2GKernels.empty() ? 0 : (ctx.slot() % m_pixelL2GKernels.size());
  const size_t pixelEDMIndex     = m_pixelEdmPrepKernels.empty() ? 0 : (ctx.slot() % m_pixelEdmPrepKernels.size());
  const size_t stripEDMIndex     = m_stripEdmPrepKernels.empty() ? 0 : (ctx.slot() % m_stripEdmPrepKernels.size());

  ATH_MSG_INFO("Thread number " << ctx.slot()
               << " running on buffer " << bufferIndex
               << " pixelClusterIndex: " << pixelClusterIndex
               << " stripClusterIndex: " << stripClusterIndex
               << " stripL2GIndex: "    << stripL2GIndex
               << " pixelL2GIndex: "    << pixelL2GIndex
               << " pixelEDMIndex: "    << pixelEDMIndex
               << " stripEDMIndex: "    << stripEDMIndex);

  // BO aliases
  auto& bo_pix_in   = m_pixelClusterInputBOList[bufferIndex];
  auto& bo_str_in   = m_stripClusterInputBOList[bufferIndex];

  xrt::bo* bo_pix_cl_out = (!m_doF110) ? &m_pixelClusterOutputBOList[bufferIndex] : nullptr;
  auto& bo_pix_cl_edm = m_pixelClusterEDMOutputBOList[bufferIndex];

  auto& bo_str_cl        = m_stripClusterOutputBOList[bufferIndex];
  auto& bo_str_cl_edm    = m_stripClusterEDMOutputBOList[bufferIndex];

  xrt::bo* bo_pix_l2g_out = (!m_doF110) ? &m_pixelL2GOutputBOList[bufferIndex] : nullptr;
  xrt::bo* bo_pix_l2g_edm = (!m_doF110) ? &m_pixelL2GEDMOutputBOList[bufferIndex] : nullptr;

  auto& bo_str_l2g_out   = m_stripL2GOutputBOList[bufferIndex];
  auto& bo_str_l2g_edm   = m_stripL2GEDMOutputBOList[bufferIndex];

  auto& bo_pix_edm_cont  = m_edmPixelOutputBOList[bufferIndex];
  auto& bo_str_edm_cont  = m_edmStripOutputBOList[bufferIndex];

  // Write inputs (and time them)
  const auto t_wi0 = std::chrono::steady_clock::now();
  bo_pix_in.write(pixelInput->data(), pixelInput->size() * sizeof(uint64_t), 0);
  bo_pix_in.sync(XCL_BO_SYNC_BO_TO_DEVICE);
  const auto t_wi1 = std::chrono::steady_clock::now();
  m_pixelInputTime += ns_between(t_wi0, t_wi1);
  ATH_MSG_DEBUG("Pixel input buffer write time: " << (ns_between(t_wi0, t_wi1) / 1e6) << " ms");

  const auto t_wi2 = std::chrono::steady_clock::now();
  bo_str_in.write(stripInput->data(), stripInput->size() * sizeof(uint64_t), 0);
  bo_str_in.sync(XCL_BO_SYNC_BO_TO_DEVICE);
  const auto t_wi3 = std::chrono::steady_clock::now();
  m_stripInputTime += ns_between(t_wi2, t_wi3);
  ATH_MSG_DEBUG("Strip input buffer write time: " << (ns_between(t_wi2, t_wi3) / 1e6) << " ms");

  // Launch kernels
  const auto t_k0 = std::chrono::steady_clock::now();

  // Pixel clustering
  auto& k_pix_cl = m_pixelClusteringKernels[pixelClusterIndex];
  xrt::run r_pix_cl{k_pix_cl};
  r_pix_cl.set_arg(0, bo_pix_in);
  if (m_doF110) {
    r_pix_cl.set_arg(1, bo_pix_cl_edm);
  } else {
    r_pix_cl.set_arg(1, *bo_pix_cl_out);
    r_pix_cl.set_arg(2, bo_pix_cl_edm);

    // extra size args (bytes), rounded to 256 elements
    int rounded = static_cast<int>(std::ceil(static_cast<double>(*pixelInputSize) / 256.0)) * 256;
    uint32_t hit_bytes     = static_cast<uint32_t>(sizeof(uint64_t) * rounded);
    uint32_t cluster_bytes = static_cast<uint32_t>(sizeof(uint64_t) * rounded);
    uint32_t edm_bytes     = static_cast<uint32_t>(sizeof(uint64_t) * rounded * 8);
    r_pix_cl.set_arg(3, hit_bytes);
    r_pix_cl.set_arg(4, cluster_bytes);
    r_pix_cl.set_arg(5, edm_bytes);
  }
  const auto t_pc_start = std::chrono::steady_clock::now();
  r_pix_cl.start();
  // Strip clustering
  auto& k_str_cl = m_stripClusteringKernels[stripClusterIndex];
  xrt::run r_str_cl{k_str_cl};
  r_str_cl.set_arg(0, bo_str_in);
  r_str_cl.set_arg(1, bo_str_cl);
  r_str_cl.set_arg(2, bo_str_cl_edm);
  r_str_cl.set_arg(3, static_cast<unsigned int>(*stripInputSize));
  const auto t_sc_start = std::chrono::steady_clock::now();
  r_str_cl.start();

  r_pix_cl.wait();
  const auto t_pc_done = std::chrono::steady_clock::now();
  m_pixelClusteringTime += ns_between(t_pc_start, t_pc_done);
  ATH_MSG_DEBUG("Pixel clustering time: " << (ns_between(t_pc_start, t_pc_done) / 1e6) << " ms");

  r_str_cl.wait();
  const auto t_sc_done = std::chrono::steady_clock::now();
  m_stripClusteringTime += ns_between(t_sc_start, t_sc_done);
  ATH_MSG_DEBUG("Strip clustering time: " << (ns_between(t_sc_start, t_sc_done) / 1e6) << " ms");

  // Pixel L2G (only for F100)
  std::chrono::steady_clock::time_point t_pl2g_done = t_pc_done;
  if (!m_doF110) {
    auto& k_pix_l2g = m_pixelL2GKernels[pixelL2GIndex];
    xrt::run r_pix_l2g{k_pix_l2g};
    r_pix_l2g.set_arg(0, *bo_pix_cl_out);
    r_pix_l2g.set_arg(1, bo_pix_cl_edm);
    r_pix_l2g.set_arg(2, *bo_pix_l2g_out);
    r_pix_l2g.set_arg(3, *bo_pix_l2g_edm);
    const auto t_pl2g_start = std::chrono::steady_clock::now();
    r_pix_l2g.start();
    r_pix_l2g.wait();
    t_pl2g_done = std::chrono::steady_clock::now();
    m_pixelL2GTime += ns_between(t_pl2g_start, t_pl2g_done);
    ATH_MSG_DEBUG("Pixel L2G time: " << (ns_between(t_pl2g_start, t_pl2g_done) / 1e6) << " ms");
  }

  // Strip L2G
  auto& k_str_l2g = m_stripL2GKernels[stripL2GIndex];
  xrt::run r_str_l2g{k_str_l2g};
  r_str_l2g.set_arg(0, bo_str_cl);
  r_str_l2g.set_arg(1, bo_str_cl_edm);
  r_str_l2g.set_arg(2, bo_str_l2g_out);
  r_str_l2g.set_arg(3, bo_str_l2g_edm);
  const auto t_sl2g_start = std::chrono::steady_clock::now();
  r_str_l2g.start();
  r_str_l2g.wait();
  const auto t_sl2g_done = std::chrono::steady_clock::now();
  m_stripL2GTime += ns_between(t_sl2g_start, t_sl2g_done);
  ATH_MSG_DEBUG("Strip L2G time: " << (ns_between(t_sl2g_start, t_sl2g_done) / 1e6) << " ms");

  // EDM Prep (always use PixelEDM and StripEDM kernels)
  auto& k_pedm = m_pixelEdmPrepKernels[pixelEDMIndex];
  auto& k_sedm = m_stripEdmPrepKernels[stripEDMIndex];

  xrt::run r_pedm{k_pedm};
  r_pedm.set_arg(0, bo_pix_cl_edm);
  r_pedm.set_arg(1, bo_pix_edm_cont);

  xrt::run r_sedm{k_sedm};
  r_sedm.set_arg(0, bo_str_l2g_edm);
  r_sedm.set_arg(1, bo_str_edm_cont);

  // Respect dependencies:
  // - PixelEDM depends on pixel clustering (F110) or pixel L2G (F100)
  // - StripEDM depends on strip L2G
  const auto t_pedm_start = std::chrono::steady_clock::now();
  r_pedm.start();

  const auto t_sedm_start = std::chrono::steady_clock::now();
  // already waited for r_str_l2g
  r_sedm.start();

  r_pedm.wait();
  const auto t_pedm_done = std::chrono::steady_clock::now();
  m_pixelEdmPrepTime += ns_between(t_pedm_start, t_pedm_done);
  ATH_MSG_DEBUG("PixelEDMPrep time: " << (ns_between(t_pedm_start, t_pedm_done) / 1e6) << " ms");

  r_sedm.wait();
  const auto t_sedm_done = std::chrono::steady_clock::now();
  m_stripEdmPrepTime += ns_between(t_sedm_start, t_sedm_done);
  ATH_MSG_DEBUG("StripEDMPrep time: " << (ns_between(t_sedm_start, t_sedm_done) / 1e6) << " ms");

  // Kernel window = [first start, last end]
  const auto t_kend = std::max(t_pedm_done, t_sedm_done);
  m_kernelTime += ns_between(t_k0, t_kend);
  ATH_MSG_DEBUG("Kernel execution time: " << (ns_between(t_k0, t_kend) / 1e6) << " ms");

  // Output handles and readbacks
  SG::WriteHandle<std::vector<uint32_t>> FPGAPixelOutput(m_FPGAPixelOutput, ctx);
  ATH_CHECK(FPGAPixelOutput.record(std::make_unique<std::vector<uint32_t>>(EFTrackingTransient::PIXEL_CONTAINER_BUF_SIZE, 0)));

  SG::WriteHandle<std::vector<uint32_t>> FPGAStripOutput(m_FPGAStripOutput, ctx);
  ATH_CHECK(FPGAStripOutput.record(std::make_unique<std::vector<uint32_t>>(EFTrackingTransient::STRIP_CONTAINER_BUF_SIZE, 0)));

  const auto t_ro0 = std::chrono::steady_clock::now();
  bo_pix_edm_cont.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
  bo_pix_edm_cont.read(FPGAPixelOutput->data(), FPGAPixelOutput->size() * sizeof(uint64_t), 0);
  const auto t_ro1 = std::chrono::steady_clock::now();
  m_pixelOutputTime += ns_between(t_ro0, t_ro1);
  ATH_MSG_DEBUG("Pixel output buffer read time: " << (ns_between(t_ro0, t_ro1) / 1e6) << " ms");

  const auto t_ro2 = std::chrono::steady_clock::now();
  bo_str_edm_cont.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
  bo_str_edm_cont.read(FPGAStripOutput->data(), FPGAStripOutput->size() * sizeof(uint64_t), 0);
  const auto t_ro3 = std::chrono::steady_clock::now();
  m_stripOutputTime += ns_between(t_ro2, t_ro3);
  ATH_MSG_DEBUG("Strip output buffer read time: " << (ns_between(t_ro2, t_ro3) / 1e6) << " ms");


  if(*pixelInputSize == 6) (*FPGAPixelOutput)[0] = 0; // if no pixel input, set the first element to 0
  if(*stripInputSize == 6) (*FPGAStripOutput)[0] = 0; // if no strip input, set the first element to 0

  return StatusCode::SUCCESS;
}

StatusCode F1X0XRTIntegrationAlg::finalize()
{
  ATH_MSG_INFO("Finalizing F1X0XRTIntegrationAlg");
  ATH_MSG_INFO("Number of events: " << m_numEvents);

  if (m_numEvents > 0) {
    ATH_MSG_INFO("Pixel input ave time: "   << m_pixelInputTime   / m_numEvents / 1e6 << " ms");
    ATH_MSG_INFO("Strip input ave time: "   << m_stripInputTime   / m_numEvents / 1e6 << " ms");
    ATH_MSG_INFO("Pixel clustering ave time: " << m_pixelClusteringTime / m_numEvents / 1e6 << " ms");
    ATH_MSG_INFO("Strip clustering ave time: " << m_stripClusteringTime / m_numEvents / 1e6 << " ms");
    if (!m_doF110) {
      ATH_MSG_INFO("Pixel L2G ave time: "   << m_pixelL2GTime     / m_numEvents / 1e6 << " ms");
    }
    ATH_MSG_INFO("Strip L2G ave time: "     << m_stripL2GTime     / m_numEvents / 1e6 << " ms");
    ATH_MSG_INFO("PixelEDMPrep ave time: "  << m_pixelEdmPrepTime / m_numEvents / 1e6 << " ms");
    ATH_MSG_INFO("StripEDMPrep ave time: "  << m_stripEdmPrepTime / m_numEvents / 1e6 << " ms");
    ATH_MSG_INFO("Kernel execution ave time: " << m_kernelTime    / m_numEvents / 1e6 << " ms");
    ATH_MSG_INFO("Pixel output ave time: "  << m_pixelOutputTime  / m_numEvents / 1e6 << " ms");
    ATH_MSG_INFO("Strip output ave time: "  << m_stripOutputTime  / m_numEvents / 1e6 << " ms");
  }

  return StatusCode::SUCCESS;
}

void F1X0XRTIntegrationAlg::getListofCUs(std::vector<std::string>& cuNames)
{
  xrt::xclbin xrt_xclbin(m_xclbin.value());

  ATH_MSG_INFO("xsa name: "  << xrt_xclbin.get_xsa_name());
  ATH_MSG_INFO("fpga name: " << xrt_xclbin.get_fpga_device_name());
  ATH_MSG_INFO("uuid: "      << xrt_xclbin.get_uuid().to_string());

  for (const xrt::xclbin::kernel &kernel : xrt_xclbin.get_kernels()) {
    const std::string& kernelName = kernel.get_name();
    ATH_MSG_INFO("kernelName: " << kernelName);
    for (const xrt::xclbin::ip &computeUnit : kernel.get_cus()) {
      const std::string& computeUnitName = computeUnit.get_name();
      const std::string computeUnitIsolatedName = computeUnitName.substr(kernelName.size() + 1);
      const std::string computeUnitUsableName   = kernelName + ":{" + computeUnitIsolatedName + "}";
      ATH_MSG_INFO("CU name: " << computeUnitUsableName);
      cuNames.push_back(computeUnitUsableName);
    }
  }
}

} // namespace EFTrackingFPGAIntegration
