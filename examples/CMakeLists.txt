add_executable(simple_patch
  simple_patch.cc
)
target_link_libraries(simple_patch PRIVATE iubpatch)

add_executable(batch_patch
  batch_patch.cc
)
target_link_libraries(batch_patch PRIVATE iubpatch)

add_executable(patch_info
  patch_info.cc
)
target_link_libraries(patch_info PRIVATE iubpatch)
