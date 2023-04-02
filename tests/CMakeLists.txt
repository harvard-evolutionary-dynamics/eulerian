file(GLOB SOURCE_LIST CONFIGURE_DEPENDS
        "${Eulerian_SOURCE_DIR}/tests/*.h"
        "${Eulerian_SOURCE_DIR}/tests/*.hpp"
        "${Eulerian_SOURCE_DIR}/tests/*.cc"
        "${Eulerian_SOURCE_DIR}/tests/*.cpp")

# This target runs all of the tests.
# Make sure CATCH_CONFIG_MAIN is only defined in one test file.
add_executable(test-eulerian ${SOURCE_LIST})

# Should be linked to the main library, as well as the Catch2 testing library
target_link_libraries(test-eulerian PRIVATE eulerian-lib catch2 pybind11 lrcalc Boost::boost)

# If you register a test, then ctest and make test will run it.
# You can also run examples and check the output, as well.
add_test(NAME test-eulerian COMMAND test-eulerian) # Command can be a target

# Add folders
set_target_properties(test-eulerian PROPERTIES FOLDER harvard-evolutionary-dynamics)

add_custom_command(
        TARGET test-eulerian
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -DSRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data" -DDEST_DIR="${CMAKE_CURRENT_BINARY_DIR}/data" -P "${CMAKE_SOURCE_DIR}/cmake/configure_files.cmake"
)

