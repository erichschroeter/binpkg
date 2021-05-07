Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.0-preview3)

FetchContent_MakeAvailable(Catch2)

add_executable(tests
  test.cpp
  $<$<BOOL:FEATURE_X>:test_feature_x.cpp>
  $<$<BOOL:FEATURE_Y>:
    test_feature_y_1.cpp
    test_feature_y_2.cpp>)
target_link_libraries(
  tests
    PRIVATE
      Catch2::Catch2WithMain
      binpkg
)

enable_testing()
include(ParseAndAddCatchTests)
ParseAndAddCatchTests(tests)
