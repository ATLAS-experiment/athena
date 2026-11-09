/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#ifndef TILERECUTILS_TILECELLNOISEFILTER_H
#define TILERECUTILS_TILECELLNOISEFILTER_H

// Tile includes
#include "TileConditions/ITileBadChanTool.h"
#include "TileConditions/TileEMScale.h"
#include "TileConditions/TileSampleNoise.h"

// Calo includes
#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloInterface/ICaloCellMakerTool.h"
#include "CaloConditions/CaloNoise.h"

// Atlas includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"


// forward declarations
class TileID;
class TileHWID;
class TileCell;
class CaloCellContainer;

/**
 @class TileCellNoiseFilter
 @brief This tool subtracts common-mode noise from all TileCells
 */
class TileCellNoiseFilter: public extends<AthAlgTool, ICaloCellMakerTool> {

  public:

    /** AlgTool like constructor */
    TileCellNoiseFilter(const std::string& type, const std::string& name,
        const IInterface* parent);

    /** Virtual destructor */
    virtual ~TileCellNoiseFilter() {};

    /** AlgTool InterfaceID */
    static const InterfaceID& interfaceID();

    /** AlgTool initialize method.*/
    StatusCode initialize() override;
    /** AlgTool finalize method */
    StatusCode finalize() override;

    /** proceed the coherent noise subtraction algorithm and correct Tile cell energies */
    virtual StatusCode process (CaloCellContainer* cellcoll,
                                const EventContext& ctx) const override;

  private:

    static const int s_maxPartition = 4; // LBA,LBC,EBA,EBC
    static const int s_maxDrawer = 64;   // # of drawers per partition
    static const int s_maxMOB = 4;       // # of motherboards per drawer
    static const int s_maxChannel = 12;  // # of channels per motherboard

    typedef float cmdata_t[s_maxPartition][s_maxDrawer][s_maxMOB];

    // set common-mode subtructed energy
    void setCMSEnergy(const TileEMScale* emScale, const cmdata_t& commonMode, TileCell *cell) const;

    // calculate common-mode for all the motherboards
    int calcCM(const CaloNoise* caloNoise, const TileSampleNoise* sampleNoise, const TileEMScale* emScale,
               const CaloCellContainer *cellcoll, cmdata_t& commonMode) const;

    // derive a value of common-mode shift
    float getCMShift(const cmdata_t& commonMode,
                     int partition, int drawer, int channel) const
    {
      return commonMode[partition][drawer][channel / s_maxChannel];
    }

    const TileID* m_tileID{nullptr};   //!< Pointer to TileID
    const TileHWID* m_tileHWID{nullptr}; //!< Pointer to TileHWID

   /**
    * @brief Name of TileEMScale in condition store
    */
    SG::ReadCondHandleKey<TileEMScale> m_emScaleKey{this,
        "TileEMScale", "TileEMScale", "Input Tile EMS calibration constants"};

    /**
     * @brief Name of TileSampleNoise in condition store
     */
    SG::ReadCondHandleKey<TileSampleNoise> m_sampleNoiseKey{this,
        "TileSampleNoise", "TileSampleNoise", "Input Tile sample noise"};

    SG::ReadCondHandleKey<CaloNoise> m_caloNoiseKey{this, "CaloNoise",
                                                    "",
                                                    "CaloNoise object to read, or null to use the DB directly"};

    ToolHandle<ITileBadChanTool> m_tileBadChanTool{this,
        "TileBadChanTool", "TileBadChanTool", "Tile bad channel tool"};

    // properties
    Gaudi::Property<float> m_truncationThresholdOnAbsEinSigma{this,
        "TruncationThresholdOnAbsEinSigma", 4.0, "Truncation threshold on |E| in units of noise sigma"}; // 4 sigma of ADC HF noise by default
    Gaudi::Property<float> m_minimumNumberOfTruncatedChannels{this,
        "MinimumNumberOfTruncatedChannels", 0.6, "Minimum number (or fraction if < 1) of truncated channels"}; // at least 60% of channels should be below threshold
    Gaudi::Property<bool> m_useTwoGaussNoise{this,
        "UseTwoGaussNoise", false, "Use two-gaussian noise"}; // do not use 2G - has no sense for ADC HF noise for the moment

    static const CaloCell_ID::SUBCALO s_caloIndex = CaloCell_ID::TILE;

    Gaudi::Property<float> m_maxNoiseSigma{this, "MaxNoiseSigma", 5.0,
        "Channels with noise more than that value are igonred in calculation of correction"};

};

#endif // TILERECUTILS_TILECELLNOISEFILTER_H
