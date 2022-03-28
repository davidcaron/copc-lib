#ifndef COPCLIB_PARTS_READER_H_
#define COPCLIB_PARTS_READER_H_

#include <istream>
#include <limits>
#include <map>
#include <string>

#include "copc-lib/copc/config.hpp"
#include "copc-lib/hierarchy/key.hpp"
#include "copc-lib/io/base_io.hpp"
#include "copc-lib/las/points.hpp"
#include "copc-lib/las/vlr.hpp"

namespace copc
{

class PartsReader : public BaseIO
{
  public:
    PartsReader() = default;

    void ReadHeader(std::string &header_data);

    void InitCopcConfig(std::string &vlr_data, std::string &evlr_data);

    las::Points GetPoints(std::string &data, uint64_t point_count);

    // Return all children of a page with a given key
    // (or the node itself, if it exists, if there isn't a page with that key)
    std::vector<Node> GetAllChildrenOfPage(const VoxelKey &key);
    // Helper function to get all nodes from the root
    std::vector<Node> GetAllNodes() { return GetAllChildrenOfPage(VoxelKey::RootKey()); }

    uint64_t point_offset{0};
    uint64_t evlr_offset{0};

    copc::CopcConfig CopcConfig() { return config_; }

  protected:
    std::string evlr_data_;

    copc::CopcConfig config_;

    header14 lazperf_header_;

    std::shared_ptr<copc::CopcInfo> copc_info_;

    std::map<uint64_t, las::VlrHeader> vlrs_;

    las::WktVlr ReadWktVlr(std::map<uint64_t, las::VlrHeader> &vlrs, std::istringstream &evlr_stream);

    las::EbVlr ReadExtraBytesVlr(std::map<uint64_t, las::VlrHeader> &vlrs, std::istringstream &vlr_stream);

    static uint64_t FetchVlr(const std::map<uint64_t, las::VlrHeader> &vlrs, const std::string &user_id,
                             uint16_t record_id);

    std::vector<Entry> ReadPage(std::shared_ptr<Internal::PageInternal> page) override;
};

} // namespace copc
#endif // COPCLIB_PARTS_READER_H_
