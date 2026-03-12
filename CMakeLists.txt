cmake_minimum_required(VERSION 3.20)
project(ArchTrackTransmutator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(AUBIO REQUIRED IMPORTED_TARGET aubio)
pkg_check_modules(RTAUDIO REQUIRED IMPORTED_TARGET rtaudio)

pkg_check_modules(AVFORMAT REQUIRED IMPORTED_TARGET libavformat)
pkg_check_modules(AVCODEC REQUIRED IMPORTED_TARGET libavcodec)
pkg_check_modules(AVUTIL REQUIRED IMPORTED_TARGET libavutil)
pkg_check_modules(SWRESAMPLE REQUIRED IMPORTED_TARGET libswresample)

add_executable(ArchTrackTransmutator
    src/main.cpp
    src/audioio/FfmpegInspector.h
    src/audioio/FfmpegInspector.cpp
)

target_link_libraries(ArchTrackTransmutator PRIVATE
    PkgConfig::AUBIO
    PkgConfig::RTAUDIO
    PkgConfig::AVFORMAT
    PkgConfig::AVCODEC
    PkgConfig::AVUTIL
    PkgConfig::SWRESAMPLE
)