/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */


#include "./EratioMaker.h"
#include <algorithm>
#include <sstream>
#include <iostream>
#include "TVector2.h"

namespace Gep {


EratioMaker::EratioMaker(const GepCellMap& caloCellsMap)
    : m_caloCellsMap{caloCellsMap}
{

}


// ==========================================================
// Construct window of 17x3 cells around seed
// ==========================================================

std::vector<std::vector<GepCaloCell>>
EratioMaker::makeWindow(const ROOT::Math::PtEtaPhiEVector &seed) const
{
    std::vector<std::vector<GepCaloCell>> window(17, std::vector<GepCaloCell>(3));

    auto cellMap = m_caloCellsMap.getCellMap();
    
    // 1. Find cell that contains the seed 
    const GepCaloCell* centerCell = nullptr;
    unsigned int centerCellId = 0;
    bool exactMatchFound = false;
    
    for (auto it = cellMap->begin(); it != cellMap->end(); ++it) {
        const auto& cell = it->second;
        if (cell.sampling != 1) continue; // Only ECAL layer 1 
        
        // Verify if seed coordinates fall inside the limits of some cell
        if (cell.etaMin < seed.eta() && seed.eta() < cell.etaMax &&
            cell.phiMin < seed.phi() && seed.phi() < cell.phiMax) {
            centerCell = &cell;
            centerCellId = cell.id;
            exactMatchFound = true;
            break;
        }
    }
    
    // If not exact match, return empty window
    if (!exactMatchFound || !centerCell) {
        return window;
    }

    // 2. Construct 17x3 (rows x cols) window based on cell ID relations starting from seed
    // Rows: -8 to +8 from central cell in eta (17 total)
    // Columns: -1, 0, +1 from central cell in phi (3 total)
    
    // Construct each window row
    for (int etaOffset = -8; etaOffset <= 8; etaOffset++) {
        unsigned int windowRow = etaOffset + 8; // Convertir a índice 0..16
        
        // Get base ID for this row (central cell of the phi column)
        unsigned int baseId = static_cast<unsigned int>(static_cast<int>(centerCellId) + (etaOffset * 512));
        
        // Loop phi columns (-1, 0, +1)
        for (int phiOffset = -1; phiOffset <= 1; phiOffset++) {
            int windowCol = phiOffset + 1; 
            
            // Compute desired cell ID 
            unsigned int cellId = static_cast<unsigned int>(static_cast<int>(baseId) + phiOffset * 2);
            
            // Look for that cell within the map
            auto it = cellMap->find(cellId);
            if (it != cellMap->end()) {
                const auto& cell = it->second;
                if (cell.sampling == 1) {
                    window[windowRow][windowCol] = cell;
                }
            }
        }
    }

    return window;
}

// ==========================================================
// Find local maxima within the window
// ==========================================================

std::pair<double, double>
EratioMaker::findLocalMaxima(const std::vector<std::vector<GepCaloCell>>& window) const 
{

    struct LocalMax {
        double energy;
        int i;
        int j;
    };

    std::vector<LocalMax> localMaxima;

    int rows = window.size();
    int cols = window[0].size();

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            double current = window[i][j].e;
            bool isMax = true;

            for (int di = -1; di <= 1; ++di) {
                for (int dj = -1; dj <= 1; ++dj) {
                    if (di == 0 && dj == 0) continue;
                    int ni = i + di;
                    int nj = j + dj;
                    if (ni >= 0 && ni < rows && nj >= 0 && nj < cols) {
                        if (window[ni][nj].e >= current) {
                            isMax = false;
                            break;
                        }
                    }
                }
                if (!isMax) break;
            }

            if (isMax) localMaxima.push_back({current, i, j });
        }
    }

    if (localMaxima.empty()) return {0., 0.};

    // Sort local maximas in descendent fashion
    std::sort(localMaxima.begin(), localMaxima.end(),
                [](const LocalMax& a, const LocalMax& b){ return a.energy > b.energy; });

    double E1 = localMaxima[0].energy;
    double E2 = (localMaxima.size() > 1) ? localMaxima[1].energy : 0.0;

    return {E1, E2};

}

// ==========================================================
// Compute Eratio 
// ==========================================================

double EratioMaker::computeEratio(double E1, double E2) const
{
    return (E1 + E2 > 0) ? (E1 - E2) / (E1 + E2) : -999;
}

// ==========================================================
// Build eratio object
// ==========================================================

const EratioObj EratioMaker::makeEratio(const ROOT::Math::PtEtaPhiEVector seed) const
{
    auto window = makeWindow(seed);
    auto [E1, E2] = findLocalMaxima(window);
    double eratio = computeEratio(E1, E2);

    EratioObj obj;
    obj.seedEta = seed.eta();
    obj.seedPhi = seed.phi();
    obj.E1 = E1;
    obj.E2 = E2;
    obj.Eratio = eratio;

    return obj;
}

 
} // namespace Gep


