cmake_minimum_required(VERSION 3.20)
project(DobryFarmarReconstruction LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# There is deliberately no game target until the game is reconstructed.
if(WIN32 AND NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(FATAL_ERROR "The original game and DLL ABI require an x86 (Win32) build.")
endif()
set(LIBMNG_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.local/libmng-source" CACHE PATH "Pinned LIBMNG source directory")
if(NOT EXISTS "${LIBMNG_SOURCE_DIR}/libmng.h")
    message(FATAL_ERROR "Fetch the pinned decoder source first: python tools/fetch_libmng.py")
endif()
find_package(Python3 COMPONENTS Interpreter REQUIRED)
execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/fetch_libmng.py"
    --verify-only --source-dir "${LIBMNG_SOURCE_DIR}" RESULT_VARIABLE LIBMNG_VERIFY_RESULT)
if(NOT LIBMNG_VERIFY_RESULT EQUAL 0)
    message(FATAL_ERROR "Pinned LIBMNG source verification failed")
endif()
include(tools/cmake/OriginalCodecs.cmake)
file(GLOB LIBMNG_SOURCES CONFIGURE_DEPENDS "${LIBMNG_SOURCE_DIR}/libmng_*.c")
add_library(recovered_mng_dependency STATIC ${LIBMNG_SOURCES})
set_target_properties(recovered_mng_dependency PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_compile_definitions(recovered_mng_dependency PUBLIC MNG_SUPPORT_FULL MNG_NO_OPEN_CLOSE_STREAM MNG_INTERNAL_MEMMNGMT)
target_include_directories(recovered_mng_dependency PUBLIC "${LIBMNG_SOURCE_DIR}")
target_link_libraries(recovered_mng_dependency PUBLIC original_zlib original_jpeg)
if(NOT WIN32)
    target_link_libraries(recovered_mng_dependency PUBLIC m)
endif()
add_library(JNGLoad SHARED src/jngload/JNGLoad.cpp)
target_compile_definitions(JNGLoad PRIVATE JNGLOAD_BUILD_DLL)
target_include_directories(JNGLoad PUBLIC src/jngload)
target_link_libraries(JNGLoad PRIVATE recovered_mng_dependency)
if(WIN32)
    if(NOT MSVC)
        message(FATAL_ERROR "Use MSVC for the original C++ decorated exports.")
    endif()
    target_sources(JNGLoad PRIVATE src/jngload/JNGLoad.def)
    add_executable(jng-reference-decode tools/jng_reference_decode.cpp)
endif()
add_executable(jng-decode tools/jng_decode.cpp)
target_link_libraries(jng-decode PRIVATE JNGLoad)
add_library(recovered_game_utilities STATIC src/game/core/String.cpp src/game/core/Color.cpp src/game/file_manager/PathNames.cpp)
target_include_directories(recovered_game_utilities PUBLIC src/game/core src/game/file_manager)
if(WIN32)
    target_sources(recovered_game_utilities PRIVATE src/game/core/WString.cpp src/game/file_manager/RealFile.cpp
        src/game/file_manager/FileIndex.cpp src/game/xml/XmlDocument.cpp
        src/game/core/StringConversion.cpp src/game/settings/Settings.cpp src/game/engine/GameWindow.cpp src/game/engine/GameTimer.cpp src/game/profiles/ProfileHeaders.cpp src/game/profiles/ProfileProperties.cpp src/game/profiles/Profile.cpp src/game/profiles/GameProfileDefaults.cpp)
    target_sources(recovered_game_utilities PRIVATE src/game/audio/SoundFiles.cpp src/game/audio/SoundVolume.cpp)
    target_include_directories(recovered_game_utilities PUBLIC src/game/audio)
    target_include_directories(recovered_game_utilities PUBLIC src/game/xml src/game/settings src/game/engine src/game/profiles)
    execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/fetch_expat.py"
        --verify-only RESULT_VARIABLE EXPAT_VERIFY_RESULT)
    if(NOT EXPAT_VERIFY_RESULT EQUAL 0)
        message(FATAL_ERROR "Run python tools/fetch_expat.py before configuring")
    endif()
    set(EXPAT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.local/expat-source/expat/lib")
    add_library(recovered_expat_dependency STATIC "${EXPAT_SOURCE_DIR}/xmlparse.c"
        "${EXPAT_SOURCE_DIR}/xmlrole.c" "${EXPAT_SOURCE_DIR}/xmltok.c")
    target_include_directories(recovered_expat_dependency PUBLIC "${EXPAT_SOURCE_DIR}")
    target_compile_definitions(recovered_expat_dependency PUBLIC XML_STATIC XML_UNICODE_WCHAR_T
        PRIVATE COMPILED_FROM_DSP _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(recovered_game_utilities PRIVATE recovered_expat_dependency)
    target_compile_definitions(recovered_game_utilities PRIVATE _CRT_SECURE_NO_WARNINGS)
    execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/fetch_minizip.py"
        --verify-only RESULT_VARIABLE MINIZIP_VERIFY_RESULT)
    if(NOT MINIZIP_VERIFY_RESULT EQUAL 0)
        message(FATAL_ERROR "Run python tools/fetch_minizip.py before configuring")
    endif()
    set(MINIZIP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.local/minizip-1.01")
    add_library(original_minizip STATIC "${MINIZIP_SOURCE_DIR}/unzip.c" "${MINIZIP_SOURCE_DIR}/ioapi.c")
    target_include_directories(original_minizip PUBLIC "${MINIZIP_SOURCE_DIR}")
    target_compile_definitions(original_minizip PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(original_minizip PUBLIC original_zlib)
    target_sources(recovered_game_utilities PRIVATE src/game/file_manager/ZipFile.cpp)
    target_link_libraries(recovered_game_utilities PRIVATE original_minizip)
    target_link_libraries(recovered_game_utilities PRIVATE advapi32 user32 gdi32 winmm)
    execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/fetch_direct3d_headers.py"
        --verify-only RESULT_VARIABLE D3D8_VERIFY_RESULT)
    if(NOT D3D8_VERIFY_RESULT EQUAL 0)
        message(FATAL_ERROR "Run python tools/fetch_direct3d_headers.py before configuring")
    endif()
    add_library(recovered_graphics_utilities STATIC src/game/graphics/JngTexture.cpp src/game/graphics/GraphicsPresentation.cpp
        src/game/graphics/SpriteBuffers.cpp src/game/graphics/Matrix.cpp src/game/graphics/GraphicsRenderState.cpp
        src/game/graphics/SpriteGeometry.cpp src/game/graphics/NativeImage.cpp src/game/graphics/NativeJpeg.cpp src/game/graphics/JpegColor.cpp src/game/graphics/ImageTexture.cpp src/game/engine/GameCursor.cpp)
    target_include_directories(recovered_graphics_utilities PUBLIC src/game/graphics "${CMAKE_CURRENT_SOURCE_DIR}/.local/direct3d8-headers")
    target_link_libraries(recovered_graphics_utilities PUBLIC JNGLoad recovered_game_utilities PRIVATE recovered_game_jpeg)
    add_library(recovered_resource_utilities STATIC src/game/resources/Resource.cpp src/game/resources/Texture.cpp
        src/game/resources/ResourceManager.cpp src/game/resources/Shader.cpp src/game/resources/Font.cpp src/game/graphics/SpriteBatches.cpp
        src/game/ui/TextLineLayout.cpp src/game/ui/LetterSprite.cpp src/game/ui/StringManager.cpp src/game/ui/ControlSprite.cpp
        src/game/ui/ControlAnimation.cpp src/game/ui/View.cpp src/game/ui/Control.cpp src/game/ui/TextLine.cpp src/game/ui/EditBox.cpp src/game/ui/ListBox.cpp src/game/ui/MultilineText.cpp
        src/game/ui/InputBindings.cpp src/game/ui/InputDevices.cpp src/game/ui/Button.cpp src/game/ui/ButtonText.cpp src/game/ui/ExtendedTextButton.cpp src/game/ui/MenuView.cpp src/game/ui/MainMenuContent.cpp src/game/ui/YesNoDialog.cpp
        src/game/ui/ProfileSession.cpp src/game/ui/SelectProfile.cpp src/game/ui/Credits.cpp src/game/ui/ExitDialog.cpp src/game/ui/CheckBox.cpp src/game/ui/Slider.cpp src/game/ui/Options.cpp src/game/graphics/ClickMask.cpp)
    target_sources(recovered_resource_utilities PRIVATE src/game/audio/SoundManager.cpp src/game/audio/Sound.cpp src/game/audio/Channel.cpp src/game/audio/UiSound.cpp src/game/audio/MusicPlayback.cpp src/game/ui/ProgressBar.cpp src/game/ui/Histogram.cpp src/game/ui/ProgressSprite.cpp src/game/ui/LoadingFromLog.cpp src/game/ui/TooltipFrame.cpp src/game/ui/AwardEffect.cpp src/game/ui/AwardButton.cpp src/game/ui/Awards.cpp src/game/ui/AwardPanel.cpp)
    target_sources(recovered_resource_utilities PRIVATE src/game/ui/MainMenu.cpp src/game/ui/NewUser.cpp src/game/ui/ProgressDiscrete.cpp src/game/ui/SeedPackage.cpp)
    target_sources(recovered_resource_utilities PRIVATE src/game/resources/AnimableAppearance.cpp src/game/engine/GameConfiguration.cpp src/game/engine/GameAction.cpp src/game/ui/FarmField.cpp src/game/ui/FarmFieldActions.cpp src/game/ui/FarmAreas.cpp src/game/ui/Farm.cpp src/game/ui/PersonTasks.cpp src/game/ui/PersonVisuals.cpp src/game/ui/PersonWalk.cpp src/game/ui/PersonVehicleWalk.cpp src/game/ui/PersonSounds.cpp src/game/ui/Person.cpp src/game/ui/PersonActions.cpp src/game/ui/ElasticAnimation.cpp src/game/ui/FieldTip.cpp)
    target_sources(recovered_resource_utilities PRIVATE src/game/engine/GameplayTime.cpp)
    target_sources(recovered_resource_utilities PRIVATE src/game/engine/GameplayEvents.cpp)
    target_sources(recovered_resource_utilities PRIVATE src/game/engine/Gameplay.cpp src/game/engine/SalesHistory.cpp src/game/engine/GameEffects.cpp)
    target_sources(recovered_resource_utilities PRIVATE src/game/engine/Game.cpp src/game/engine/GamePersistence.cpp)
    target_sources(recovered_resource_utilities PRIVATE src/game/engine/Upgrade.cpp src/game/ui/ContractStatus.cpp src/game/ui/PriceText.cpp src/game/ui/ContractDialogs.cpp src/game/ui/ContractProvide.cpp src/game/ui/ContractLetterSlot.cpp src/game/ui/GamePauseMenu.cpp src/game/ui/HelpView.cpp src/game/ui/Journal.cpp src/game/ui/GameAwardManager.cpp src/game/ui/StateView.cpp src/game/ui/GameMenu.cpp src/game/ui/TutorialTips.cpp src/game/ui/TutorialManager.cpp src/game/ui/MovableUnion.cpp src/game/ui/BuySeeds.cpp src/game/ui/PictureButton.cpp src/game/ui/BuyBuilding.cpp src/game/ui/ProductionControls.cpp src/game/engine/ProductionLevel.cpp src/game/ui/ProductionPlacement.cpp)
    target_sources(recovered_resource_utilities PRIVATE src/game/ui/ProductionBuilding.cpp src/game/ui/Manufacture.cpp src/game/ui/FieldObject.cpp src/game/ui/ActionArea.cpp src/game/ui/Warehouse.cpp src/game/ui/WarehouseTip.cpp src/game/ui/Market.cpp src/game/ui/FlyingText.cpp src/game/ui/DigBuilding.cpp src/game/ui/WaterSource.cpp src/game/ui/MowBuilding.cpp src/game/ui/WaterParticles.cpp src/game/ui/WaterEmitters.cpp src/game/ui/Sprinkle.cpp)
    add_executable(progress-sprite-reference-test tools/progress_sprite_reference_test.cpp)
    target_link_libraries(progress-sprite-reference-test PRIVATE recovered_resource_utilities)
    target_include_directories(recovered_resource_utilities PUBLIC src/game/resources src/game/ui)
    target_link_libraries(recovered_resource_utilities PUBLIC recovered_graphics_utilities recovered_game_utilities dinput8 dxguid)
    add_executable(texture-test tools/texture_test.cpp)
    target_link_libraries(texture-test PRIVATE recovered_resource_utilities)
    add_executable(resource-manager-test tools/resource_manager_test.cpp)
    target_link_libraries(resource-manager-test PRIVATE recovered_resource_utilities)
    add_executable(pack-decode tools/pack_decode.cpp)
    target_link_libraries(pack-decode PRIVATE recovered_game_utilities)
    option(BUILD_EXPERIMENTAL_XML_ORACLE "Build unfinished fixed-address original parser oracle" OFF)
    if(BUILD_EXPERIMENTAL_XML_ORACLE)
        add_executable(xml-reference-test tools/xml_reference_test.cpp)
        add_library(reference-reservation SHARED tools/reference_reservation.cpp)
        target_link_options(reference-reservation PRIVATE /BASE:0x00400000 /DYNAMICBASE:NO /FIXED /NOENTRY)
        target_link_libraries(xml-reference-test PRIVATE recovered_expat_dependency reference-reservation)
        target_link_options(xml-reference-test PRIVATE /BASE:0x20000000 /DYNAMICBASE:NO)
    endif()
    add_executable(resource-inspect tools/resource_inspect.cpp)
    target_link_libraries(resource-inspect PRIVATE recovered_game_utilities JNGLoad)
    add_executable(file-index-test tools/file_index_test.cpp)
    target_link_libraries(file-index-test PRIVATE recovered_game_utilities)
    add_executable(settings-test tools/settings_test.cpp)
    target_link_libraries(settings-test PRIVATE recovered_game_utilities)
    add_executable(string-reference-test tools/string_reference_test.cpp)
    target_link_libraries(string-reference-test PRIVATE recovered_game_utilities)
    add_executable(wstring-reference-test tools/wstring_reference_test.cpp)
    target_link_libraries(wstring-reference-test PRIVATE recovered_game_utilities)
    add_executable(color-reference-test tools/color_reference_test.cpp)
    target_link_libraries(color-reference-test PRIVATE recovered_game_utilities)
    add_executable(matrix-reference-test tools/matrix_reference_test.cpp)
    target_link_libraries(matrix-reference-test PRIVATE recovered_graphics_utilities)
    add_executable(sprite-geometry-reference-test tools/sprite_geometry_reference_test.cpp)
    target_link_libraries(sprite-geometry-reference-test PRIVATE recovered_graphics_utilities)
    add_executable(sprite-draw-reference-test tools/sprite_draw_reference_test.cpp)
    target_link_libraries(sprite-draw-reference-test PRIVATE recovered_resource_utilities)
    add_executable(native-image-reference-test tools/native_image_reference_test.cpp)
    target_link_libraries(native-image-reference-test PRIVATE recovered_graphics_utilities)
    add_executable(font-reference-test tools/font_reference_test.cpp)
    target_link_libraries(font-reference-test PRIVATE recovered_resource_utilities)
    add_executable(text-layout-reference-test tools/text_layout_reference_test.cpp)
    target_link_libraries(text-layout-reference-test PRIVATE recovered_resource_utilities)
    add_executable(string-manager-reference-test tools/string_manager_reference_test.cpp)
    target_link_libraries(string-manager-reference-test PRIVATE recovered_resource_utilities)
    add_executable(control-sprite-reference-test tools/control_sprite_reference_test.cpp)
    target_link_libraries(control-sprite-reference-test PRIVATE recovered_resource_utilities)
    add_executable(view-reference-test tools/view_reference_test.cpp)
    target_link_libraries(view-reference-test PRIVATE recovered_resource_utilities)
    add_executable(ui-frame-reference-test tools/ui_frame_reference_test.cpp)
    target_link_libraries(ui-frame-reference-test PRIVATE recovered_resource_utilities)
    add_executable(sound-manager-reference-test tools/sound_manager_reference_test.cpp)
    target_link_libraries(sound-manager-reference-test PRIVATE recovered_resource_utilities)
    add_executable(timer-reference-test tools/timer_reference_test.cpp)
    target_link_libraries(timer-reference-test PRIVATE recovered_game_utilities)
    add_executable(cursor-reference-test tools/cursor_reference_test.cpp)
    target_link_libraries(cursor-reference-test PRIVATE recovered_graphics_utilities)
    add_executable(profile-headers-reference-test tools/profile_headers_reference_test.cpp)
    target_link_libraries(profile-headers-reference-test PRIVATE recovered_game_utilities)
    target_compile_options(profile-headers-reference-test PRIVATE /utf-8)
    add_executable(profile-properties-reference-test tools/profile_properties_reference_test.cpp)
    target_link_libraries(profile-properties-reference-test PRIVATE recovered_game_utilities)
    target_compile_options(profile-properties-reference-test PRIVATE /utf-8)
    add_executable(profile-lifecycle-reference-test tools/profile_lifecycle_reference_test.cpp)
    target_link_libraries(profile-lifecycle-reference-test PRIVATE recovered_game_utilities)
    target_compile_options(profile-lifecycle-reference-test PRIVATE /utf-8)
    add_executable(control-reference-test tools/control_reference_test.cpp)
    target_link_libraries(control-reference-test PRIVATE recovered_resource_utilities)
    add_executable(text-control-reference-test tools/text_control_reference_test.cpp)
    target_link_libraries(text-control-reference-test PRIVATE recovered_resource_utilities)
    add_executable(button-reference-test tools/button_reference_test.cpp)
    target_link_libraries(button-reference-test PRIVATE recovered_resource_utilities)
    add_executable(button-text-reference-test tools/button_text_reference_test.cpp)
    target_link_libraries(button-text-reference-test PRIVATE recovered_resource_utilities)
    add_executable(text-buttons-reference-test tools/text_buttons_reference_test.cpp)
    target_link_libraries(text-buttons-reference-test PRIVATE recovered_resource_utilities)
    if(MSVC)
        # The original EXE strips relocations and prefers 00400000. Keep this
        # diagnostic image away from that range even when ASLR is disabled by
        # the host environment. This changes no production target.
        target_link_options(text-buttons-reference-test PRIVATE /BASE:0x20000000 /DYNAMICBASE:NO)
    endif()
    add_executable(jpeg-reference-test tools/jpeg_reference_test.cpp)
    target_link_libraries(jpeg-reference-test PRIVATE recovered_resource_utilities)
    add_executable(click-mask-reference-test tools/click_mask_reference_test.cpp)
    target_link_libraries(click-mask-reference-test PRIVATE recovered_resource_utilities)
    add_executable(input-bindings-reference-test tools/input_bindings_reference_test.cpp)
    target_link_libraries(input-bindings-reference-test PRIVATE recovered_resource_utilities)
endif()
add_library(recovered_squall_utilities STATIC src/squall/VolumeTable.cpp)
target_include_directories(recovered_squall_utilities PUBLIC src/squall)
if(WIN32)
    target_sources(recovered_squall_utilities PRIVATE src/squall/Parameters.cpp)
    target_link_libraries(recovered_squall_utilities PUBLIC user32)
    add_executable(squall-parameters-reference-test tools/squall_parameters_reference_test.cpp)
    target_link_libraries(squall-parameters-reference-test PRIVATE recovered_squall_utilities)
    option(BUILD_SQUALL_CANDIDATE "Enable the full Squall runtime native-audit target" OFF)
    if(BUILD_SQUALL_CANDIDATE)
        include(tools/cmake/SquallCandidate.cmake)
    endif()
    option(BUILD_SQUALL_REFERENCE_LAYOUT "Build the pinned public Squall layout probe (not the runtime)" OFF)
    if(BUILD_SQUALL_REFERENCE_LAYOUT)
        execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/inspect_squall.py"
            RESULT_VARIABLE SQUALL_REFERENCE_VERIFY_RESULT)
        if(NOT SQUALL_REFERENCE_VERIFY_RESULT EQUAL 0)
            message(FATAL_ERROR "Pinned Squall reference verification failed")
        endif()
        add_executable(squall-reference-layout EXCLUDE_FROM_ALL tools/squall_reference_layout.cpp)
        target_compile_definitions(squall-reference-layout PRIVATE _USRDLL)
        target_include_directories(squall-reference-layout PRIVATE .local/squall-source/source .local/squall-source/external/dshow)
    endif()
endif()
include(CTest)
if(BUILD_TESTING)
    if(WIN32)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../farmář/Dobrý farmář/Squall.dll")
            add_test(NAME original-squall-parameters COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_squall_parameters.py"
                --oracle "$<TARGET_FILE:squall-parameters-reference-test>")
        endif()
        add_test(NAME recovered-jng-export-contract COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_exports.py" --binary "$<TARGET_FILE:JNGLoad>"
            --original-name JNGLoad.dll --output "${CMAKE_CURRENT_SOURCE_DIR}/.local/jngload-ctest-abi.json")
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../farmář/Dobrý farmář/Dobrý farmář.exe")
            add_test(NAME original-string-comparison COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_string.py" --oracle "$<TARGET_FILE:string-reference-test>")
            add_test(NAME original-wstring-comparison COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_string.py" --wide --oracle "$<TARGET_FILE:wstring-reference-test>")
            add_test(NAME original-color-comparison COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_color.py" --oracle "$<TARGET_FILE:color-reference-test>")
            add_test(NAME original-matrix-comparison COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_matrix.py" --oracle "$<TARGET_FILE:matrix-reference-test>")
            add_test(NAME original-sprite-geometry COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_sprite_geometry.py" --oracle "$<TARGET_FILE:sprite-geometry-reference-test>")
            add_test(NAME original-jpeg COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_jpeg.py" --oracle "$<TARGET_FILE:jpeg-reference-test>")
            add_test(NAME original-native-image-storage COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_native_images.py" --oracle "$<TARGET_FILE:native-image-reference-test>")
            add_test(NAME original-font-lookups COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_fonts.py" --oracle "$<TARGET_FILE:font-reference-test>")
            add_test(NAME original-text-layout COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_text_layout.py" --oracle "$<TARGET_FILE:text-layout-reference-test>")
            add_test(NAME original-localization COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_string_manager.py" --oracle "$<TARGET_FILE:string-manager-reference-test>")
            add_test(NAME original-control-sprites COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_control_sprites.py" --oracle "$<TARGET_FILE:control-sprite-reference-test>")
            add_test(NAME original-progress-bar COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_progress.py" --sprite-oracle "$<TARGET_FILE:progress-sprite-reference-test>" --ui-oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-market-histogram COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_histogram.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-loading-from-log COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_loading.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-award-tooltip COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_tooltip.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-award-effect COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_award_effect.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-awards-controller COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_awards.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-award-panel COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_award_panel.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-resource-log-writer COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_resource_log.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-new-user COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_new_user.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-main-menu-navigation COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_main_menu.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-game-configuration COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_game_configuration.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-gameplay-definitions COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_gameplay.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-gameplay-runtime COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_gameplay_runtime.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-price-money-text COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_price_text.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-state-view COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_state_view.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-game-menu COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_game_menu.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-tutorial-tips COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_tutorial_tips.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-tutorial-manager COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_tutorial_manager.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-movable-union COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_movable_union.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-buy-seeds COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_buy_seeds.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-buy-building COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_buy_building.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-production-houses COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_production_houses.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-manufacture COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_manufacture.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-field-object COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_field_object.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-action-area COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_action_area.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-warehouse COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_warehouse.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-warehouse-tip COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_warehouse_tip.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-dig-building COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_dig_building.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-water-source COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_water_source.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-mow-building COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_mow_building.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-water-particles COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_water_effects.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-water-emitters COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_water_effects.py" --emitters --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-sprinkle COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_sprinkle.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-farm-field COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_farm_field.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-seed-package COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_seed_package.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-person-tasks COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_person_tasks.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            if(TARGET person-sounds-reference-test)
                add_test(NAME original-person-sounds COMMAND "${Python3_EXECUTABLE}"
                    "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_person_sounds.py" --oracle "$<TARGET_FILE:person-sounds-reference-test>")
            endif()
            add_test(NAME original-elastic-animation COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_elastic_animation.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-person-controller COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_person_controller.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-person-flows COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_person_flows.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-contract-letter COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_contract_letter.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-game-award-manager COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_game_award_manager.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-journal COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_journal.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-help-view COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_help_view.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-farm-scene COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_farm_scene.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-person-vehicle COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_person_vehicle.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-person-walk COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_person_walk.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-person-visuals COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_person_visuals.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-market-controller COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_market.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-sales-history COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_sales_history.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-flying-text COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_flying_text.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-flying-market COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_flying_text.py" --connected --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-game-pause COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_game_pause.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-contract-provide COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_contract_provide.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-contract-dialogs COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_contract_dialogs.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-contract-status COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_contract_status.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-upgrade-state COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_upgrade.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-extended-button-tooltips COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_extended_buttons.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-view-hierarchy COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_views.py" --oracle "$<TARGET_FILE:view-reference-test>")
            add_test(NAME original-main-frame-ui COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_views.py" --frame --oracle "$<TARGET_FILE:ui-frame-reference-test>")
            add_test(NAME original-sound-files-volume COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_sound_manager.py" --oracle "$<TARGET_FILE:sound-manager-reference-test>")
            add_test(NAME original-timer COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_timer.py" --oracle "$<TARGET_FILE:timer-reference-test>")
            add_test(NAME original-cursor COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_cursor.py" --oracle "$<TARGET_FILE:cursor-reference-test>")
            add_test(NAME original-profile-headers COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_profile_headers.py" --oracle "$<TARGET_FILE:profile-headers-reference-test>")
            add_test(NAME original-profile-properties COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_profile_properties.py" --oracle "$<TARGET_FILE:profile-properties-reference-test>")
            add_test(NAME original-profile-lifecycle COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_profile_lifecycle.py" --oracle "$<TARGET_FILE:profile-lifecycle-reference-test>")
            add_test(NAME original-control-state COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_controls.py" --oracle "$<TARGET_FILE:control-reference-test>")
            add_test(NAME original-text-control-lifecycle COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_text_controls.py" --oracle "$<TARGET_FILE:text-control-reference-test>")
            add_test(NAME original-button-state COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_buttons.py" --oracle "$<TARGET_FILE:button-reference-test>")
            add_test(NAME original-button-text-motion COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_button_text.py" --oracle "$<TARGET_FILE:button-text-reference-test>")
            add_test(NAME original-owned-text-buttons COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_text_buttons.py" --oracle "$<TARGET_FILE:text-buttons-reference-test>")
            add_test(NAME original-click-masks COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_click_masks.py" --oracle "$<TARGET_FILE:click-mask-reference-test>")
            add_test(NAME original-input-bindings-dispatch COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_input_bindings.py" --oracle "$<TARGET_FILE:input-bindings-reference-test>")
        endif()
        add_executable(real-file-test tests/real_file_test.cpp)
        target_link_libraries(real-file-test PRIVATE recovered_game_utilities)
        add_test(NAME recovered-real-file COMMAND real-file-test)
        add_executable(wstring-test tests/wstring_test.cpp)
        target_link_libraries(wstring-test PRIVATE recovered_game_utilities)
        add_test(NAME recovered-wstring COMMAND wstring-test)
        add_test(NAME recovered-resource-pipeline COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_resources.py" --loader "$<TARGET_FILE:resource-inspect>"
            --file-index-test "$<TARGET_FILE:file-index-test>"
            --fixtures-only --output "${CMAKE_CURRENT_SOURCE_DIR}/.local/resource-ctest.json")
        add_test(NAME recovered-settings COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_settings.py" --loader "$<TARGET_FILE:resource-inspect>"
            --test "$<TARGET_FILE:settings-test>")
        add_test(NAME recovered-resource-manager COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_resource_manager.py" --test "$<TARGET_FILE:resource-manager-test>")
        set(RECOVERED_GAME_ICON "${CMAKE_CURRENT_SOURCE_DIR}/.local/game-artwork/game.ico" CACHE FILEPATH "Original icon artwork extracted without executable code")
        if(EXISTS "${RECOVERED_GAME_ICON}")
            enable_language(RC)
            configure_file("${RECOVERED_GAME_ICON}" "${CMAKE_CURRENT_BINARY_DIR}/game.ico" COPYONLY)
            file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/window-artwork.rc" "101 ICON \"game.ico\"\n")
            add_executable(window-test tools/window_test.cpp "${CMAKE_CURRENT_BINARY_DIR}/window-artwork.rc")
            target_link_libraries(window-test PRIVATE recovered_game_utilities)
            add_executable(graphics-presentation-test tools/graphics_presentation_test.cpp "${CMAKE_CURRENT_BINARY_DIR}/window-artwork.rc")
            target_link_libraries(graphics-presentation-test PRIVATE recovered_resource_utilities)
            add_test(NAME recovered-window COMMAND "${Python3_EXECUTABLE}"
                "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_window.py" --test "$<TARGET_FILE:window-test>")
        else()
            message(STATUS "Window diagnostic needs icon artwork: python tools/extract_game_icon.py")
        endif()
    endif()
    add_executable(string-test tests/string_test.cpp)
    target_link_libraries(string-test PRIVATE recovered_game_utilities)
    add_test(NAME recovered-string COMMAND string-test)
    add_executable(path-names-test tests/path_names_test.cpp)
    target_link_libraries(path-names-test PRIVATE recovered_game_utilities)
    add_test(NAME recovered-path-names COMMAND path-names-test)
    add_executable(squall-volume-test tests/squall_volume_test.cpp)
    target_link_libraries(squall-volume-test PRIVATE recovered_squall_utilities)
    add_test(NAME recovered-squall-volume COMMAND squall-volume-test)
    add_test(NAME reconstruction-tools COMMAND "${Python3_EXECUTABLE}" -m unittest discover -s tests -v)
    set_tests_properties(reconstruction-tools PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        ENVIRONMENT "DF_JNG_DECODER=$<TARGET_FILE:jng-decode>")
endif()
