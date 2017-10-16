
# License: BSD 3-clause
# Copyright: Nick Porcino, 2017

include(FindPackageHandleStandardArgs)

find_path(LABTEXT_INCLUDE_DIR LabText/TextScanner.h
    PATHS
    ${LABTEXT_LOCATION}
    $ENV{LABTEXT_DIR}
    $ENV{PROGRAMFILES}/LabText
    /usr
    /usr/local
    /sw
    /opt/local

    PATH_SUFFIXES
    /include

    DOC "LabText include directory")

set(LABTEXT_LIB_NAMES LabText)

foreach(LIB ${LABTEXT_LIB_NAMES})
    find_library(LABTEXT_${LIB}_LIB_RELEASE ${LIB}
        HINTS ${LABTEXT_INCLUDE_DIR}/..

        PATHS
        ${LABTEXT_LOCATION}
        $ENV{LABTEXT_DIR}
        /usr
        /usr/local
        /sw
        /opt/local

        PATH_SUFFIXES
        /lib
        DOC "LABTEXT library ${LIB}")

        if (LABTEXT_${LIB}_LIB_RELEASE)
            list(APPEND LABTEXT_LIBRARIES "${LABTEXT_${LIB}_LIB_RELEASE}")
            set(LABTEXT_${LIB}_FOUND TRUE)
            set(LABTEXT_${LIB}_LIB_RELEASE "${LABTEXT_${LIB}_LIB_RELEASE}")
        else()
            set(LABTEXT_${LIB}_FOUND FALSE)
        endif()

        mark_as_advanced(LABTEXT_${LIB}_LIB_RELEASE)
endforeach()

foreach(BASE_LIB ${LABTEXT_LIB_NAMES})
    set(LIB ${BASE_LIB}d)
    find_library(LABTEXT_${LIB}_LIB_DEBUG ${LIB}
        HINTS ${LABTEXT_INCLUDE_DIR}/..

        PATHS
        ${LABTEXT_LOCATION}
        $ENV{LABTEXT_DIR}
        /usr
        /usr/local
        /sw
        /opt/local

        PATH_SUFFIXES
        /lib
        DOC "LABTEXT library ${LIB}")

        if (LABTEXT_${LIB}_LIB_DEBUG)
            list(APPEND LABTEXT_DEBUG_LIBRARIES "${LABTEXT_${LIB}_LIB_DEBUG}")
            set(LABTEXT_${LIB}_FOUND TRUE)
            set(LABTEXT_${LIB}_LIB_DEBUG "${LABTEXT_${LIB}_LIB_DEBUG}")
        else()
            set(LABTEXT_${LIB}_FOUND FALSE)
        endif()

        mark_as_advanced(LABTEXT_${LIB}_LIB_DEBUG)
    endforeach()

find_package_handle_standard_args(LABTEXT
    REQUIRED_VARS LABTEXT_LIBRARIES LABTEXT_INCLUDE_DIR)

mark_as_advanced(LABTEXT_INCLUDE_DIR LABTEXT_LIBRARIES LABTEXT_DEBUG_LIBRARIES)

message(info "#### ${LABTEXT_LIBRARIES}")
message(info "#### ${LABTEXT_DEBUG_LIBRARIES}")
