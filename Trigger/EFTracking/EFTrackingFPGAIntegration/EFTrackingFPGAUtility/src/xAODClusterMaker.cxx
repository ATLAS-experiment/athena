/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
   */

/**
 * @file src/xAODClusterMaker.cxx
 * @author zhaoyuan.cui@cern.ch
 * @author yuan-tang.chou@cern.ch
 * @author levi.samuel.evans@cern.ch
 * @date Mar. 11, 2025
 */

constexpr bool enableBenchmark = 
#ifdef STANDALONE_FPGA_BENCHMARK
true;
#else
false;
#endif

#include "EFTrackingFPGAUtility/xAODClusterMaker.h"

#include "AthContainers/Accessor.h"
#include "AthenaKernel/Chrono.h"
#include "CxxUtils/copy_bounded.h"
#include "CxxUtils/span.h"
#include "Identifier/Identifier.h"
#include "StoreGate/WriteHandle.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"
#include "AthAllocators/DataPool.h"
#include <bit>


namespace {

uint64_t to64 (const uint32_t* x)
{
  struct { uint32_t a[2]; } s = {x[0], x[1]};
  return std::bit_cast<uint64_t> (s);
}

} // anonymous namespace


StatusCode xAODClusterMaker::initialize() {
    ATH_MSG_INFO("Initialising xAODClusterMaker tool");

    // Initialise the write handles
    ATH_CHECK(m_pixelClustersKey.initialize());
    ATH_CHECK(m_stripClustersKey.initialize());

    // Initialise the chrono service
    ATH_CHECK(m_chronoSvc.retrieve());

    return StatusCode::SUCCESS;
}

