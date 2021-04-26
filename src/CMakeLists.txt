add_library(binpkg
    binpkg.cpp)
target_compile_features(binpkg
    PRIVATE
        cxx_auto_type
        cxx_constexpr
        cxx_range_for
        cxx_raw_string_literals
        cxx_reference_qualified_functions)
target_include_directories(binpkg
    PUBLIC
        .)

add_executable(binpkg-exe
    main.cpp)
set_target_properties(binpkg-exe
    PROPERTIES
        OUTPUT_NAME "binpkg")
target_link_libraries(binpkg-exe
    binpkg)

install(TARGETS binpkg-exe
    RUNTIME DESTINATION bin)