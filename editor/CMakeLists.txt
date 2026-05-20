target_sources(taller_editor
    PRIVATE
    # .cpp files
        EditorWindow.ui
        main.cpp
        editor_window.cpp
        map_canvas.cpp
        scene_controller.cpp
        item_builder.cpp
        verificator.cpp
        map/yaml_map_io.cpp
        assets/templates/template_registry.cpp
        dialogs/new_map_dialog.cpp
        dialogs/forest_spawn_dialog.cpp
    PUBLIC
    # .h files
        editor_window.h
        map_canvas.h
        scene_controller.h
        item_builder.h
        verificator.h
        editor_constants.h
        tool_info.h
        map/map_data.h
        map/yaml_map_io.h
        assets/templates/template_registry.h
        dialogs/new_map_dialog.h
        dialogs/forest_spawn_dialog.h
    )

target_include_directories(taller_editor
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

target_compile_definitions(taller_editor
    PRIVATE
        EDITOR_TEMPLATES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/templates"
        EDITOR_TEMPLATES_CITIES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/templates/cities"
        EDITOR_TEMPLATES_FORESTS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/templates/forests"
        EDITOR_SAVE_MAP_PATH="${CMAKE_SOURCE_DIR}/server/assets/maps"
    )