StatusCode xAODClusterMaker::makeStripClusterContainer(
        const uint32_t* stripClusters,
        const EFTrackingTransient::Metadata *metadata,
        const EventContext &ctx) const {
    ATH_MSG_DEBUG("Making xAOD::StripClusterContainer");

    SG::WriteHandle<xAOD::StripClusterContainer> stripClustersHandle{m_stripClustersKey, ctx};

    if (!m_doBulkCopy) {
        ATH_CHECK(stripClustersHandle.record(std::make_unique<xAOD::StripClusterContainer>(), std::make_unique<xAOD::StripClusterAuxContainer>()));


        for (unsigned int i = 0; i < metadata->numOfStripClusters; i++)
        {
            // Push back numClusters of StripCluster
            auto stripCl = stripClustersHandle->push_back(std::make_unique<xAOD::StripCluster>());

            // Build Matrix
            Eigen::Matrix<float, 1, 1> localPosition;
            Eigen::Matrix<float, 1, 1> localCovariance;
            int row = 0; // idhash
            unsigned int idHash   = stripClusters[row * EFTrackingTransient::MAX_STRIP_CLUSTERS + i + 16];
            row = 1; // id
            uint64_t id = to64 (stripClusters + row * EFTrackingTransient::MAX_STRIP_CLUSTERS + i*2 + 16);

            std::vector<Identifier> RDOs;
            row = 3; // rdo w1
            unsigned long long rdo = to64 (stripClusters + row * EFTrackingTransient::MAX_STRIP_CLUSTERS + i*2 + 16);
            if (rdo) RDOs.push_back(Identifier(rdo));

            row = 5; // rdo w2
            rdo = to64 (stripClusters + row * EFTrackingTransient::MAX_STRIP_CLUSTERS + i*2 + 16);
            if (rdo) RDOs.push_back(Identifier(rdo));

            row = 7; // local x
            localPosition(0, 0) = std::bit_cast<float>(stripClusters[row * EFTrackingTransient::MAX_STRIP_CLUSTERS + i + 16]);
            row = 8; // local covariance xx
            localCovariance(0, 0) = std::bit_cast<float>(stripClusters[row * EFTrackingTransient::MAX_STRIP_CLUSTERS + i + 16]);
            row = 9; // global x
            float globalX = std::bit_cast<float>(stripClusters[row * EFTrackingTransient::MAX_STRIP_CLUSTERS + i + 16]);
            row = 10; // global y
            float globalY = std::bit_cast<float>(stripClusters[row * EFTrackingTransient::MAX_STRIP_CLUSTERS + i + 16]);
            row = 11; // global z
            float globalZ = std::bit_cast<float>(stripClusters[row * EFTrackingTransient::MAX_STRIP_CLUSTERS + i + 16]);
            row = 12; // channels in phi
            auto channelsinPhi = stripClusters[row * EFTrackingTransient::MAX_STRIP_CLUSTERS + i + 16];


            Eigen::Matrix<float, 3, 1> globalPosition(globalX, globalY, globalZ);
            stripCl->setMeasurement<1>(idHash, localPosition, localCovariance);

            stripCl->setIdentifier(id);
            stripCl->setRDOlist(RDOs);
            stripCl->globalPosition() = globalPosition;
            stripCl->setChannelsInPhi(channelsinPhi);
        }

        return StatusCode::SUCCESS;
    }

    ATH_MSG_DEBUG("Doing Strip bulk copy (optimized)");
    // --------------------------
    // Create the container and aux. container
    // --------------------------
    const size_t nClusters = metadata->numOfStripClusters;
    DataPool<xAOD::StripCluster> stripPool{ctx};
    stripPool.reserve(nClusters);
    auto stripCl = std::make_unique<xAOD::StripClusterContainer>(SG::VIEW_ELEMENTS, SG::ALWAYS_TRACK_INDICES);
    {
        if constexpr (enableBenchmark) Athena::Chrono chrono("Strip object creating", m_chronoSvc.get());
        stripCl->push_new(nClusters, [&stripPool]() { return stripPool.nextElementPtr(); });
    }
    auto stripClAux = std::make_unique<xAOD::StripClusterAuxContainer>();
    stripClAux->resize(nClusters);
    stripCl->setStore(stripClAux.get());
    // --------------------------
    // Access fixed-size attributes directly (rows/offsets consistent with !m_doBulkCopy)
    // --------------------------
    const size_t N      = EFTrackingTransient::MAX_STRIP_CLUSTERS;
    const size_t offset = 16;

    // Treat the raw column-major buffer as 32-bit words; 64-bit values span two words.
    const uint32_t* base = stripClusters;

    auto load_u64 = [&](size_t row, size_t idx) -> uint64_t {
        const uint32_t* p32 = base + row * N + offset + 2 * idx;
        return *reinterpret_cast<const uint64_t*>(p32);
    };
    auto load_f32 = [&](size_t row, size_t idx) -> float {
        return std::bit_cast<float>(*(base + row * N + offset + idx));
    };
    auto load_u32 = [&](size_t row, size_t idx) -> uint32_t {
        return *(base + row * N + offset + idx);
    };

    // Column rows (as in !m_doBulkCopy)
    constexpr size_t ROW_idHash      = 0;
    constexpr size_t ROW_identifier  = 1;  // 64-bit (two u32 words)
    // (row 2 unused for Strip; keep aligned with transient layout)
    constexpr size_t ROW_rdo_w1      = 3;  // 64-bit (two u32 words)
    constexpr size_t ROW_rdo_w2      = 5;  // 64-bit (two u32 words)
    constexpr size_t ROW_localPosX   = 7;  // float (from u32)
    constexpr size_t ROW_localCovXX  = 8;  // float (from u32)
    constexpr size_t ROW_gpX         = 9;  // float (from u32)
    constexpr size_t ROW_gpY         = 10; // float (from u32)
    constexpr size_t ROW_gpZ         = 11; // float (from u32)
    constexpr size_t ROW_channelsPhi = 12; // int (from u32)

    // --------------------------
    // Precompute spans into aux data store (types match the non-bulk path)
    // --------------------------
    if (nClusters > 0) {
        static const SG::Accessor<unsigned int> idHashAcc("identifierHash");
        static const SG::Accessor<unsigned long> idAcc("identifier");
        static const SG::Accessor<std::array<float, 1>> locPosXAcc("localPositionDim1");
        static const SG::Accessor<std::array<float, 1>> locCovXXAcc("localCovarianceDim1");
        static const SG::Accessor<std::array<float, 3>> gpAcc("globalPosition");
        static const SG::Accessor<int> channelsPhiAcc("channelsInPhi");
        static const SG::Accessor<std::vector<unsigned long long>> rdoListAcc("rdoList");

        auto idHashSpan      = idHashAcc.getDataSpan(*stripCl);
        auto idSpan          = idAcc.getDataSpan(*stripCl);
        auto locPosXSpan     = locPosXAcc.getDataSpan(*stripCl);
        auto locCovXXSpan    = locCovXXAcc.getDataSpan(*stripCl);
        auto gpSpan          = gpAcc.getDataSpan(*stripCl);
        auto channelsPhiSpan = channelsPhiAcc.getDataSpan(*stripCl);
        auto rdoSpan         = rdoListAcc.getDataSpan(*stripCl);

        // --------------------------
        // Vectorized bulk assignments
        // --------------------------
        {
            if constexpr (enableBenchmark) Athena::Chrono chrono("Strip assignments", m_chronoSvc.get());
            for (size_t i = 0; i < nClusters; ++i) {
                // idHash: 32-bit value
                idHashSpan[i] = static_cast<unsigned int>(load_u32(ROW_idHash, i));

                // identifier: 64-bit (two words)
                idSpan[i] = static_cast<unsigned long long>(load_u64(ROW_identifier, i));

                // local position / covariance (Dim1)
                locPosXSpan[i][0]  = load_f32(ROW_localPosX,  i);
                locCovXXSpan[i][0] = load_f32(ROW_localCovXX, i);

                // global position (x,y,z)
                gpSpan[i][0] = load_f32(ROW_gpX, i);
                gpSpan[i][1] = load_f32(ROW_gpY, i);
                gpSpan[i][2] = load_f32(ROW_gpZ, i);

                // channels in phi
                channelsPhiSpan[i] = static_cast<int>(load_u32(ROW_channelsPhi, i));

                // RDO list: up to 2 entries (Identifier)
                auto& rdoList = rdoSpan[i];
                rdoList.reserve(2);
                uint64_t rdo1 = load_u64(ROW_rdo_w1, i);
                uint64_t rdo2 = load_u64(ROW_rdo_w2, i);
                if (rdo1) rdoList.emplace_back(rdo1);
                if (rdo2) rdoList.emplace_back(rdo2);
            }
        }
    }

    {
        if constexpr (enableBenchmark) Athena::Chrono chrono("Strip Copy", m_chronoSvc.get());
        ATH_CHECK(stripClustersHandle.record(std::move(stripCl), std::move(stripClAux)));
    }

    return StatusCode::SUCCESS;
}



