#include <catch2/catch.hpp>
#include <copc-lib/copc/config.hpp>
#include <copc-lib/geometry/vector3.hpp>
#include <copc-lib/las/header.hpp>
#include <copc-lib/las/utils.hpp>
#include <copc-lib/las/vlr.hpp>
#include <string>

using namespace copc;

TEST_CASE("CopcConfig", "[CopcConfig]")
{

    int8_t point_format_id = 7;
    int num_eb_items = 1;
    double test_spacing = 12;
    double test_intensity_min = 5;
    double test_intensity_max = 155;
    Vector3 test_scale(1, 1, 1);
    Vector3 test_offset(50, 50, 50);
    std::string test_wkt = "test_wkt";

    las::EbVlr test_extra_bytes_vlr(num_eb_items);
    test_extra_bytes_vlr.items[0].data_type = 0;
    test_extra_bytes_vlr.items[0].options = 4;
    test_extra_bytes_vlr.items[0].name = "eb1";

    CopcInfo copc_info;
    copc_info.spacing = test_spacing;
    std::string wkt(test_wkt);

    las::LasHeader header(point_format_id, las::PointBaseByteSize(point_format_id) + test_extra_bytes_vlr.size(),
                          test_scale, test_offset);

    CopcConfig cfg(header, copc_info, wkt, test_extra_bytes_vlr);

    REQUIRE(cfg.LasHeader().PointFormatId() == point_format_id);
    REQUIRE(cfg.LasHeader().Scale() == test_scale);
    REQUIRE(cfg.LasHeader().Offset() == test_offset);

    REQUIRE(cfg.CopcInfo().spacing == test_spacing);

    REQUIRE(cfg.Wkt() == test_wkt);

    REQUIRE(cfg.ExtraBytesVlr().items[0].name == test_extra_bytes_vlr.items[0].name);
}

TEST_CASE("CopcConfigWriter", "[CopcConfigWriter]")
{
    int8_t point_format_id = 7;
    int num_eb_items = 1;
    double test_spacing = 12;
    double test_intensity_min = 5;
    double test_intensity_max = 155;
    Vector3 test_scale(1, 1, 1);
    Vector3 test_offset(50, 50, 50);
    Vector3 test_min(-5, -6, -7);
    Vector3 test_max(5, 6, 7);
    std::string test_wkt = "test_wkt";

    las::EbVlr test_extra_bytes_vlr(num_eb_items);
    test_extra_bytes_vlr.items[0].data_type = 0;
    test_extra_bytes_vlr.items[0].options = 4;
    test_extra_bytes_vlr.items[0].name = "eb1";

    SECTION("Constructor with default arguments")
    {
        CopcConfigWriter cfg(point_format_id);

        REQUIRE(cfg.LasHeader()->PointFormatId() == point_format_id);
        REQUIRE(cfg.LasHeader()->Scale() == Vector3::DefaultScale());

        REQUIRE(cfg.CopcInfo()->spacing == 0);

        REQUIRE(cfg.Wkt().empty());

        REQUIRE(cfg.ExtraBytesVlr().items.empty());
    }

    SECTION("Constructor with specified arguments")
    {
        CopcConfigWriter cfg(point_format_id, test_scale, test_offset, test_wkt, test_extra_bytes_vlr);

        REQUIRE(cfg.LasHeader()->PointFormatId() == point_format_id);
        REQUIRE(cfg.LasHeader()->Scale() == test_scale);
        REQUIRE(cfg.LasHeader()->Offset() == test_offset);

        REQUIRE(cfg.CopcInfo()->spacing == 0);

        REQUIRE(cfg.Wkt() == test_wkt);

        REQUIRE(cfg.ExtraBytesVlr().items[0].name == test_extra_bytes_vlr.items[0].name);
    }

    SECTION("Copy constructor from CopcConfig")
    {
        CopcInfo copc_info;
        copc_info.spacing = test_spacing;
        std::string wkt(test_wkt);

        las::LasHeader header(point_format_id, las::PointBaseByteSize(point_format_id) + test_extra_bytes_vlr.size(),
                              test_scale, test_offset);

        header.min = test_min;

        CopcConfig original(header, copc_info, wkt, test_extra_bytes_vlr);

        CopcConfigWriter copy(original);

        copy.LasHeader()->min = Vector3();
        copy.CopcInfo()->spacing = 1;

        // Updating copy should not change original
        REQUIRE(original.LasHeader().min == test_min);
        REQUIRE(original.CopcInfo().spacing == test_spacing);
    }

    SECTION("Updating Config Values")
    {
        CopcConfigWriter cfg(point_format_id, test_scale, test_offset, test_wkt, test_extra_bytes_vlr);

        // Las Header
        cfg.LasHeader()->min = test_min;
        cfg.LasHeader()->max = test_max;
        REQUIRE(cfg.LasHeader()->min == test_min);
        REQUIRE(cfg.LasHeader()->max == test_max);

        // Copc Info
        cfg.CopcInfo()->spacing = test_spacing;
        REQUIRE(cfg.CopcInfo()->spacing == test_spacing);
    }

    SECTION("Copy constructor")
    {
        CopcConfigWriter original(point_format_id);

        CopcConfigWriter copy(original);

        original.LasHeader()->min = test_min;
        original.CopcInfo()->spacing = test_spacing;

        // Updating original should not change copy
        REQUIRE(copy.LasHeader()->min == Vector3());
        REQUIRE(copy.CopcInfo()->spacing == 0);
    }
}
