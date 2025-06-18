//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_ETAPHIMAP_H
#define CALORECGPU_ETAPHIMAP_H

#include "BaseDefinitions.h"
#include "Helpers.h"

#include <utility>
#include <cmath>
#include <new>
#include <algorithm>

namespace CaloRecGPU
{

  /// @class EtaPhiMapEntry
  /// @brief Holds an (eta, phi) to cell map for a given sampling.
  ///
  /// @par eta_grid: number of subdivisions in eta (doubled for non-continuous)
  ///
  /// @par phi_grid: number of subdivisions in phi
  ///
  /// @par respect_deltas: If @p true, the cells stretch only as far as their @p deta and @p dphi (Tile).
  ///                      If @p false, always return closest cell (LAr).
  ///
  /// @par continuous: If @p true, the sampling provides continuous coverage for positive and negative etas (e. g. contains eta == 0).
  ///                  If @p false, the sampling is separated between its positive and negative eta cells.
  template <int eta_grid, int phi_grid, bool respect_deltas, bool continuous, int sampling_number>
  struct EtaPhiMapEntry
  {
    static constexpr int s_max_overlap_cells = 8;

   private:

    static constexpr float s_phi_min   =  - Helpers::Constants::pi<float>;
    static constexpr float s_phi_max   =  + Helpers::Constants::pi<float>;
    static constexpr float s_delta_phi = (s_phi_max - s_phi_min) / phi_grid;

    float m_eta_limits[1 + !continuous];
    float m_delta_eta;
    
    static constexpr int s_eta_grid_size = eta_grid * (1 + !continuous);

    int   m_cells           [s_eta_grid_size][phi_grid][s_max_overlap_cells];
    float m_eta_coordinates [s_eta_grid_size][phi_grid][s_max_overlap_cells];
    float m_phi_coordinates [s_eta_grid_size][phi_grid][s_max_overlap_cells];
    //If respect_deltas is true:
    // -> m_{eta, phi}_coordinates[h][f][n] > 0
    //    means the cell ends at this fraction of the grid.
    // -> m_{eta, phi}_coordinates[h][f][n] < 0
    //    means the cell starts at this fraction of the grid.
    // -> m_{eta, phi}_coordinates[h][f][n] == 0
    //    means the cell covers the entire part of the grid.
    //If respect_deltas is false, this stores simply
    //the eta, phi of the centers of the cells.

   public:

    constexpr float delta_eta() const
    {
      return m_delta_eta;
    }

    constexpr float delta_phi() const
    {
      return s_delta_phi;
    }

    constexpr float start_eta(const bool positive = true) const
    {
      if (continuous)
        {
          return -m_eta_limits[0];
        }
      else if (positive)
        {
          return m_eta_limits[0];
        }
      else
        {
          return -m_eta_limits[1];
        }
    }

    constexpr float start_phi() const
    {
      return s_phi_min;
    }

    constexpr float end_eta(const bool positive = true) const
    {
      if (continuous)
        {
          return m_eta_limits[0];
        }
      else if (positive)
        {
          return m_eta_limits[1];
        }
      else
        {
          return -m_eta_limits[0];
        }
    }

    constexpr float end_phi() const
    {
      return s_phi_max;
    }

    constexpr int eta_coordinate(const float eta, float & interval) const
    {
      using namespace std;
      const float frac = (eta - start_eta(eta > 0)) / delta_eta();
      const float rounded = floorf(frac);
      interval = frac - rounded;
      const int casted = static_cast<int>(rounded);
      if ((casted == eta_grid || casted == 2 * eta_grid) && interval == 0.f)
        {
          interval = 1.f;
          return casted - 1 + (eta > 0 && !continuous) * eta_grid;
        }
      else
        {
          return casted + (eta > 0 && !continuous) * eta_grid;
        }
    }

    constexpr int eta_coordinate(const float eta) const
    {
      float dummy = 0.f;
      return eta_coordinate(eta, dummy);
    }

    constexpr int phi_coordinate(const float phi, float & interval) const
    {
      using namespace std;
      const float frac = (phi - start_phi()) / delta_phi();
      const float rounded = floorf(frac);
      interval = frac - rounded;
      const int to_return = static_cast<int>(rounded) % phi_grid;
      return to_return + (to_return < 0) * phi_grid;
    }

    constexpr int phi_coordinate(const float phi) const
    {
      float dummy = 0.f;
      return phi_coordinate(phi, dummy);
    }

    constexpr bool coordinates_in_range(const float eta, const float /*phi*/) const
    {
      const bool eta_in_range = eta >= start_eta(eta > 0) && eta <= end_eta(eta > 0);
      const bool phi_in_range = true; //By definition, we wrap around in phi...
      return eta_in_range && phi_in_range;
    }

    constexpr float eta_value(const int eta_coord) const
    {
      if (!continuous && eta_coord >= eta_grid)
        {
          return start_eta(true) + (eta_coord - eta_grid) * delta_eta();
        }
      else
        {
          return start_eta(false) + eta_coord * delta_eta();
        }
    }

    constexpr float phi_value(const int phi_coord) const
    {
      return start_phi() + phi_coord * delta_phi();
    }

   private:

    constexpr void add_cell_to_grid(const int cell, const float eta_fraction, const float phi_fraction, const int eta, const int phi)
    {
      if (eta < 0 || eta >= s_eta_grid_size || phi < 0 || phi >= phi_grid)
        {
#if CALORECGPU_ETA_PHI_MAP_DEBUG
          printf("CALORECGPU ETA PHI MAP DEBUG OUTPUT: Attempt out of bounds store %d (%d): %f %f (%d / %d, %d / %d)\n",
                 cell, sampling_number, eta_fraction, phi_fraction, eta, eta_grid, phi, phi_grid);
#endif
          return;
        }

      for (int i = 0; i < s_max_overlap_cells; ++i)
        {
          if (m_cells[eta][phi][i] < 0)
            {
              m_cells           [eta][phi][i] = cell;
              m_eta_coordinates [eta][phi][i] = eta_fraction;
              m_phi_coordinates [eta][phi][i] = phi_fraction;
              return;
            }
        }

#if CALORECGPU_ETA_PHI_MAP_DEBUG
      printf("CALORECGPU ETA PHI MAP DEBUG OUTPUT: Unable to store %d (%d): ", cell, sampling_number);
      for (int i = 0; i < s_max_overlap_cells; ++i)
        {
          printf("%d ", m_cells[eta][phi][i]);
        }
      printf("(%d / %d , %d / %d)\n", eta, s_eta_grid_size, phi, phi_grid);

      if (!respect_deltas)
        {
          const float this_eta = eta_value(eta);
          const float this_phi = phi_value(phi);
          for (int i = 0; i < s_max_overlap_cells; ++i)
            {
              const float cell_eta = m_eta_coordinates [eta][phi][i];
              const float cell_phi = m_phi_coordinates [eta][phi][i];

              const float d_eta_1 = fabsf(this_eta - cell_eta);
              const float d_eta_2 = fabsf(this_eta + delta_eta() - cell_eta);

              const float d_phi_1 = fabsf(Helpers::angular_difference(this_phi, cell_phi));
              const float d_phi_2 = fabsf(Helpers::angular_difference(this_phi + delta_phi(), cell_phi));

              printf("   %f %f %f %f | %f %f %f %f (%f %f | %f %f %f %f)\n",
                     d_eta_1 + d_phi_1,
                     d_eta_1 + d_phi_2,
                     d_eta_2 + d_phi_1,
                     d_eta_2 + d_phi_2,
                     d_eta_1,
                     d_eta_2,
                     d_phi_1,
                     d_phi_2,
                     cell_eta,
                     cell_phi,
                     this_eta,
                     this_eta + delta_eta(),
                     this_phi,
                     this_phi + delta_phi());
            }
        }
#endif
    }

   public:

    constexpr void register_cell(const int cell, const float cell_eta, const float cell_phi, const float cell_deta, const float cell_dphi)
    {
      int start_eta_coord = -1, end_eta_coord = -1;
      float start_eta_frac = 0.f, end_eta_frac = 0.f;

      start_eta_coord = eta_coordinate(cell_eta - cell_deta / 2, start_eta_frac);
      end_eta_coord   = eta_coordinate(cell_eta + cell_deta / 2,   end_eta_frac);

      int   start_phi_coord = -1, end_phi_coord = -1;
      float start_phi_frac = 0.f, end_phi_frac = 0.f;
      int   start_phi_extra = -1, end_phi_extra = -1;
      float start_phi_extra_frac = 0.f, end_phi_extra_frac = 0.f;


      if (cell_phi - cell_dphi / 2 < s_phi_min && cell_phi + cell_dphi / 2 > s_phi_max)
        //The (impossible) case of a full wraparound in phi
        {
          start_phi_coord = 0;
          start_phi_frac  = 0.f;

          end_phi_coord   = phi_grid - 1;
          end_phi_frac    = 1.f;
        }
      else if (cell_phi - cell_dphi / 2 < s_phi_min)
        {
          start_phi_coord    = 0;
          start_phi_frac     = 0.f;

          end_phi_coord      = phi_coordinate(cell_phi + cell_dphi / 2, end_phi_frac);

          start_phi_extra    = phi_coordinate(Helpers::regularize_angle(cell_phi - cell_dphi / 2), start_phi_extra_frac);

          end_phi_extra      = phi_grid - 1;
          end_phi_extra_frac = 1.f;
        }
      else if (cell_phi + cell_dphi / 2 > s_phi_max)
        {
          start_phi_coord      = phi_coordinate(cell_phi - cell_dphi / 2, start_phi_frac);

          end_phi_coord        = phi_grid - 1;
          end_phi_frac         = 1.f;

          start_phi_extra      = phi_coordinate(Helpers::regularize_angle(cell_phi - cell_dphi / 2), start_phi_extra_frac);

          start_phi_extra      = 0;
          start_phi_extra_frac = 1.f;

          end_phi_extra        = phi_coordinate(Helpers::regularize_angle(cell_phi + cell_dphi / 2), end_phi_extra_frac);
        }
      else
        {
          start_phi_coord = phi_coordinate(cell_phi - cell_dphi / 2, start_phi_frac);
          end_phi_coord   = phi_coordinate(cell_phi + cell_dphi / 2, end_phi_frac);
        }

      for (int i = start_eta_coord; i <= end_eta_coord; ++i)
        {
          const float eta_frac = (i == start_eta_coord ? -start_eta_frac : i == end_eta_coord ? end_eta_frac : 0.f);

          for (int j = start_phi_coord; j <= end_phi_coord; ++j)
            {
              const float phi_frac = (j == start_phi_coord ? -start_phi_frac : j == end_phi_coord ? end_phi_frac : 0.f);

              add_cell_to_grid(cell, (respect_deltas ? eta_frac : cell_eta), (respect_deltas ? phi_frac : cell_phi), i, j);
            }

          if (start_phi_extra >= 0)
            {
              for (int j = start_phi_extra; j <= end_phi_extra; ++j)
                {
                  const float phi_frac = (j == start_phi_extra ? -start_phi_extra_frac : j == end_phi_extra ? end_phi_extra_frac : 0.f);

                  add_cell_to_grid(cell, (respect_deltas ? eta_frac : cell_eta), (respect_deltas ? phi_frac : cell_phi), i, j);
                }
            }
        }
    }

    constexpr void initialize()
    {
      for (int i = 0; i < s_eta_grid_size; ++i)
        {
          for (int j = 0; j < phi_grid; ++j)
            {
              for (int k = 0; k < s_max_overlap_cells; ++k)
                {
                  m_cells[i][j][k] = -1;
                }
            }
        }
    }

    constexpr void initialize(const float min_eta, const float max_eta)
    {
      initialize();

      if (continuous)
        {
          m_eta_limits[0] = max_eta;

          m_delta_eta = (max_eta / eta_grid) * 2;
        }
      else
        {
          m_eta_limits[0] = min_eta;
          m_eta_limits[1] = max_eta;

          m_delta_eta = ((max_eta - min_eta) / eta_grid) * 2;
        }

#if CALORECGPU_ETA_PHI_MAP_DEBUG
      if (sampling_number < 24)
        {
          printf("CALORECGPU ETA PHI MAP DEBUG OUTPUT: %d | %d | %f %f | %f %f | %f %f | %f %f\n",
                 sampling_number, static_cast<int>(continuous),
                 start_eta(false), end_eta(false),
                 start_eta(true), end_eta(true),
                 start_phi(), end_phi(),
                 delta_eta(), delta_phi());
        }
#endif
    }

   private:

    struct FinishInitializingTemporaries
    {
      int   cells  [s_eta_grid_size][phi_grid][s_max_overlap_cells];
      float etas   [s_eta_grid_size][phi_grid][s_max_overlap_cells];
      float phis   [s_eta_grid_size][phi_grid][s_max_overlap_cells];