StatusCode xAODClusterMaker::makeStripClusterContainer(
        const EFTrackingTransient::StripClusterAuxInput &scAux,
        const EFTrackingTransient::Metadata *metadata,
        const EventContext &ctx) const {
    ATH_MSG_DEBUG("Making xAOD::StripClusterContainer");

    SG::WriteHandle<xAOD::StripClusterContainer> stripClustersHandle{
        m_stripClustersKey , ctx};

    ATH_CHECK(stripClustersHandle.record(
                std::make_unique<xAOD::StripClusterContainer>(),
                std::make_unique<xAOD::StripClusterAuxContainer>()));

    int rdoIndexCounter = 0;

    for (unsigned int i = 0; i < metadata->numOfStripClusters; i++) {
        // Push back numClusters of StripCluster
        auto stripCl =
            stripClustersHandle->push_back(std::make_unique<xAOD::StripCluster>());

        // Build Matrix
        Eigen::Matrix<float, 1, 1> localPosition;
        Eigen::Matrix<float, 1, 1> localCovariance;

        localPosition(0, 0) = scAux.localPosition.at(i);
        localCovariance(0, 0) = scAux.localCovariance.at(i);

        Eigen::Matrix<float, 3, 1> globalPosition(
                scAux.globalPosition.at(i * 3), scAux.globalPosition.at(i * 3 + 1),
                scAux.globalPosition.at(i * 3 + 2));

        std::vector<Identifier> RDOs;
        RDOs.reserve(metadata->scRdoIndex[i]);
        // Cover RDO
        for (unsigned int j = 0; j < metadata->scRdoIndex[i]; ++j) {
            RDOs.push_back(Identifier(scAux.rdoList.at(rdoIndexCounter + j)));
        }

        rdoIndexCounter += metadata->scRdoIndex[i];

        stripCl->setMeasurement<1>(scAux.idHash.at(i), localPosition,
                localCovariance);
        stripCl->setIdentifier(scAux.id.at(i));
        stripCl->setRDOlist(RDOs);
        stripCl->globalPosition() = globalPosition;
        stripCl->setChannelsInPhi(scAux.channelsInPhi.at(i));
    }
    return StatusCode::SUCCESS;
}


