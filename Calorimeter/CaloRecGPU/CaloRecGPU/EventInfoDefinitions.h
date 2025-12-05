//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_EVENTINFODEFINITIONS_H
#define CALORECGPU_EVENTINFODEFINITIONS_H

#include "BaseDefinitions.h"
#include "TagDefinitions.h"
#include "ConstantInfoDefinitions.h"

#include <cstdint>
#include <cmath>
//For fabsf...

#include "CaloGeoHelpers/CaloSampling.h"
#include "CaloIdentifier/LArNeighbours.h"

namespace CaloRecGPU
{

  /*! @class GainConversion
      Provides utility functions to handle the gain conversion.
  */
  class GainConversion
  {
    using GainType = unsigned int;
   protected:

    constexpr static GainType s_TileLowLow = 0,
                              s_TileLowHigh = 1,
                              s_TileHighLow = 2,
                              s_TileHighHigh = 3;

    constexpr static GainType s_TileOneLow = 0,
                              s_TileOneHigh = 3;
    //Are these valid/used?

    constexpr static GainType s_LArHigh = 0,
                              s_LArMedium = 1,
                              s_LArLow = 2;

    constexpr static GainType s_InvalidCell = 4;

    constexpr static GainType s_gain_values_minimum = 0;
    constexpr static GainType s_gain_values_maximum = 4;
   public:

    inline static constexpr GainType invalid_gain()
    {
      return s_InvalidCell;
    }

    inline static constexpr GainType max_gain_value()
    {
      return s_gain_values_maximum;
    }

    inline static constexpr GainType min_gain_value()
    {
      return s_gain_values_minimum;
    }

    inline static constexpr GainType num_gain_values()
    {
      return max_gain_value() - min_gain_value() + 1;
    }

    template <class T>
    inline static constexpr bool is_valid(const T & gain)
    {
      return gain != s_InvalidCell;
    }

    inline static constexpr GainType from_standard_gain(const int gain)
    //Basically, CaloCondUtils::getDbCaloGain without the Athena logging.
    {
      switch (gain)
        {
          case -16: //Tile LOWLOW
            return s_TileLowLow;
          case -15: //Tile LOWHIGH
            return s_TileLowHigh;
          case -12: //Tile HIGHLOW
            return s_TileHighLow;
          case -11: //Tile HIGHHIGH
            return s_TileHighHigh;
          case -4 : //Tile ONELOW
            return s_TileOneLow;
          case -3 : //Tile ONEHIGH
            return s_TileOneHigh;
          case 0  : //LAr High
            return s_LArHigh;
          case 1  : //LAr Medium
            return s_LArMedium;
          case 2  : //Lar Low
            return s_LArLow;
          default:
            return s_InvalidCell;
        }
    }

  };

  /*! @class QualityProvenance
      Just two uint16_t bit-packed onto a uint32_t.

      Not too worrisome for GPU performance since the kinds of things
      that ask the most for quality and provenance (cluster moments calculation)
      do ask for them both at the same time, so slower separate accesses
      are a non-issue.
  */
  struct QualityProvenance
  {
    using carrier = unsigned int;

    carrier value;

    constexpr static carrier s_16_bit_mask = 0xFFFFU;
    constexpr static carrier s_8_bit_mask = 0x00FFU;

   public:

    constexpr operator carrier () const
    {
      return value;
    }

    constexpr QualityProvenance (const carrier v): value(v)
    {
    }

    constexpr QualityProvenance & operator = (const carrier v)
    {
      value = v;
      return (*this);
    }

    constexpr unsigned int quality() const
    {
      return value & s_16_bit_mask;
    }

    constexpr unsigned int provenance() const
    {
      return (value >> 16) & s_16_bit_mask;
    }

    constexpr QualityProvenance(const uint16_t quality, const uint16_t provenance): value(provenance)
      //For non-tile
    {
      value = (value << 16) | quality;
    }

    constexpr QualityProvenance(const uint8_t q1, const uint8_t q2, const uint8_t q3, const uint8_t q4):
      value(q4)
      //For tile
    {
      value = (value << 8) | q3;
      value = (value << 8) | q2;
      value = (value << 8) | q1;
    }

    constexpr unsigned int tile_qual1() const
    {
      return value & s_8_bit_mask;
    }

    constexpr unsigned int tile_qual2() const
    {
      return (value >> 8) & s_8_bit_mask;
    }

    constexpr unsigned int tile_qbit1() const
    {
      return (value >> 16) & s_8_bit_mask;
    }

