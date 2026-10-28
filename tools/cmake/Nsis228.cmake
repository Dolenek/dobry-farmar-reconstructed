# Exact upstream NSIS2.28 compiler, runtime, Modern UI and LangDLL built from source.
set(NSIS_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.local/nsis-2.28-source" CACHE PATH "Hash-pinned NSIS2.28 sources")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/fetch_nsis.py" --verify-only --source-dir "${NSIS_SOURCE_DIR}" RESULT_VARIABLE NSIS_VERIFY)
if(NOT NSIS_VERIFY EQUAL 0)
 message(FATAL_ERROR "Fetch exact source first: python tools/fetch_nsis.py")
endif()
enable_language(RC)
set(NSIS_GENERATED "${CMAKE_CURRENT_BINARY_DIR}/nsis228")
execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/prepare_nsis_toolchain.py" --generated "${NSIS_GENERATED}" RESULT_VARIABLE NSIS_PREPARE)
if(NOT NSIS_PREPARE EQUAL 0)
 message(FATAL_ERROR "NSIS2.28 configuration generation failed")
endif()
set(NSIS_SRC "${NSIS_SOURCE_DIR}/Source")
set(NSIS_INC "${NSIS_GENERATED}/include")
set(NSIS_COMPILER_FILES build.cpp clzma.cpp crc32.c DialogTemplate.cpp dirreader.cpp fileform.cpp growbuf.cpp lang.cpp lineparse.cpp makenssi.cpp manifest.cpp mmap.cpp Plugins.cpp ResourceEditor.cpp ResourceVersionInfo.cpp script.cpp ShConstants.cpp strlist.cpp tokens.cpp util.cpp winchar.cpp writer.cpp bzip2/blocksort.c bzip2/bzlib.c bzip2/compress.c bzip2/huffman.c 7zip/7zGuids.cpp 7zip/7zip/Common/OutBuffer.cpp 7zip/7zip/Common/StreamUtils.cpp 7zip/7zip/Compress/LZ/LZInWindow.cpp 7zip/7zip/Compress/LZMA/LZMAEncoder.cpp 7zip/7zip/Compress/RangeCoder/RangeCoderBit.cpp 7zip/Common/Alloc.cpp 7zip/Common/CRC.cpp zlib/deflate.c zlib/trees.c)
list(TRANSFORM NSIS_COMPILER_FILES PREPEND "${NSIS_SRC}/")
add_executable(nsis228-compiler ${NSIS_COMPILER_FILES})
set_target_properties(nsis228-compiler PROPERTIES CXX_STANDARD 14 MSVC_RUNTIME_LIBRARY MultiThreaded)
target_include_directories(nsis228-compiler PRIVATE "${NSIS_INC}" "${NSIS_SRC}")
target_compile_definitions(nsis228-compiler PRIVATE _WIN32_IE=0x0500 COMPRESS_MF_BT _CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS _CRT_NON_CONFORMING_SWPRINTFS)
target_compile_options(nsis228-compiler PRIVATE /Zc:strictStrings- /Zc:forScope- /permissive /Brepro)
target_link_options(nsis228-compiler PRIVATE /Brepro /INCREMENTAL:NO)
target_link_libraries(nsis228-compiler PRIVATE gdi32 user32 version comctl32)
set(NSIS_HEAD_FILES bgbg.c components.c exec.c fileform.c Main.c Ui.c util.c resource.rc)
list(TRANSFORM NSIS_HEAD_FILES PREPEND "${NSIS_SRC}/exehead/")
set(NSIS_MEMSET "${NSIS_SOURCE_DIR}/SCons/Config/memset.c")
add_executable(nsis228-lzma-solid WIN32 ${NSIS_HEAD_FILES} "${NSIS_SRC}/crc32.c" "${NSIS_SRC}/7zip/LZMADecode.c" "${NSIS_MEMSET}")
add_executable(nsis228-zlib-head WIN32 ${NSIS_HEAD_FILES} "${NSIS_SRC}/crc32.c" "${NSIS_SRC}/zlib/INFBLOCK.C" "${NSIS_MEMSET}")
set_source_files_properties("${NSIS_SRC}/zlib/INFBLOCK.C" PROPERTIES LANGUAGE C)
foreach(target IN ITEMS nsis228-lzma-solid nsis228-zlib-head)
 target_include_directories(${target} PRIVATE "${NSIS_INC}" "${NSIS_SRC}" "${NSIS_SRC}/exehead")
 target_compile_definitions(${target} PRIVATE EXEHEAD WIN32_LEAN_AND_MEAN _WIN32_IE=0x0500 _CRT_SECURE_NO_WARNINGS)
 target_compile_options(${target} PRIVATE /GS- /Zl /O1 /Oi- /Brepro)
 target_link_options(${target} PRIVATE /NODEFAULTLIB /ENTRY:WinMain /INCREMENTAL:NO /SAFESEH:NO /DYNAMICBASE:NO /FIXED /NXCOMPAT:NO /MANIFEST:NO /Brepro)
 target_link_libraries(${target} PRIVATE kernel32 user32 gdi32 shell32 advapi32 comdlg32 comctl32 ole32 version uuid)
