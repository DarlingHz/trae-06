#!/bin/bash

# 共享停车位预约系统构建脚本

set -e

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 默认选项
BUILD_TESTS=0
BUILD_TYPE=Release
CLEAN=0

# 显示使用说明
usage() {
    echo "${YELLOW}使用方法:${NC} $0 [选项]"
    echo ""
    echo "${YELLOW}选项:${NC}"
    echo "  -h, --help          显示此帮助信息"
    echo "  -t, --tests         构建测试套件"
    echo "  -d, --debug         Debug模式构建"
    echo "  -c, --clean         清理构建目录"
    echo ""
    echo "${YELLOW}示例:${NC}"
    echo "  $0                  # 标准Release构建"
    echo "  $0 -t               # 构建测试"
    echo "  $0 -d               # Debug构建"
    echo "  $0 -tc              # 清理并构建测试"
}

# 解析命令行选项
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            exit 0
            ;;
        -t|--tests)
            BUILD_TESTS=1
            shift
            ;;
        -d|--debug)
            BUILD_TYPE=Debug
            shift
            ;;
        -c|--clean)
            CLEAN=1
            shift
            ;;
        -*|--*)
            echo "${RED}未知选项: $1${NC}"
            usage
            exit 1
            ;;
        *)
            usage
            exit 1
            ;;
    esac
done

# 检查配置文件
if [ ! -f "config.json" ]; then
    if [ -f "config.json.example" ]; then
        echo -e "${YELLOW}警告: 未找到config.json, 从config.json.example复制${NC}"
        cp config.json.example config.json
    else
        echo -e "${RED}错误: 未找到config.json或config.json.example${NC}"
        exit 1
    fi
fi

# 清理构建目录
if [ $CLEAN -eq 1 ]; then
    echo -e "${YELLOW}清理构建目录...${NC}"
    rm -rf build
fi

# 初始化子模块
if [ ! -f "thirdparty/cpp-httplib/httplib.h" ]; then
    echo -e "${YELLOW}初始化git子模块...${NC}"
    git submodule update --init --recursive
fi

# 创建构建目录
mkdir -p build
cd build

# 运行CMake配置
echo -e "${YELLOW}配置CMake...${NC}"
CMAKE_ARGS="-DCMAKE_BUILD_TYPE=${BUILD_TYPE}"

if [ $BUILD_TESTS -eq 1 ]; then
    CMAKE_ARGS="${CMAKE_ARGS} -DBUILD_TESTS=ON"
fi

cmake .. ${CMAKE_ARGS}

# 编译
CPU_COUNT=$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)

echo -e "${YELLOW}开始编译 (使用 ${CPU_COUNT} 个线程)...${NC}"
make -j${CPU_COUNT}

# 运行测试
if [ $BUILD_TESTS -eq 1 ]; then
    echo -e "${YELLOW}运行测试...${NC}"
    ctest
fi

# 检查编译结果
if [ $BUILD_TESTS -eq 1 ]; then
    BINARIES=("./bin/parking-system" "./bin/parking-system-tests")
else
    BINARIES=("./bin/parking-system")
fi

ALL_SUCCESS=1
for BIN in "${BINARIES[@]}"; do
    if [ -f "${BIN}" ]; then
        echo -e "${GREEN}✓ ${BIN} 编译成功${NC}"
    else
        echo -e "${RED}✗ ${BIN} 编译失败${NC}"
        ALL_SUCCESS=0
    fi
done

if [ $ALL_SUCCESS -eq 1 ]; then
    echo -e ""
    echo -e "${GREEN}===========================================${NC}"
    echo -e "${GREEN}编译完成!${NC}"
    echo -e "${GREEN}===========================================${NC}"
    echo -e ""
    echo -e "${YELLOW}运行命令:${NC}"
    echo -e "  ./build/bin/parking-system"
    if [ $BUILD_TESTS -eq 1 ]; then
        echo -e "  ./build/bin/parking-system-tests"
    fi
    echo -e ""
    echo -e "${YELLOW}API文档:${NC}"
    echo -e "  http://localhost:8080"
    echo -e ""
else
    echo -e ""
    echo -e "${RED}===========================================${NC}"
    echo -e "${RED}编译失败!${NC}"
    echo -e "${RED}===========================================${NC}"
    exit 1
fi
