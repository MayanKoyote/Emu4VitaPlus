cmake_minimum_required(VERSION 3.12)

set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${CORE})
set(CORE_MAKEFILE_NAME Makefile)
set(CORE_ARGS platform=vita EXTERNAL_ZLIB=1)
set(LIBRETRO_LIBRARY "")

if(CORE STREQUAL "gpsp")
elseif(CORE STREQUAL "vba_next")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vba-next)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile.libretro)
elseif(CORE STREQUAL "gambatte")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/gambatte-libretro)
    set(CORE_MAKEFILE_NAME Makefile.libretro)
elseif(CORE STREQUAL "fba_lite")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libretro-fba-lite)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/makefile.libretro)
    set(CORE_ARGS ${CORE_ARGS} HAVE_NEON=1)
elseif(CORE STREQUAL "fbneo")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libretro-fbneo/src/burner/libretro)
    set(CORE_ARGS ${CORE_ARGS} STATIC_LINKING=1)
elseif(CORE STREQUAL "fbalpha2012")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fbalpha2012/svn-current/trunk)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/makefile.libretro)
elseif(CORE STREQUAL "snes9x2002")
elseif(CORE STREQUAL "snes9x2005_plus")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/snes9x2005)
    set(CORE_ARGS ${CORE_ARGS} USE_BLARGG_APU=1)
elseif(CORE STREQUAL "snes9x2010")
elseif(CORE STREQUAL "fceumm")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libretro-fceumm)
elseif(CORE STREQUAL "nestopia")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nestopia/libretro)
elseif(CORE STREQUAL "genesis_plus_gx")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Genesis-Plus-GX)
    set(CORE_MAKEFILE_NAME Makefile.libretro)
elseif(CORE STREQUAL "genesis_plus_gx_wide")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Genesis-Plus-GX-Wide)
    set(CORE_MAKEFILE_NAME Makefile.libretro)
elseif(CORE STREQUAL "picodrive")
    set(CORE_MAKEFILE_NAME Makefile.libretro)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile.libretro)
elseif(CORE STREQUAL "mednafen_pce_fast")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/beetle-pce-fast-libretro)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile)
    set(CORE_ARGS ${CORE_ARGS} SYSTEM_ZLIB=1)
elseif(CORE STREQUAL "mednafen_supergrafx")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/beetle-supergrafx-libretro)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile)
    set(CORE_ARGS ${CORE_ARGS} SYSTEM_ZLIB=1)
elseif(CORE STREQUAL "mednafen_ngp")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/beetle-ngp-libretro)
elseif(CORE STREQUAL "mednafen_wswan")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/beetle-wswan-libretro)
elseif(CORE STREQUAL "pcsx_rearmed")
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile.libretro)
    set(CORE_ARGS ${CORE_ARGS} DYNAREC=ari64)
elseif(CORE STREQUAL "stella2014")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/stella2014-libretro)
elseif(CORE STREQUAL "prosystem")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/prosystem-libretro)
elseif(CORE STREQUAL "atari800")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libretro-atari800)
elseif(CORE STREQUAL "dosbox_pure")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dosbox-pure)
elseif(CORE STREQUAL "mame2000")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mame2000-libretro)
elseif(CORE STREQUAL "mame2003_plus")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mame2003-plus-libretro)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile)
    set(CORE_ARGS ${CORE_ARGS} SPLIT_UP_LINK=1)
elseif(CORE STREQUAL "mame2003")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mame2003-libretro)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile)
    set(CORE_ARGS ${CORE_ARGS} SPLIT_UP_LINK=1)
elseif(CORE STREQUAL "vecx")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libretro-vecx)
elseif(CORE STREQUAL "uae4arm")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/uae4arm-libretro)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile)
elseif(CORE STREQUAL "fuse")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fuse-libretro)
elseif(CORE STREQUAL "neocd")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/neocd_libretro)
elseif(CORE STREQUAL "mednafen_supafaust")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/supafaust)
    set(CORE_ARGS ${CORE_ARGS} USE_BLARGG_APU=1)
elseif(CORE STREQUAL "km_fbneo_xtreme_amped")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/FBNeo-Xtreme-Amped/src/burner/libretro)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile)
    set(CORE_ARGS ${CORE_ARGS} STATIC_LINKING=1)
elseif(CORE STREQUAL "km_mame2003_xtreme_amped")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mame2003-xtreme)
    set(CORE_ARGS ${CORE_ARGS} SPLIT_UP_LINK=1)
elseif(CORE STREQUAL "chimerasnes")
    set(CORE_ARGS ${CORE_ARGS} USE_BLARGG_APU=1)
elseif(CORE STREQUAL "tgbdual")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tgbdual-libretro)
elseif(CORE STREQUAL "nekop2")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libretro-meowPC98/libretro)
    set(CORE_MAKEFILE_NAME Makefile.libretro)
elseif(CORE STREQUAL "fmsx")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/fmsx-libretro)
elseif(CORE STREQUAL "bluemsx")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/blueMSX-libretro)
    set(CORE_MAKEFILE_NAME Makefile.libretro)
elseif(CORE STREQUAL "np2kai")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/NP2kai/sdl)
    set(CORE_MAKEFILE_NAME Makefile.libretro)
elseif(CORE STREQUAL "vice")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vice-libretro)
    set(LIBRETRO_LIBRARY ${CORE_DIR}/vice_x64_libretro_vita.a CACHE INTERNAL "LIBRETRO_LIBRARY")
elseif(CORE STREQUAL "px68k")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/px68k-libretro)
    set(CORE_MAKEFILE_NAME Makefile.libretro)
elseif(CORE STREQUAL "mednafen_lynx")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/beetle-lynx-libretro)
elseif(CORE STREQUAL "handy")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libretro-handy)
elseif(CORE STREQUAL "cap32")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libretro-cap32)
elseif(CORE STREQUAL "crocods")
    set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libretro-crocods)
    set(CORE_MAKEFILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/../apps/${CORE}/Makefile)
else()
    message(FATAL_ERROR "unknown core: " ${CORE})
endif()

if(NOT LIBRETRO_LIBRARY)
    set(LIBRETRO_LIBRARY ${CORE_DIR}/${CORE}_libretro_vita.a CACHE INTERNAL "LIBRETRO_LIBRARY")
endif()

set(CORE_TAG ${CORE}_build CACHE INTERNAL "CORE_TAG")
add_custom_target(
    ${CORE_TAG} ALL
    COMMAND make -f ${CORE_MAKEFILE_NAME} ${CORE_ARGS} -j4
    WORKING_DIRECTORY ${CORE_DIR})

add_library(${CORE} STATIC IMPORTED)
set_property(
    TARGET ${CORE}
    APPEND
    PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)

add_dependencies(${CORE} ${CORE_TAG})
set_target_properties(${CORE} PROPERTIES IMPORTED_LOCATION "${LIBRETRO_LIBRARY}")
