include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

export(EXPORT iubpatchTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/iubpatchTargets.cmake"
    NAMESPACE iubpatch::
)

install(EXPORT iubpatchTargets
    FILE iubpatchTargets.cmake
    NAMESPACE iubpatch::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/iubpatch
    COMPONENT development
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/iubpatchConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/iubpatchConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/iubpatch
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/iubpatchConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/iubpatchConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/cmake/iubpatchConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/iubpatch
    COMPONENT development
)
