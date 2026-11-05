cmake_minimum_required(VERSION 3.16)
project(DreamyMusic LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    add_library(DreamyMusic SHARED
        src/DreamyMusic.cpp
        src/DreamyMusic.h
        src/DreamyDSP.h
        Win/DreamyMusicPiPL.rc
        Win/DreamyMusic.def
    )

    target_include_directories(DreamyMusic PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include/AE_SDK
    )

    target_compile_definitions(DreamyMusic PRIVATE
        AE_OS_WIN=1
        AE_PROC_INTELx64=1
        _CRT_SECURE_NO_WARNINGS=1
    )

    if(MSVC)
        target_compile_options(DreamyMusic PRIVATE
            $<$<CONFIG:Release>:/O2 /fp:fast /Oi /Ot /GL>
            /W3 /EHsc
        )
        target_link_options(DreamyMusic PRIVATE
            $<$<CONFIG:Release>:/LTCG /OPT:REF /OPT:ICF>
        )
    else()
        target_compile_options(DreamyMusic PRIVATE
            -O3 -ffast-math -fno-finite-math-only -mavx2 -mfma
        )
        target_link_options(DreamyMusic PRIVATE
            -static -static-libgcc -static-libstdc++
        )
    endif()

    set_target_properties(DreamyMusic PROPERTIES
        PREFIX ""
        OUTPUT_NAME "DreamyMusic"
        SUFFIX ".aex"
    )
elseif(APPLE)
    add_library(DreamyMusic MODULE
        src/DreamyMusic.cpp
        src/DreamyMusic.h
        src/DreamyDSP.h
    )

    target_include_directories(DreamyMusic PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/include/AE_SDK
    )

    target_compile_definitions(DreamyMusic PRIVATE
        AE_OS_MAC=1
    )

    target_compile_options(DreamyMusic PRIVATE
        -O3 -ffast-math -fno-finite-math-only
    )

    set_target_properties(DreamyMusic PROPERTIES
        BUNDLE TRUE
        BUNDLE_EXTENSION "plugin"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Mac/Info.plist"
    )
endif()

