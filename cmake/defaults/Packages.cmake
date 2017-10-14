
set(LABTEXT_LOCATION "${LOCAL_ROOT}")
find_package(LabText REQUIRED)

# --math
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_library(M_LIB m)
endif()
