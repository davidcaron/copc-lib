#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include "copc-lib/copc/config.hpp"
#include "copc-lib/hierarchy/internal/hierarchy.hpp"
#include "copc-lib/io/reader.hpp"
#include "copc-lib/las/header.hpp"
#include "copc-lib/laz/decompressor.hpp"

#include "parts_reader.hpp"

#include <lazperf/header.hpp>
#include <lazperf/vlr.hpp>

const int LAS_HEADER14_LENGTH = 375;
const int COPC_VLR_LENGTH = 160;

namespace copc
{

void PartsReader::ReadHeader(std::string &header_with_copc_vlr)
{
    const int header_length = LAS_HEADER14_LENGTH + COPC_VLR_LENGTH;
    if (header_with_copc_vlr.length() != header_length)
        throw std::runtime_error("PartsReader::ReadHeader: header_with_copc_vlr must be " +
                                 std::to_string(header_length) + " bytes.");
    std::istringstream stream(header_with_copc_vlr);
    lazperf_header_ = header14::create(stream);
    // fix point_format_id when compressed, see lazperf's validateHeader
    lazperf_header_.point_format_id &= 0x3f;

    // header_ = las::LasHeader::FromLazPerf(lazperf_header_);

    point_offset = lazperf_header_.point_offset;
    evlr_offset = lazperf_header_.evlr_offset;
}

void PartsReader::InitCopcConfig(std::string &vlr_data, std::string &evlr_data)
{
    evlr_data_ = evlr_data;
    std::istringstream vlr_stream(vlr_data);

    vlr_stream.seekg(0);

    vlrs_.clear();

    // Iterate through all vlr's and add them to the `vlrs` list
    for (int i = 0; i < lazperf_header_.vlr_count; i++)
    {
        uint64_t cur_pos = vlr_stream.tellg();
        auto h = las::VlrHeader(lazperf::vlr_header::create(vlr_stream));
        vlrs_.insert({cur_pos, h});

        vlr_stream.seekg(h.data_length, std::ios::cur); // jump foward
    }

    std::istringstream evlr_stream(evlr_data);

    // Iterate through all evlr's and add them to the `vlrs` list
    for (int i = 0; i < lazperf_header_.evlr_count; i++)
    {
        uint64_t cur_pos = evlr_stream.tellg();
        auto h = las::VlrHeader(lazperf::evlr_header::create(evlr_stream));
        vlrs_.insert({cur_pos, h});

        evlr_stream.seekg(h.data_length, std::ios::cur); // jump foward
    }

    vlr_stream.seekg(COPC_OFFSET - LAS_HEADER14_LENGTH);
    auto copc_info = lazperf::copc_info_vlr::create(vlr_stream);

    vlr_stream.seekg(0);
    evlr_stream.seekg(0);
    auto wkt = ReadWktVlr(vlrs_, evlr_stream);
    auto eb = ReadExtraBytesVlr(vlrs_, vlr_stream);

    auto header = las::LasHeader::FromLazPerf(lazperf_header_);

    config_ = copc::CopcConfig(header, copc_info, wkt.wkt, eb);

    hierarchy_ = std::make_shared<Internal::Hierarchy>(copc_info.root_hier_offset, copc_info.root_hier_size);
}

las::WktVlr PartsReader::ReadWktVlr(std::map<uint64_t, las::VlrHeader> &vlrs, std::istringstream &evlr_stream)
{
    auto offset = FetchVlr(vlrs, "LASF_Projection", 2112);
    if (offset != 0)
    {
        evlr_stream.seekg(offset + lazperf::evlr_header::Size);
        return las::WktVlr::create(evlr_stream, vlrs[offset].data_length);
    }
    return las::WktVlr();
}

las::EbVlr PartsReader::ReadExtraBytesVlr(std::map<uint64_t, las::VlrHeader> &vlrs, std::istringstream &vlr_stream)
{
    auto offset = FetchVlr(vlrs, "LASF_Spec", 4);
    if (offset != 0)
    {
        vlr_stream.seekg(offset + lazperf::vlr_header::Size);
        return las::EbVlr::create(vlr_stream, vlrs[offset].data_length);
    }
    return las::EbVlr();
}

uint64_t PartsReader::FetchVlr(const std::map<uint64_t, las::VlrHeader> &vlrs, const std::string &user_id,
                               uint16_t record_id)
{
    for (auto &[offset, vlr_header] : vlrs)
    {
        if (vlr_header.user_id == user_id && vlr_header.record_id == record_id)
        {
            return offset;
        }
    }
    return 0;
}

las::Points PartsReader::GetPoints(std::string &data, uint64_t point_count)
{
    std::istringstream stream(data);

    auto las_header = config_.LasHeader();
    std::vector<char> point_data = laz::Decompressor::DecompressBytes(stream, las_header, point_count);

    return las::Points::Unpack(point_data, config_.LasHeader());
}

std::vector<Node> PartsReader::GetAllChildrenOfPage(const VoxelKey &key)
{
    std::vector<Node> out;
    if (!key.IsValid())
        return out;

    // Load all pages upto the current key
    auto node = FindNode(key);
    // If a page with this key doesn't exist, check if the node itself exists and return it
    if (!hierarchy_->PageExists(key))
    {
        if (node.IsValid())
            out.push_back(node);
        return out;
    }

    // If the page does exist, we need to read all its children and subpages into memory recursively
    LoadPageHierarchy(hierarchy_->seen_pages_[key], out);
    return out;
}

std::vector<Entry> PartsReader::ReadPage(std::shared_ptr<Internal::PageInternal> page)
{
    std::vector<Entry> out;
    if (!page->IsValid())
        throw std::runtime_error("Reader::ReadPage: Cannot load an invalid page.");

    std::istringstream evlr_stream(evlr_data_);

    // reset the stream to the page's offset
    evlr_stream.seekg(page->offset - evlr_offset);

    // Iterate through each Entry in the page
    int num_entries = int(page->byte_size / Entry::ENTRY_SIZE);
    for (int i = 0; i < num_entries; i++)
    {
        Entry e = Entry::Unpack(evlr_stream);
        if (!e.IsValid())
            throw std::runtime_error("Entry is invalid! " + e.ToString());

        out.push_back(e);
    }

    page->loaded = true;
    return out;
}

} // namespace copc
