cmake_minimum_required(VERSION 3.22)

set(CMAKE_PROJECT_NAME openosd-x)

set(PRJ_VERSION "0.1.3")
#set(PRJ_VERSION "")


set(VERSION_C "${CMAKE_CURRENT_BINARY_DIR}/version.c")

configure_file(
    ${CMAKE_CURRENT_LIST_DIR}/cmake/version.c.in
    ${VERSION_C}
    @ONLY
)
#target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${VERSION_C})

# Require TARGET to be set from command line (-DTARGET=xx)
set(TARGET "" CACHE STRING "Target device (e.g., breakoutboard, etc.)")
if("${TARGET}" STREQUAL "")
    set(TARGET "breakoutboard")
    message(STATUS "No TARGET specified. Defaulting to: ${TARGET}")
else()
    message(STATUS "Building for target: ${TARGET}")
endif()

# Add TARGET macro to compiler definitions
add_compile_definitions(TARGET=${TARGET})
if("${TARGET}" STREQUAL "breakoutboard")
    add_compile_definitions(TARGET_BREAKOUTBOARD=1)
elseif("${TARGET}" STREQUAL "novtx")
    add_compile_definitions(TARGET_NOVTX=1)
endif()

option(USE_RTT "Enable SEGGER RTT support" OFF)

# Add options to select resolution mode
option(RESOLUTION_HD "Build with HD resolution" OFF)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, defaulting to Release")
    # Default to Release if no build type is specified
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type")
endif()

# Set compile definition and suffix only if RESOLUTION_HD is ON
if(RESOLUTION_HD)
    message(STATUS "Resolution HD")
    add_compile_definitions(RESOLUTION_HD)
    set(HEX_SUFFIX "hd")
else()
    # No resolution macro defined
    message(STATUS "Resolution SD")
    set(HEX_SUFFIX "sd")
endif()

# Include toolchain file
include("cmake/gcc-arm-none-eabi.cmake")

if(USE_RTT)
    message(STATUS "RTT enabled")
    include(${CMAKE_CURRENT_LIST_DIR}/cmake/rtt.cmake)
    add_definitions(-DUSE_RTT=1)
else()
    message(STATUS "RTT disabled")
    set(RTT_SOURCES "")
    set(RTT_INCLUDE_DIR "")
    add_definitions(-DUSE_RTT=0)
endif()
if(DEV_MODE)
    message(STATUS "DEV_MODE enabled")
	set(CLI_SOURCES ${CMAKE_CURRENT_LIST_DIR}/Src/Core/Src/cli.c)
    add_definitions(-DDEV_MODE=1)
    set(HEX_SUFFIX "dev")
else()
    set(CLI_SOURCES "")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

enable_language(C ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add HAL stm32g4xx sources
add_subdirectory(cmake/stm32g4xx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add sources here
    ./Src/Core/Src/main.c
    ./Src/Core/Src/stm32g4xx_it.c
    ./Src/Core/Src/stm32g4xx_hal_msp.c
    ./Src/Core/Src/system_stm32g4xx.c
    ./Src/Core/Src/sysmem.c
    ./Src/Core/Src/syscalls.c
    ./Src/Core/Src/led.c
    ./Src/Core/Src/char_canvas.c
    ./Src/Core/Src/msp.c
    ./Src/Core/Src/sys_timer.c
    ./Src/Core/Src/cli.c
    ./Src/Core/Src/rtc6705.c
    ./Src/Core/Src/vtx.c
    ./Src/Core/Src/uart_dma.c
    ./Src/Core/Src/mspvtx.c
    ./Src/Core/Src/flash.c
    ./Src/Core/Src/videosignal_gen.c
    ./Src/Core/Src/setting.c
    ./Src/Core/Src/temp.c
    ./Src/Core/Src/target.c
    ./Src/Core/Startup/startup_stm32g431kbtx.s
    ${RTT_SOURCES}
    ${CLI_SOURCES}
	${VERSION_C}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add defined include paths
    ./Src/Core/Inc
    ${RTT_INCLUDE_DIR}
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add defined symbols
)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32g4xx
    # Add defined libraries
)

# Define output file names
if("${PRJ_VERSION}" STREQUAL "")
    set(BIN_FILE    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_${TARGET}_${HEX_SUFFIX}.bin")
    set(HEX_FILE    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_${TARGET}_${HEX_SUFFIX}.hex")
else()
    set(BIN_FILE    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_${TARGET}_${HEX_SUFFIX}_v${PRJ_VERSION}.bin")
    set(HEX_FILE    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_${TARGET}_${HEX_SUFFIX}_v${PRJ_VERSION}.hex")
endif()
set(HEXTM1_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_${TARGET}_${HEX_SUFFIX}_temp1.hex")
set(HEXTM2_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_${TARGET}_${HEX_SUFFIX}_temp2.hex")
set(HEXSIZECRC_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_${TARGET}_${HEX_SUFFIX}_header.hex")
set(HEXBOARD_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_${TARGET}_${HEX_SUFFIX}_board.hex")
set(HEXVERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}_${TARGET}_${HEX_SUFFIX}_version.hex")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/scripts/string2hex.py 0x08004080 "${TARGET}" 16 > ${HEXBOARD_FILE}
        COMMAND python ${CMAKE_CURRENT_SOURCE_DIR}/scripts/string2hex.py 0x08004090 "${PRJ_VERSION}" 16 > ${HEXVERSION_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}> ${BIN_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Oihex --gap-fill=0xFF $<TARGET_FILE:${PROJECT_NAME}> ${HEXTM1_FILE}
        COMMAND srec_cat ${HEXTM1_FILE} -Intel -crop 0x08004200 0x08017000 -STM32 0x08004004 -generate 0x08004000 0x08004004 -constant-l-e 0x12E00 4 -output ${HEXTM2_FILE} -Intel
        COMMAND srec_cat ${HEXTM2_FILE} -Intel -crop 0x08004000 0x08004008 -output ${HEXSIZECRC_FILE} -Intel
        COMMAND srec_cat ${HEXSIZECRC_FILE} -Intel ${HEXBOARD_FILE} -Intel ${HEXVERSION_FILE} -Intel ${HEXTM1_FILE} -Intel -output ${HEX_FILE} -Intel
        COMMAND ${CMAKE_COMMAND} -E rm -f ${HEXTM1_FILE} ${HEXTM2_FILE} ${HEXSIZECRC_FILE} ${HEXBOARD_FILE} ${HEXVERSION_FILE}

       )
