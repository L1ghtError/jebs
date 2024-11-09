#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "cjson_utils" for configuration "Debug"
set_property(TARGET cjson_utils APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(cjson_utils PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "/usr/local/lib/libcjson_utils.a"
  )

list(APPEND _cmake_import_check_targets cjson_utils )
list(APPEND _cmake_import_check_files_for_cjson_utils "/usr/local/lib/libcjson_utils.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
