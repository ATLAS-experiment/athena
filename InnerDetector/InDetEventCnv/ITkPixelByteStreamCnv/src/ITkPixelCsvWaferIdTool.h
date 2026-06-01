/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXELBYTESTREAMCNV_ITKPIXELCSVWAFERIDTOOL_H
#define ITKPIXELBYTESTREAMCNV_ITKPIXELCSVWAFERIDTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "Identifier/Identifier.h"
#include "InDetIdentifier/PixelID.h"

#include <string>
#include <vector>

class PixelID;

class ITkPixelCsvWaferIdTool : public AthAlgTool {
public:
    struct CsvRow {
        std::string spChain;
        std::string md;
        int fe = -1;
    };

    ITkPixelCsvWaferIdTool(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~ITkPixelCsvWaferIdTool() = default;

    virtual StatusCode initialize() override;

    const std::vector<CsvRow>& rows() const { return m_rows; }

    StatusCode loadCsv();
    Identifier waferId(const CsvRow& row) const;

private:
    static std::string trim(const std::string& input);
    static std::vector<std::string> splitCsvLine(const std::string& line);
    static std::vector<std::string> parseSPChain(const std::string& spChain);

    // Helper functions implemented in the .cxx file
    int barrel_ec(std::vector<std::string> spchain);
    int layer_disk(std::vector<std::string> spchain);
    int phi_module(std::vector<std::string> spchain, int mod);
    int eta_module(std::vector<std::string> spchain, int mod);
    Gaudi::Property<std::string> m_csvFile{this,
                                           "CsvFile",
                                           "AT2-IP-ES-0016_v1.41_INCOMPLETE-ModuleA.csv",
                                           "CSV file containing SP chain, Module and FE columns"};

    const PixelID* m_pixIdHelper = nullptr;
    std::vector<CsvRow> m_rows;
};

#endif // ITKPIXELBYTESTREAMCNV_ITKPIXELCSVWAFERIDTOOL_H
