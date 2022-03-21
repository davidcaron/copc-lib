#include <copc-lib/io/reader.hpp>
#include <copc-lib/laz/decompressor.hpp>

using namespace copc;
using namespace std;

void ReaderExample()
{
    // Create a reader object
    FileReader reader("autzen-classified.copc.laz");

    // Get the Las Header
    auto las_header = reader.CopcConfig().LasHeader();
    cout << endl << "Las Header:" << endl;
    cout << "\tPoint Format: " << (int)las_header.PointFormatId() << endl
         << "\tPoint Count: " << (int)las_header.PointCount() << endl;

    // Get the CopcInfo struct
    auto copc_info = reader.CopcConfig().CopcInfo();
    cout << "Copc Info: " << endl;
    cout << "\tSpacing: " << copc_info.spacing << endl
         << "\tRoot Offset: " << copc_info.root_hier_offset << endl
         << "\tRoot Size: " << copc_info.root_hier_size << endl
         << "\tGpsTime : (" << copc_info.gpstime_minimum << "/" << copc_info.gpstime_maximum << ")" << endl;

    // Get the WKT string
    cout << "WKT: " << reader.CopcConfig().Wkt() << endl;

    cout << endl;

    {
        VoxelKey load_key(4, 11, 9, 0);

        // FindNode will automatically load the minimum pages needed
        // to find the key you request
        Node node = reader.FindNode(load_key);
        // If FindNode can't find the node, it will return an "invalid" node:
        if (!node.IsValid())
            exit(123);

        // GetPoints returns a Points object, which provides helper functions
        // as well as a Get() function to access the underlying point vector
        las::Points node_points = reader.GetPoints(node);

        cout << node_points.ToString() << endl;

        cout << endl << "First 5 points: " << endl;
        for (int i = 0; i < 5; i++)
        {
            cout << node_points.Get(i)->ToString() << endl;
        }
    }

    // We can also get the raw compressed data if we want to decompress it ourselves:
    {
        VoxelKey loadKey(4, 11, 9, 0);

        Node node = reader.FindNode(loadKey);
        if (!node.IsValid())
            exit(123);
        vector<char> compressed_data = reader.GetPointDataCompressed(node);

        // We can decompress `n` number of points, or we can decompress the entire node
        // by setting n=node.point_count
        int num_points_to_decompress = 5;
        vector<char> uncompressed_data =
            copc::laz::Decompressor::DecompressBytes(compressed_data, las_header, num_points_to_decompress);

        cout << endl
             << "Successfully decompressed " << uncompressed_data.size() / las_header.PointRecordLength() << "points!"
             << endl;
    }
}

int main() { ReaderExample(); }
