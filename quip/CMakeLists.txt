# QuipCL/cl
cmake_minimum_required (VERSION 3.20)

file(GLOB HEADERS
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"*.h*"
)
file(GLOB SRCS
	RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
	CONFIGURE_DEPENDS
	"*.c*"
)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/rc")
include(VersionTag)
MAKE_VERSION_HEADER("${CMAKE_CURRENT_SOURCE_DIR}/rc/version.h" "quip" "${QuipCL_VERSION}")

add_executable(quip "${SRCS}" "${CMAKE_CURRENT_SOURCE_DIR}/rc/version.h")

set_property(TARGET quip PROPERTY CXX_STANDARD 20)
set_property(TARGET quip PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET quip PROPERTY POSITION_INDEPENDENT_CODE ON)

if (MSVC)
	target_compile_options(quip PRIVATE "/Zc:__cplusplus" "/Zc:preprocessor")
endif()

target_sources(quip PRIVATE "${HEADERS}")

target_link_libraries(quip PRIVATE TermAPI optlib filelib)

include(PackageInstaller)

INSTALL_EXECUTABLE("quip" "${CMAKE_INSTALL_PREFIX}")
