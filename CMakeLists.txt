cmake_minimum_required(VERSION 3.31)
project(valkyrie)
set(PROJECT_NAME valkyrie)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE VALK_SOURCE CONFIGURE_DEPENDS "src/*.hpp" "src/*.cpp")

set(CPM_SOURCE_CACHE "${CMAKE_CURRENT_SOURCE_DIR}/cpmcache")
include(cmake/CPM.cmake)

set(BUILD_SHARED_LIBS OFF)

set(shared_recursive_mutex_OPT_BUILD_EXAMPLES OFF)
set(shared_recursive_mutex_OPT_BUILD_TESTS OFF)

CPMAddPackage("gh:skypjack/entt#f931687ff04d435871ac9664bb299f71f2a8fafc")
CPMAddPackage("gh:gabime/spdlog@1.15.3")
CPMAddPackage("gh:KonanM/shared_recursive_mutex#48a3624169af3e7156820ea18639c6f4d88b9d0e")
CPMAddPackage("gh:cameron314/concurrentqueue#a989e7e55335ffb231be81bf8575b89fb392c6e3")
CPMAddPackage("gh:Neargye/magic_enum#ecdeb1cea54d87b09bafe9999faf56654646209d")
CPMAddPackage("gh:jbcoe/value_types#fd9a1b55f45c13cc65171449178769862b76ec95")
CPMAddPackage("gh:greg7mdp/parallel-hashmap#88123934b46b77c3b6d80167382734cbff6eff74")

CPMAddPackage(
        NAME googletest
        GITHUB_REPOSITORY google/googletest
        GIT_TAG release-1.12.1
        VERSION 1.12.1
        OPTIONS "INSTALL_GTEST OFF" "gtest_force_shared_crt"
)

set(linked_libs spdlog::spdlog
        EnTT::EnTT
        shared_recursive_mutex
        concurrentqueue
        magic_enum
)

if (MSVC)
    add_definitions(-DNOMINMAX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus /Zc:preprocessor")
endif ()

add_Library(${PROJECT_NAME} STATIC ${VALK_SOURCE})

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)

else ()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror)
endif ()

target_include_directories(${PROJECT_NAME} PUBLIC "src/")
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
        "${parallel-hashmap_SOURCE_DIR}/")
target_link_libraries(${PROJECT_NAME} PUBLIC ${linked_libs})


add_executable(valkyrie_server main.cpp)
target_link_libraries(valkyrie_server PUBLIC ${linked_libs} ${PROJECT_NAME})


target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:VALK_DEBUG>
)

file(GLOB_RECURSE UNIT_TESTS CONFIGURE_DEPENDS "test/*.cpp")

foreach (path_to_test IN LISTS UNIT_TESTS)
    get_filename_component(test_name "${path_to_test}" NAME_WE)
    add_executable(${test_name} ${path_to_test})
    target_link_libraries(${test_name} PUBLIC ${PROJECT_NAME} ${linked_libs} gtest gtest_main)
    target_include_directories(${test_name} PUBLIC "src")
endforeach ()

enable_testing()

file(GLOB_RECURSE UNIT_TESTS CONFIGURE_DEPENDS "test/*.cpp")

foreach (path_to_test IN LISTS UNIT_TESTS)
    get_filename_component(test_name "${path_to_test}" NAME_WE)
    add_test(${test_name} ${test_name})
endforeach ()