    constexpr unsigned int tile_qbit2() const
    {
      return (value >> 24) & s_8_bit_mask;
    }
  };

  ///@class CellBaseInfo
  ///Contains the fundamental information that allows interacting
  ///with @c CellInfoArr so that it is easier to (pre-)transfer it.
  struct CellBaseInfo
  {
    bool complete;
    bool all_cells_valid;
    int number;
    
    constexpr int get_number_of_cells(const bool is_complete = false) const
    {
      if (is_complete)
      {
        return NCaloCells;
      }
      else
      {
        return number;
      }
    }
  };

  ///@class CellInfoArr
  ///Contains the per-event cell information:
  ///energy, timing, gain, quality and provenance.
  ///Since this is ordered in the same way as the CPU cell collection,
  ///functionalities to map between cell hash ID and cell index within the collection
  //are offered.
  struct CellInfoArr : CellBaseInfo
  {
    unsigned char gain[NCaloCells];
    float energy[NCaloCells];
    float time[NCaloCells];
    QualityProvenance::carrier qualityProvenance[NCaloCells];
    //We could use/type pun a short2,
    //but let's go for portability for the time being...

    int hashID[NCaloCells];
    int hashIDToCollection[NCaloCells];

    constexpr int get_hash_ID(const int cell, const bool is_complete = false) const
    {
      if (is_complete)
        {
          return cell;
        }
      else
        {
          return hashID[cell];
        }
    }

    constexpr int get_cell_with_hash_ID(const int index, const bool is_complete = false) const
    {
      if (is_complete)
        {
          return index;
        }
      else
        {
          return hashIDToCollection[index];
        }
    }

    /*! GPU version of @c CaloBadCellHelper::isBad. If @p treat_L1_predicted_as_good is false,
        has the same effect as cell->badcell() (just like @c CaloBadCellHelper::isBad).
    */
    static constexpr bool is_bad_general(const bool is_tile, const QualityProvenance qp, const bool treat_L1_predicted_as_good = false)
    {
      bool ret = false;

      if (is_tile)
        {

          const unsigned int mask = 0x08U;

          ret = (qp.tile_qbit1() & mask) && (qp.tile_qbit2() & mask);
          //From TileCell::badcell()
          //badch1() && badch2()
          //badch1() -> m_tileQual[2]&TileCell::MASK_BADCH
          //badch2() -> m_tileQual[3]&TileCell::MASK_BADCH
          //
          //TileCell::MASK_BADCH= 0x08
          //
          //From CaloCell:
          //union {
          //  int  m_quality ;
          //  uint16_t m_qualProv[2];
          //  uint8_t m_tileQual[4];
          //};
          //quality returns m_qualProv[0]
          //provenance returns m_qualProv[1]
          //These are used to build our QualityProvenance...
          //
          //It's packing and unpacking back and forth, how fun!
        }
      else
        {
          const unsigned int provenance = qp.provenance();
          ret = provenance & 0x0800U;
          //As in LArCell::badcell()

          if (treat_L1_predicted_as_good && (provenance & 0x0200U))
            //As in CaloBadCellHelper::isBad
            {
              ret = false;
            }
        }
      return ret;
    }

    /*! GPU version of @c CaloBadCellHelper::isBad. If @p treat_L1_predicted_as_good is false,
        has the same effect as cell->badcell() (just like @c CaloBadCellHelper::isBad).
    */
    constexpr bool is_bad(const int cell, const bool treat_L1_predicted_as_good = false, const bool is_complete = false) const
    {
      return is_bad_general(GeometryArr::is_tile(get_hash_ID(cell, is_complete)), qualityProvenance[cell], treat_L1_predicted_as_good);
    }

