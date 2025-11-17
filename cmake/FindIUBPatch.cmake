# Use this as a fallback if the package doesn't provide a CMake config file.
#
#
# Usage:
#   find_package(IUBPatch REQUIRED)
#   target_link_libraries(mytarget PRIVATE iubpatch::iubpatch)
#

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_check_modules(PC_IUBPATCH QUIET iubpatch)
endif()

find_path(IUBPatch_INCLUDE_DIR
    NAMES iubpatch/patch.h
    PATHS ${PC_IUBPATCH_INCLUDE_DIRS}
    PATH_SUFFIXES include
)

find_library(IUBPatch_LIBRARY
    NAMES iubpatch
    PATHS ${PC_IUBPATCH_LIBRARY_DIRS}
    PATH_SUFFIXES lib
)

if(IUBPatch_INCLUDE_DIR)
    if(EXISTS "${IUBPatch_INCLUDE_DIR}/iubpatch/api.h")
        file(STRINGS "${IUBPatch_INCLUDE_DIR}/iubpatch/api.h" 
             _version_major_line REGEX "^#define[ \t]+IUBPATCH_VERSION_MAJOR")
        file(STRINGS "${IUBPatch_INCLUDE_DIR}/iubpatch/api.h" 
             _version_minor_line REGEX "^#define[ \t]+IUBPATCH_VERSION_MINOR")
        file(STRINGS "${IUBPatch_INCLUDE_DIR}/iubpatch/api.h" 
             _version_patch_line REGEX "^#define[ \t]+IUBPATCH_VERSION_PATCH")
        
        string(REGEX REPLACE "^#define[ \t]+IUBPATCH_VERSION_MAJOR[ \t]+([0-9]+)" "\\1" 
               IUBPatch_VERSION_MAJOR "${_version_major_line}")
        string(REGEX REPLACE "^#define[ \t]+IUBPATCH_VERSION_MINOR[ \t]+([0-9]+)" "\\1" 
               IUBPatch_VERSION_MINOR "${_version_minor_line}")
        string(REGEX REPLACE "^#define[ \t]+IUBPATCH_VERSION_PATCH[ \t]+([0-9]+)" "\\1" 
               IUBPatch_VERSION_PATCH "${_version_patch_line}")
        
        set(IUBPatch_VERSION "${IUBPatch_VERSION_MAJOR}.${IUBPatch_VERSION_MINOR}.${IUBPatch_VERSION_PATCH}")
    endif()
endif()

if(NOT IUBPatch_VERSION AND PC_IUBPATCH_VERSION)
    set(IUBPatch_VERSION ${PC_IUBPATCH_VERSION})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(IUBPatch
    REQUIRED_VARS IUBPatch_LIBRARY IUBPatch_INCLUDE_DIR
    VERSION_VAR IUBPatch_VERSION
)

if(IUBPatch_FOUND)
    set(IUBPatch_LIBRARIES ${IUBPatch_LIBRARY})
    set(IUBPatch_INCLUDE_DIRS ${IUBPatch_INCLUDE_DIR})
    
    if(NOT TARGET iubpatch::iubpatch)
        add_library(iubpatch::iubpatch UNKNOWN IMPORTED)
        set_target_properties(iubpatch::iubpatch PROPERTIES
            IMPORTED_LOCATION "${IUBPatch_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${IUBPatch_INCLUDE_DIR}"
            INTERFACE_COMPILE_FEATURES cxx_std_20
        )
    endif()
endif()

mark_as_advanced(IUBPatch_INCLUDE_DIR IUBPatch_LIBRARY)
