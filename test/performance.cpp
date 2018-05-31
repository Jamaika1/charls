//
// (C) Jan de Vaan 2007-2010, all rights reserved. See the accompanying "License.txt" for licensed use.
//

#include "performance.h"
#include "util.h"
#include "../src/charls.h"

#include <vector>
#include <ratio>
#include <chrono>

namespace
{

void TestFile16BitAs12(const char* filename, int ioffs, Size size2, int ccomp, bool littleEndianFile)
{
    std::vector<uint8_t> uncompressedData;
    if (!ReadFile(filename, &uncompressedData, ioffs))
        return;

    FixEndian(&uncompressedData, littleEndianFile);

    uint16_t* pushort = reinterpret_cast<uint16_t*>(uncompressedData.data());

    for (size_t i = 0; i < uncompressedData.size() / 2; ++i)
    {
        pushort[i] = pushort[i] >> 4;
    }

    TestRoundTrip(filename, uncompressedData, size2, 12, ccomp);
}


void TestPerformance(int loopCount)
{
    ////TestFile("test/bad.raw", 0, Size(512, 512),  8, 1);

    // RGBA image (This is a common PNG sample)
    TestFile("test/alphatest.raw", 0, Size(380, 287), 8, 4, false, loopCount);

    const Size size1024 = Size(1024, 1024);
    const Size size512 = Size(512, 512);

    // 16 bit mono
    TestFile("test/MR2_UNC", 1728, size1024, 16, 1, true, loopCount);

    // 8 bit mono
    TestFile("test/0015.raw", 0, size1024, 8, 1, false, loopCount);
    TestFile("test/lena8b.raw", 0, size512, 8, 1, false, loopCount);

    // 8 bit color
    TestFile("test/desktop.ppm", 40, Size(1280, 1024), 8, 3, false, loopCount);

    // 12 bit RGB
    TestFile("test/SIEMENS-MR-RGB-16Bits.dcm", -1, Size(192, 256), 12, 3, true, loopCount);
    TestFile16BitAs12("test/DSC_5455.raw", 142949, Size(300, 200), 3, true);

    // 16 bit RGB
    TestFile("test/DSC_5455.raw", 142949, Size(300, 200), 16, 3, true, loopCount);
}


} // namespace


void PerformanceTests(int loopCount)
{
#ifdef _DEBUG
    printf("NOTE: running performance test in debug mode, performance may be slow!\r\n");
#endif
    printf("Test Perf (with loop count %i)\r\n", loopCount);
    TestPerformance(loopCount);
}

void TestLargeImagePerformanceRgb8(int loopCount)
{
    // Note: the test images are very large and not included in the repository.
    //       The images can be downloaded from: http://imagecompression.info/test_images/

#ifdef _DEBUG
    printf("NOTE: running performance test in debug mode, performance may be slow!\r\n");
#endif
    printf("Test Large Images Performance\r\n");

    try
    {
        test_portable_anymap_file("test/rgb8bit/artificial.ppm", loopCount);
        test_portable_anymap_file("test/rgb8bit/big_building.ppm", loopCount);
        test_portable_anymap_file("test/rgb8bit/big_tree.ppm", loopCount);
        test_portable_anymap_file("test/rgb8bit/bridge.ppm", loopCount);
        test_portable_anymap_file("test/rgb8bit/cathedral.ppm", loopCount);
        test_portable_anymap_file("test/rgb8bit/deer.ppm", loopCount);
        test_portable_anymap_file("test/rgb8bit/fireworks.ppm", loopCount);
        test_portable_anymap_file("test/rgb8bit/flower_foveon.ppm", loopCount);
        test_portable_anymap_file("test/rgb8bit/hdr.ppm", loopCount);
        test_portable_anymap_file("test/rgb8bit/leaves_iso_200.ppm", loopCount);
        test_portable_anymap_file("test/rgb8bit/leaves_iso_1600.ppm", loopCount);
        test_portable_anymap_file("test/rgb8bit/nightshot_iso_100.ppm", loopCount);
        test_portable_anymap_file("test/rgb8bit/nightshot_iso_1600.ppm", loopCount);
        test_portable_anymap_file("test/rgb8bit/spider_web.ppm", loopCount);
    }
    catch (const std::istream::failure& error)
    {
        printf("Test failed %s\r\n", error.what());
    }
}

void DecodePerformanceTests(int loopCount)
{
    printf("Test decode Perf (with loop count %i)\r\n", loopCount);

    std::vector<uint8_t> jpeglsCompressed;
    if (!ReadFile("decodetest.jls", &jpeglsCompressed, 0, 0))
    {
        printf("Failed to load the file decodetest.jls\r\n");
        return;
    }

    JlsParameters params;
    auto result = JpegLsReadHeader(jpeglsCompressed.data(), jpeglsCompressed.size(), &params, nullptr);
    if (result != charls::ApiResult::OK)
        return;

    std::vector<uint8_t> uncompressed(static_cast<size_t>(params.height) * params.width * ((params.bitsPerSample + 7) / 8) * params.components);

    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < loopCount; ++i)
    {

        result = JpegLsDecode(uncompressed.data(), uncompressed.size(), jpeglsCompressed.data(), jpeglsCompressed.size(), &params, nullptr);
        if (result != charls::ApiResult::OK)
        {
            std::cout << "Decode failure: " << static_cast<int>(result) << "\n";
            return;
        }
    }

    const auto end = std::chrono::steady_clock::now();
    const auto diff = end - start;
    std::cout << "Total decoding time is: " << std::chrono::duration <double, std::milli>(diff).count() << " ms" << std::endl;
    std::cout << "Decoding time per image: " << std::chrono::duration <double, std::milli>(diff).count() / loopCount << " ms" << std::endl;
}