StatusCode xAODClusterMaker::makePixelClusterContainer(
        const uint32_t* pixelClusters,
        const EFTrackingTransient::Metadata *metadata,
        const EventContext &ctx) const {
    ATH_MSG_DEBUG("Making xAOD::PixelClusterContainer");


    SG::WriteHandle<xAOD::PixelClusterContainer> pixelClustersHandle{m_pixelClustersKey, ctx};

    if (!m_doBulkCopy) {
        // --------------------------------------------------------------------
        // proceed with the element-wise method
        // --------------------------------------------------------------------
        ATH_CHECK(pixelClustersHandle.record(std::make_unique<xAOD::PixelClusterContainer>(),std::make_unique<xAOD::PixelClusterAuxContainer>()));

        ATH_CHECK(pixelClustersHandle.isValid());

        for (unsigned int i = 0; i < metadata->numOfPixelClusters; i++)
        {
            // Push back numClusters of StripCluster
            auto pixelCl = pixelClustersHandle->push_back(std::make_unique<xAOD::PixelCluster>());

            int row = 0; // idhash
            unsigned int idHash = pixelClusters[row * EFTrackingTransient::MAX_PIXEL_CLUSTERS + i + 16];
            row = 1; // id
            unsigned long long id = to64 (pixelClusters + row * EFTrackingTransient::MAX_PIXEL_CLUSTERS + i*2 + 16);

            std::vector<Identifier> RDOs;
            row = 3; // rdo w1
            unsigned long long rdo = to64 (pixelClusters + row * EFTrackingTransient::MAX_PIXEL_CLUSTERS + i*2 + 16);
            if (rdo) RDOs.push_back(Identifier(rdo));

            row = 5; // rdo w2
            rdo = to64 (pixelClusters + row * EFTrackingTransient::MAX_PIXEL_CLUSTERS + i*2 + 16);
            if (rdo) RDOs.push_back(Identifier(rdo));

            row = 7; // rdo w3
            rdo = to64 (pixelClusters + row * EFTrackingTransient::MAX_PIXEL_CLUSTERS + i*2 + 16);
            if (rdo) RDOs.push_back(Identifier(rdo));

            row = 9; // rdo w4
            rdo = to64 (pixelClusters + row * EFTrackingTransient::MAX_PIXEL_CLUSTERS + i*2 + 16);
            if (rdo) RDOs.push_back(Identifier(rdo));

            Eigen::Matrix<float, 2, 1> localPosition;
            Eigen::Matrix<float, 2, 2> localCovariance;
            localCovariance.setZero();
            row = 11; // local x
            localPosition(0 , 0) = std::bit_cast<float>(pixelClusters[row * EFTrackingTransient::MAX_PIXEL_CLUSTERS + i + 16]);

            row = 12; // local y
            localPosition(1 , 1) = std::bit_cast<float>(pixelClusters[row * EFTrackingTransient::MAX_PIXEL_CLUSTERS + i + 16]);

            row = 13; // local covariance xx
            localCovariance(0, 0) = std::bit_cast<float>(pixelClusters[row * EFTrackingTransient::MAX_PIXEL_CLUSTERS + i + 16]);

            row = 14; // local covariance yy
            localCovariance(1, 1) = std::bit_cast<float>(pixelClusters[row * EFTrackingTransient::MAX_PIXEL_CLUSTERS + i + 16]);

            row = 15; // global x
            float globalX = std::bit_cast<float>(pixelClusters[row * EFTrackingTransient::MAX_PIXEL_CLUSTERS + i + 16]);

            row = 16; // global y
            float globalY = std::bit_cast<float>(pixelClusters[row * EFTrackingTransient::MAX_PIXEL_CLUSTERS + i + 16]);

            row = 17; // global 
            float globalZ = std::bit_cast<float>(pixelClusters[row * EFTrackingTransient::MAX_PIXEL_CLUSTERS + i + 16]);

            row = 18; // channels in phi
            int channelsInPhi = pixelClusters[row * EFTrackingTransient::MAX_PIXEL_CLUSTERS + i + 16];

            row = 19; // channels in eta
            int channelsInEta = pixelClusters[row * EFTrackingTransient::MAX_PIXEL_CLUSTERS + i + 16];

            row = 20; // width in eta
            double widthInEta = std::bit_cast<float>(pixelClusters[row * EFTrackingTransient::MAX_PIXEL_CLUSTERS + i + 16]);

            row = 21; // total ToT
            int totalToT = pixelClusters[row * EFTrackingTransient::MAX_PIXEL_CLUSTERS + i + 16];

            Eigen::Matrix<float, 3, 1> globalPosition(globalX, globalY, globalZ);

            pixelCl->setMeasurement<2>(idHash, localPosition,localCovariance);
            pixelCl->setIdentifier(id);
            pixelCl->setRDOlist(RDOs);
            pixelCl->globalPosition() = globalPosition;
            pixelCl->setTotalToT(totalToT);
            pixelCl->setChannelsInPhiEta(channelsInPhi, channelsInEta);
            pixelCl->setWidthInEta(widthInEta);
        }
        return StatusCode::SUCCESS;
    }

    ATH_MSG_DEBUG("Doing bulk copy (optimized)");
    // --------------------------
    // Create the container and aux. container
    // --------------------------
    const size_t nClusters = metadata->numOfPixelClusters;  
    DataPool<xAOD::PixelCluster> pixelPool{ctx};
    pixelPool.reserve(nClusters);
    auto pixelCl = std::make_unique<xAOD::PixelClusterContainer>(SG::VIEW_ELEMENTS, SG::ALWAYS_TRACK_INDICES);
    {
        if constexpr (enableBenchmark) Athena::Chrono chrono("Pixel object creating", m_chronoSvc.get());
        pixelCl->push_new(nClusters, [&pixelPool]() { return pixelPool.nextElementPtr(); });
    }
    auto pixelClAux = std::make_unique<xAOD::PixelClusterAuxContainer>();
    pixelClAux->resize(nClusters);
    pixelCl->setStore(pixelClAux.get());

    // --------------------------
    // Access fixed-size attributes directly (rows/offsets as in !m_doBulkCopy)
    // --------------------------
    const size_t N      = EFTrackingTransient::MAX_PIXEL_CLUSTERS;
    const size_t offset = 16;

    // Treat the raw column-major buffer as 32-bit words; 64-bit values span two words.
    const uint32_t* base = pixelClusters;

    auto load_u64 = [&](size_t row, size_t idx) -> uint64_t {
        const uint32_t* p32 = base + row * N + offset + 2 * idx;
        return *reinterpret_cast<const uint64_t*>(p32);
    };
    auto load_f32 = [&](size_t row, size_t idx) -> float {
        return std::bit_cast<float>(*(base + row * N + offset + idx));
    };
    auto load_u32 = [&](size_t row, size_t idx) -> uint32_t {
        return *(base + row * N + offset + idx);
    };

    // Row layout (must match the element-wise path)
    constexpr size_t ROW_idHash       = 0;
    constexpr size_t ROW_identifier   = 1;   // u64
    constexpr size_t ROW_rdo_w1       = 3;   // u64
    constexpr size_t ROW_rdo_w2       = 5;   // u64
    constexpr size_t ROW_rdo_w3       = 7;   // u64
    constexpr size_t ROW_rdo_w4       = 9;   // u64
    constexpr size_t ROW_localX       = 11;  // f32
    constexpr size_t ROW_localY       = 12;  // f32
    constexpr size_t ROW_covXX        = 13;  // f32
    constexpr size_t ROW_covYY        = 14;  // f32
    constexpr size_t ROW_gpX          = 15;  // f32
    constexpr size_t ROW_gpY          = 16;  // f32
    constexpr size_t ROW_gpZ          = 17;  // f32
    constexpr size_t ROW_channelsPhi  = 18;  // i32
    constexpr size_t ROW_channelsEta  = 19;  // i32
    constexpr size_t ROW_widthEta     = 20;  // f32
    constexpr size_t ROW_totalToT     = 21;  // i32

    // --------------------------
    // Precompute spans into aux data store (types aligned with !m_doBulkCopy behavior)
    // --------------------------
    if (nClusters > 0) {
        static const SG::Accessor<unsigned int> idHashAcc("identifierHash");
        static const SG::Accessor<unsigned long> idAcc("identifier");
        static const SG::Accessor<std::array<float, 3>> gpAcc("globalPosition");
        static const SG::Accessor<std::array<float, 2>> locPosAcc("localPositionDim2");
        static const SG::Accessor<std::array<float, 4>> locCovAcc("localCovarianceDim2");
        static const SG::Accessor<int> totalToTAcc("totalToT");
        static const SG::Accessor<int> channelsPhiAcc("channelsInPhi");
        static const SG::Accessor<int> channelsEtaAcc("channelsInEta");
        static const SG::Accessor<float> widthEtaAcc("widthInEta");
        static const SG::Accessor<std::vector<unsigned long long>> rdoListAcc("rdoList");


        auto idHashSpan      = idHashAcc.getDataSpan(*pixelCl);
        auto idSpan          = idAcc.getDataSpan(*pixelCl);
        auto gpSpan          = gpAcc.getDataSpan(*pixelCl);
        auto locPosSpan      = locPosAcc.getDataSpan(*pixelCl);
        auto locCovSpan      = locCovAcc.getDataSpan(*pixelCl);
        auto totalToTSpan    = totalToTAcc.getDataSpan(*pixelCl);
        auto channelsPhiSpan = channelsPhiAcc.getDataSpan(*pixelCl);
        auto channelsEtaSpan = channelsEtaAcc.getDataSpan(*pixelCl);
        auto widthEtaSpan    = widthEtaAcc.getDataSpan(*pixelCl);
        auto rdoSpan         = rdoListAcc.getDataSpan(*pixelCl);

        // --------------------------
        // Vectorized bulk assignments
        // --------------------------
        {
            if constexpr (enableBenchmark) Athena::Chrono chrono("Pixel assignments", m_chronoSvc.get());
            for (size_t i = 0; i < nClusters; ++i) {
                // Basic ids
                idHashSpan[i] = static_cast<unsigned int>(load_u32(ROW_idHash, i));
                idSpan[i]     = static_cast<unsigned long long>(load_u64(ROW_identifier, i));

                // Local position (2x1)
                locPosSpan[i][0] = load_f32(ROW_localX, i);
                locPosSpan[i][1] = load_f32(ROW_localY, i);

                // Local covariance (2x2) flattened as [xx, xy, yx, yy]
                locCovSpan[i][0] = load_f32(ROW_covXX, i);
                locCovSpan[i][3] = load_f32(ROW_covYY, i);

                // Global position
                gpSpan[i][0] = load_f32(ROW_gpX, i);
                gpSpan[i][1] = load_f32(ROW_gpY, i);
                gpSpan[i][2] = load_f32(ROW_gpZ, i);

                // Discrete/int attrs
                channelsPhiSpan[i] = static_cast<int>(load_u32(ROW_channelsPhi, i));
                channelsEtaSpan[i] = static_cast<int>(load_u32(ROW_channelsEta, i));
                widthEtaSpan[i]    = load_f32(ROW_widthEta, i);
                totalToTSpan[i]    = static_cast<int>(load_u32(ROW_totalToT, i));

                // RDOs (up to 4)
                auto& rdoList = rdoSpan[i];
                rdoList.reserve(4);
                uint64_t r1 = load_u64(ROW_rdo_w1, i);
                uint64_t r2 = load_u64(ROW_rdo_w2, i);
                uint64_t r3 = load_u64(ROW_rdo_w3, i);
                uint64_t r4 = load_u64(ROW_rdo_w4, i);
                if (r1) rdoList.emplace_back(r1);
                if (r2) rdoList.emplace_back(r2);
                if (r3) rdoList.emplace_back(r3);
                if (r4) rdoList.emplace_back(r4);
            }
        }
    }

    {
        if constexpr (enableBenchmark) Athena::Chrono chrono("Pixel Copy", m_chronoSvc.get());
        ATH_CHECK(pixelClustersHandle.record(std::move(pixelCl), std::move(pixelClAux)));
    }

    return StatusCode::SUCCESS;



    return StatusCode::SUCCESS;
}



