/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ITKPIXELBYTESTREAMCNV_ITKPIXELCSVWAFERIDALG_H
#define ITKPIXELBYTESTREAMCNV_ITKPIXELCSVWAFERIDALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/EventContext.h"
#include "GaudiKernel/ISvcLocator.h"
#include "Identifier/Identifier.h"
#include "InDetIdentifier/PixelID.h"

#include <atomic>
#include <string>
#include <vector>
#include <utility>
#include <tuple>
#include <bitset>


class PixelID;

class ITkPixelCsvWaferIdAlg : public AthReentrantAlgorithm {
public:
    struct CsvRow {
        std::string spChain;
        std::string md;
        int fe = -1;
        std::string flx_card_device;
        unsigned int fiber = 0;
    };

    ITkPixelCsvWaferIdAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~ITkPixelCsvWaferIdAlg() = default;

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

    const std::vector<CsvRow>& rows() const { return m_rows; }

    StatusCode loadCsv();
    std::tuple<Identifier,int,unsigned long> waferId(const CsvRow& row) const;
    unsigned long onlineId(const std::vector<std::string>& spchain, const std::string& mod, int fe) const;

private:
    static std::string trim(const std::string& input);
    static std::vector<std::string> splitCsvLine(const std::string& line);
    static std::vector<std::string> parseSPChain(const std::string& spChain);
    std::vector<int> DmaBuffer() const;
    std::bitset<32> sourceID(const std::vector<std::string>& spchain, const std::string& flx, const unsigned int dma) const;
    std::vector<std::string>  splitFLX_card_device(const std::string& s) const;
    std::bitset<32>  subDetID(int barrel_endcap, int layer_disk) const;


    // Helper functions implemented in the .cxx file
    int barrel_ec(const std::vector<std::string>& spchain) const;
    int layer_disk(const std::vector<std::string>& spchain) const;
    int phi_module(const std::vector<std::string>& spchain, const std::string& mod, int fe) const;
    int eta_module(const std::vector<std::string>& spchain, const std::string& mod, int fe) const;
    int feID(const std::vector<std::string>& spchain, int fe) const;

    Gaudi::Property<std::string> m_csvFile{this,
                                           "CsvFile",
                                           "AT2-IP-ES-0016_v1.41_INCOMPLETE-ModuleA_slim.csv",
                                           "CSV file containing SP chain, Module and FE columns"};
    Gaudi::Property<std::string> m_outputFile{this,
                                              "OutputFile",
                                              "ITkPixelWaferIds.txt",
                                              "Output text file for one 32-bit waferID per line"};

    const PixelID* m_pixIdHelper = nullptr;
    std::vector<CsvRow> m_rows;

    mutable std::atomic<bool> m_done{false};
};

#endif // ITKPIXELBYTESTREAMCNV_ITKPIXELCSVWAFERIDALG_H
