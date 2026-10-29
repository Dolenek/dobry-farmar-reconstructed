# The full editable runtime is an explicit audit target until all native
# comparisons pass. No forwarding to or linkage with original program code.
execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/fetch_squall_codecs.py"
    --verify-only RESULT_VARIABLE SQUALL_CODECS_VERIFY_RESULT)
if(NOT SQUALL_CODECS_VERIFY_RESULT EQUAL 0)
    message(FATAL_ERROR "Run python tools/fetch_squall_codecs.py before building SquallCandidate")
endif()
set(SQUALL_VORBIS_CANDIDATE "1.1.0" CACHE STRING "Historical candidate decoder; original version remains unproved")
if(NOT SQUALL_VORBIS_CANDIDATE STREQUAL "1.1.0" AND NOT SQUALL_VORBIS_CANDIDATE STREQUAL "1.0.1")
    message(FATAL_ERROR "Only hash-pinned Vorbis 1.1.0 and 1.0.1 candidates are available")
endif()
set(SQUALL_OGG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.local/libogg-1.1")
set(SQUALL_VORBIS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.local/libvorbis-${SQUALL_VORBIS_CANDIDATE}")
add_library(squall_ogg_candidate STATIC EXCLUDE_FROM_ALL "${SQUALL_OGG_DIR}/src/bitwise.c" "${SQUALL_OGG_DIR}/src/framing.c")
target_include_directories(squall_ogg_candidate PUBLIC "${SQUALL_OGG_DIR}/include")
# Original upstream lib/Makefile.am library list; the standalone diagnostic
# programs and encoder convenience library are not linked into the decoder.
set(SQUALL_VORBIS_FILES smallft block envelope window lsp lpc analysis synthesis psy info floor1 floor0
    res0 mapping0 registry codebook sharedbook lookup bitrate vorbisfile)
set(SQUALL_VORBIS_SOURCES)
foreach(NAME IN LISTS SQUALL_VORBIS_FILES)
    list(APPEND SQUALL_VORBIS_SOURCES "${SQUALL_VORBIS_DIR}/lib/${NAME}.c")
endforeach()
add_library(squall_vorbis_candidate STATIC EXCLUDE_FROM_ALL ${SQUALL_VORBIS_SOURCES} "${CMAKE_CURRENT_SOURCE_DIR}/src/squall/codecs/vorbis/mdct.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/squall/codecs/vorbis/NativeButterfly.c")
set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/src/squall/codecs/vorbis/NativeButterfly.c" PROPERTIES COMPILE_OPTIONS "/fp:strict")
target_include_directories(squall_vorbis_candidate PUBLIC "${SQUALL_VORBIS_DIR}/include" PRIVATE "${SQUALL_VORBIS_DIR}/lib")
target_link_libraries(squall_vorbis_candidate PUBLIC squall_ogg_candidate)
set(SQUALL_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/squall/runtime")
set(SQUALL_RUNTIME_FILES SoundEngine/Channels SoundEngine/Context SoundEngine/DirectSound SoundEngine/HiddenWindow
    SoundEngine/Squall SoundEngine/SquallApi SoundEngine/ext/Eax SoundEngine/ext/ZoomFX SoundFile/Reader SoundFile/SoundFile
    SoundFile/WAV/WavFile SoundFile/WAV/PcmDecoder SoundFile/WAV/MsAdpcmDecoder SoundFile/WAV/ImaAdpcmDecoder
    SoundFile/MP3/MpegFile SoundFile/MP3/MpegDecoder SoundFile/MP3/l1dec SoundFile/MP3/l2dec SoundFile/MP3/l3dec
    SoundFile/Ogg/OggFile SoundFile/Ogg/OggDecoder SoundFile/Wma/WmaFile SoundFile/Wma/WmaDecoder)
set(SQUALL_RUNTIME_SOURCES)
foreach(NAME IN LISTS SQUALL_RUNTIME_FILES)
    list(APPEND SQUALL_RUNTIME_SOURCES "${SQUALL_RUNTIME_DIR}/${NAME}.cpp")
endforeach()
file(GLOB SQUALL_TRACK_SOURCES CONFIGURE_DEPENDS "${SQUALL_RUNTIME_DIR}/SoundFile/track/*.cpp")
list(APPEND SQUALL_RUNTIME_SOURCES ${SQUALL_TRACK_SOURCES})
add_library(squall_runtime_candidate STATIC EXCLUDE_FROM_ALL ${SQUALL_RUNTIME_SOURCES} "${SQUALL_RUNTIME_DIR}/Abi.cpp")
target_compile_definitions(squall_runtime_candidate PUBLIC _USRDLL PRIVATE _WINDOWS NDEBUG SQUALL_DEBUG=0 _CRT_SECURE_NO_WARNINGS)
# Use the installed SDK dsound.h: the bundled ext/dsound.h is an older header
# without the algorithm GUID field used by the actual source and native DLL.
target_include_directories(squall_runtime_candidate PUBLIC "${SQUALL_RUNTIME_DIR}" PRIVATE "${SQUALL_RUNTIME_DIR}/SoundFile")
target_link_libraries(squall_runtime_candidate PUBLIC recovered_squall_utilities squall_vorbis_candidate PRIVATE dsound dxguid winmm ole32 user32 strmiids)
add_library(SquallCandidate SHARED EXCLUDE_FROM_ALL "${SQUALL_RUNTIME_DIR}/Abi.cpp" "${SQUALL_RUNTIME_DIR}/Squall.def")
# Visual Studio skips Link with no object inputs. Compile the actual ABI proof
# into the DLL too, so changes in the static runtime reliably relink the DLL.
target_compile_options(SquallCandidate PRIVATE /utf-8)
set_target_properties(SquallCandidate PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(SquallCandidate PRIVATE squall_runtime_candidate)
target_link_options(SquallCandidate PRIVATE "/MAP:$<TARGET_FILE_DIR:SquallCandidate>/Squall.map")
set_target_properties(SquallCandidate PROPERTIES OUTPUT_NAME Squall
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/squall-candidate"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/squall-candidate")
foreach(TARGET squall_ogg_candidate squall_vorbis_candidate squall_runtime_candidate)
    target_compile_options(${TARGET} PRIVATE /arch:IA32 /fp:precise)
    target_compile_definitions(${TARGET} PRIVATE _CRT_SECURE_NO_WARNINGS)
endforeach()
# Audit the historical x87 evaluation policy separately from the release tag.
set(SQUALL_VORBIS_FP "fast" CACHE STRING "Candidate Vorbis x87 evaluation policy")
if(NOT SQUALL_VORBIS_FP MATCHES "^(precise|fast|strict)$")
    message(FATAL_ERROR "Unknown Vorbis floating point policy")
endif()
target_compile_options(squall_vorbis_candidate PRIVATE /fp:${SQUALL_VORBIS_FP})
target_compile_options(squall_runtime_candidate PRIVATE /utf-8)
add_executable(squall-runtime-reference-test EXCLUDE_FROM_ALL "${CMAKE_CURRENT_SOURCE_DIR}/tools/squall_runtime_reference_test.cpp")
target_link_libraries(squall-runtime-reference-test PRIVATE squall_runtime_candidate recovered_game_utilities)
add_executable(squall-vorbis-reference-test EXCLUDE_FROM_ALL "${CMAKE_CURRENT_SOURCE_DIR}/tools/squall_vorbis_reference_test.cpp")
target_include_directories(squall-vorbis-reference-test PRIVATE "${SQUALL_VORBIS_DIR}/lib")
target_link_libraries(squall-vorbis-reference-test PRIVATE squall_vorbis_candidate)
add_executable(squall-dll-reference-test EXCLUDE_FROM_ALL "${CMAKE_CURRENT_SOURCE_DIR}/tools/squall_dll_reference_test.cpp")
target_compile_definitions(squall-dll-reference-test PRIVATE _USRDLL)
target_compile_options(squall-dll-reference-test PRIVATE /utf-8)
target_include_directories(squall-dll-reference-test PRIVATE "${SQUALL_RUNTIME_DIR}")
target_link_libraries(squall-dll-reference-test PRIVATE recovered_game_utilities)
add_dependencies(squall-dll-reference-test SquallCandidate)
add_executable(squall-groups-reference-test EXCLUDE_FROM_ALL "${CMAKE_CURRENT_SOURCE_DIR}/tools/squall_groups_reference_test.cpp")
target_compile_definitions(squall-groups-reference-test PRIVATE _USRDLL)
target_compile_options(squall-groups-reference-test PRIVATE /utf-8)
target_include_directories(squall-groups-reference-test PRIVATE "${SQUALL_RUNTIME_DIR}")
target_link_libraries(squall-groups-reference-test PRIVATE recovered_game_utilities)
add_dependencies(squall-groups-reference-test SquallCandidate)
add_executable(squall-channel-ranges-reference-test EXCLUDE_FROM_ALL "${CMAKE_CURRENT_SOURCE_DIR}/tools/squall_channel_ranges_reference_test.cpp")
target_compile_definitions(squall-channel-ranges-reference-test PRIVATE _USRDLL)
target_compile_options(squall-channel-ranges-reference-test PRIVATE /utf-8)
target_include_directories(squall-channel-ranges-reference-test PRIVATE "${SQUALL_RUNTIME_DIR}")
target_link_libraries(squall-channel-ranges-reference-test PRIVATE recovered_game_utilities)
add_dependencies(squall-channel-ranges-reference-test SquallCandidate)
add_executable(squall-listener-factors-reference-test EXCLUDE_FROM_ALL "${CMAKE_CURRENT_SOURCE_DIR}/tools/squall_listener_factors_reference_test.cpp")
target_compile_definitions(squall-listener-factors-reference-test PRIVATE _USRDLL)
target_compile_options(squall-listener-factors-reference-test PRIVATE /utf-8)
target_include_directories(squall-listener-factors-reference-test PRIVATE "${SQUALL_RUNTIME_DIR}")
add_dependencies(squall-listener-factors-reference-test SquallCandidate)
add_library(recovered_game_audio STATIC EXCLUDE_FROM_ALL "${CMAKE_CURRENT_SOURCE_DIR}/src/game/audio/SquallSoundApi.cpp")
add_executable(game-source-runtime-test "${CMAKE_CURRENT_SOURCE_DIR}/tools/game_source_runtime_test.cpp" "${CMAKE_CURRENT_BINARY_DIR}/window-artwork.rc")
target_link_libraries(game-source-runtime-test PRIVATE recovered_game_audio)
target_compile_options(game-source-runtime-test PRIVATE /utf-8)
target_link_options(game-source-runtime-test PRIVATE "/MAP:$<TARGET_FILE_DIR:game-source-runtime-test>/game-source-runtime-test.map" /DYNAMICBASE:NO)
add_custom_command(TARGET game-source-runtime-test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SquallCandidate>" "$<TARGET_FILE_DIR:game-source-runtime-test>/Squall.dll")
add_test(NAME recovered-game-source-world COMMAND "${Python3_EXECUTABLE}"
    "${CMAKE_CURRENT_SOURCE_DIR}/tools/validate_game_source_world.py" --test "$<TARGET_FILE:game-source-runtime-test>")
target_compile_definitions(recovered_game_audio PRIVATE _USRDLL)
target_compile_options(recovered_game_audio PRIVATE /utf-8)
target_link_libraries(recovered_game_audio PUBLIC recovered_resource_utilities SquallCandidate)
add_executable(game-audio-reference-test EXCLUDE_FROM_ALL "${CMAKE_CURRENT_SOURCE_DIR}/tools/game_audio_reference_test.cpp")
target_compile_definitions(game-audio-reference-test PRIVATE _USRDLL)
target_compile_options(game-audio-reference-test PRIVATE /utf-8)
target_link_libraries(game-audio-reference-test PRIVATE recovered_game_audio)
add_custom_command(TARGET game-audio-reference-test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SquallCandidate>" "$<TARGET_FILE_DIR:game-audio-reference-test>/Squall.dll")

add_executable(person-sounds-reference-test EXCLUDE_FROM_ALL "${CMAKE_CURRENT_SOURCE_DIR}/tools/person_sounds_reference_test.cpp")
target_compile_definitions(person-sounds-reference-test PRIVATE _USRDLL)
target_compile_options(person-sounds-reference-test PRIVATE /utf-8)
target_link_libraries(person-sounds-reference-test PRIVATE recovered_game_audio)
add_custom_command(TARGET person-sounds-reference-test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SquallCandidate>" "$<TARGET_FILE_DIR:person-sounds-reference-test>/Squall.dll")

# Actual source application. Missing runtime behavior remains explicitly audited
# in docs; this target never loads the original game EXE or original DLLs.
add_executable(DobryFarmar WIN32 "${CMAKE_CURRENT_SOURCE_DIR}/src/game/main.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/game/engine/GameApplication.cpp" "${CMAKE_CURRENT_BINARY_DIR}/window-artwork.rc")
set_target_properties(DobryFarmar PROPERTIES OUTPUT_NAME "Dobrý farmář")
target_link_libraries(DobryFarmar PRIVATE recovered_game_audio shlwapi shell32)
target_compile_options(DobryFarmar PRIVATE /utf-8)
# Retain private symbol addresses for read-only external runtime diagnostics.
# The map is a build artifact; production code and ASLR remain unchanged.
target_link_options(DobryFarmar PRIVATE "/MAP:$<TARGET_FILE_DIR:DobryFarmar>/DobryFarmar.map")
add_executable(game-runtime-layout "${CMAKE_CURRENT_SOURCE_DIR}/tools/game_runtime_layout.cpp")
target_link_libraries(game-runtime-layout PRIVATE recovered_game_audio)
target_compile_options(game-runtime-layout PRIVATE /utf-8)
add_custom_command(TARGET DobryFarmar POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SquallCandidate>" "$<TARGET_FILE_DIR:DobryFarmar>/Squall.dll")
