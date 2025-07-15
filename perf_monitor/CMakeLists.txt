set(DLL_NAME perf_monitor)

# Only include local headers here; Detours include is set in parent CMakeLists.txt
include_directories(
    Include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}
)

set(SOURCE_FILES
        cdatastore.hpp
        cdatastore.cpp
        logging.hpp
        logging.cpp
        main.hpp
        main.cpp
        offsets.hpp
        stats.hpp
        stats.cpp
        events.hpp
        events.cpp
)

add_library(${DLL_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${DLL_NAME} shlwapi.lib asmjit.lib udis86.lib detours.lib)

install(TARGETS ${DLL_NAME} RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}")
