## Cliente: agregar los fuentes del cliente al target creado en el CMake raíz
# Recolecta recursivamente los archivos .cpp/.c en las carpetas client y common
file(GLOB_RECURSE CLIENT_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

file(GLOB_RECURSE COMMON_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/common/*.cpp
    ${CMAKE_SOURCE_DIR}/common/*.c
)

list(APPEND ALL_SOURCES ${CLIENT_SOURCES} ${COMMON_SOURCES})

if(NOT ALL_SOURCES)
    message(FATAL_ERROR "No source files found for taller_client in ${CMAKE_CURRENT_SOURCE_DIR} or ${CMAKE_SOURCE_DIR}/common")
endif()

target_sources(taller_client PRIVATE ${ALL_SOURCES})

# Include dirs: client, common y SDL2pp sources (si están disponibles)
target_include_directories(taller_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/common
)
if(DEFINED libSDL2pp_SOURCE_DIR)
    target_include_directories(taller_client PRIVATE ${libSDL2pp_SOURCE_DIR})
endif()

# Link libraries: SDL2pp provisto por FetchContent en el CMake raíz.
target_link_libraries(taller_client PRIVATE SDL2pp::SDL2pp)

# Aplicar las reglas de warnings del proyecto (macro definida en CMake principal)
if(COMMAND set_project_warnings)
    set_project_warnings(taller_client ${TALLER_MAKE_WARNINGS_AS_ERRORS} FALSE)
endif()