      static constexpr int s_max_cells = phi_grid * s_eta_grid_size;

      int grid_list[2][s_max_cells];
      int counter[2];
      bool select;
      char added[s_max_cells];

      constexpr const int & get_counter() const
      {
        return counter[select];
      }
      constexpr int & get_counter()
      {
        return counter[select];
      }
      constexpr const int & get_next_counter() const
      {
        return counter[!select];
      }
      constexpr int & get_next_counter()
      {
        return counter[!select];
      }
      constexpr const int * get_gridcells() const
      {
        return grid_list[select];
      }
      constexpr int * get_gridcells()
      {
        return grid_list[select];
      }
      constexpr const int * get_next_gridcells() const
      {
        return grid_list[!select];
      }
      constexpr int * get_next_gridcells()
      {
        return grid_list[!select];
      }

      constexpr void swap()
      {
        for (int i = 0; i < s_max_cells; ++i)
          {
            added[i] = 0;
          }
        select = !select;
      }

      constexpr void clear_next_gridcells()
      {
        get_next_counter() = 0;
      }

      constexpr bool add_next_gridcell(const int value)
      {
        if (get_next_counter() >= s_max_cells)
          {
#if CALORECGPU_ETA_PHI_MAP_DEBUG
            printf("CALORECGPU ETA PHI MAP DEBUG OUTPUT: cannot add more cells! (%d)\n", sampling_number);
#endif
            return false;
          }
        get_next_gridcells()[get_next_counter()] = value;
        ++get_next_counter();
        return true;
      }

      constexpr bool add_next_gridcell(const int eta, const int phi)
      {
        return add_next_gridcell(eta * phi_grid + phi);
      }

      constexpr bool try_add_next_gridcell(const int eta, const int phi)
      {
        const int value = eta * phi_grid + phi;

        if (added[value])
          {
            return false;
          }

        added[value] = 1;

        return add_next_gridcell(value);
      }

      constexpr void get_gridcell(const int i, int & eta, int & phi)
      {
        const int v = get_gridcells()[i];

        eta = v / phi_grid;
        phi = v % phi_grid;
      }
    };

   public:

    constexpr static size_t finish_initializing_buffer_size()
    {
      return sizeof(FinishInitializingTemporaries);
    }

