# 设置测试源文件路径
set(TEST_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/miniredis)

# 收集测试源文件
file(GLOB TEST_SOURCES
    ${TEST_SRC_DIR}/*.cpp
    ${CMAKE_SOURCE_DIR}/miniredis/DataStruct/*.cpp
)

# 创建测试可执行文件
add_executable(DataStructTest ${TEST_SOURCES})

find_package(yalantinglibs CONFIG REQUIRED)
target_link_libraries(DataStructTest PRIVATE yalantinglibs::yalantinglibs)

# 添加测试到 CTest
add_test(NAME DataStructTest COMMAND DataStructTest)