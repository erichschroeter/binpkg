add_library(
    binpkg_lib
    binpkg.cpp
)
target_include_directories(
    binpkg_lib
    PUBLIC
        .
)

add_executable(
    binpkg
    main.cpp
)
target_link_libraries(
    binpkg
    binpkg_lib
)