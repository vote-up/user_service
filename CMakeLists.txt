cmake_minimum_required(VERSION 3.12)
project(user_service CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)

find_package(userver COMPONENTS universal core postgresql grpc chaotic REQUIRED)

userver_setup_environment()

# Common sources
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp")
list(FILTER SOURCES EXCLUDE REGEX ".*(_test|_benchmark)\\.cpp$")
list(FILTER SOURCES EXCLUDE REGEX ".*/main\\.cpp$")
add_library(${PROJECT_NAME}_objs OBJECT ${SOURCES})

target_link_libraries(${PROJECT_NAME}_objs PUBLIC userver::postgresql userver::grpc ${PROJECT_NAME}-chgen ${PROJECT_NAME}_proto  ${PROJECT_NAME}_sql)
target_include_directories(${PROJECT_NAME}_objs PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB_RECURSE SCHEMAS ${CMAKE_CURRENT_SOURCE_DIR}/docs/yaml/*.yaml)
userver_target_generate_chaotic(
  ${PROJECT_NAME}-chgen
  ARGS
  -n
  "/components/schemas/([^/]*)/=dto::user::{0}"
  -f
  "(.*)={0}"
  --clang-format=
  --generate-serializers
  OUTPUT_DIR
  ${CMAKE_CURRENT_BINARY_DIR}/src
  SCHEMAS
  ${SCHEMAS}
  RELATIVE_TO
  ${CMAKE_CURRENT_SOURCE_DIR}
)

# Create a proto library with userver extensions
userver_add_grpc_library(${PROJECT_NAME}_proto PROTOS handlers/user.proto)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC ${PROJECT_NAME}_proto)

userver_add_sql_library(
  ${PROJECT_NAME}_sql
  NAMESPACE
  ${PROJECT_NAME}
  OUTPUT_DIR
  ${CMAKE_CURRENT_BINARY_DIR}
  SQL_FILES
  src/sql/*.sql
)

# The Service
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)


# Unit Tests
# add_executable(${PROJECT_NAME}_unittest
# )
# target_link_libraries(${PROJECT_NAME}_unittest PRIVATE ${PROJECT_NAME}_objs userver::utest)
# add_google_tests(${PROJECT_NAME}_unittest)


# Benchmarks
# add_executable(${PROJECT_NAME}_benchmark
# )
# target_link_libraries(${PROJECT_NAME}_benchmark PRIVATE ${PROJECT_NAME}_objs userver::ubench)
# add_google_benchmark_tests(${PROJECT_NAME}_benchmark)

userver_testsuite_add_simple()


# Install
include(GNUInstallDirs)

if(DEFINED ENV{PREFIX})
  message(STATUS "Set install prefix: $ENV{PREFIX}")
  file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
  set(CMAKE_INSTALL_PREFIX "${PREFIX_PATH}")
endif()

file(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${PROJECT_NAME})
install(FILES ${CONFIGS_FILES} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME} COMPONENT ${PROJECT_NAME})