    ///! @par buffer is casted to a sufficiently large array (minimum size given by `finish_initializing_buffer_size()`).
    CUDA_HOS_DEV void finish_initializing(void * buffer)
    {
      if (!respect_deltas)
        {
          using namespace std;

          FinishInitializingTemporaries * temp_ptr = new (buffer) FinishInitializingTemporaries;

          FinishInitializingTemporaries & temps = *temp_ptr;

          auto calculate_minima = [&](float & min_dist_eta, float & min_dist_phi,
                                      const float this_eta, const float this_phi,
                                      const float gridcell_eta, const float gridcell_phi)
          {
            if (this_eta >= gridcell_eta && this_eta <= gridcell_eta + delta_eta())
              {
                min_dist_eta = this_eta;
              }
            else
              {
                if (this_eta < gridcell_eta)
                  {
                    min_dist_eta = gridcell_eta;
                  }
#if CALORECGPU_ETA_PHI_MAP_DEBUG
                else if (this_eta <= gridcell_eta + delta_eta())
                  {
                    printf("CALORECGPU ETA PHI MAP DEBUG OUTPUT: Strange things going on with eta distance (%d): %f | %f %f\n",
                           sampling_number, this_eta, gridcell_eta, gridcell_eta + delta_eta());
                  }
#endif
                else
                  {
                    min_dist_eta = gridcell_eta + delta_eta();
                  }
              }

            if (this_phi >= gridcell_phi && this_phi <= gridcell_phi + delta_phi())
              {
                min_dist_phi = this_phi;
              }
            else
              {
                const float d1 = fabsf(Helpers::angular_difference(gridcell_phi, this_phi));
                const float d2 = fabsf(Helpers::angular_difference(gridcell_phi + delta_phi(), this_phi));
                if (d1 < d2)
                  {
                    min_dist_phi = gridcell_phi;
                  }
#if CALORECGPU_ETA_PHI_MAP_DEBUG
                else if (d1 == d2)
                  {
                    printf("CALORECGPU ETA PHI MAP DEBUG OUTPUT: Strange things going on with phi distance (%d): %f | %f %f\n",
                           sampling_number, this_phi, gridcell_phi, gridcell_phi + delta_phi());
                  }
#endif
                else
                  {
                    min_dist_phi = gridcell_phi + delta_phi();
                  }
              }
          };

          auto calculate_dist = [](const float e_1, const float p_1,
                                   const float e_2, const float p_2)
          {
            return fabsf(e_1 - e_2) + fabsf(Helpers::angular_difference(p_1, p_2));
          };

          auto update_cell_list = [&](int         (&cells)    [s_max_overlap_cells],
                                      float       (&etas)     [s_max_overlap_cells],
                                      float       (&phis)     [s_max_overlap_cells],
                                      float       (&min_etas) [s_max_overlap_cells],
                                      float       (&min_phis) [s_max_overlap_cells],
                                      const int   this_cell,
                                      const float this_eta,
                                      const float this_phi,
                                      const float min_dist_eta,
                                      const float min_dist_phi)
          {
            int i = 0;
            int to_replace[s_max_overlap_cells];
            int replace_count = 0;

            const float new_dist_at_new_minimum = calculate_dist(this_eta, this_phi, min_dist_eta, min_dist_phi);

            for (i = 0; i < s_max_overlap_cells; ++i)
              {
                if (cells[i] < 0)
                  {
                    break;
                  }

                if (cells[i] == this_cell)
                  {
                    //Already part of the list...
                    return false;
                  }

                const float old_dist_at_new_minimum = calculate_dist(etas[i], phis[i], min_dist_eta, min_dist_phi);

                if (old_dist_at_new_minimum <= new_dist_at_new_minimum)
                  {
                    //There is a cell that will always be closer
                    return false;
                  }

                const float new_dist_at_old_minimum = calculate_dist(this_eta, this_phi, min_etas[i], min_phis[i]);
                const float old_dist_at_old_minimum = calculate_dist(etas[i], phis[i], min_etas[i], min_phis[i]);

                if (new_dist_at_old_minimum < old_dist_at_old_minimum)
                  {
                    //An old cell was actually further away.
                    to_replace[replace_count] = i;
                    ++replace_count;
                  }
              }

            while (replace_count > 0 && i > 0)
              {
                const int orig = to_replace[replace_count - 1];
                if (orig != i - 1)
                  {
                    cells    [orig] = cells    [i - 1];
                    etas     [orig] = etas     [i - 1];
                    phis     [orig] = phis     [i - 1];
                    min_etas [orig] = min_etas [i - 1];
                    min_phis [orig] = min_phis [i - 1];
                  }
                cells[i - i] = -1;
                --replace_count;
                --i;
                if (i < 0 || (i == 0 && replace_count > 0))
                  {
#if CALORECGPU_ETA_PHI_MAP_DEBUG
                    printf("CALORECGPU ETA PHI MAP DEBUG OUTPUT: Negative count on cell list update, somehow... (%d)\n", sampling_number);
#endif
                    break;
                  }
              }

            if (i < s_max_overlap_cells)
              {
                cells    [i] = this_cell;
                etas     [i] = this_eta;
                phis     [i] = this_phi;
                min_etas [i] = min_dist_eta;
                min_phis [i] = min_dist_phi;
                return true;
              }
#if CALORECGPU_ETA_PHI_MAP_DEBUG
            else
              {
                printf("CALORECGPU ETA PHI MAP DEBUG OUTPUT: Warning: overfull list of overlapping cells: %d (%d)\n", i, sampling_number);
              }
#endif
            return false;
          };

          auto add_possible_cells = [&](int         (&cells)    [s_max_overlap_cells],
                                        float       (&etas)     [s_max_overlap_cells],
                                        float       (&phis)     [s_max_overlap_cells],
                                        float       (&min_etas) [s_max_overlap_cells],
                                        float       (&min_phis) [s_max_overlap_cells],
                                        const float gridcell_eta,
                                        const float gridcell_phi,
                                        const int   orig_eta,
                                        const int   orig_phi)
          {
            bool updated = false;
            for (int i = 0; i < s_max_overlap_cells; ++i)
              {
                const int this_cell = m_cells[orig_eta][orig_phi][i];

                if (this_cell < 0)
                  {
                    break;
                  }

                const float this_eta = m_eta_coordinates[orig_eta][orig_phi][i];
                const float this_phi = m_phi_coordinates[orig_eta][orig_phi][i];

                float min_dist_eta = 0.f, min_dist_phi = 0.f;

                calculate_minima(min_dist_eta, min_dist_phi, this_eta, this_phi, gridcell_eta, gridcell_phi);

                updated = updated || update_cell_list(cells, etas, phis, min_etas, min_phis,
                                                      this_cell, this_eta, this_phi, min_dist_eta, min_dist_phi);

              }

            return updated;
          };

          auto process_cell = [&](const int eta, const int phi)
          {
            const int phi_before = (phi == 0 ? phi_grid - 1 : phi - 1);
            const int phi_after  = (phi == phi_grid - 1 ? 0 : phi + 1);
            const int eta_before = (eta == 0 || eta == eta_grid ? -1 : eta - 1);
            const int eta_after  = (eta == eta_grid - 1 || eta == 2 * eta_grid - 1 ? -1 : eta + 1);

            const float this_grid_eta = eta_value(eta);
            const float this_grid_phi = phi_value(phi);

            float min_etas[s_max_overlap_cells], min_phis[s_max_overlap_cells];

            for (int i = 0; i < s_max_overlap_cells; ++i)
              {
                if (temps.cells[eta][phi][i] < 0)
                  {
                    break;
                  }

                const float this_eta = m_eta_coordinates[eta][phi][i];
                const float this_phi = m_phi_coordinates[eta][phi][i];

                calculate_minima(min_etas[i], min_phis[i], this_eta, this_phi, this_grid_eta, this_grid_phi);
              }

            bool added = false;

            added = added || add_possible_cells(temps.cells[eta][phi],
                                                temps.etas[eta][phi], temps.phis[eta][phi],
                                                min_etas, min_phis,
                                                this_grid_eta, this_grid_phi,
                                                eta, phi_before);
            added = added || add_possible_cells(temps.cells[eta][phi],
                                                temps.etas[eta][phi], temps.phis[eta][phi],
                                                min_etas, min_phis,
                                                this_grid_eta, this_grid_phi,
                                                eta, phi_after);

            if (eta_before >= 0)
              {
                added = added || add_possible_cells(temps.cells[eta][phi],
                                                    temps.etas[eta][phi], temps.phis[eta][phi],
                                                    min_etas, min_phis,
                                                    this_grid_eta, this_grid_phi,
                                                    eta_before, phi);
              }
            if (eta_after >= 0)
              {
                added = added || add_possible_cells(temps.cells[eta][phi],
                                                    temps.etas[eta][phi], temps.phis[eta][phi],
                                                    min_etas, min_phis,
                                                    this_grid_eta, this_grid_phi,
                                                    eta_after, phi);
              }

            bool ret = false;

            if (added)
              {
                ret = ret || temps.try_add_next_gridcell(eta, phi_before);
                ret = ret || temps.try_add_next_gridcell(eta, phi_after);
                if (eta_before >= 0)
                  {
                    ret = ret || temps.try_add_next_gridcell(eta_before, phi);
                  }
                if (eta_after >= 0)
                  {
                    ret = ret || temps.try_add_next_gridcell(eta_after, phi);
                  }
              }

            return ret;
          };

          temps.counter[0] = 0;
          temps.counter[1] = 0;
          temps.select = false;

          temps.swap();

          for (int eta = 0; eta < s_eta_grid_size; ++eta)
            {
              for (int phi = 0; phi < phi_grid; ++phi)
                {
                  memcpy(temps.cells[eta][phi], m_cells[eta][phi], s_max_overlap_cells * sizeof(int));

                  if (m_cells[eta][phi][0] >= 0)
                    {
                      memcpy(temps.etas[eta][phi], m_eta_coordinates[eta][phi], s_max_overlap_cells * sizeof(float));
                      memcpy(temps.phis[eta][phi], m_phi_coordinates[eta][phi], s_max_overlap_cells * sizeof(float));

                      const int phi_before = (phi == 0 ? phi_grid - 1 : phi - 1);
                      const int phi_after  = (phi == phi_grid - 1 ? 0 : phi + 1);
                      const int eta_before = (eta == 0 || eta == eta_grid ? -1 : eta - 1);
                      const int eta_after  = (eta == eta_grid - 1 || eta == s_eta_grid_size - 1 ? -1 : eta + 1);

                      temps.try_add_next_gridcell(eta, phi);

                      temps.try_add_next_gridcell(eta, phi_before);
                      temps.try_add_next_gridcell(eta, phi_after);
                      if (eta_before >= 0)
                        {
                          temps.try_add_next_gridcell(eta_before, phi);
                        }
                      if (eta_after >= 0)
                        {
                          temps.try_add_next_gridcell(eta_after, phi);
                        }
                    }
                }
            }

          temps.swap();
          temps.clear_next_gridcells();

#if CALORECGPU_ETA_PHI_MAP_DEBUG
          int iter_count = 0;
          int equal_iter_counter = 0;
#endif

          while (temps.get_counter() > 0)
            {
              for (int i = 0; i < temps.get_counter(); ++i)
                {
                  int eta, phi;
                  temps.get_gridcell(i, eta, phi);
                  process_cell(eta, phi);
                }

#if CALORECGPU_ETA_PHI_MAP_DEBUG
              ++iter_count;

              printf("CALORECGPU ETA PHI MAP DEBUG OUTPUT: Init %d: %d | %d %d (%d)\n",
                     sampling_number, iter_count, temps.get_counter(), temps.get_next_counter(), equal_iter_counter);

              if (temps.get_counter() == temps.get_next_counter())
                {
                  int count_same = 0;

                  for (int i = 0; i < temps.get_counter(); ++i)
                    {
                      count_same += (temps.get_gridcells()[i] == temps.get_next_gridcells()[i]);
                    }
                  if (count_same == temps.get_counter())
                    {
                      ++equal_iter_counter;
                      if (equal_iter_counter >= 4)
                        //Assume we got stuck in some weird loop...
                        {
                          temps.clear_next_gridcells();
                        }
                      if (temps.get_counter() < 10)
                        {
                          for (int i = 0; i < temps.get_counter(); ++i)
                            {
                              int eta, phi;
                              temps.get_gridcell(i, eta, phi);
                              printf("   %d %d", eta, phi);
                              for (int j = 0; j < s_max_overlap_cells; ++j)
                                {
                                  if (temps.cells[eta][phi][j] < 0)
                                    {
                                      break;
                                    }
                                  printf(" (%d: %f %f)", temps.cells[eta][phi][j], temps.etas[eta][phi][j], temps.phis[eta][phi][j]);
                                }
                              printf("\n");
                            }
                        }
                    }
                  else
                    {
                      equal_iter_counter = 0;
                    }
                }
              else
                {
                  equal_iter_counter = 0;
                }
#endif

              for (int i = 0; i < temps.get_counter(); ++i)
                {
                  int eta, phi;
                  temps.get_gridcell(i, eta, phi);

                  memcpy(m_cells          [eta][phi], temps.cells[eta][phi], s_max_overlap_cells * sizeof(int)  );
                  memcpy(m_eta_coordinates[eta][phi], temps.etas [eta][phi], s_max_overlap_cells * sizeof(float));
                  memcpy(m_phi_coordinates[eta][phi], temps.phis [eta][phi], s_max_overlap_cells * sizeof(float));
                }

              temps.swap();
              temps.clear_next_gridcells();

            }

#if CALORECGPU_ETA_PHI_MAP_DEBUG
          printf("CALORECGPU ETA PHI MAP DEBUG OUTPUT: Finished %d: %d (%d %d)\n",
                 sampling_number, get_max_real_overlap(), iter_count, equal_iter_counter);
#endif
        }
      else
        {
          //Do nothing: when respecting deltas,
          //cells get properly initialized outright...
        }
    }

