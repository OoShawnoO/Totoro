include(FetchContent)

set(CONTENT_DIR ${PROJECT_SOURCE_DIR}/3rdparty)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG master
        SOURCE_DIR "${CONTENT_DIR}/fmt/src"
        BINARY_DIR "${CONTENT_DIR}/fmt/build"
        DOWNLOAD_DIR "${CONTENT_DIR/fmt/download}"
)

FetchContent_MakeAvailable(fmt)