# Core library sources
set(IUBPATCH_SOURCES
  patch_base.cc
  apply.cc
  io.cc
  formats/ips.cc
  formats/ups.cc
  formats/bps.cc
)

# Platform-specific memory mapping
if(WIN32)
  list(APPEND IUBPATCH_SOURCES platform/mmap_windows.cc)
else()
  list(APPEND IUBPATCH_SOURCES platform/mmap_posix.cc)
endif()

# Build shared library
if(BUILD_SHARED_LIBS)
  add_library(iubpatch SHARED ${IUBPATCH_SOURCES})
  target_compile_definitions(iubpatch PRIVATE IUBPATCH_EXPORTS)
  target_compile_definitions(iubpatch INTERFACE IUBPATCH_SHARED)
  
  target_include_directories(iubpatch PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )
  target_compile_features(iubpatch PUBLIC cxx_std_20)
  
  set_target_properties(iubpatch PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
  )
endif()

# Build static library
if(BUILD_STATIC_LIBS)
  add_library(iubpatch-static STATIC ${IUBPATCH_SOURCES})
  
  target_include_directories(iubpatch-static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
  )
  target_compile_features(iubpatch-static PUBLIC cxx_std_20)
  
  # On Unix, static library typically has same name as shared but .a extension
  set_target_properties(iubpatch-static PROPERTIES
    OUTPUT_NAME iubpatch
  )
  
  # Add alias for consistent target naming
  add_library(iubpatch::static ALIAS iubpatch-static)
endif()

# Ensure at least one library type is built
if(NOT BUILD_SHARED_LIBS AND NOT BUILD_STATIC_LIBS)
  message(FATAL_ERROR "At least one of BUILD_SHARED_LIBS or BUILD_STATIC_LIBS must be ON")
endif()

# Create alias for the default library (prefer shared if both are built)
if(BUILD_SHARED_LIBS)
  add_library(iubpatch::iubpatch ALIAS iubpatch)
elseif(BUILD_STATIC_LIBS)
  add_library(iubpatch::iubpatch ALIAS iubpatch-static)
endif()

# Apply common settings to whichever libraries were built
foreach(target iubpatch iubpatch-static)
  if(TARGET ${target})
    target_compile_definitions(${target} PRIVATE
      IUBPATCH_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
      IUBPATCH_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    )
    
    if(IUB_ENABLE_MMAP)
      target_compile_definitions(${target} PUBLIC IUB_ENABLE_MMAP=1)
    endif()
  endif()
endforeach()

# Installation
include(GNUInstallDirs)

# Install both libraries if they were built
set(INSTALL_TARGETS)
if(BUILD_SHARED_LIBS)
  list(APPEND INSTALL_TARGETS iubpatch)
endif()
if(BUILD_STATIC_LIBS)
  list(APPEND INSTALL_TARGETS iubpatch-static)
endif()

install(TARGETS ${INSTALL_TARGETS}
  EXPORT iubpatchTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/iubpatch
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  FILES_MATCHING PATTERN "*.h"
  PATTERN "*.in" EXCLUDE
)

# Install generated version header
install(FILES ${CMAKE_BINARY_DIR}/include/iubpatch/version.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/iubpatch
)
