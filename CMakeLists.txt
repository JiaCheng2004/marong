cmake_minimum_required (VERSION 3.6)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(BOT_NAME "marong")

project(${BOT_NAME})
aux_source_directory("src" coresrc)
add_executable(${BOT_NAME} ${coresrc})

string(ASCII 27 Esc)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set_target_properties(${BOT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
find_package(DPP)
find_package(OGG)  # Add Ogg library
find_package(OGGZ)  # Add Oggz library
find_package(OPUSFILE)  # Add OpusFile library

if(APPLE)
    if(CMAKE_APPLE_SILICON_PROCESSOR)
        set(OPENSSL_ROOT_DIR "/opt/homebrew/opt/openssl")
    else()
        set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
    endif()
    find_package(OpenSSL REQUIRED)
else()
    find_package(OpenSSL REQUIRED)
endif()

# Find CURL
find_package(CURL REQUIRED)

target_include_directories(${BOT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS} # Add CURL include directories
    ${OGG_INCLUDE_DIRS} # Add Ogg include directories
    ${OGGZ_INCLUDE_DIRS} # Add Oggz include directories
    ${OPUSFILE_INCLUDE_DIRS} # Add OpusFile include directories
)

target_link_libraries(${BOT_NAME}
    dl
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENSSL_CRYPTO_LIBRARY} 
    ${OPENSSL_SSL_LIBRARY}
    ${CURL_LIBRARIES} # Link CURL library
    ${OGG_LIBRARIES} # Link Ogg library
    ${OGGZ_LIBRARIES} # Link Oggz library
    ${OPUSFILE_LIBRARIES} # Link OpusFile library
)

if (DPP_FOUND)
    target_link_libraries(${BOT_NAME} ${DPP_LIBRARIES})
    target_include_directories(${BOT_NAME} PUBLIC ${DPP_INCLUDE_DIR})
    target_compile_definitions(${BOT_NAME} PUBLIC DPP_CORO) # Add coroutines definition
    target_compile_features(${BOT_NAME} PUBLIC cxx_std_20) # Set C++20 standard
else()
    message(WARNING "Could not find DPP install. Building from source instead.")
    option(DPP_BUILD_TEST "" OFF)
    include(FetchContent)

    FetchContent_Declare(
            libdpp
            GIT_REPOSITORY https://github.com/brainboxdotcc/DPP.git
            GIT_TAG master)

    FetchContent_GetProperties(libdpp)
    if(NOT libdpp_POPULATED)
        FetchContent_Populate(libdpp)
        target_include_directories(${BOT_NAME} PUBLIC
            ${libdpp_SOURCE_DIR}/include
        )
        add_subdirectory(
            ${libdpp_SOURCE_DIR}
            ${libdpp_BINARY_DIR}
            EXCLUDE_FROM_ALL)
    endif()

    target_link_libraries(${BOT_NAME} dpp)
    target_compile_definitions(${BOT_NAME} PUBLIC DPP_CORO) # Add coroutines definition
    target_compile_features(${BOT_NAME} PUBLIC cxx_std_20) # Set C++20 standard
endif()