    ///We assume @p cell_arr is large enough.
    constexpr int get_possible_cells_from_coords(const float test_eta, const float test_phi, int * cell_arr) const
    {
      if (!coordinates_in_range(test_eta, test_phi))
        {
          return 0;
        }

      int num_cells = 0;

      float frac_eta = 0.f, frac_phi = 0.f;

      const int eta_coord = eta_coordinate(test_eta, frac_eta);
      const int phi_coord = phi_coordinate(test_phi, frac_phi);
      
      if (eta_coord < 0 || eta_coord >= s_eta_grid_size || phi_coord < 0 || phi_coord >= phi_grid)
        {
          return 0;
        }

      if (respect_deltas)
        {
          auto check_coord = [](const float test, const float target)
          {
            using namespace std;
            if (target > 0 && fabsf(test) <= fabsf(target))
              {
                return true;
              }
            else if (target < 0 && fabsf(test) >= fabsf(target))
              {
                return true;
              }
            else if (target == 0)
              {
                return true;
              }
            else
              {
                return false;
              }
          };

          for (int i = 0; i < s_max_overlap_cells; ++i)
            {
              const int this_cell = m_cells[eta_coord][phi_coord][i];

              if (this_cell < 0)
                {
                  break;
                }

              if (check_coord(frac_eta, m_eta_coordinates[eta_coord][phi_coord][i]) &&
                  check_coord(frac_phi, m_phi_coordinates[eta_coord][phi_coord][i])   )
                {
                  cell_arr[num_cells] = this_cell;
                  ++num_cells;
                }
            }
        }
      else
        {
          float distance = 1e38f;
          int ret = -1;

          for (int i = 0; i < s_max_overlap_cells; ++i)
            {
              const int this_cell = m_cells[eta_coord][phi_coord][i];

              if (this_cell < 0)
                {
                  break;
                }

              const float this_delta_eta = m_eta_coordinates[eta_coord][phi_coord][i] - test_eta;

              const float this_delta_phi = Helpers::angular_difference(m_phi_coordinates[eta_coord][phi_coord][i], test_phi);

              using namespace std;

              const float this_dist = fabsf(this_delta_eta) + fabsf(this_delta_phi);

              if (this_dist < distance || (this_dist == distance && this_cell > ret))
                {
                  distance = this_dist;
                  ret = this_cell;
                }
            }

          if (ret > 0)
            {
              cell_arr[0] = ret;
              num_cells = 1;
            }
        }

      return num_cells;
    }

