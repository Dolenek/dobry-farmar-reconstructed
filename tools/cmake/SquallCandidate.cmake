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
