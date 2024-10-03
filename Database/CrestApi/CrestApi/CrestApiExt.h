/*
   Copyright (C) 2019-2024 CERN for the benefit of the ATLAS collaboration
 */


/*!
   \file
   \brief Header file for CREST C++ Client Library (CrestClientExt)

   This file contains the CrestClientExt class.
   This is a CrestClient class version with additional methods based on
   Boost Parameter Library.
 */

#include <CrestApi/CrestApi.h>
#include <boost/parameter/name.hpp>
#include <boost/parameter/preprocessor.hpp>


BOOST_PARAMETER_NAME(page)
BOOST_PARAMETER_NAME(size)
BOOST_PARAMETER_NAME(tagname)
BOOST_PARAMETER_NAME(sort)
BOOST_PARAMETER_NAME(dateformat)
BOOST_PARAMETER_NAME(name)

namespace Crest {
  class CrestClientExt: public CrestClient {
  public:
    CrestClientExt(const std::string& _host, const std::string& _port, bool _check_version = s_CREST_CLIENT_CHECK) : CrestClient(_host, _port, _check_version) {}
    CrestClientExt(const std::string& url, bool check_version = s_CREST_CLIENT_CHECK) : CrestClient(url, check_version) {}

// ~CrestClientExt();

/**
 * This method returns the global tag list. It has boost parameters, all of them are optional.
 * @param _name - global tag name pattern, optional parameter,
 * @param _size - page size, a number of tags per a page, optional parameter, default value is 1000,
 * @param _page - page number optional parameter, default value is 0,
 * @param _sort - sorting order, default value is "name:ASC".
 * If you dont use the optional parameter names, you have to use the same parameter order.
 * Example: <br>
 * <pre>
 *   nlohmann::json list1 = myCrestClient.listGlobalTagsParams();
 *   nlohmann::json list2 = myCrestClient.listGlobalTagsParams("",3,5);
 *   nlohmann::json list2 = myCrestClient.listGlobalTagsParams(_page=3,_size=5);
 * </pre>
 *
 *  nlohmann::json listGlobalTagsParams(std::string _name, int _size, int _page);
 */
    BOOST_PARAMETER_MEMBER_FUNCTION(
      (GlobalTagSetDto),
      listGlobalTagsParams,
      tag,
      (required
      )
      (optional
         (name, (std::string), "")
         (size, (int), 1000)
         (page, (int), 0)
         (sort, (std::string), "name:ASC")
      )
      ) {
      return listGlobalTags(name, size, page, sort);
    }

/**
 * This method returns the tag list. It has boost parameters, all of them are optional.
 * (This method is an analogue of the list_tags method in Python)
 * @param _name - tag name pattern, optional parameter,
 * @param _size - page size, a number of tags per a page, optional parameter, default value is 1000,
 * @param _page - page number optional parameter, default value is 0,
 * @param _sort - sorting order, default value is "name:ASC".
 * If you dont use the optional parameter names, you have to use the same parameter order.
 * Example: <br>
 * <pre>
 *   nlohmann::json list1 = myCrestClient.listTagsParams();
 *   nlohmann::json list2 = myCrestClient.listTagsParams("",3,5);
 *   nlohmann::json list2 = myCrestClient.listTagsParams(_page=3,_size=5);
 * </pre>
 *
 *  nlohmann::json listTagsParams(std::string _name, int _size, int _page);
 */
    BOOST_PARAMETER_MEMBER_FUNCTION(
      (TagSetDto),
      listTagsParams,
      tag,
      (required
      )
      (optional
         (name, (std::string), "")
         (size, (int), 1000)
         (page, (int), 0)
         (sort, (std::string), "name:ASC")
      )
      ) {
      return listTags(name, size, page, sort);
    }

  }; // CrestClientExt (end)

} // namespace Crest