    constexpr bool has_cell_in_coords(const float test_eta, const float test_phi) const
    {
      if (!coordinates_in_range(test_eta, test_phi))
        {
          return false;
        }

      float frac_eta = 0.f, frac_phi = 0.f;

      const int eta_coord = eta_coordinate(test_eta, frac_eta);
      const int phi_coord = phi_coordinate(test_phi, frac_phi);

      if (eta_coord < 0 || eta_coord >= s_eta_grid_size || phi_coord < 0 || phi_coord >= phi_grid)
        {
          return false;
        }
        
      if (respect_deltas)
        {
          auto check_coord = [](const float test, const float target)
          {
            using namespace std;
            if (target > 0 && fabsf(test) <= fabsf(target))
              {
                return true;
              }
            else if (target < 0 && fabsf(test) >= fabsf(target))
              {
                return true;
              }
            else if (target == 0)
              {
                return true;
              }
            else
              {
                return false;
              }
          };

          for (int i = 0; i < s_max_overlap_cells; ++i)
            {
              if (m_cells[eta_coord][phi_coord][i] < 0)
                {
                  break;
                }
              if ( check_coord(frac_eta, m_eta_coordinates[eta_coord][phi_coord][i]) &&
                   check_coord(frac_phi, m_phi_coordinates[eta_coord][phi_coord][i])      )
                {
                  return true;
                }
            }
          return false;
        }
      else
        {
          return true;
          //Except for being out-of-bounds,
          //if respect_deltas == false
          //we can always find a closest cell
          //for each coordinate.
        }
    }

    constexpr int get_max_real_overlap() const
    {
      int ret = 0;

      for (int eta = 0; eta < s_eta_grid_size; ++eta)
        {
          for (int phi = 0; phi < phi_grid; ++phi)
            {
              int this_count = 0;

              for (int i = 0; i < s_max_overlap_cells; ++i)
                {
                  if (m_cells[eta][phi][i] >= 0)
                    {
                      ++this_count;
                    }
                  else
                    {
                      break;
                    }
                }

              if (this_count > ret)
                {
                  ret = this_count;
                }
            }
        }

      return ret;
    }

  };


  struct EtaPhiToCellMap
  {
    static constexpr int s_max_overlap_cells = EtaPhiMapEntry<1, 1, true, true, 0>::s_max_overlap_cells;

    //Samplings have custom, hard-coded sizes to save space.
    //Could've gone with one-size-fits all maxima,
    //but it'd likely be unnecessarily wasteful.
    EtaPhiMapEntry<126,  66, false,  true,  0> sampling_0;       //PreSamplerB
    EtaPhiMapEntry<958, 262, false, false,  1> sampling_1;       //EMB1
    EtaPhiMapEntry<124, 262, false, false,  2> sampling_2;       //EMB2
    EtaPhiMapEntry< 58, 262, false, false,  3> sampling_3;       //EMB3
    EtaPhiMapEntry< 25,  68, false, false,  4> sampling_4;       //PreSamplerE
    EtaPhiMapEntry<732,  68, false, false,  5> sampling_5;       //EME1
    EtaPhiMapEntry<150, 260, false, false,  6> sampling_6;       //EME2
    EtaPhiMapEntry< 72, 260, false, false,  7> sampling_7;       //EME3
    EtaPhiMapEntry< 40,  66, false, false,  8> sampling_8;       //HEC0
    EtaPhiMapEntry< 34,  66, false, false,  9> sampling_9;       //HEC1
    EtaPhiMapEntry< 32,  66, false, false, 10> sampling_10;      //HEC2
    EtaPhiMapEntry< 34,  66, false, false, 11> sampling_11;      //HEC3
    EtaPhiMapEntry< 24,  66,  true,  true, 12> sampling_12;      //TileBar0
    EtaPhiMapEntry< 20,  66,  true,  true, 13> sampling_13;      //TileBar1
    EtaPhiMapEntry< 10,  66,  true,  true, 14> sampling_14;      //TileBar2
    EtaPhiMapEntry<  6,  66,  true, false, 15> sampling_15;      //TileGap1
    EtaPhiMapEntry<  6,  66,  true, false, 16> sampling_16;      //TileGap2
    EtaPhiMapEntry< 16,  66,  true, false, 17> sampling_17;      //TileGap3
    EtaPhiMapEntry< 12,  66,  true, false, 18> sampling_18;      //TileExt0
    EtaPhiMapEntry< 12,  66,  true, false, 19> sampling_19;      //TileExt1
    EtaPhiMapEntry<  6,  66,  true, false, 20> sampling_20;      //TileExt2
    EtaPhiMapEntry<148, 232, false, false, 21> sampling_21;      //FCAL0
    EtaPhiMapEntry< 98, 164, false, false, 22> sampling_22;      //FCAL1
    EtaPhiMapEntry< 64, 130, false, false, 23> sampling_23;      //FCAL2
    EtaPhiMapEntry<  1,   1,  true,  true, 24> sampling_24;      //MINIFCAL0
    EtaPhiMapEntry<  1,   1,  true,  true, 25> sampling_25;      //MINIFCAL1
    EtaPhiMapEntry<  1,   1,  true,  true, 26> sampling_26;      //MINIFCAL2
    EtaPhiMapEntry<  1,   1,  true,  true, 27> sampling_27;      //MINIFCAL3

    static_assert(NumSamplings == 28, "Written under the assumption there are 28 samplings.");

    ///@p F must be prepared to receive as first argument a EtaPhiMapEntry<N, M>, as well as any arguments.
    template <class Func, class ... Args>
    constexpr void apply_to_all_samplings(Func && F, Args && ... args)
    {
      F(sampling_0, std::forward<Args>(args)...);
      F(sampling_1, std::forward<Args>(args)...);
      F(sampling_2, std::forward<Args>(args)...);
      F(sampling_3, std::forward<Args>(args)...);
      F(sampling_4, std::forward<Args>(args)...);
      F(sampling_5, std::forward<Args>(args)...);
      F(sampling_6, std::forward<Args>(args)...);
      F(sampling_7, std::forward<Args>(args)...);
      F(sampling_8, std::forward<Args>(args)...);
      F(sampling_9, std::forward<Args>(args)...);
      F(sampling_10, std::forward<Args>(args)...);
      F(sampling_11, std::forward<Args>(args)...);
      F(sampling_12, std::forward<Args>(args)...);
      F(sampling_13, std::forward<Args>(args)...);
      F(sampling_14, std::forward<Args>(args)...);
      F(sampling_15, std::forward<Args>(args)...);
      F(sampling_16, std::forward<Args>(args)...);
      F(sampling_17, std::forward<Args>(args)...);
      F(sampling_18, std::forward<Args>(args)...);
      F(sampling_19, std::forward<Args>(args)...);
      F(sampling_20, std::forward<Args>(args)...);
      F(sampling_21, std::forward<Args>(args)...);
      F(sampling_22, std::forward<Args>(args)...);
      F(sampling_23, std::forward<Args>(args)...);
      F(sampling_24, std::forward<Args>(args)...);
      F(sampling_25, std::forward<Args>(args)...);
      F(sampling_26, std::forward<Args>(args)...);
      F(sampling_27, std::forward<Args>(args)...);
    }

