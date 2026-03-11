cmake_minimum_required(VERSION 3.15)
project(elim3b LANGUAGES CXX RC)

# Force 32-bit build (this is a Win32 game from 2000)
# Must be invoked with: cmake -A Win32

set(CMAKE_CXX_STANDARD 14)

# Source files
set(SOURCES
    computer.cpp
    file.cpp
    game.cpp
    network.cpp
    sound.cpp
    user.cpp
    utility.cpp
    window.cpp
    compat/compat_guids.cpp
)

# Resource file
set(RESOURCES
    elim3.rc
)

# Create the Win32 executable
add_executable(elim3b WIN32 ${SOURCES} ${RESOURCES})

# Add compat headers directory FIRST so our stubs override missing SDK headers
target_include_directories(elim3b PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/compat
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler settings matching the original .dsp
target_compile_definitions(elim3b PRIVATE
    WIN32
    NDEBUG
    _WINDOWS
    _MBCS
    # DirectInput version set in utility.h
)

# Disable some warnings for legacy code
target_compile_options(elim3b PRIVATE
    /W3
    /wd4996   # deprecated functions (strcpy, sprintf, etc.)
    /wd4244   # conversion warnings
    /wd4267   # size_t to int
    /EHsc     # C++ exception handling (replaces /GX)
    /Zc:wchar_t
)

# Link libraries (matching the original .dsp link line)
target_link_libraries(elim3b PRIVATE
    winmm
    ddraw
    dsound
    dxguid
    dinput8    # Modern replacement for dinput.lib (exports are compatible)
    kernel32
    user32
    gdi32
    ole32
    oleaut32
    uuid
)

# Linker settings
set_target_properties(elim3b PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:WINDOWS"
)

# Copy res.dat and data folder next to the executable after build
add_custom_command(TARGET elim3b POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/Release/res.dat"
        "$<TARGET_FILE_DIR:elim3b>/res.dat"
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:elim3b>/data"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/Release/data/settings.dat"
        "$<TARGET_FILE_DIR:elim3b>/data/settings.dat"
    COMMENT "Copying game assets..."
)
