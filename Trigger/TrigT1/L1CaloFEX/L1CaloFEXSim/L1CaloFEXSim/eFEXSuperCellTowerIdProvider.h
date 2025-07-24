/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file eFEXSuperCellTowerIdProvider.h
 * @brief Provide Supercell-tower mapping
 *
 * Load information stored in a .csv file and
 * provide information needed to connect to SuperCells to towers.
 *
 * @author Tong Qiu (tong.qiu@cern.ch)
 * @date 13 May 2021
 */

#ifndef EFEXSUPERCELLTOWERIDPROVIDER
#define EFEXSUPERCELLTOWERIDPROVIDER
#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include <string>
#include <unordered_map>

namespace LVL1 {
  /**
   * @class eFEXSuperCellTowerIdProvider
   * @brief Provide Supercell-tower mapping
   * Load information stored in a .csv file and
   * provide information needed to connect the SuperCells to its corresponding tower.
   */
  static const InterfaceID IID_IIeFEXSuperCellTowerIdProvider("LVL1::eFEXSuperCellTowerIdProvider", 1 , 0);

    class eFEXSuperCellTowerIdProvider: public AthAlgTool {

  /**
   * @brief Sturcture stores mapping information of a SuperCell
   */
  struct towerinfo {
    int eTowerID; ///< tower ID
    int slot; ///< slot of the tower in which the SuperCell belongs to
    bool isSplit; ///< if the SuperCell Et needs to be splited
  };

  public:
        static const InterfaceID& interfaceID() { return IID_IIeFEXSuperCellTowerIdProvider; };
    /// @brief Constructor
    eFEXSuperCellTowerIdProvider(const std::string& type,const std::string& name,const IInterface* parent);

    /// @brief Destructor
    ~eFEXSuperCellTowerIdProvider();

    /// @brief initialize the tool
    StatusCode initialize();

    /// @brief set the address to the csv file and load
    StatusCode setAddress(std::string);

    /**
     * @brief obtain ordered tower IDs in an eFEX
     *
     * @param scid HEX SuperCell ID
     * @param eTowerID variable to store the output tower ID
     * @param slot variable to store the output slot number
     * @param issplite variable to store if the SuperCell needs to be splited
     *
     * @return status code
     *
     */
    StatusCode geteTowerIDandslot(uint64_t scid, int &eTowerID, int &slot, bool &issplit) const;

    /// @brief Check if the csv file is valid
    bool ifhaveinputfile() const;

  private:
    bool m_hascsvfile{false}; ///< if the csv file is valid

    std::string m_csvaddress; ///< path to the csv file

    /// @brief Check if the SuperCell id exists in the csv file
    bool hasSuperCell(uint64_t) const;

    /// @brief load the csv file
    StatusCode loadcsv();
  
    /// map to store information about SuperCells with SuperCell ID as the key.
    std::unordered_map<uint64_t, towerinfo> m_SuperCelltoTowerIdmap;
  };
}
#endif