    ///@p F must be prepared to receive as first argument a EtaPhiMapEntry<N, M>, as well as any arguments.
    template <class Func, class ... Args>
    constexpr void apply_to_all_samplings(Func && F, Args && ... args) const
    {
      F(sampling_0, std::forward<Args>(args)...);
      F(sampling_1, std::forward<Args>(args)...);
      F(sampling_2, std::forward<Args>(args)...);
      F(sampling_3, std::forward<Args>(args)...);
      F(sampling_4, std::forward<Args>(args)...);
      F(sampling_5, std::forward<Args>(args)...);
      F(sampling_6, std::forward<Args>(args)...);
      F(sampling_7, std::forward<Args>(args)...);
      F(sampling_8, std::forward<Args>(args)...);
      F(sampling_9, std::forward<Args>(args)...);
      F(sampling_10, std::forward<Args>(args)...);
      F(sampling_11, std::forward<Args>(args)...);
      F(sampling_12, std::forward<Args>(args)...);
      F(sampling_13, std::forward<Args>(args)...);
      F(sampling_14, std::forward<Args>(args)...);
      F(sampling_15, std::forward<Args>(args)...);
      F(sampling_16, std::forward<Args>(args)...);
      F(sampling_17, std::forward<Args>(args)...);
      F(sampling_18, std::forward<Args>(args)...);
      F(sampling_19, std::forward<Args>(args)...);
      F(sampling_20, std::forward<Args>(args)...);
      F(sampling_21, std::forward<Args>(args)...);
      F(sampling_22, std::forward<Args>(args)...);
      F(sampling_23, std::forward<Args>(args)...);
      F(sampling_24, std::forward<Args>(args)...);
      F(sampling_25, std::forward<Args>(args)...);
      F(sampling_26, std::forward<Args>(args)...);
      F(sampling_27, std::forward<Args>(args)...);
    }

    ///@p F must be prepared to receive as first argument a EtaPhiMapEntry<N, M>, as well as any arguments.
    template <class Func, class ... Args>
    constexpr void apply_to_sampling(const int sampling, Func && F, Args && ... args)
    {
      switch (sampling)
        {
          case 0:
            F(sampling_0, std::forward<Args>(args)...);
            break;
          case 1:
            F(sampling_1, std::forward<Args>(args)...);
            break;
          case 2:
            F(sampling_2, std::forward<Args>(args)...);
            break;
          case 3:
            F(sampling_3, std::forward<Args>(args)...);
            break;
          case 4:
            F(sampling_4, std::forward<Args>(args)...);
            break;
          case 5:
            F(sampling_5, std::forward<Args>(args)...);
            break;
          case 6:
            F(sampling_6, std::forward<Args>(args)...);
            break;
          case 7:
            F(sampling_7, std::forward<Args>(args)...);
            break;
          case 8:
            F(sampling_8, std::forward<Args>(args)...);
            break;
          case 9:
            F(sampling_9, std::forward<Args>(args)...);
            break;
          case 10:
            F(sampling_10, std::forward<Args>(args)...);
            break;
          case 11:
            F(sampling_11, std::forward<Args>(args)...);
            break;
          case 12:
            F(sampling_12, std::forward<Args>(args)...);
            break;
          case 13:
            F(sampling_13, std::forward<Args>(args)...);
            break;
          case 14:
            F(sampling_14, std::forward<Args>(args)...);
            break;
          case 15:
            F(sampling_15, std::forward<Args>(args)...);
            break;
          case 16:
            F(sampling_16, std::forward<Args>(args)...);
            break;
          case 17:
            F(sampling_17, std::forward<Args>(args)...);
            break;
          case 18:
            F(sampling_18, std::forward<Args>(args)...);
            break;
          case 19:
            F(sampling_19, std::forward<Args>(args)...);
            break;
          case 20:
            F(sampling_20, std::forward<Args>(args)...);
            break;
          case 21:
            F(sampling_21, std::forward<Args>(args)...);
            break;
          case 22:
            F(sampling_22, std::forward<Args>(args)...);
            break;
          case 23:
            F(sampling_23, std::forward<Args>(args)...);
            break;
          case 24:
            F(sampling_24, std::forward<Args>(args)...);
            break;
          case 25:
            F(sampling_25, std::forward<Args>(args)...);
            break;
          case 26:
            F(sampling_26, std::forward<Args>(args)...);
            break;
          case 27:
            F(sampling_27, std::forward<Args>(args)...);
            break;
          default:
            break;
        }
    }

    ///@p F must be prepared to receive as first argument a EtaPhiMapEntry<N, M>, as well as any arguments.
    template <class Func, class ... Args>
    constexpr void apply_to_sampling(const int sampling, Func && F, Args && ... args) const
    {
      switch (sampling)
        {
          case 0:
            F(sampling_0, std::forward<Args>(args)...);
            break;
          case 1:
            F(sampling_1, std::forward<Args>(args)...);
            break;
          case 2:
            F(sampling_2, std::forward<Args>(args)...);
            break;
          case 3:
            F(sampling_3, std::forward<Args>(args)...);
            break;
          case 4:
            F(sampling_4, std::forward<Args>(args)...);
            break;
          case 5:
            F(sampling_5, std::forward<Args>(args)...);
            break;
          case 6:
            F(sampling_6, std::forward<Args>(args)...);
            break;
          case 7:
            F(sampling_7, std::forward<Args>(args)...);
            break;
          case 8:
            F(sampling_8, std::forward<Args>(args)...);
            break;
          case 9:
            F(sampling_9, std::forward<Args>(args)...);
            break;
          case 10:
            F(sampling_10, std::forward<Args>(args)...);
            break;
          case 11:
            F(sampling_11, std::forward<Args>(args)...);
            break;
          case 12:
            F(sampling_12, std::forward<Args>(args)...);
            break;
          case 13:
            F(sampling_13, std::forward<Args>(args)...);
            break;
          case 14:
            F(sampling_14, std::forward<Args>(args)...);
            break;
          case 15:
            F(sampling_15, std::forward<Args>(args)...);
            break;
          case 16:
            F(sampling_16, std::forward<Args>(args)...);
            break;
          case 17:
            F(sampling_17, std::forward<Args>(args)...);
            break;
          case 18:
            F(sampling_18, std::forward<Args>(args)...);
            break;
          case 19:
            F(sampling_19, std::forward<Args>(args)...);
            break;
          case 20:
            F(sampling_20, std::forward<Args>(args)...);
            break;
          case 21:
            F(sampling_21, std::forward<Args>(args)...);
            break;
          case 22:
            F(sampling_22, std::forward<Args>(args)...);
            break;
          case 23:
            F(sampling_23, std::forward<Args>(args)...);
            break;
          case 24:
            F(sampling_24, std::forward<Args>(args)...);
            break;
          case 25:
            F(sampling_25, std::forward<Args>(args)...);
            break;
          case 26:
            F(sampling_26, std::forward<Args>(args)...);
            break;
          case 27:
            F(sampling_27, std::forward<Args>(args)...);
            break;
          default:
            break;
        }
    }

