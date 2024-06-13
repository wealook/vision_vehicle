add_definitions(-DCPPHTTPLIB_OPENSSL_SUPPORT)
find_package(OpenSSL REQUIRED)
set(OpenSSL_INCLUDE_DIRS "${OPENSSL_ROOT_DIR}/include")
include_directories(
    ${OpenSSL_INCLUDE_DIRS}
)
add_library(httplib STATIC httplib.h httplib.cc)
set_target_properties(httplib PROPERTIES FOLDER "vendor")
