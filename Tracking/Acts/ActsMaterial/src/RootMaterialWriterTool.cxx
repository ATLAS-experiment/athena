/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "RootMaterialWriterTool.h"
#include "ActsInterop/Logger.h"
#include "ActsPlugins/Root/RootMaterialMapIo.hpp"
#include "Acts/Material/InterpolatedMaterialMap.hpp"
#include "Acts/Material/MaterialGridHelper.hpp"

#include <TFile.h>
#include <TH1.h>
#include <TH2.h>

ActsTrk::RootMaterialWriterTool::RootMaterialWriterTool(const std::string& type,
                                                        const std::string& name,
                                                        const IInterface* parent)
    : base_class(type, name, parent)
{
}

ActsTrk::RootMaterialWriterTool::~RootMaterialWriterTool()
{
    if (m_outputFile != nullptr)
        m_outputFile->Close();
}

StatusCode ActsTrk::RootMaterialWriterTool::initialize()
{
    // Setup ROOT I/O
    m_outputFile = TFile::Open(m_fileName.value().c_str(), "RECREATE");
    if (m_outputFile == nullptr) {
        ATH_MSG_ERROR("Could not open '" + m_fileName + "'");
        return StatusCode::FAILURE;
    }

    return StatusCode::SUCCESS;
}


