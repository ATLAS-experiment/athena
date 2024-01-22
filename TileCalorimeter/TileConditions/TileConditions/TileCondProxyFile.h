/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TILECONDITIONS_TILECONDPROXYFILE_H
#define TILECONDITIONS_TILECONDPROXYFILE_H


// Tile includes
#include "TileConditions/ITileCondProxy.h"

// Athena includes
#include "AthenaBaseComps/AthAlgTool.h"



#include <string>
#include <vector>
#include <tuple>
#include <memory>

/**
 * @class TileCondProxyFile
 * @brief The tool to get Tile conditions data from file
 *
 */
template<typename T>
class TileCondProxyFile: public AthAlgTool, virtual public ITileCondProxy<T> {
  public:

    TileCondProxyFile(const std::string& type, const std::string& name, const IInterface* parent);
    virtual ~TileCondProxyFile() {};

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

   /**
    * @brief Fill TileCalibData with Tile conditions data from file
    * @param[out] calibData TileCalibData to fill with Tile conditions data
    * @param[out] eventRange EventIDRange during wich Tile conditions data are valid
    */
    virtual StatusCode fillCalibData(TileCalibData<T>& calibData, EventIDRange& eventRange) const override;

  private:

    typedef typename T::DefType::value_type DataVec;
    typedef std::tuple<unsigned int, unsigned int, unsigned int> DataKey;
    typedef std::map<DataKey, DataVec> DataMap;

    /** Creates a calibDrawer of type T */
    std::unique_ptr<const T> createCalibDrawer(unsigned int drawerIdx, unsigned int nChannels
                                               , unsigned int objVers, const DataMap& dataMap) const;

    /** Source = COOL folder */
    std::string m_source;

};

#endif // TILECONDITIONS_TILECONDPROXYFILE_H
