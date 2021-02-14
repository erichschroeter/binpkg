add_library(
    binpkg
    binpkg.cpp
)
target_include_directories(
    binpkg
    PUBLIC
        .
)

add_executable(
    binpkg-exe
    main.cpp
)
set_target_properties(
    binpkg-exe
    PROPERTIES
        OUTPUT_NAME "binpkg"
)
target_link_libraries(
    binpkg-exe
    binpkg
)