endforeach()
target_compile_definitions(nsis228-lzma-solid PRIVATE NSIS_COMPRESS_USE_LZMA NSIS_COMPRESS_WHOLE LZMACALL=__fastcall)
target_compile_definitions(nsis228-zlib-head PRIVATE NSIS_COMPRESS_USE_ZLIB ZEXPORT=__stdcall)
add_library(nsis228-LangDLL SHARED "${NSIS_SOURCE_DIR}/Contrib/LangDLL/LangDLL.c" "${NSIS_SOURCE_DIR}/Contrib/LangDLL/resource.rc" "${NSIS_MEMSET}")
target_include_directories(nsis228-LangDLL PRIVATE "${NSIS_INC}")
target_compile_options(nsis228-LangDLL PRIVATE /GS- /Zl /O1 /Oi- /Brepro)
target_link_options(nsis228-LangDLL PRIVATE /NODEFAULTLIB /ENTRY:DllMain /INCREMENTAL:NO /SAFESEH:NO /MANIFEST:NO /Brepro)
target_link_libraries(nsis228-LangDLL PRIVATE kernel32 user32 gdi32)
foreach(kind IN ITEMS modern modern_headerbmp)
 add_executable(nsis228-${kind} WIN32 "${NSIS_SOURCE_DIR}/Contrib/UIs/ui.c" "${NSIS_SOURCE_DIR}/Contrib/UIs/${kind}.rc")
 target_include_directories(nsis228-${kind} PRIVATE "${NSIS_INC}")
 target_compile_options(nsis228-${kind} PRIVATE /GS- /Zl /O1 /Oi- /Brepro)
 target_link_options(nsis228-${kind} PRIVATE /NODEFAULTLIB /ENTRY:WinMain /INCREMENTAL:NO /SAFESEH:NO /MANIFEST:NO /Brepro)
 target_link_libraries(nsis228-${kind} PRIVATE kernel32 user32 gdi32 comctl32)
endforeach()
file(MAKE_DIRECTORY "${NSIS_GENERATED}/application")
set(NSIS_FINAL "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/Uninstall.exe")
add_custom_command(OUTPUT "${NSIS_FINAL}"
 COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/prepare_nsis_toolchain.py"
  --generated "${NSIS_GENERATED}" --source "${NSIS_SOURCE_DIR}" --stage
  --compiler "$<TARGET_FILE:nsis228-compiler>" --lzma "$<TARGET_FILE:nsis228-lzma-solid>" --zlib "$<TARGET_FILE:nsis228-zlib-head>"
  --langdll "$<TARGET_FILE:nsis228-LangDLL>" --modern "$<TARGET_FILE:nsis228-modern>" --modern-header "$<TARGET_FILE:nsis228-modern_headerbmp>"
 COMMAND "${CMAKE_COMMAND}" -E env "NSISDIR=${NSIS_GENERATED}/distribution" "$<TARGET_FILE:nsis228-compiler>" /V3 "${NSIS_GENERATED}/application/reconstructed_uninstall.nsi"
 COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/emit_nsis_uninstaller.py"
  --builder "${NSIS_GENERATED}/application/UninstallSourceBuilder.exe" --output "${NSIS_FINAL}" --metadata "${NSIS_GENERATED}/emission-$<CONFIG>.json"
 DEPENDS nsis228-compiler nsis228-lzma-solid nsis228-zlib-head nsis228-LangDLL nsis228-modern nsis228-modern_headerbmp
  "${CMAKE_CURRENT_SOURCE_DIR}/src/uninstall/reconstructed_uninstall.nsi" "${CMAKE_CURRENT_SOURCE_DIR}/src/uninstall/original_removal.nsh"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/uninstall/special_offer_page.ini" "${CMAKE_CURRENT_SOURCE_DIR}/src/uninstall/uninstall.ico"
  "${CMAKE_CURRENT_SOURCE_DIR}/tools/prepare_nsis_toolchain.py" "${CMAKE_CURRENT_SOURCE_DIR}/tools/emit_nsis_uninstaller.py"
  "${CMAKE_CURRENT_SOURCE_DIR}/tools/nsis_solid.py" "${CMAKE_CURRENT_SOURCE_DIR}/tools/nsis_extract.py"
  "${CMAKE_CURRENT_SOURCE_DIR}/analysis/nsis-build-config.json"
 COMMENT "Build source Uninstall.exe with exact NSIS2.28 and offline WriteUninstaller emission"
 WORKING_DIRECTORY "${NSIS_GENERATED}/application" VERBATIM)
add_custom_target(FarmarUninstall ALL DEPENDS "${NSIS_FINAL}")
