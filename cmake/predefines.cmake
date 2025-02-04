if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE CACHE INTERNAL "")
    set(PLATFORM_PREFIX "osx" CACHE INTERNAL "")
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(WINDOWS TRUE CACHE INTERNAL "")
    set(_WINDOWS TRUE CACHE INTERNAL "")
    add_definitions(-DWINDOWS)
    add_definitions(-D_WINDOWS)
    if (ARCH_X86)
        set(PLATFORM_PREFIX "win32" CACHE INTERNAL "")
        add_definitions(-DWIN32)
    else ()
        set(PLATFORM_PREFIX "win64" CACHE INTERNAL "")
        add_definitions(-DWIN64)
    endif ()
elseif (ANDROID)
    set(PLATFORM_PREFIX "android" CACHE INTERNAL "")
elseif (ARMLINUX)
    set(ARM TRUE CACHE INTERNAL "")
    set(PLATFORM_PREFIX "arm" CACHE INTERNAL "")
    add_definitions(-DARMLINUX)
    add_definitions(-DLINUX_GCC)
elseif (ARMLINUX_NATIVE)
    set(ARM TRUE CACHE INTERNAL "")
    set(PLATFORM_PREFIX "linux" CACHE INTERNAL "")
    add_definitions(-DARMLINUX)
    add_definitions(-DLINUX_GCC)

elseif (NANO)
    set(ARM TRUE CACHE INTERNAL "")
    set(PLATFORM_PREFIX "linux" CACHE INTERNAL "")
    add_definitions(-DNANO)
    add_definitions(-DLINUX_GCC)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(XLINUX TRUE CACHE INTERNAL "")
    set(PLATFORM_PREFIX "linux" CACHE INTERNAL "")
    add_definitions(-DLINUX)
    set(LINUX TRUE CACHE INTERNAL "")
    add_definitions(-DLINUX_GCC)
endif ()
message("CMAKE_SYSTEM_NAME=${CMAKE_SYSTEM_NAME}")
message("PLATFORM_PREFIX=${PLATFORM_PREFIX}")
if (CMAKE_HOST_WIN32)
    set(HOME_DIR $ENV{HOMEDRIVE} $ENV{HOMEPATH} CACHE INTERNAL "")
else ()
    set(HOME_DIR $ENV{HOME} CACHE INTERNAL "")
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


if (NOT ROOT_DIR_DEFINED)
    set(ROOT_DIR .)
    if (ARM)
        add_definitions(-DROOT_DIR="./")
    else ()
        add_definitions(-DROOT_DIR="${PROJECT_SOURCE_DIR}")
    endif ()
    set(ROOT_DIR_DEFINED ON)
endif ()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if (MACOSX)
    #add_compile_options(-x objective-c++)
    set(CMAKE_EXE_LINKER_FLAGS "-framework Cocoa -framework AppKit -framework CoreData -framework Foundation -framework IOKit")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -framework IOKit")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
endif ()

if (LINUX)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
#    set(CMAKE_BUILD_TYPE "Release")
endif ()
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")
if (ARM OR LINUX)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "\$ORIGIN:${CMAKE_INSTALL_RPATH}")
endif ()

if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
    set(CMAKE_PDB_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/pdbs)
else ()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g")
endif ()

set(CMAKE_CXX_STANDARD 17)
if ("TEST$ENV{WLP_MAKE_DEPS_WORKING_DIR}" STREQUAL "TEST")
    set(WLP_MAKE_DEPS_WORKING_DIR ${HOME_DIR}/wlp_makedeps)
else ()
    set(WLP_MAKE_DEPS_WORKING_DIR $ENV{WLP_MAKE_DEPS_WORKING_DIR})
endif ()
if (EXISTS ${WLP_MAKE_DEPS_WORKING_DIR}/${PLATFORM_PREFIX}/deps.cmake)
    include(${WLP_MAKE_DEPS_WORKING_DIR}/${PLATFORM_PREFIX}/deps.cmake)
else ()
    message(WARNING "depsfile ${WLP_MAKE_DEPS_WORKING_DIR}/${PLATFORM_PREFIX}/deps.cmake not exists")
endif ()


find_package(OpenMP)
if (OpenMP_CXX_FOUND)
    message("OPENMP FOUND")
    set(OPENMP_LIBS OpenMP::OpenMP_CXX)
endif ()

if (WINDOWS)
    set(SYSTEM_LIBS winmm Shell32 CACHE INTERNAL "")
endif ()

if (ARM)
    set(SYSTEM_LIBS udev CACHE INTERNAL "")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif ()

if (LINUX)
    set(SYSTEM_LIBS X11 CACHE INTERNAL "")
endif ()


if (MSVC)
    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /INCREMENTAL" CACHE INTERNAL "")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /INCREMENTAL" CACHE INTERNAL "")
endif ()