   private:

    //CUDA and lambdas is still a bit tricky,
    //hence we'll use explicitly written functors
    //to implement several things.

    struct initialize_all_functor
    {
      template <class Entry>
      constexpr void operator() (Entry & entry) const
      {
        entry.initialize();
      }
    };

    struct initialize_sampling_functor
    {
      template <class Entry>
      constexpr void operator() (Entry & entry, const float min_eta, const float max_eta) const
      {
        entry.initialize(min_eta, max_eta);
      }
    };

    struct register_cell_functor
    {
      template <class Entry>
      constexpr void operator() (Entry & entry, const int cell,
                                 const float cell_eta, const float cell_phi,
                                 const float cell_deta, const float cell_dphi ) const
      {
        entry.register_cell(cell, cell_eta, cell_phi, cell_deta, cell_dphi);
      }
    };

    struct buffer_size_functor
    {
      template <class Entry>
      constexpr void operator() (Entry & entry, size_t & ret) const
      {
        using namespace std;
        ret = max(ret, entry.finish_initializing_buffer_size());
      }
    };

    struct finish_functor
    {
      template <class Entry>
      constexpr void operator() (Entry & entry, void * buffer) const
      {
        entry.finish_initializing(buffer);
      }
    };
    //Aliohjelma-olio?
    //(To not go with the obvious funktio-olio...)

    struct get_cell_from_sampling_functor
    {
      template <class Entry>
      constexpr void operator() (Entry & entry, int & ret, const float test_eta, const float test_phi, int * cell_arr) const
      {
        ret = entry.get_possible_cells_from_coords(test_eta, test_phi, cell_arr);
      }
    };

    struct check_cell_in_sampling_functor
    {
      template <class Entry>
      constexpr void operator() (Entry & entry, bool & ret, const float test_eta, const float test_phi) const
      {
        ret = entry.has_cell_in_coords(test_eta, test_phi);
      }
    };

    struct get_cell_from_all_functor
    {
      template <class Entry>
      constexpr void operator() (Entry & entry, int & ret, const float test_eta, const float test_phi, int * cell_arr) const
      {
        ret += entry.get_possible_cells_from_coords(test_eta, test_phi, cell_arr + ret);
      }
    };

    struct check_cell_in_all_functor
    {
      template <class Entry>
      constexpr void operator() (Entry & entry, bool & ret, const float test_eta, const float test_phi) const
      {
        ret = ( ret || entry.has_cell_in_coords(test_eta, test_phi) );
        //Short circuit evaluation?
      }
    };

    struct max_real_overlap_getter_functor
    {
      template <class Entry>
      constexpr void operator() (const Entry & entry, int & ret) const
      {
        using namespace std;
        ret = max(ret, entry.get_max_real_overlap());
      }
    };

   public:

    ///Initialize all cells of all samplings.
    constexpr void initialize()
    {
      apply_to_all_samplings(initialize_all_functor{});
    }

    ///Initialize a specific sampling with known eta and phi ranges.
    constexpr void initialize(const int sampling, const float min_eta, const float max_eta)
    {
      apply_to_sampling(sampling, initialize_sampling_functor{}, min_eta, max_eta);
    }

    constexpr void register_cell(const int cell, const int sampling, const float cell_eta, const float cell_phi, const float cell_deta, const float cell_dphi)
    {
      apply_to_sampling(sampling, register_cell_functor{}, cell, cell_eta, cell_phi, cell_deta, cell_dphi);
    }


    constexpr size_t finish_initializing_buffer_size() const
    {
      size_t ret = 0;

      apply_to_all_samplings(buffer_size_functor{}, ret);

      return ret;
    }

    ///! @par buffer is casted to a sufficiently large array (mininum size given by @p finish_initializing_buffer_size).
    CUDA_HOS_DEV void finish_initializing(void * buffer)
    {
      apply_to_all_samplings(finish_functor{}, buffer);
    }

    ///We assume @p cell_arr is large enough.
    constexpr int get_possible_cells_from_coords(const int sampling, const float test_eta, const float test_phi, int * cell_arr) const
    {
      int ret = 0;

      apply_to_sampling(sampling, get_cell_from_sampling_functor{}, ret, test_eta, test_phi, cell_arr);

      return ret;
    }

    constexpr bool has_cell_in_coords(const int sampling, const float test_eta, const float test_phi) const
    {
      bool ret = false;

      apply_to_sampling(sampling, check_cell_in_sampling_functor{}, ret, test_eta, test_phi);

      return ret;
    }

    ///We assume @p cell_arr is large enough.
    constexpr int get_possible_cells_from_coords(const float test_eta, const float test_phi, int * cell_arr) const
    {
      int ret = 0;

      apply_to_all_samplings(get_cell_from_all_functor{}, ret, test_eta, test_phi, cell_arr);

      return ret;
    }

    constexpr bool has_cell_in_coords(const float test_eta, const float test_phi) const
    {
      bool ret = false;

      apply_to_all_samplings(check_cell_in_all_functor{}, ret, test_eta, test_phi);

      return ret;
    }

    constexpr int get_max_real_overlap() const
    {
      int ret = 0;

      apply_to_all_samplings(max_real_overlap_getter_functor{}, ret);

      return ret;
    }

  };

}

#endif //CALORECGPU_ETAPHIMAP_H
