cmake_minimum_required (VERSION 2.6)

project (metric)

execute_process(
        COMMAND llvm-config-7 --includedir
        OUTPUT_VARIABLE INCLUDEDIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
include_directories(SYSTEM "${INCLUDEDIR}")

execute_process(
	COMMAND llvm-config-7 --bindir
	OUTPUT_VARIABLE BINDIR
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(CMAKE_CXX "${BINDIR}/clang++")

execute_process(
	COMMAND llvm-config-7 --cxxflags
	OUTPUT_VARIABLE CXX_FLAGS
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(CMAKE_CXX_FLAGS "${CXX_FLAGS} -std=c++11 -O2 -frtti -fexceptions")

execute_process(
	COMMAND llvm-config-7 --ldflags
	OUTPUT_VARIABLE LD_FLAGS
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
	COMMAND llvm-config-7 --libdir
	OUTPUT_VARIABLE LIB_DIR
	OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(CMAKE_EXE_LINKER_FLAGS "${LD_FLAGS} -L${LIB_DIR}")

add_executable(metric
	Clang.cpp
	Visitor.cpp
	VisitorFactory.cpp
        kohesion.cpp
        dependency.cpp
        graphviz.cpp
	metrics.cpp
        utils.cpp
)

target_link_libraries(
	metric

	boost_program_options
)

target_link_libraries(
	metric

	clang
	clangTooling
	clangFrontendTool
	clangFrontend
	clangParse
	clangSema
	clangStaticAnalyzerFrontend
	clangStaticAnalyzerCheckers
	clangStaticAnalyzerCore
	clangAnalysis
	clangARCMigrate
	clangEdit
	clangAST
	clangASTMatchers
	clangLex
	clangDriver
	clangSerialization
	clangRewrite
	clangRewriteFrontend
	clangCodeGen
	clangBasic
)
