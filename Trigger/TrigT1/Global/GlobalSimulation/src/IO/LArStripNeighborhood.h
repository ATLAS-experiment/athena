/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file GlobalSimulation/ICommonTOB.h
 * @author Peter Sherwood, peter@cern.ch
 * @date July 2024
 * @brief Class to hold windows of LAr strip cells in a the neighbourhood of a eFexRoI
 */


#ifndef GLOBALSIM_LARSTRIPNEIGHBORHOOD_H
#define GLOBALSIM_LARSTRIPNEIGHBORHOOD_H

#include "StripData.h"

#include <ostream>
#include <vector>
#include <utility> //std::pair
#include <iosfwd>

namespace GlobalSim {
  using StripDataVector = std::vector<StripData>;
  using Coords = std::pair<double, double>;


  /**
   * @brief Class to hold windows of LAr strip cells in a the neighbourhood of a eFexRoI
   *
   * This class represents windows of LAr strip cells centred on a eFexRoI.
   * It defines three rows of cells e/eta/phi — low, center, high — surrounding the eFexRoI. It 
   * defines which cell has the maximum energy, and the  max_cell/RoI eta/phi coordinates are 
   * also stored.
   */

  class LArStripNeighborhood {
  public:
 
    /** 
     * @brief Constructor to define a LArStripNeighborhood from input objects
     * @param[in] phi_low A vector of e/eta/phi coordinates for the low phi row
     * @param[in] phi_center A vector of e/eta/phi coordinates for the center phi row
     * @param[in] phi_high A vector of e/eta/phi coordinates for the high phi row
     * @param[in] roiCoords The eta/phi coordinates of the eFexRoI that produced this object
     * @param[in] cellCoords The eta/phi coordinates of the maximum energy cell
     * @param[in] max_cell_pos The index of the maximum energy cell
     *
     * To be used to create, and initilise a neighborhood of LAr Strip cells centred on a
     * incoming eFeXRoITOB. The neighbourhood is organised as three rows of 17 strips in phi.
     */
    LArStripNeighborhood(const StripDataVector& phi_low,
			 const StripDataVector& phi_center,
			 const StripDataVector& phi_high,
			 const Coords& roiCoords,
			 const Coords& cellCoords,
			 std::size_t max_cell_pos);

    /** @brief Returns a vector of strip cell e/eta/phi data for the low phi row of the neighborhood*/
    const StripDataVector& phi_low() const {return m_phi_low;}
    /** @brief Returns a vector of strip cell e/eta/phi data for the central phi row of the neighborhood*/
    const StripDataVector& phi_center() const {return m_phi_center;}
    /** @brief Returns a vector of strip cell e/eta/phi data for the central high row of the neighborhood*/
    const StripDataVector& phi_high() const {return m_phi_high;}

    /** @brief Returns the index of the maximum energy cell in this neighbourhood*/
    std::size_t maxCellIndex() const {return m_max_cell_pos;}

    /** @brief Returns the eta/phi coordinates of the RoI used to seed this neighbourhood*/
    const Coords& roiCoords() const {return m_roiCoords;}
    /** @brief Returns the eta/phi coordinates of the maximum energy cell*/
    const Coords& cellCoords() const {return m_cellCoords;}

    /** @brief print out contents to string*/
    std::string to_string() const;
    
  private:
    /// Parameter: Vector strip cell e/eta/phi, low phi row
    StripDataVector m_phi_low;
    /// Parameter: Vector strip cell e/eta/phi, central phi row
    StripDataVector m_phi_center;
    /// Parameter: Vector strip cell e/eta/phi, high phi row
    StripDataVector m_phi_high;
    /// Parameter: eta/Phi coordinate of the seed eFexRoITOB
    Coords m_roiCoords{0., 0.};
    /// Parameter: eta/phi coords of cell in RoI with maximum energy
    Coords m_cellCoords{0., 0.};
    /// Parameter: Index of the cell with the maximum energy
    std::size_t m_max_cell_pos{0};
  };
}//End of namespace

std::ostream&
operator<< (std::ostream&, const GlobalSim::LArStripNeighborhood&);

#endif //GLOBALSIM_LARSTRIPNEIGHBORHOOD_H
