# Library source lists from JPEG 6b makefile.vc and zlib 1.2.3 Makefile.
execute_process(COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/fetch_codecs.py"
    --verify-only RESULT_VARIABLE CODECS_VERIFY_RESULT)
if(NOT CODECS_VERIFY_RESULT EQUAL 0)
    message(FATAL_ERROR "Run python tools/fetch_codecs.py before configuring")
endif()
set(RECOVERED_JPEG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.local/jpeg-6b")
set(RECOVERED_ZLIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.local/zlib-1.2.3")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/jpeg-config")
# The original DLL used the Microsoft configuration, including byte-sized boolean.
configure_file("${RECOVERED_JPEG_DIR}/jconfig.vc" "${CMAKE_CURRENT_BINARY_DIR}/jpeg-config/jconfig.h" COPYONLY)
set(RECOVERED_JPEG_FILES
    jcomapi jutils jerror jmemmgr jmemnobs
    jcapimin jcapistd jctrans jcparam jdatadst jcinit jcmaster jcmarker jcmainct
    jcprepct jccoefct jccolor jcsample jchuff jcphuff jcdctmgr jfdctfst jfdctflt jfdctint
    jdapimin jdapistd jdtrans jdatasrc jdmaster jdinput jdmarker jdhuff jdphuff
    jdmainct jdcoefct jdpostct jddctmgr jidctfst jidctflt jidctint jidctred
    jdsample jdcolor jquant1 jquant2 jdmerge)
set(RECOVERED_JPEG_SOURCES)
foreach(NAME IN LISTS RECOVERED_JPEG_FILES)
    list(APPEND RECOVERED_JPEG_SOURCES "${RECOVERED_JPEG_DIR}/${NAME}.c")
endforeach()
add_library(original_jpeg STATIC ${RECOVERED_JPEG_SOURCES})
set_target_properties(original_jpeg PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(original_jpeg PUBLIC "${RECOVERED_JPEG_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/jpeg-config")
set(RECOVERED_ZLIB_FILES adler32 compress crc32 gzio uncompr deflate trees zutil inflate infback inftrees inffast)
set(RECOVERED_ZLIB_SOURCES)
foreach(NAME IN LISTS RECOVERED_ZLIB_FILES)
    list(APPEND RECOVERED_ZLIB_SOURCES "${RECOVERED_ZLIB_DIR}/${NAME}.c")
endforeach()
add_library(original_zlib STATIC ${RECOVERED_ZLIB_SOURCES})
set_target_properties(original_zlib PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(original_zlib PUBLIC "${RECOVERED_ZLIB_DIR}")
if(NOT WIN32)
    # Modern POSIX headers declare functions used by the historical gzio source.
    target_compile_definitions(original_zlib PRIVATE HAVE_UNISTD_H)
endif()
if(MSVC)
    target_compile_definitions(original_jpeg PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(original_zlib PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()