void ActsTrk::RootMaterialWriterTool::writeMaterial(const ActsTrk::GeometryContext& /*gctx*/,
                                                    const Acts::TrackingGeometryMaterial& detMaterial) const
{
    /// The accessor configuration
    ActsPlugins::RootMaterialMapIo::Config accessorConfig;
    /// The accessor options
    ActsPlugins::RootMaterialMapIo::Options accessorOptions;

    // Change to the output file
    m_outputFile->cd();

    const auto& [surfaceMaps, volumeMaps] = detMaterial;

    // Write the surface material maps
    ActsPlugins::RootMaterialMapIo accessor(accessorConfig,
                                            makeActsAthenaLogger(this, "RootMaterialWriterTool"));

    for (const auto& [geoId, sMap] : surfaceMaps) {
        // Get the Surface material
        accessor.write(*m_outputFile, geoId, *sMap, accessorOptions);
    }

    // Write the volume material maps
    for (auto& [key, value] : volumeMaps) {
        // Get the Volume material
        const Acts::IVolumeMaterial* vMaterial = value.get();
        if (vMaterial == nullptr) {
            ATH_MSG_WARNING("No material for volume " << key << " skipping");
            continue;
        }

        // get the geometry ID
        Acts::GeometryIdentifier geoID = key;
        // decode the geometryID
        const auto gvolID = geoID.volume();

        // create the directory
        std::string tdName = accessorOptions.folderVolumeNameBase.c_str();
        tdName += accessorConfig.volumePrefix + std::to_string(gvolID);

        // create a new directory
        m_outputFile->mkdir(tdName.c_str());
        m_outputFile->cd(tdName.c_str());

        ATH_MSG_VERBOSE("Writing out map at " << tdName);

        // understand what sort of material you have in mind
        auto bvMaterial3D = dynamic_cast<const Acts::InterpolatedMaterialMap<
            Acts::MaterialMapLookup<Acts::MaterialGrid3D>>*>(vMaterial);
        auto bvMaterial2D = dynamic_cast<const Acts::InterpolatedMaterialMap<
            Acts::MaterialMapLookup<Acts::MaterialGrid2D>>*>(vMaterial);

        int points = 1;
        if (bvMaterial3D != nullptr || bvMaterial2D != nullptr) {
            // Get the binning data
            std::vector<Acts::BinningData> binningData;
            if (bvMaterial3D != nullptr) {
                binningData = bvMaterial3D->binUtility().binningData();
                Acts::MaterialGrid3D grid = bvMaterial3D->getMapper().getGrid();
                points = static_cast<int>(grid.size());
            } else {
                binningData = bvMaterial2D->binUtility().binningData();
                Acts::MaterialGrid2D grid = bvMaterial2D->getMapper().getGrid();
                points = static_cast<int>(grid.size());
            }

            // 2-D or 3-D maps
            auto bins = static_cast<int>(binningData.size());
            auto fBins = static_cast<float>(bins);

            // The bin number information
            TH1F n(accessorConfig.nBinsHistName.c_str(), "bins; bin", bins, -0.5, fBins - 0.5);

            // The binning value information
            TH1F v(accessorConfig.axisDirHistName.c_str(), "binning values; bin", bins, -0.5, fBins - 0.5);

            // The binning option information
            TH1F o(accessorConfig.axisBoundaryTypeHistName.c_str(), "binning options; bin", bins, -0.5, fBins - 0.5);

            // The binning option information
            TH1F rmin(accessorConfig.minRangeHistName.c_str(), "min; bin", bins, -0.5, fBins - 0.5);

            // The binning option information
            TH1F rmax(accessorConfig.maxRangeHistName.c_str(), "max; bin", bins, -0.5, fBins - 0.5);

            // Now fill the histogram content
            for (const auto& [b, bData] : enumerate(binningData)) {
                // Fill: nbins, value, option, min, max
                n.SetBinContent(static_cast<int>(b), static_cast<int>(binningData[b - 1].bins()));
                v.SetBinContent(static_cast<int>(b), static_cast<int>(binningData[b - 1].binvalue));
                o.SetBinContent(static_cast<int>(b), static_cast<int>(binningData[b - 1].option));
                rmin.SetBinContent(static_cast<int>(b), binningData[b - 1].min);
                rmax.SetBinContent(static_cast<int>(b), binningData[b - 1].max);
            }
            n.Write();
            v.Write();
            o.Write();
            rmin.Write();
            rmax.Write();
        }

        auto fPoints = static_cast<float>(points);
        TH1F x0(accessorConfig.x0HistName.c_str(), "X_{0} [mm] ;gridPoint", points, -0.5, fPoints - 0.5);
        TH1F l0(accessorConfig.l0HistName.c_str(), "#Lambda_{0} [mm] ;gridPoint", points, -0.5, fPoints - 0.5);
        TH1F A(accessorConfig.aHistName.c_str(), "X_{0} [mm] ;gridPoint", points, -0.5, fPoints - 0.5);
        TH1F Z(accessorConfig.zHistName.c_str(), "#Lambda_{0} [mm] ;gridPoint", points, -0.5, fPoints - 0.5);
        TH1F rho(accessorConfig.rhoHistName.c_str(), "#rho [g/mm^3] ;gridPoint", points, -0.5, fPoints - 0.5);
        // homogeneous volume
        if (points == 1) {
            auto mat = vMaterial->material({0, 0, 0});
            x0.SetBinContent(1, mat.X0());
            l0.SetBinContent(1, mat.L0());
            A.SetBinContent(1, mat.Ar());
            Z.SetBinContent(1, mat.Z());
            rho.SetBinContent(1, mat.massDensity());
        } else {
            // 3d grid volume
            if (bvMaterial3D != nullptr) {
                Acts::MaterialGrid3D grid = bvMaterial3D->getMapper().getGrid();
                for (int point = 0; point < points; point++) {
                    auto mat = Acts::Material(grid.at(point));
                    if (!mat.isVacuum()) {
                        x0.SetBinContent(point + 1, mat.X0());
                        l0.SetBinContent(point + 1, mat.L0());
                        A.SetBinContent(point + 1, mat.Ar());
                        Z.SetBinContent(point + 1, mat.Z());
                        rho.SetBinContent(point + 1, mat.massDensity());
                    }
                }
            }
            // 2d grid volume
            else if (bvMaterial2D != nullptr) {
                Acts::MaterialGrid2D grid = bvMaterial2D->getMapper().getGrid();
                for (int point = 0; point < points; point++) {
                    auto mat = Acts::Material(grid.at(point));
                    if (!mat.isVacuum()) {
                        x0.SetBinContent(point + 1, mat.X0());
                        l0.SetBinContent(point + 1, mat.L0());
                        A.SetBinContent(point + 1, mat.Ar());
                        Z.SetBinContent(point + 1, mat.Z());
                        rho.SetBinContent(point + 1, mat.massDensity());
                    }
                }
            }
        }
        x0.Write();
        l0.Write();
        A.Write();
        Z.Write();
        rho.Write();
    }

    return;
}

