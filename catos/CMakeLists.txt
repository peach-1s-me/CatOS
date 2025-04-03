cmake_minimum_required(VERSION 3.22)

project(catos)
add_library(catos INTERFACE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

target_compile_definitions(catos INTERFACE 
    $<$<CONFIG:Debug>:DEBUG>
)

target_include_directories(catos INTERFACE
    include
    component
    component/cat_shell
)

target_sources(catos INTERFACE
    src/cat_device.c
    src/cat_idle.c
    src/cat_init.c
    src/cat_intr.c
    src/cat_ipc.c
    src/cat_lib.c
    src/cat_log.c
    src/cat_mem.c
    src/cat_stdio.c
    src/cat_task.c
    component/cat_component.c
    component/cat_shell/cat_shell_cmds.c
    component/cat_shell/cat_shell_port.c
    component/cat_shell/cat_shell.c
    component/meoo/meoo.c
)

target_link_directories(catos INTERFACE
)

target_link_libraries(catos INTERFACE
)

# Validate that catos code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()


