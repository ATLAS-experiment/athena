#include <CrestApi/picosha2.h>
#include <iostream>
#include <CrestApi/CrestApiBase.h>

namespace Crest
{

  std::string CrestApiBase::getHashForFile(const std::string &file)
  {
    std::ifstream ifs(file);
    if (!ifs)
    {
      throw CrestException(
          "ERROR in CrestFsClient::getHashForFile cannot open file \"" + file + "\".");
    }

    picosha2::hash256_one_by_one hasher;
    std::vector<char> buffer(1024 * 1024); // use 1M memory
    while (ifs.read(buffer.data(), static_cast<std::streamsize>(buffer.size())))
    {
      std::cout << "#" << std::flush;
      hasher.process(buffer.begin(), buffer.end());
    }
    // process remains
    hasher.process(buffer.begin(), buffer.begin() + static_cast<int>(ifs.gcount()));
    hasher.finish();

    std::string hash = picosha2::get_hash_hex_string(hasher);
    return hash;
  }

  std::string CrestApiBase::getHash(std::string_view str)
  {
    std::string hash_hex_str = picosha2::hash256_hex_string(str.begin(), str.end());
    return hash_hex_str;
  }

  const std::string& CrestApiBase::getClientVersion()
  {
    return s_CREST_CLIENT_VERSION;
  }
  
} // namespace Crest