StatusCode xAODClusterMaker::makePixelClusterContainer(
        const EFTrackingTransient::PixelClusterAuxInput &pxAux,
        const EFTrackingTransient::Metadata *metadata,
        const EventContext &ctx) const {
    ATH_MSG_DEBUG("Making xAOD::PixelClusterContainer");


    SG::WriteHandle<xAOD::PixelClusterContainer> pixelClustersHandle{m_pixelClustersKey, ctx};

    if (!m_doBulkCopy) {
        // --------------------------------------------------------------------
        // proceed with the element-wise method
        // --------------------------------------------------------------------
        ATH_MSG_DEBUG("You are running the element-wise container creation method.");
        if constexpr (enableBenchmark) Athena::Chrono chrono("ElementWiseMethod", m_chronoSvc.get());

        ATH_CHECK(pixelClustersHandle.record(
                    std::make_unique<xAOD::PixelClusterContainer>(),
                    std::make_unique<xAOD::PixelClusterAuxContainer>()));

        ATH_CHECK(pixelClustersHandle.isValid());
        ATH_MSG_DEBUG("Container '" << m_pixelClustersKey << "' initialised");

        int rdoIndexCounter = 0;

        for (unsigned int i = 0; i < metadata->numOfPixelClusters; i++) {
            // Push back numClusters of PixelCluster
            auto pixelCl = pixelClustersHandle->push_back(
                    std::make_unique<xAOD::PixelCluster>());

            Eigen::Matrix<float, 2, 1> localPosition(
                    pxAux.localPosition[i * 2], pxAux.localPosition[i * 2 + 1]);
            Eigen::Matrix<float, 2, 2> localCovariance;
            localCovariance.setZero();
            localCovariance(0, 0) = pxAux.localCovariance[i * 2];
            localCovariance(1, 1) = pxAux.localCovariance[i * 2 + 1];
            Eigen::Matrix<float, 3, 1> globalPosition(
                    pxAux.globalPosition[i * 3], pxAux.globalPosition[i * 3 + 1],
                    pxAux.globalPosition[i * 3 + 2]);

            std::vector<Identifier> RDOs;
            RDOs.reserve(metadata->pcRdoIndex[i]);
            // Cover RDO
            for (unsigned int j = 0; j < metadata->pcRdoIndex[i]; ++j) {
                RDOs.push_back(Identifier(pxAux.rdoList[rdoIndexCounter + j]));
            }

            rdoIndexCounter += metadata->pcRdoIndex[i];

            pixelCl->setMeasurement<2>(pxAux.idHash[i], localPosition,
                    localCovariance);
            pixelCl->setIdentifier(pxAux.id[i]);
            pixelCl->setRDOlist(RDOs);
            pixelCl->globalPosition() = globalPosition;
            pixelCl->setTotalToT(pxAux.totalToT[i]);
            pixelCl->setChannelsInPhiEta(pxAux.channelsInPhi[i],
                    pxAux.channelsInEta[i]);
            pixelCl->setWidthInEta(pxAux.widthInEta[i]);
        }

        return StatusCode::SUCCESS;
    }

    // --------------------------------------------------------------------
    // proceed with the bulk copy method
    // --------------------------------------------------------------------

    ATH_MSG_DEBUG("You are running the bulk copy container creation method.");
    if constexpr (enableBenchmark) Athena::Chrono chrono("BulkCopyMethod", m_chronoSvc.get());

    // --------------------------
    // Create the container and aux. container
    // --------------------------
    auto pixelCl = std::make_unique<xAOD::PixelClusterContainer>();
    auto pixelClAux = std::make_unique<xAOD::PixelClusterAuxContainer>();
    pixelCl->setStore(pixelClAux.get());

    // Pre-allocate memory for all clusters in the AuxContainer
    // and reserve the same space in the container.
    const size_t nClusters = metadata->numOfPixelClusters;
    pixelClAux->resize(nClusters);
    pixelCl->reserve(nClusters);

    // Now, push back the PixelCluster objects
    // and create them all at once.
    // Note, that no data is set yet,
    // we will do that in the next step.
    for (size_t i = 0; i < nClusters; ++i) {
        pixelCl->push_back(std::make_unique<xAOD::PixelCluster>());
    }

    // --------------------------
    // Prepare local buffers for each of the
    // fixed-size attributes.
    // --------------------------

    std::vector<long unsigned int> identifierBuffer(nClusters);
    std::vector<unsigned int> idHashBuffer(nClusters);
    std::vector<std::array<float, 3>> gpBuffer(nClusters);
    std::vector<float> localPosX(nClusters);
    std::vector<float> localPosY(nClusters);
    std::vector<float> localCovXX(nClusters);
    std::vector<float> localCovYY(nClusters);
    std::vector<int> totalToTBuffer(nClusters);
    std::vector<float> widthInEtaBuffer(nClusters);
    std::vector<int> channelsInPhiBuffer(nClusters);
    std::vector<int> channelsInEtaBuffer(nClusters);

    for (size_t i = 0; i < nClusters; ++i) {

        // Fill the identifierBuffer
        identifierBuffer[i] = pxAux.id[i];

        // Fill the idHashBuffer
        idHashBuffer[i] = pxAux.idHash[i];

        // Fill the globalPositionBuffers
        gpBuffer[i] = {
            pxAux.globalPosition[3 * i],
            pxAux.globalPosition[3 * i + 1],
            pxAux.globalPosition[3 * i + 2],
        };

        // Fill the localPositionBuffers
        localPosX[i] = pxAux.localPosition[2 * i];
        localPosY[i] = pxAux.localPosition[2 * i + 1];

        // Fill the localCovBuffer[i]
        localCovXX[i] = pxAux.localCovariance[2 * i];
        localCovYY[i] = pxAux.localCovariance[2 * i + 1];

        // Fill the totalToTBuffer
        totalToTBuffer[i] = pxAux.totalToT[i];

        // Fill the widthInEtaBuffer
        widthInEtaBuffer[i] = pxAux.widthInEta[i];

        // Fill the channelsInPhiEtaBuffer
        channelsInPhiBuffer[i] = pxAux.channelsInPhi[i];
        channelsInEtaBuffer[i] = pxAux.channelsInEta[i];
    }

    // --------------------------
    // Now, use SG::Accessors to do a bulk copy into
    // the container memory.
    // --------------------------

    // FIXED-SIZE ATTRIBUTES
    static const SG::Accessor<long unsigned int> idAcc("identifier");
    static const SG::Accessor<unsigned int> idHashAcc("idHash");
    static const SG::Accessor<float> localPosXAcc("localPositionX");
    static const SG::Accessor<float> localPosYAcc("localPositionY");
    static const SG::Accessor<float> localCovXXAcc("localCovarianceXX");
    static const SG::Accessor<float> localCovYYAcc("localCovarianceYY");
    static const SG::Accessor<int> totAcc("totalToT");
    static const SG::Accessor<float> widthEtaAcc("widthInEta");
    static const SG::Accessor<int> channelsInPhiAcc("channelsInPhi");
    static const SG::Accessor<int> channelsInEtaAcc("channelsInEta");

    // VARIABLE-LENGTH ATTRIBUTES
    static const SG::Accessor<std::array<float, 3>> globalPosAcc(
            "globalPosition");

    // Get spans into the container
    auto idSpan = idAcc.getDataSpan(*pixelCl);
    auto idHashSpan = idHashAcc.getDataSpan(*pixelCl);
    auto locPosXSpan = localPosXAcc.getDataSpan(*pixelCl);
    auto locPosYSpan = localPosYAcc.getDataSpan(*pixelCl);
    auto locCovXXSpan = localCovXXAcc.getDataSpan(*pixelCl);
    auto locCovYYSpan = localCovYYAcc.getDataSpan(*pixelCl);
    auto totSpan = totAcc.getDataSpan(*pixelCl);
    auto wEtaSpan = widthEtaAcc.getDataSpan(*pixelCl);
    auto gpSpan = globalPosAcc.getDataSpan(*pixelCl);
    auto channelsInPhiSpan = channelsInPhiAcc.getDataSpan(*pixelCl);
    auto channelsInEtaSpan = channelsInEtaAcc.getDataSpan(*pixelCl);

    // Bulk copy the fixed-size attributes
    CxxUtils::copy_bounded(identifierBuffer, idSpan);
    CxxUtils::copy_bounded(idHashBuffer, idHashSpan);
    CxxUtils::copy_bounded(localPosX, locPosXSpan);
    CxxUtils::copy_bounded(localPosY, locPosYSpan);
    CxxUtils::copy_bounded(localCovXX, locCovXXSpan);
    CxxUtils::copy_bounded(localCovYY, locCovYYSpan);
    CxxUtils::copy_bounded(totalToTBuffer, totSpan);
    CxxUtils::copy_bounded(widthInEtaBuffer, wEtaSpan);
    CxxUtils::copy_bounded(gpBuffer, gpSpan);
    CxxUtils::copy_bounded(channelsInPhiBuffer, channelsInPhiSpan);
    CxxUtils::copy_bounded(channelsInEtaBuffer, channelsInEtaSpan);

    // --------------------------
    // Copy the variable-length data.
    // We need to loop over the number of
    // clusters to get access to each
    // element.
    // --------------------------
    static const SG::Accessor<std::vector<unsigned long long>> rdoListAcc(
            "rdoList");
    auto rdoSpan = rdoListAcc.getDataSpan(*pixelCl);

    int rdoIndexCounter = 0;
    // loop over the clusters
    for (size_t i = 0; i < nClusters; ++i) {
        size_t nRDOs = metadata->pcRdoIndex[i];

        // direct ref to destination vector
        std::vector<unsigned long long> &rdosForThisCluster = rdoSpan[i];

        // pre-size the vector
        rdosForThisCluster.resize(nRDOs);

        // direct element access
        for (size_t j = 0; j < nRDOs; ++j) {
            rdosForThisCluster[j] = pxAux.rdoList[rdoIndexCounter + j];
        }
        rdoIndexCounter += nRDOs;
    }
    // --------------------------
    // Record the container + aux container into StoreGate,
    // but now we do it AFTER filling the container!
    // --------------------------

    ATH_CHECK(pixelClustersHandle.record(std::move(pixelCl), std::move(pixelClAux)));

    ATH_MSG_DEBUG("Bulk copy for fixed-size variables done.");
    return StatusCode::SUCCESS;
}
