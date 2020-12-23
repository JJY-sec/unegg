cmake_minimum_required(VERSION 3.1.0)

project(unegg_con)

if(WIN32)
  set(PLATFORM_DIRECTORY "")
  set(PLATFORM_FILES "../common/Interfaces.cpp")
  set(PLATFORM_DEFINITION -D_CRT_SECURE_NO_WARNINGS)
else()
  set(PLATFORM_DIRECTORY "")
  set(PLATFORM_FILES "")
  set(PLATFORM_DEFINITION "")
endif()

add_definitions(-D_7ZIP_ST -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_REENTRANT -DENV_UNIX -DBREAK_HANDLER -DUNIX_USE_WIN_FILE
    -DNO_PCLMUL
    -DNO_XOP
    -DNO_AVX2
    -DNO_SSE2
    ${PLATFORM_DEFINITION}
)

include_directories(
    "."
    "../"
)

add_executable(unegg_con
    main.cpp

    ${PLATFORM_FILES}

    "../guiddef.cpp"

    "../lib/bzip2/blocksort.c"
    "../lib/bzip2/bzlib.c"
    "../lib/bzip2/compress.c"
    "../lib/bzip2/crctable.c"
    "../lib/bzip2/decompress.c"
    "../lib/bzip2/huffman.c"
    "../lib/bzip2/randtable.c"

    "../lib/lzma/7zBuf.c"
    "../lib/lzma/7zBuf2.c"
    "../lib/lzma/7zCrc.c"
    "../lib/lzma/7zFile.c"
    "../lib/lzma/7zStream.c"
    "../lib/lzma/Alloc.c"
    "../lib/lzma/Bcj2.c"
    "../lib/lzma/Bra.c"
    "../lib/lzma/Bra86.c"
    "../lib/lzma/BraIA64.c"
    "../lib/lzma/CpuArch.c"
    "../lib/lzma/LzFind.c"
    "../lib/lzma/LzmaDec.c"

    "../lib/zlib/adler32.c"
    "../lib/zlib/compress.c"
    "../lib/zlib/crc32.c"
    "../lib/zlib/deflate.c"
    "../lib/zlib/gzclose.c"
    "../lib/zlib/gzlib.c"
    "../lib/zlib/gzread.c"
    "../lib/zlib/gzwrite.c"
    "../lib/zlib/infback.c"
    "../lib/zlib/inffast.c"
    "../lib/zlib/inflate.c"
    "../lib/zlib/inftrees.c"
    "../lib/zlib/trees.c"
    "../lib/zlib/uncompr.c"
    "../lib/zlib/zutil.c"

    "../lib/azo/unAZO.cpp"
    "../lib/azo/Common/Allocator.cpp"
    "../lib/azo/Decoder/AlphaCodeD.cpp"
    "../lib/azo/Decoder/BitCodeD.cpp"
    "../lib/azo/Decoder/BlockCodeD.cpp"
    "../lib/azo/Base/BoolStateB.cpp"
    "../lib/azo/Decoder/BoolStateD.cpp"
    "../lib/azo/Common/BufferMan.cpp"
    "../lib/azo/Base/DictionaryTableB.cpp"
    "../lib/azo/Decoder/DictionaryTableD.cpp"
    "../lib/azo/Decoder/DistanceCodeD.cpp"
    "../lib/azo/Base/EntropyBitProbB.cpp"
    "../lib/azo/Decoder/EntropyBitProbD.cpp"
    "../lib/azo/Base/EntropyCodeB.cpp"
    "../lib/azo/Decoder/EntropyCodeD.cpp"
    "../lib/azo/Base/HistoryListB.cpp"
    "../lib/azo/Decoder/HistoryListD.cpp"
    "../lib/azo/Decoder/LengthCodeD.cpp"
    "../lib/azo/Base/MainCodeB.cpp"
    "../lib/azo/Decoder/MainCodeD.cpp"
    "../lib/azo/Decoder/MatchCodeD.cpp"
    "../lib/azo/Base/PredictProbB.cpp"
    "../lib/azo/Decoder/PredictProbD.cpp"
    "../lib/azo/Common/ProbCount.cpp"
    "../lib/azo/Decoder/SymbolCodeD.cpp"
    "../lib/azo/Common/x86Filter.cpp"

    "../lib/aes/aescrypt.c"
    "../lib/aes/aeskey.c"
    "../lib/aes/aestab.c"
    "../lib/aes/fileenc.c"
    "../lib/aes/hmac.c"
    "../lib/aes/prng.c"
    "../lib/aes/pwd2key.c"
    "../lib/aes/sha1.c"

    "../lib/lea/cpu_info_ia32.c"
    "../lib/lea/lea_base.c"
    "../lib/lea/lea_core.c"
    "../lib/lea/lea_core_xop.c"
    "../lib/lea/lea_gcm_generic.c"
    "../lib/lea/lea_gcm_pclmul.c"
    "../lib/lea/lea_online.c"
    "../lib/lea/lea_t_avx2_sse2.c"
    "../lib/lea/lea_t_avx2_xop.c"
    "../lib/lea/lea_t_fallback.c"
    "../lib/lea/lea_t_generic.c"
    "../lib/lea/lea_t_neon.c"
    "../lib/lea/lea_t_sse2.c"
    "../lib/lea/lea_t_xop.c"

    "../lib/egg/PropService.cpp"
    "../lib/egg/eggoper.cpp"
    "../lib/egg/EggArchiveInfo.cpp"

    "../lib/MyWindows.cpp"
    "../lib/PropVariant.cpp"

    "../common/PropId.cpp"
    "../common/TimeService.cpp"
    "../common/crc32.c"

    "../common/seven_stream.cpp"
    "../common/GeneralFileStream.cpp"
    "../common/AZFileStreams.cpp"
    "../common/MultipleStream.cpp"

    "../common/StoreCoder.cpp"
    "../common/DeflateCoder.cpp"
    "../common/AZOCoder.cpp"
    "../common/Bzip2Coder.cpp"
    "../common/LZMACoder.cpp"
    "../common/CoderImpl.cpp"

    "../common/ZipCrypto.cpp"
    "../common/AESCrypto.cpp"
    "../common/LEACrypto.cpp"
)

if(!WIN32)
target_link_libraries(unegg_con -static-libgcc -static-libstdc++ -static)
endif()
