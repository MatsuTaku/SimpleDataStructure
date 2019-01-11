# Check if the CPU provides fast operation for popcnt.

set(BUILTIN_POPCNT OFF)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    #  Use /proc/cpuinfo to get the information
    file(STRINGS "/proc/cpuinf" _cpuinfo)
    if(_cpuinfo MATCHES "(sse4_2)|sse4a")
        set(BUILTIN_POPCNT ON)
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Handle windows...
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # Handle macOS
    execute_process(COMMAND sysctl -n machdep.cpu.features
                    OUTPUT_VARIABLE _cpuinfo OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(_cpuinfo MATCHES "SSE4.2")
        set(BUILTIN_POPCNT ON)
    endif()
endif()