    /*! GPU equivalent of CaloTopoClusterMaker::passCellTimeCut.
    */
    constexpr bool passes_time_cut(const GeometryArr & geom, const int cell, const float threshold,
                                   const bool use_crosstalk, const float crosstalk_delta, const bool is_complete = false) const
    {
      const int real_cell = get_hash_ID(cell, is_complete);
      const int sampling = geom.sampling(real_cell);
      if (sampling == CaloSampling::PreSamplerB ||
          sampling == CaloSampling::PreSamplerE ||
          sampling == CaloSampling::Unknown        )
        {
          return true;
        }
      else
        {
          const QualityProvenance qp = qualityProvenance[cell];
          const unsigned int mask = geom.is_tile(real_cell) ? 0x8080U : 0x2000U;
          if (qp.provenance() & mask)
            {
              const float this_time = time[cell];
              if (fabsf(this_time) >= threshold)
                {
                  if (use_crosstalk)
                    {
                      const float this_energy = energy[cell];
                      const bool eligible = (sampling == CaloSampling::EMB2 || (sampling == CaloSampling::EME2 && fabsf(geom.eta[real_cell]) < 2.5));
                      if (this_energy > 0 && eligible)
                        {
                          int neighbours[NMaxNeighbours] = {};

                          constexpr unsigned int neigh_options = LArNeighbours::neighbourOption::prevInPhi | LArNeighbours::neighbourOption::nextInPhi;

                          const int num_neighs = geom.get_neighbours(neigh_options, real_cell, neighbours);

                          for (int i = 0; i < num_neighs; ++i)
                            {
                              const int neigh_index = get_cell_with_hash_ID(neighbours[i], is_complete);
                              if (neigh_index >= 0 && energy[neigh_index] > 4 * this_energy)
                                {
                                  const QualityProvenance neigh_qp = qualityProvenance[neigh_index];
                                  if ( !(neigh_qp.provenance() & mask) || fabsf(time[neigh_index]) < threshold )
                                    {
                                      if (this_time > -threshold && this_time < threshold + crosstalk_delta)
                                        {
                                          return true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                  return false;
                }
            }
        }
      return true;
    }

    constexpr bool is_valid(const int hash_ID, const bool is_complete = false, const bool all_cells_are_valid = false) const
    {
      if (all_cells_are_valid)
        {
          return true;
        }
      else if (is_complete)
        {
          return GainConversion::is_valid(gain[hash_ID]);
        }
      else
        {
          return hashIDToCollection[hash_ID] >= 0;
        }
    }
  };

  enum class ClusterInformationState : uint8_t
  {
    None,              //No cell assignment to clusters
    Tags,              //Cell assignment to clusters using the generic tags, unspecified order, some may be invalid
    TagsWithBasicInfo, //As before, but also with the basic information having been calculated
    Full,              //Proper cell assignment, all clusters are valid and sorted, but no info
    WithBasicInfo,     //As before, but also with the basic information having been calculated
    WithMoments,       //Moments have been calculated too
    WithExtraMoments,  //And also the additional moments beyond cluster moments calculation
  };

  ///@class ClusterBaseInfo
  ///Contains the fundamental information that allows interacting
  ///with @c ClusterInfoArr so that it is easier to (pre-)transfer it.
  struct ClusterBaseInfo
  {
    ClusterInformationState state;
    bool has_deleted_clusters;
    int number;
    int number_cells;
    
    constexpr static bool has_cells_per_cluster(const ClusterInformationState this_state)
    {
      switch(this_state)
      {
        case ClusterInformationState::Full:
          return true;
        case ClusterInformationState::WithBasicInfo:
          return true;
        case ClusterInformationState::WithMoments:
          return true;
        case ClusterInformationState::WithExtraMoments:
          return true;
        default:
          return false;
      }
    }
    
    constexpr bool has_cells_per_cluster() const
    {
      return has_cells_per_cluster(state);
    }
    
    constexpr bool has_basic_info(const ClusterInformationState this_state) const
    {
      switch(this_state)
      {
        case ClusterInformationState::TagsWithBasicInfo:
          return true;
        case ClusterInformationState::WithBasicInfo:
          return true;
        case ClusterInformationState::WithMoments:
          return true;
        case ClusterInformationState::WithExtraMoments:
          return true;
        default:
          return false;
      }
    }
    
    constexpr bool has_basic_info() const
    {
      return has_basic_info(state);
    }
    
    constexpr static bool has_moments(const ClusterInformationState this_state)
    {
      switch(this_state)
      {
        case ClusterInformationState::WithMoments:
          return true;
        case ClusterInformationState::WithExtraMoments:
          return true;
        default:
          return false;
      }
    }
    
    constexpr bool has_moments() const
    {
      return has_moments(state);
    }
  };

  ///@class ClusterInfoArr
  ///Contains the per-event information of the clusters,
  ///including both the kinematic variables and the moments.
  ///Also includes an easy look-up for the index of the seed cell,
  ///with clusters with seed cells less than zero being invalid;
  ///this is useful for e. g. applying cuts on the GPU without
  ///requiring re-sorting everything.
  ///Cluster assignment depends on the @c ClusterInformationState
  ///as reported by the @c ClusterBaseInfo:
  ///we may have an array of tags
  ///(which assumes `primary_weight = 1 - secondary_weight`)
  ///or an actual jagged vector expressed by the
  ///@c cellsPrefixSum.
  ///In the latter case, we store cell index, cell weight and cluster index
  ///so that it is easy to iterate through all cells inside clusters.
  struct ClusterInfoArr : ClusterBaseInfo
  {
    float clusterEnergy[NMaxClusters];
    float clusterEt[NMaxClusters];
    float clusterEta[NMaxClusters];
    float clusterPhi[NMaxClusters];
    ///Invalid(ated) clusters have seedCellIndex < 0.
    ///This is the index within the collection,
    ///not the hash ID.
    int seedCellIndex[NMaxClusters];
    
    int cellsPrefixSum[NMaxClusters + 1];
    
    union 
    {
      int indices[2 * NCaloCells];
      tag_type tags[NCaloCells];
    } cells;
    
    alignas(tag_type) float cellWeights[2 * NCaloCells];
    
    alignas(tag_type) int clusterIndices[2 * NCaloCells];

    //We align all the moments to 8 bytes
    //so that we can use them for larger
    //temporaries without out-of-bounds access.
    //Given that NMaxClusters = 2^16,
    //this would be nonetheless true,
    //but now it's guaranteed.
    struct ClusterMomentsArr
    {
      alignas(double) float energyPerSample     [NumSamplings][NMaxClusters];
      alignas(double) float maxEPerSample       [NumSamplings][NMaxClusters];
      alignas(double) float maxPhiPerSample[NumSamplings][NMaxClusters];
      alignas(double) float maxEtaPerSample[NumSamplings][NMaxClusters];
      alignas(double) float etaPerSample        [NumSamplings][NMaxClusters];
      alignas(double) float phiPerSample        [NumSamplings][NMaxClusters];
      alignas(double) float time                [NMaxClusters];
      //These are, strictly speaking, not moments,
      //but I think they are best left here rather than
      //in the ClusterInfoArr since they are only filled in
      //during cluster moments calculation...

      alignas(double) float firstPhi            [NMaxClusters];
      alignas(double) float firstEta            [NMaxClusters];
      alignas(double) float secondR             [NMaxClusters];
      alignas(double) float secondLambda        [NMaxClusters];
      alignas(double) float deltaPhi            [NMaxClusters];
      alignas(double) float deltaTheta          [NMaxClusters];
      alignas(double) float deltaAlpha          [NMaxClusters];
      alignas(double) float centerX             [NMaxClusters];
      alignas(double) float centerY             [NMaxClusters];
      alignas(double) float centerZ             [NMaxClusters];
      alignas(double) float centerMag           [NMaxClusters];
      alignas(double) float centerLambda        [NMaxClusters];
      alignas(double) float lateral             [NMaxClusters];
      alignas(double) float longitudinal        [NMaxClusters];
      alignas(double) float engFracEM           [NMaxClusters];
      alignas(double) float engFracMax          [NMaxClusters];
      alignas(double) float engFracCore         [NMaxClusters];
      alignas(double) float firstEngDens        [NMaxClusters];
      alignas(double) float secondEngDens       [NMaxClusters];
      alignas(double) float isolation           [NMaxClusters];
      alignas(double) float engBadCells         [NMaxClusters];
      alignas(double) int   nBadCells           [NMaxClusters];
      alignas(double) int   nBadCellsCorr       [NMaxClusters];
      alignas(double) float badCellsCorrE       [NMaxClusters];
      alignas(double) float badLArQFrac         [NMaxClusters];
      alignas(double) float engPos              [NMaxClusters];
      alignas(double) float significance        [NMaxClusters];
      alignas(double) float cellSignificance    [NMaxClusters];
      alignas(double) int   cellSigSampling     [NMaxClusters];
      alignas(double) float avgLArQ             [NMaxClusters];
      alignas(double) float avgTileQ            [NMaxClusters];
      alignas(double) float engBadHVCells       [NMaxClusters];
      alignas(double) float nBadHVCells         [NMaxClusters];
      alignas(double) float PTD                 [NMaxClusters];
      alignas(double) float mass                [NMaxClusters];
      alignas(double) float EMProbability       [NMaxClusters];
      alignas(double) float hadWeight           [NMaxClusters];
      alignas(double) float OOCweight           [NMaxClusters];
      alignas(double) float DMweight            [NMaxClusters];
      alignas(double) float tileConfidenceLevel [NMaxClusters];
      alignas(double) float secondTime          [NMaxClusters];
      alignas(double) int   nCellSampling       [NumSamplings][NMaxClusters];
      alignas(double) int   nExtraCellSampling  [NMaxClusters];
      alignas(double) float vertexFraction      [NMaxClusters];
      alignas(double) int   nVertexFraction     [NMaxClusters];
      alignas(double) float etaCaloFrame        [NMaxClusters];
      alignas(double) float phiCaloFrame        [NMaxClusters];
      alignas(double) float eta1CaloFrame       [NMaxClusters];
      alignas(double) float phi1CaloFrame       [NMaxClusters];
      alignas(double) float eta2CaloFrame       [NMaxClusters];
      alignas(double) float phi2CaloFrame       [NMaxClusters];
      alignas(double) float engCalibTot         [NMaxClusters];
      alignas(double) float engCalibOutL        [NMaxClusters];
      alignas(double) float engCalibOutM        [NMaxClusters];
      alignas(double) float engCalibOutT        [NMaxClusters];
      alignas(double) float engCalibDeadL       [NMaxClusters];
      alignas(double) float engCalibDeadM       [NMaxClusters];
      alignas(double) float engCalibDeadT       [NMaxClusters];
      alignas(double) float engCalibEMB0        [NMaxClusters];
      alignas(double) float engCalibEME0        [NMaxClusters];
      alignas(double) float engCalibTileG3      [NMaxClusters];
      alignas(double) float engCalibDeadTot     [NMaxClusters];
      alignas(double) float engCalibDeadEMB0    [NMaxClusters];
      alignas(double) float engCalibDeadTile0   [NMaxClusters];
      alignas(double) float engCalibDeadTileG3  [NMaxClusters];
      alignas(double) float engCalibDeadEME0    [NMaxClusters];
      alignas(double) float engCalibDeadHEC0    [NMaxClusters];
      alignas(double) float engCalibDeadFCAL    [NMaxClusters];
      alignas(double) float engCalibDeadLeakage [NMaxClusters];
      alignas(double) float engCalibDeadUnclass [NMaxClusters];
      alignas(double) float engCalibFracEM      [NMaxClusters];
      alignas(double) float engCalibFracHad     [NMaxClusters];
      alignas(double) float engCalibFracRest    [NMaxClusters];
      //And DigiHSTruth ones are reused here if that is the case?
      //Maybe counting from the end if we need to keep both?
    } moments;


//MACRO takes as arguments: the identifier corresponding to the moments,
//a 0/1 number that defines whether or not it's part of the actual moments
//or is another cluster property (e. g. per sample things),
//a 0/1 number that defines whether or not it is a moment that can be easily
//assigned directly using the normal interface, a 0/1 number that
//defines whether this moment is calculated as part of the default set, the token/enum name
//that corresponds to the moment if it can be assigned through the normal interface
//(or NONE if not) and any additional arguments passed to CALORECGPU_FORALLMOMENTS_INSTANTIATE.
//For the latter, due to how macros work (and for portability's sake),
//your macro should expect at least one extra argument (which will be empty
//if no extra arguments are provided), due to the trailing comma...
//WARNING: for maximum flexibility, no semicolons are added or expected!
//Example definition: YOUR_MACRO(VAR_NAME, PROPER_MOMENT, NORMAL_ASSIGN, IS_CALCULATED, MOMENT_NAME, ...)
#define CALORECGPU_FORALLMOMENTS_INSTANTIATE(MACRO, ...) \
  MACRO(energyPerSample     , 0, 0, 1, NONE                   , __VA_ARGS__) \
  MACRO(maxEPerSample       , 0, 0, 1, NONE                   , __VA_ARGS__) \
  MACRO(maxPhiPerSample     , 0, 0, 1, NONE                   , __VA_ARGS__) \
  MACRO(maxEtaPerSample     , 0, 0, 1, NONE                   , __VA_ARGS__) \
  MACRO(etaPerSample        , 0, 0, 1, NONE                   , __VA_ARGS__) \
  MACRO(phiPerSample        , 0, 0, 1, NONE                   , __VA_ARGS__) \
  MACRO(time                , 0, 0, 1, NONE                   , __VA_ARGS__) \
  MACRO(firstPhi            , 1, 1, 1, FIRST_PHI              , __VA_ARGS__) \
  MACRO(firstEta            , 1, 1, 1, FIRST_ETA              , __VA_ARGS__) \
  MACRO(secondR             , 1, 1, 1, SECOND_R               , __VA_ARGS__) \
  MACRO(secondLambda        , 1, 1, 1, SECOND_LAMBDA          , __VA_ARGS__) \
  MACRO(deltaPhi            , 1, 1, 1, DELTA_PHI              , __VA_ARGS__) \
  MACRO(deltaTheta          , 1, 1, 1, DELTA_THETA            , __VA_ARGS__) \
  MACRO(deltaAlpha          , 1, 1, 1, DELTA_ALPHA            , __VA_ARGS__) \
  MACRO(centerX             , 1, 1, 1, CENTER_X               , __VA_ARGS__) \
  MACRO(centerY             , 1, 1, 1, CENTER_Y               , __VA_ARGS__) \
  MACRO(centerZ             , 1, 1, 1, CENTER_Z               , __VA_ARGS__) \
  MACRO(centerMag           , 1, 1, 1, CENTER_MAG             , __VA_ARGS__) \
  MACRO(centerLambda        , 1, 1, 1, CENTER_LAMBDA          , __VA_ARGS__) \
  MACRO(lateral             , 1, 1, 1, LATERAL                , __VA_ARGS__) \
  MACRO(longitudinal        , 1, 1, 1, LONGITUDINAL           , __VA_ARGS__) \
  MACRO(engFracEM           , 1, 1, 1, ENG_FRAC_EM            , __VA_ARGS__) \
  MACRO(engFracMax          , 1, 1, 1, ENG_FRAC_MAX           , __VA_ARGS__) \
  MACRO(engFracCore         , 1, 1, 1, ENG_FRAC_CORE          , __VA_ARGS__) \
  MACRO(firstEngDens        , 1, 1, 1, FIRST_ENG_DENS         , __VA_ARGS__) \
  MACRO(secondEngDens       , 1, 1, 1, SECOND_ENG_DENS        , __VA_ARGS__) \
  MACRO(isolation           , 1, 1, 1, ISOLATION              , __VA_ARGS__) \
  MACRO(engBadCells         , 1, 1, 1, ENG_BAD_CELLS          , __VA_ARGS__) \
  MACRO(nBadCells           , 1, 1, 1, N_BAD_CELLS            , __VA_ARGS__) \
  MACRO(nBadCellsCorr       , 1, 1, 1, N_BAD_CELLS_CORR       , __VA_ARGS__) \
  MACRO(badCellsCorrE       , 1, 1, 1, BAD_CELLS_CORR_E       , __VA_ARGS__) \
  MACRO(badLArQFrac         , 1, 1, 1, BADLARQ_FRAC           , __VA_ARGS__) \
  MACRO(engPos              , 1, 1, 1, ENG_POS                , __VA_ARGS__) \
  MACRO(significance        , 1, 1, 1, SIGNIFICANCE           , __VA_ARGS__) \
  MACRO(cellSignificance    , 1, 1, 1, CELL_SIGNIFICANCE      , __VA_ARGS__) \
  MACRO(cellSigSampling     , 1, 1, 1, CELL_SIG_SAMPLING      , __VA_ARGS__) \
  MACRO(avgLArQ             , 1, 1, 1, AVG_LAR_Q              , __VA_ARGS__) \
  MACRO(avgTileQ            , 1, 1, 1, AVG_TILE_Q             , __VA_ARGS__) \
  MACRO(engBadHVCells       , 1, 1, 0, ENG_BAD_HV_CELLS       , __VA_ARGS__) \
  MACRO(nBadHVCells         , 1, 1, 0, N_BAD_HV_CELLS         , __VA_ARGS__) \
  MACRO(PTD                 , 1, 1, 1, PTD                    , __VA_ARGS__) \
  MACRO(mass                , 1, 1, 1, MASS                   , __VA_ARGS__) \
  MACRO(EMProbability       , 1, 1, 1, EM_PROBABILITY         , __VA_ARGS__) \
  MACRO(hadWeight           , 1, 1, 1, HAD_WEIGHT             , __VA_ARGS__) \
  MACRO(OOCweight           , 1, 1, 1, OOC_WEIGHT             , __VA_ARGS__) \
  MACRO(DMweight            , 1, 1, 1, DM_WEIGHT              , __VA_ARGS__) \
  MACRO(tileConfidenceLevel , 1, 1, 1, TILE_CONFIDENCE_LEVEL  , __VA_ARGS__) \
  MACRO(secondTime          , 1, 0, 1, SECOND_TIME            , __VA_ARGS__) \
  MACRO(nCellSampling       , 1, 0, 1, NCELL_SAMPLING         , __VA_ARGS__) \
  MACRO(nExtraCellSampling  , 0, 0, 1, NCELL_SAMPLING         , __VA_ARGS__) \
  MACRO(vertexFraction      , 1, 1, 0, VERTEX_FRACTION        , __VA_ARGS__) \
  MACRO(nVertexFraction     , 1, 1, 0, NVERTEX_FRACTION       , __VA_ARGS__) \
  MACRO(etaCaloFrame        , 1, 1, 0, ETACALOFRAME           , __VA_ARGS__) \
  MACRO(phiCaloFrame        , 1, 1, 0, PHICALOFRAME           , __VA_ARGS__) \
  MACRO(eta1CaloFrame       , 1, 1, 0, ETA1CALOFRAME          , __VA_ARGS__) \
  MACRO(phi1CaloFrame       , 1, 1, 0, PHI1CALOFRAME          , __VA_ARGS__) \
  MACRO(eta2CaloFrame       , 1, 1, 0, ETA2CALOFRAME          , __VA_ARGS__) \
  MACRO(phi2CaloFrame       , 1, 1, 0, PHI2CALOFRAME          , __VA_ARGS__) \
  MACRO(engCalibTot         , 1, 1, 0, ENG_CALIB_TOT          , __VA_ARGS__) \
  MACRO(engCalibOutL        , 1, 1, 0, ENG_CALIB_OUT_L        , __VA_ARGS__) \
  MACRO(engCalibOutM        , 1, 1, 0, ENG_CALIB_OUT_M        , __VA_ARGS__) \
  MACRO(engCalibOutT        , 1, 1, 0, ENG_CALIB_OUT_T        , __VA_ARGS__) \
  MACRO(engCalibDeadL       , 1, 1, 0, ENG_CALIB_DEAD_L       , __VA_ARGS__) \
  MACRO(engCalibDeadM       , 1, 1, 0, ENG_CALIB_DEAD_M       , __VA_ARGS__) \
  MACRO(engCalibDeadT       , 1, 1, 0, ENG_CALIB_DEAD_T       , __VA_ARGS__) \
  MACRO(engCalibEMB0        , 1, 1, 0, ENG_CALIB_EMB0         , __VA_ARGS__) \
  MACRO(engCalibEME0        , 1, 1, 0, ENG_CALIB_EME0         , __VA_ARGS__) \
  MACRO(engCalibTileG3      , 1, 1, 0, ENG_CALIB_TILEG3       , __VA_ARGS__) \
  MACRO(engCalibDeadTot     , 1, 1, 0, ENG_CALIB_DEAD_TOT     , __VA_ARGS__) \
  MACRO(engCalibDeadEMB0    , 1, 1, 0, ENG_CALIB_DEAD_EMB0    , __VA_ARGS__) \
  MACRO(engCalibDeadTile0   , 1, 1, 0, ENG_CALIB_DEAD_TILE0   , __VA_ARGS__) \
  MACRO(engCalibDeadTileG3  , 1, 1, 0, ENG_CALIB_DEAD_TILEG3  , __VA_ARGS__) \
  MACRO(engCalibDeadEME0    , 1, 1, 0, ENG_CALIB_DEAD_EME0    , __VA_ARGS__) \
  MACRO(engCalibDeadHEC0    , 1, 1, 0, ENG_CALIB_DEAD_HEC0    , __VA_ARGS__) \
  MACRO(engCalibDeadFCAL    , 1, 1, 0, ENG_CALIB_DEAD_FCAL    , __VA_ARGS__) \
  MACRO(engCalibDeadLeakage , 1, 1, 0, ENG_CALIB_DEAD_LEAKAGE , __VA_ARGS__) \
  MACRO(engCalibDeadUnclass , 1, 1, 0, ENG_CALIB_DEAD_UNCLASS , __VA_ARGS__) \
  MACRO(engCalibFracEM      , 1, 1, 0, ENG_CALIB_FRAC_EM      , __VA_ARGS__) \
  MACRO(engCalibFracHad     , 1, 1, 0, ENG_CALIB_FRAC_HAD     , __VA_ARGS__) \
  MACRO(engCalibFracRest    , 1, 1, 0, ENG_CALIB_FRAC_REST    , __VA_ARGS__)

//Note: SECOND_TIME is weird


#define CALORECGPU_FORALLMOMENTS_HELPER(MOMENTNAME, ...) \
  std::forward<F>(f)(this->moments. MOMENTNAME, std::forward<Args>(args)...);

    ///F receives the array associated with the moment
    ///and any additional arguments.
    ///Given that the moments are of different types,
    ///it must be a functor or lambda ready to deal with that!
    template <class F, class ... Args>
    constexpr void for_all_moments(F && f, Args && ... args) const
    {
      CALORECGPU_FORALLMOMENTS_INSTANTIATE(CALORECGPU_FORALLMOMENTS_HELPER)
    }

    ///F receives the array associated with the moment
    ///and any additional arguments.
    ///Given that the moments are of different types,
    ///it must be a functor or lambda ready to deal with that!
    template <class F, class ... Args>
    constexpr void for_all_moments(F && f, Args && ... args)
    {
      CALORECGPU_FORALLMOMENTS_INSTANTIATE(CALORECGPU_FORALLMOMENTS_HELPER)
    }
    
    ///Extra representation for the cell tags,
    ///to save time for calculating the isolation
    ///moment after building the list of cells per cluster.
    ///Can (and should) also be used if any other processing
    ///(before calculating the relevant moments...)
    ///needs some sort of cell -> cluster mapping.
    constexpr const tag_type & get_extra_cell_info(const int idx) const
    {
      constexpr auto max_size = NMaxClusters * sizeof(float) / sizeof(tag_type);
      
      const int outer_idx = idx / max_size;
      const int inner_idx = idx % max_size;

      using PtrType = const tag_type *;
      
      auto get_laundered_pointer = [&](auto * ptr) -> PtrType
      {
        return std::launder(static_cast<PtrType>(static_cast<const void *>(ptr)));
      };

      PtrType arr[6] = { get_laundered_pointer(this->moments.engCalibTot),
                         get_laundered_pointer(this->moments.engCalibOutL),
                         get_laundered_pointer(this->moments.engCalibOutM),
                         get_laundered_pointer(this->moments.engCalibOutT),
                         get_laundered_pointer(this->moments.engCalibDeadL),
                         get_laundered_pointer(this->moments.engCalibDeadM)  };

      return *(arr[outer_idx] + inner_idx);
    }
    
    constexpr tag_type & get_extra_cell_info(const int idx)
    {
      constexpr auto max_size = NMaxClusters * sizeof(float) / sizeof(tag_type);
      
      const int outer_idx = idx / max_size;
      const int inner_idx = idx % max_size;
      
      using PtrType = tag_type *;
      
      auto get_laundered_pointer = [&](auto * ptr) -> PtrType
      {
        return std::launder(static_cast<PtrType>(static_cast<void *>(ptr)));
      };

      PtrType arr[6] = { get_laundered_pointer(this->moments.engCalibTot),
                         get_laundered_pointer(this->moments.engCalibOutL),
                         get_laundered_pointer(this->moments.engCalibOutM),
                         get_laundered_pointer(this->moments.engCalibOutT),
                         get_laundered_pointer(this->moments.engCalibDeadL),
                         get_laundered_pointer(this->moments.engCalibDeadM)  };

      return *(arr[outer_idx] + inner_idx);
    }

    template <class NewT = tag_type>
    constexpr const NewT * secondary_tag_array() const
    {
      return std::launder(static_cast<const NewT *>(static_cast<const void *>(this->cellWeights)));
    }
    template <class NewT = tag_type>
    constexpr NewT * secondary_tag_array()
    {
      return std::launder(static_cast<NewT *>(static_cast<void *>(this->cellWeights)));
    }
    template <class NewT = tag_type>
    constexpr const NewT & secondary_tag_array(const int idx) const
    {
      return this->template secondary_tag_array<NewT>()[idx];
    }
    template <class NewT = tag_type>
    constexpr NewT & secondary_tag_array(const int idx)
    {
      return this->template secondary_tag_array<NewT>()[idx];
    }

    template <class NewT = tag_type>
    constexpr const NewT * tertiary_tag_array() const
    {
      return std::launder(static_cast<const NewT *>(static_cast<const void *>(this->clusterIndices)));
    }
    template <class NewT = tag_type>
    constexpr NewT * tertiary_tag_array()
    {
      return std::launder(static_cast<NewT *>(static_cast<void *>(this->clusterIndices)));
    }
    template <class NewT = tag_type>
    constexpr const NewT & tertiary_tag_array(const int idx) const
    {
      return this->template tertiary_tag_array<NewT>()[idx];
    }
    template <class NewT = tag_type>
    constexpr NewT & tertiary_tag_array(const int idx)
    {
      return this->template tertiary_tag_array<NewT>()[idx];
    }
    

  };

}

#endif
