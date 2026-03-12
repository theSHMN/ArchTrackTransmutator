#include <iostream>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswresample/swresample.h>
}

int main() {
    std::cout << "avformat: " << avformat_version() << std::endl;
    std::cout << "avcodec: " << avcodec_version() << std::endl;
    std::cout << "avutil: " << avutil_version() << std::endl;
    std::cout << "swresample: " << swresample_version() << std::endl;
    return 0;
}