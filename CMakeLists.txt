cmake_minimum_required(VERSION 3.20)
project(ArchTrackTransmutator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(ArchTrackTransmutator
    src/main.cpp
    src/audioio/FfmpegInspector.cpp
    src/audioio/FfmpegHelpers.cpp
    src/audioio/FfmpegDecoder.cpp
    src/analysis/BpmAnalyzer.cpp

)

if(MSVC)
    target_compile_options(ArchTrackTransmutator PRIVATE /EHsc)
endif()

if(APPLE)
    find_package(PkgConfig REQUIRED)

    pkg_check_modules(AUBIO REQUIRED IMPORTED_TARGET aubio)
    pkg_check_modules(RTAUDIO REQUIRED IMPORTED_TARGET rtaudio)

    pkg_check_modules(AVFORMAT REQUIRED IMPORTED_TARGET libavformat)
    pkg_check_modules(AVCODEC REQUIRED IMPORTED_TARGET libavcodec)
    pkg_check_modules(AVUTIL REQUIRED IMPORTED_TARGET libavutil)
    pkg_check_modules(SWRESAMPLE REQUIRED IMPORTED_TARGET libswresample)

    target_link_libraries(ArchTrackTransmutator PRIVATE
        PkgConfig::AUBIO
        PkgConfig::RTAUDIO
        PkgConfig::AVFORMAT
        PkgConfig::AVCODEC
        PkgConfig::AVUTIL
        PkgConfig::SWRESAMPLE
    )

elseif(WIN32)
    find_package(Aubio CONFIG REQUIRED)
    find_package(RtAudio CONFIG REQUIRED)
    find_package(FFMPEG REQUIRED)

    target_include_directories(ArchTrackTransmutator PRIVATE
        ${FFMPEG_INCLUDE_DIRS}
    )

    target_link_libraries(ArchTrackTransmutator PRIVATE
        Aubio::aubio
        RtAudio::rtaudio
        ${FFMPEG_LIBRARIES}
    )
endif()