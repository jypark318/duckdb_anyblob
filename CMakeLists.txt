cmake_minimum_required(VERSION 3.5)
set(TARGET_NAME anyblob)

find_package(OpenSSL REQUIRED)

set(EXTENSION_NAME ${TARGET_NAME}_extension)
set(LOADABLE_EXTENSION_NAME ${TARGET_NAME}_loadable_extension)

project(${TARGET_NAME})
include_directories(src/include)

# 소스 설정
set(EXTENSION_SOURCES src/anyblob.cpp)
file(GLOB_RECURSE ANYBLOB_SRC "src/anyblob_src/*.cpp")
list(APPEND EXTENSION_SOURCES ${ANYBLOB_SRC})

# 확장 등록
build_static_extension(${TARGET_NAME} "${EXTENSION_SOURCES}")
build_loadable_extension(${TARGET_NAME} " " "${EXTENSION_SOURCES}")

# OpenSSL 링크
target_link_libraries(${EXTENSION_NAME} OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(${LOADABLE_EXTENSION_NAME} OpenSSL::SSL OpenSSL::Crypto)

# install 설정은 그대로 유지
install(
  TARGETS ${EXTENSION_NAME}
  EXPORT "${DUCKDB_EXPORT_SET}"
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}")

