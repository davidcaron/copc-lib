#ifndef COPCLIB_LAS_VLR_H_
#define COPCLIB_LAS_VLR_H_

#include <cstring>
#include <stdexcept>
#include <vector>

#include <lazperf/vlr.hpp>

namespace copc::las
{

using WktVlr = lazperf::wkt_vlr;
using EbVlr = lazperf::eb_vlr;
using CopcExtentsVlr = lazperf::copc_extents_vlr;

int NumBytesFromExtraBytes(const std::vector<EbVlr::ebfield> &items);

class VlrHeader : public lazperf::evlr_header
{
  public:
    bool evlr_flag{false};

    VlrHeader() = default;
    VlrHeader(const lazperf::evlr_header &evlr_header) : evlr_flag(true), lazperf::evlr_header(evlr_header){};
    VlrHeader(const lazperf::vlr_header &vlr_header);
    VlrHeader(const VlrHeader &vlr_header);

    lazperf::vlr_header ToLazperfVlrHeader() const;
    lazperf::evlr_header ToLazperfEvlrHeader() const;
};

} // namespace copc::las

namespace lazperf
{
// Equality operations
inline bool operator==(const eb_vlr::ebfield &a, const eb_vlr::ebfield &b)
{
    return std::memcmp(a.reserved, b.reserved, 2) == 0 && a.data_type == b.data_type && a.options == b.options &&
           a.name == b.name && std::memcmp(a.no_data, b.no_data, 3) == 0 && std::memcmp(a.minval, b.minval, 3) == 0 &&
           std::memcmp(a.maxval, b.maxval, 3) == 0 && std::memcmp(a.scale, b.scale, 3) == 0 &&
           std::memcmp(a.offset, b.offset, 3) == 0 && a.description == b.description;
}
} // namespace lazperf

#endif // COPCLIB_LAS_VLR_H_
