add_executable(iubpatch-cli
  iubpatch_cli.cc
)

target_link_libraries(iubpatch-cli PRIVATE iubpatch)

install(TARGETS iubpatch-cli
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
