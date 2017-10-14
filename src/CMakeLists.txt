# License: BSD 3-clause
# Copyright: Nick Porcino, 2017

add_executable(LabScreenplay "")

source_file(main.cpp)
source_file(OptionParser.h)
source_file(OptionParser.cpp)
source_file(Screenplay.h)
source_file(Screenplay.cpp)

target_compile_definitions(LabScreenplay PRIVATE PLATFORM_WINDOWS=1)
target_compile_definitions(LabScreenplay PRIVATE ASSET_ROOT="${LABRENDER_ROOT}/assets")

target_include_directories(LabScreenplay PRIVATE "${LOCAL_ROOT}/include")
#target_include_directories(LabScreenplay PRIVATE "${LABSCREENPLAY_ROOT}/include")

target_link_libraries(LabScreenplay debug
    ${LABTEXT_LIBRARIES})

target_link_libraries(LabScreenplay optimized
    ${LABTEXT_LIBRARIES})

if (MSVC_IDE)
    # hack to get around the "Debug" and "Release" directories cmake tries to add on Windows
    #set_target_properties (LabScreenplay PROPERTIES PREFIX "../")
    set_target_properties(LabScreenplay PROPERTIES IMPORT_PREFIX "../")
endif()

install (TARGETS LabScreenplay RUNTIME DESTINATION "${LOCAL_ROOT}/bin")
