cmake_minimum_required(VERSION 2.8.12...3.19.1)
if(CMAKE_VERSION VERSION_LESS 3.12)
  cmake_policy(VERSION ${CMAKE_VERSION})
endif()

file(STRINGS src/game/version.h VERSION_LINE
  LIMIT_COUNT 1
  REGEX "^#define GAME_RELEASE_VERSION "
)

if(VERSION_LINE MATCHES "\"([0-9]+)\\.([0-9]+)\\.([0-9]+)\"")
  set(VERSION_MAJOR ${CMAKE_MATCH_1})
  set(VERSION_MINOR ${CMAKE_MATCH_2})
  set(VERSION_PATCH ${CMAKE_MATCH_3})
elseif(VERSION_LINE MATCHES "\"([0-9]+)\\.([0-9]+)\"")
  set(VERSION_MAJOR ${CMAKE_MATCH_1})
  set(VERSION_MINOR ${CMAKE_MATCH_2})
  set(VERSION_PATCH "0")
else()
  message(FATAL_ERROR "Couldn't parse version from src/game/version.h")
endif()

# Extra support for CMake pre-3.0
if(NOT POLICY CMP0048)
  set(PROJECT_VERSION_MAJOR ${VERSION_MAJOR})
  set(PROJECT_VERSION_MINOR ${VERSION_MINOR})
  set(PROJECT_VERSION_PATCH ${VERSION_PATCH})
  if(VERSION_PATCH STREQUAL "0")
    set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR})
  else()
    set(PROJECT_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
  endif()
endif()
if(VERSION_PATCH STREQUAL "0")
  project(DDNetPP VERSION ${VERSION_MAJOR}.${VERSION_MINOR})
else()
  project(DDNetPP VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})
endif()

set(ORIGINAL_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH})
set(ORIGINAL_CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES})
set(ORIGINAL_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES})
set(OWN_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
set(CMAKE_MODULE_PATH ${OWN_CMAKE_MODULE_PATH})

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  set(TARGET_BITS "32")
else()
  set(TARGET_BITS "64")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(TARGET_OS "windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(TARGET_OS "linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(TARGET_OS "mac")
endif()

include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)
include(CheckSymbolExists)

check_symbol_exists(__i386 "" TARGET_ARCH_X86_i386)
if(TARGET_ARCH_X86_i386)
  set(TARGET_ARCH x86)
else()
  set(TARGET_ARCH)
endif()

set(AUTO_DEPENDENCIES_DEFAULT OFF)
set(AUTOUPDATE_DEFAULT OFF)
if(TARGET_OS STREQUAL "windows")
  set(AUTO_DEPENDENCIES_DEFAULT ON)
  set(AUTOUPDATE_DEFAULT ON)
elseif(TARGET_OS STREQUAL "linux")
  set(AUTOUPDATE_DEFAULT ON)
endif()

option(WEBSOCKETS "Enable websockets support" OFF)
option(MYSQL "Enable mysql support" OFF)
option(AUTOUPDATE "Enable the autoupdater" ${AUTOUPDATE_DEFAULT})
option(CLIENT "Compile client" OFF)
option(PREFER_BUNDLED_LIBS "Prefer bundled libraries over system libraries" ${AUTO_DEPENDENCIES_DEFAULT})
option(DEV "Don't generate stuff necessary for packaging" OFF)

set(OpenGL_GL_PREFERENCE LEGACY)

# Set the default build type to Release
if(NOT(CMAKE_BUILD_TYPE))
  if(NOT(DEV))
    set(CMAKE_BUILD_TYPE Release)
  else()
    set(CMAKE_BUILD_TYPE Debug)
  endif()
endif()

if(NOT(CMAKE_BUILD_TYPE MATCHES "^(Release|Debug|RelWithDebInfo|MinSizeRel)$"))
  message(WARNING "Unknown CMAKE_BUILD_TYPE, should be one of Release, Debug, RelWithDebInfo or MinSizeRel")
endif()

set(DBG $<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>)

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  src/game/version.h
)

set(SERVER_EXECUTABLE DDNetPP CACHE STRING "Name of the built server executable")

########################################################################
# Compiler flags
########################################################################

function(add_c_compiler_flag_if_supported VARIABLE FLAG)
  if(ARGC GREATER 2)
    set(CHECKED_FLAG "${ARGV2}")
  else()
    set(CHECKED_FLAG "${FLAG}")
  endif()
  string(REGEX REPLACE "[^A-Za-z0-9]" "_" CONFIG_VARIABLE "FLAG_SUPPORTED${CHECKED_FLAG}")
  check_c_compiler_flag("${CHECKED_FLAG}" ${CONFIG_VARIABLE})
  if(${CONFIG_VARIABLE})
    if(${VARIABLE})
      set("${VARIABLE}" "${${VARIABLE}};${FLAG}" PARENT_SCOPE)
    else()
      set("${VARIABLE}" "${FLAG}" PARENT_SCOPE)
    endif()
  endif()
endfunction()

# Force compiler colors on when using ninja. Ninja filters the colors out when
# it's not printing to a terminal on its own.
if(CMAKE_GENERATOR STREQUAL "Ninja")
  add_c_compiler_flag_if_supported(OUR_FLAGS -fdiagnostics-color=always)
  add_c_compiler_flag_if_supported(OUR_FLAGS -fcolor-diagnostics)
endif()

if(NOT MSVC)
  if(CMAKE_VERSION VERSION_LESS 3.1 OR TARGET_OS STREQUAL "mac")
    check_cxx_compiler_flag(-std=gnu++11 FLAG_SUPPORTED_std_gnu__11)
    if(FLAG_SUPPORTED_std_gnu__11)
      if(CMAKE_CXX_FLAGS)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
      else()
        set(CMAKE_CXX_FLAGS -std=gnu++11)
      endif()
    endif()
  endif()

  # Protect the stack pointer.
  # -fstack-protector-all doesn't work on MinGW.
  add_c_compiler_flag_if_supported(OUR_FLAGS -fstack-protector-all)

  # Inaccurate floating point numbers cause problems on mingw-w64-gcc when
  # compiling for x86, might cause problems elsewhere. So don't store floats
  # in registers but keep them at higher accuracy.
  if(TARGET_ARCH STREQUAL "x86")
    add_c_compiler_flag_if_supported(OUR_FLAGS -ffloat-store)
  endif()

  # Don't insert timestamps into PEs to keep the build reproducible.
  if(TARGET_OS STREQUAL "windows")
    add_c_compiler_flag_if_supported(OUR_FLAGS_LINK -Wl,--no-insert-timestamp)
  endif()

  if(TARGET_OS STREQUAL "mac")
    add_c_compiler_flag_if_supported(OUR_FLAGS -stdlib=libc++)
    add_c_compiler_flag_if_supported(OUR_FLAGS -mmacosx-version-min=10.7)
  endif()

  add_c_compiler_flag_if_supported(OUR_FLAGS_OWN -Wall)
  if(CMAKE_VERSION VERSION_GREATER 3.3 OR CMAKE_VERSION VERSION_EQUAL 3.3)
    add_c_compiler_flag_if_supported(OUR_FLAGS_OWN
      $<$<COMPILE_LANGUAGE:C>:-Wdeclaration-after-statement>
      -Wdeclaration-after-statement
    )
  endif()
  add_c_compiler_flag_if_supported(OUR_FLAGS_OWN -Wextra)
  add_c_compiler_flag_if_supported(OUR_FLAGS_OWN -Wno-unused-parameter)
  add_c_compiler_flag_if_supported(OUR_FLAGS_OWN -Wno-missing-field-initializers)
  add_c_compiler_flag_if_supported(OUR_FLAGS_OWN -Wformat=2) # Warn about format strings.
  add_c_compiler_flag_if_supported(OUR_FLAGS_DEP -Wno-implicit-function-declaration)
  add_c_compiler_flag_if_supported(OUR_FLAGS_OWN -Wduplicated-cond)
  add_c_compiler_flag_if_supported(OUR_FLAGS_OWN -Wduplicated-branches)
  add_c_compiler_flag_if_supported(OUR_FLAGS_OWN -Wlogical-op)
  add_c_compiler_flag_if_supported(OUR_FLAGS_OWN -Wrestrict)
  #add_cxx_compiler_flag_if_supported(OUR_FLAGS_OWN "-Wuseless-cast")
endif()

if(NOT MSVC)
  check_c_compiler_flag("-O2;-Wp,-Werror;-D_FORTIFY_SOURCE=2" DEFINE_FORTIFY_SOURCE) # Some distributions define _FORTIFY_SOURCE by themselves.
endif()

########################################################################
# COMMON FUNCTIONS
########################################################################

function(set_glob VAR GLOBBING EXTS DIRECTORY) # ...
  set(GLOBS)
  foreach(ext ${EXTS})
    list(APPEND GLOBS "${DIRECTORY}/*.${ext}")
  endforeach()
  file(${GLOBBING} GLOB_RESULT ${GLOBS})
  list(SORT GLOB_RESULT)
  set(FILES)
  foreach(file ${ARGN})
    list(APPEND FILES "${PROJECT_SOURCE_DIR}/${DIRECTORY}/${file}")
  endforeach()

  if(NOT FILES STREQUAL GLOB_RESULT)
    message(AUTHOR_WARNING "${VAR} does not contain every file from directory ${DIRECTORY}")
    set(LIST_BUT_NOT_GLOB)
    if(POLICY CMP0057)
      cmake_policy(SET CMP0057 NEW)
      foreach(file ${FILES})
        if(NOT file IN_LIST GLOB_RESULT)
          list(APPEND LIST_BUT_NOT_GLOB ${file})
        endif()
      endforeach()
      if(LIST_BUT_NOT_GLOB)
        message(AUTHOR_WARNING "Entries only present in ${VAR}: ${LIST_BUT_NOT_GLOB}")
      endif()
      set(GLOB_BUT_NOT_LIST)
      foreach(file ${GLOB_RESULT})
        if(NOT file IN_LIST FILES)
          list(APPEND GLOB_BUT_NOT_LIST ${file})
        endif()
      endforeach()
      if(GLOB_BUT_NOT_LIST)
        message(AUTHOR_WARNING "Entries only present in ${DIRECTORY}: ${GLOB_BUT_NOT_LIST}")
      endif()
      if(NOT LIST_BUT_NOT_GLOB AND NOT GLOB_BUT_NOT_LIST)
        message(AUTHOR_WARNING "${VAR} is not alphabetically sorted")
      endif()
    endif()
  endif()

  set(${VAR} ${FILES} PARENT_SCOPE)
endfunction()

function(set_src VAR GLOBBING DIRECTORY) # ...
  set_glob(${VAR} ${GLOBBING} "c;cpp;h" ${DIRECTORY} ${ARGN})
  set(${VAR} ${${VAR}} PARENT_SCOPE)
endfunction()

########################################################################
# INITIALIZE TARGET LISTS
########################################################################

set(TARGETS_OWN)
set(TARGETS_DEP)

set(TARGETS_LINK) # Targets with a linking stage.

########################################################################
# DEPENDENCIES
########################################################################

function(set_extra_dirs_lib VARIABLE NAME)
  set("PATHS_${VARIABLE}_LIBDIR" PARENT_SCOPE)
  set("HINTS_${VARIABLE}_LIBDIR" PARENT_SCOPE)
  if(PREFER_BUNDLED_LIBS)
    set(TYPE HINTS)
  else()
    set(TYPE PATHS)
  endif()
  if(TARGET_BITS AND TARGET_OS)
    set(DIR "ddnet-libs/${NAME}/${TARGET_OS}/lib${TARGET_BITS}")
    set("${TYPE}_${VARIABLE}_LIBDIR" "${DIR}" PARENT_SCOPE)
    set("EXTRA_${VARIABLE}_LIBDIR" "${DIR}" PARENT_SCOPE)
  endif()
endfunction()

function(set_extra_dirs_include VARIABLE NAME LIBRARY)
  set("PATHS_${VARIABLE}_INCLUDEDIR" PARENT_SCOPE)
  set("HINTS_${VARIABLE}_INCLUDEDIR" PARENT_SCOPE)
  is_bundled(IS_BUNDLED "${LIBRARY}")
  if(IS_BUNDLED)
    set("HINTS_${VARIABLE}_INCLUDEDIR" "ddnet-libs/${NAME}/include" "ddnet-libs/${NAME}/include/${TARGET_OS}" PARENT_SCOPE)
  endif()
endfunction()

if(CMAKE_CROSSCOMPILING)
  set(CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH NO_CMAKE_SYSTEM_PATH)
else()
  set(CROSSCOMPILING_NO_CMAKE_SYSTEM_PATH)
endif()

function(is_bundled VARIABLE PATH)
  if(PATH)
    string(FIND "${PATH}" "${PROJECT_SOURCE_DIR}" LOCAL_PATH_POS)
    if(LOCAL_PATH_POS EQUAL 0 AND TARGET_BITS AND TARGET_OS)
      set("${VARIABLE}" ON PARENT_SCOPE)
    else()
      set("${VARIABLE}" OFF PARENT_SCOPE)
    endif()
  else()
    set("${VARIABLE}" OFF PARENT_SCOPE)
  endif()
endfunction()

if(NOT CMAKE_CROSSCOMPILING)
  # Check for PkgConfig once so all the other `find_package` calls can do it
  # quietly.
  find_package(PkgConfig)
endif()
find_package(ZLIB)
find_package(Crypto)
find_package(Freetype)
find_package(GLEW)
if(MYSQL)
  find_package(MySQL)
else()
  set(MYSQL_LIBRARIES)
endif()
find_package(Pnglite)
find_package(PythonInterp)
find_package(Threads)
if(WEBSOCKETS)
  find_package(Websockets)
else()
  set(WEBSOCKETS_LIBRARIES)
  set(WEBSOCKETS_INCLUDE_DIRS)
endif()


if(TARGET_OS AND TARGET_OS STREQUAL "mac")
  find_program(DMG dmg)
  find_program(HFSPLUS hfsplus)
  find_program(NEWFS_HFS newfs_hfs)
  if(DMG AND HFSPLUS AND NEWFS_HFS)
    set(DMGTOOLS_FOUND ON)
  else()
    set(DMGTOOLS_FOUND OFF)
  endif()

  find_program(HDIUTIL hdiutil)
endif()

message(STATUS "******** DDNet++ ********")
message(STATUS "Target OS: ${TARGET_OS} ${TARGET_BITS}bit")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

message(STATUS "Dependencies:")
function(show_dependency_status OUTPUT_NAME NAME)
  if(${NAME}_FOUND)
    if(${NAME}_BUNDLED)
      message(STATUS " * ${OUTPUT_NAME} not found (using bundled version)")
    else()
      message(STATUS " * ${OUTPUT_NAME} found")
    endif()
  else()
    message(STATUS " * ${OUTPUT_NAME} not found")
  endif()
endfunction()

if(TARGET_OS AND TARGET_OS STREQUAL "mac")
  show_dependency_status("Dmg tools" DMGTOOLS)
endif()
show_dependency_status("Freetype" FREETYPE)
show_dependency_status("Glew" GLEW)
if(TARGET_OS AND TARGET_OS STREQUAL "mac")
  show_dependency_status("Hdiutil" HDIUTIL)
endif()
if(MYSQL)
  show_dependency_status("MySQL" MYSQL)
endif()
show_dependency_status("OpenSSL Crypto" CRYPTO)
show_dependency_status("Zlib" ZLIB)
show_dependency_status("Pnglite" PNGLITE)
if(WEBSOCKETS)
  show_dependency_status("Websockets" WEBSOCKETS)
endif()

if(NOT(PYTHONINTERP_FOUND))
  message(SEND_ERROR "You must install Python to compile DDNet++")
endif()

if(MYSQL AND NOT(MYSQL_FOUND))
  message(SEND_ERROR "You must install MySQL to compile the DDNet++ server with MySQL support")
endif()

if(WEBSOCKETS AND NOT(WEBSOCKETS_FOUND))
  message(SEND_ERROR "You must install libwebsockets to compile the DDNet++ server with websocket support")
endif()

if(TARGET_OS STREQUAL "windows")
  set(PLATFORM_LIBS ws2_32) # Windows sockets
elseif(TARGET_OS STREQUAL "mac")
  find_library(CARBON Carbon)
  find_library(COCOA Cocoa)
  find_library(OPENGL OpenGL)
  find_library(SECURITY Security)
  set(PLATFORM_LIBS ${CARBON} ${SECURITY})
else()
  find_package(OpenGL)
  find_package(X11)
  if(TARGET_OS STREQUAL "linux")
    set(PLATFORM_LIBS rt) # clock_gettime for glibc < 2.17
  else()
    set(PLATFORM_LIBS)
  endif()
endif()

########################################################################
# DEPENDENCY COMPILATION
########################################################################

# Static dependencies
set_src(DEP_SQLITE3_SRC GLOB src/engine/external/sqlite3 sqlite3.c sqlite3.h sqlite3ext.h)
add_library(sqlite3 EXCLUDE_FROM_ALL OBJECT ${DEP_SQLITE3_SRC})

set_src(DEP_JSON_SRC GLOB src/engine/external/json-parser json.c json.h)
add_library(json EXCLUDE_FROM_ALL OBJECT ${DEP_JSON_SRC})

set_src(DEP_MD5_SRC GLOB src/engine/external/md5 md5.c md5.h)
add_library(md5 EXCLUDE_FROM_ALL OBJECT ${DEP_MD5_SRC})

list(APPEND TARGETS_DEP sqlite3 json md5)
set(DEP_SQLITE3 $<TARGET_OBJECTS:sqlite3>)
set(DEP_JSON $<TARGET_OBJECTS:json>)
set(DEP_MD5)
if(NOT CRYPTO_FOUND)
  set(DEP_MD5 $<TARGET_OBJECTS:md5>)
endif()

########################################################################
# DATA
########################################################################

set(EXPECTED_DATA
  arrow.png
  audio/foley_body_impact-01.wv
  audio/foley_body_impact-02.wv
  audio/foley_body_impact-03.wv
  audio/foley_body_splat-01.wv
  audio/foley_body_splat-02.wv
  audio/foley_body_splat-03.wv
  audio/foley_body_splat-04.wv
  audio/foley_dbljump-01.wv
  audio/foley_dbljump-02.wv
  audio/foley_dbljump-03.wv
  audio/foley_foot_left-01.wv
  audio/foley_foot_left-02.wv
  audio/foley_foot_left-03.wv
  audio/foley_foot_left-04.wv
  audio/foley_foot_right-01.wv
  audio/foley_foot_right-02.wv
  audio/foley_foot_right-03.wv
  audio/foley_foot_right-04.wv
  audio/foley_land-01.wv
  audio/foley_land-02.wv
  audio/foley_land-03.wv
  audio/foley_land-04.wv
  audio/hook_attach-01.wv
  audio/hook_attach-02.wv
  audio/hook_attach-03.wv
  audio/hook_loop-01.wv
  audio/hook_loop-02.wv
  audio/hook_noattach-01.wv
  audio/hook_noattach-02.wv
  audio/hook_noattach-03.wv
  audio/music_menu.wv
  audio/sfx_ctf_cap_pl.wv
  audio/sfx_ctf_drop.wv
  audio/sfx_ctf_grab_en.wv
  audio/sfx_ctf_grab_pl.wv
  audio/sfx_ctf_rtn.wv
  audio/sfx_hit_strong-01.wv
  audio/sfx_hit_strong-02.wv
  audio/sfx_hit_weak-01.wv
  audio/sfx_hit_weak-02.wv
  audio/sfx_hit_weak-03.wv
  audio/sfx_msg-client.wv
  audio/sfx_msg-highlight.wv
  audio/sfx_msg-server.wv
  audio/sfx_pickup_arm-01.wv
  audio/sfx_pickup_arm-02.wv
  audio/sfx_pickup_arm-03.wv
  audio/sfx_pickup_arm-04.wv
  audio/sfx_pickup_gun.wv
  audio/sfx_pickup_hrt-01.wv
  audio/sfx_pickup_hrt-02.wv
  audio/sfx_pickup_launcher.wv
  audio/sfx_pickup_ninja.wv
  audio/sfx_pickup_sg.wv
  audio/sfx_skid-01.wv
  audio/sfx_skid-02.wv
  audio/sfx_skid-03.wv
  audio/sfx_skid-04.wv
  audio/sfx_spawn_wpn-01.wv
  audio/sfx_spawn_wpn-02.wv
  audio/sfx_spawn_wpn-03.wv
  audio/vo_teefault_cry-01.wv
  audio/vo_teefault_cry-02.wv
  audio/vo_teefault_ninja-01.wv
  audio/vo_teefault_ninja-02.wv
  audio/vo_teefault_ninja-03.wv
  audio/vo_teefault_ninja-04.wv
  audio/vo_teefault_pain_long-01.wv
  audio/vo_teefault_pain_long-02.wv
  audio/vo_teefault_pain_short-01.wv
  audio/vo_teefault_pain_short-02.wv
  audio/vo_teefault_pain_short-03.wv
  audio/vo_teefault_pain_short-04.wv
  audio/vo_teefault_pain_short-05.wv
  audio/vo_teefault_pain_short-06.wv
  audio/vo_teefault_pain_short-07.wv
  audio/vo_teefault_pain_short-08.wv
  audio/vo_teefault_pain_short-09.wv
  audio/vo_teefault_pain_short-10.wv
  audio/vo_teefault_pain_short-11.wv
  audio/vo_teefault_pain_short-12.wv
  audio/vo_teefault_sledge-01.wv
  audio/vo_teefault_sledge-02.wv
  audio/vo_teefault_sledge-03.wv
  audio/vo_teefault_spawn-01.wv
  audio/vo_teefault_spawn-02.wv
  audio/vo_teefault_spawn-03.wv
  audio/vo_teefault_spawn-04.wv
  audio/vo_teefault_spawn-05.wv
  audio/vo_teefault_spawn-06.wv
  audio/vo_teefault_spawn-07.wv
  audio/wp_flump_explo-01.wv
  audio/wp_flump_explo-02.wv
  audio/wp_flump_explo-03.wv
  audio/wp_flump_launch-01.wv
  audio/wp_flump_launch-02.wv
  audio/wp_flump_launch-03.wv
  audio/wp_gun_fire-01.wv
  audio/wp_gun_fire-02.wv
  audio/wp_gun_fire-03.wv
  audio/wp_hammer_hit-01.wv
  audio/wp_hammer_hit-02.wv
  audio/wp_hammer_hit-03.wv
  audio/wp_hammer_swing-01.wv
  audio/wp_hammer_swing-02.wv
  audio/wp_hammer_swing-03.wv
  audio/wp_ninja_attack-01.wv
  audio/wp_ninja_attack-02.wv
  audio/wp_ninja_attack-03.wv
  audio/wp_ninja_attack-04.wv
  audio/wp_ninja_hit-01.wv
  audio/wp_ninja_hit-02.wv
  audio/wp_ninja_hit-03.wv
  audio/wp_ninja_hit-04.wv
  audio/wp_noammo-01.wv
  audio/wp_noammo-02.wv
  audio/wp_noammo-03.wv
  audio/wp_noammo-04.wv
  audio/wp_noammo-05.wv
  audio/wp_rifle_bnce-01.wv
  audio/wp_rifle_bnce-02.wv
  audio/wp_rifle_bnce-03.wv
  audio/wp_rifle_fire-01.wv
  audio/wp_rifle_fire-02.wv
  audio/wp_rifle_fire-03.wv
  audio/wp_shotty_fire-01.wv
  audio/wp_shotty_fire-02.wv
  audio/wp_shotty_fire-03.wv
  audio/wp_switch-01.wv
  audio/wp_switch-02.wv
  audio/wp_switch-03.wv
  blob.png
  browse_icons.png
  ca-ddnet.pem
  console.png
  console_bar.png
  countryflags/AD.png
  countryflags/AE.png
  countryflags/AF.png
  countryflags/AG.png
  countryflags/AI.png
  countryflags/AL.png
  countryflags/AM.png
  countryflags/AO.png
  countryflags/AR.png
  countryflags/AS.png
  countryflags/AT.png
  countryflags/AU.png
  countryflags/AW.png
  countryflags/AX.png
  countryflags/AZ.png
  countryflags/BA.png
  countryflags/BB.png
  countryflags/BD.png
  countryflags/BE.png
  countryflags/BF.png
  countryflags/BG.png
  countryflags/BH.png
  countryflags/BI.png
  countryflags/BJ.png
  countryflags/BL.png
  countryflags/BM.png
  countryflags/BN.png
  countryflags/BO.png
  countryflags/BR.png
  countryflags/BS.png
  countryflags/BT.png
  countryflags/BW.png
  countryflags/BY.png
  countryflags/BZ.png
  countryflags/CA.png
  countryflags/CC.png
  countryflags/CD.png
  countryflags/CF.png
  countryflags/CG.png
  countryflags/CH.png
  countryflags/CI.png
  countryflags/CK.png
  countryflags/CL.png
  countryflags/CM.png
  countryflags/CN.png
  countryflags/CO.png
  countryflags/CR.png
  countryflags/CU.png
  countryflags/CV.png
  countryflags/CW.png
  countryflags/CX.png
  countryflags/CY.png
  countryflags/CZ.png
  countryflags/DE.png
  countryflags/DJ.png
  countryflags/DK.png
  countryflags/DM.png
  countryflags/DO.png
  countryflags/DZ.png
  countryflags/EC.png
  countryflags/EE.png
  countryflags/EG.png
  countryflags/EH.png
  countryflags/ER.png
  countryflags/ES.png
  countryflags/ET.png
  countryflags/FI.png
  countryflags/FJ.png
  countryflags/FK.png
  countryflags/FM.png
  countryflags/FO.png
  countryflags/FR.png
  countryflags/GA.png
  countryflags/GB.png
  countryflags/GD.png
  countryflags/GE.png
  countryflags/GF.png
  countryflags/GG.png
  countryflags/GH.png
  countryflags/GI.png
  countryflags/GL.png
  countryflags/GM.png
  countryflags/GN.png
  countryflags/GP.png
  countryflags/GQ.png
  countryflags/GR.png
  countryflags/GS.png
  countryflags/GT.png
  countryflags/GU.png
  countryflags/GW.png
  countryflags/GY.png
  countryflags/HK.png
  countryflags/HN.png
  countryflags/HR.png
  countryflags/HT.png
  countryflags/HU.png
  countryflags/ID.png
  countryflags/IE.png
  countryflags/IL.png
  countryflags/IM.png
  countryflags/IN.png
  countryflags/IO.png
  countryflags/IQ.png
  countryflags/IR.png
  countryflags/IS.png
  countryflags/IT.png
  countryflags/JE.png
  countryflags/JM.png
  countryflags/JO.png
  countryflags/JP.png
  countryflags/KE.png
  countryflags/KG.png
  countryflags/KH.png
  countryflags/KI.png
  countryflags/KM.png
  countryflags/KN.png
  countryflags/KP.png
  countryflags/KR.png
  countryflags/KW.png
  countryflags/KY.png
  countryflags/KZ.png
  countryflags/LA.png
  countryflags/LB.png
  countryflags/LC.png
  countryflags/LI.png
  countryflags/LK.png
  countryflags/LR.png
  countryflags/LS.png
  countryflags/LT.png
  countryflags/LU.png
  countryflags/LV.png
  countryflags/LY.png
  countryflags/MA.png
  countryflags/MC.png
  countryflags/MD.png
  countryflags/ME.png
  countryflags/MF.png
  countryflags/MG.png
  countryflags/MH.png
  countryflags/MK.png
  countryflags/ML.png
  countryflags/MM.png
  countryflags/MN.png
  countryflags/MO.png
  countryflags/MP.png
  countryflags/MQ.png
  countryflags/MR.png
  countryflags/MS.png
  countryflags/MT.png
  countryflags/MU.png
  countryflags/MV.png
  countryflags/MW.png
  countryflags/MX.png
  countryflags/MY.png
  countryflags/MZ.png
  countryflags/NA.png
  countryflags/NC.png
  countryflags/NE.png
  countryflags/NF.png
  countryflags/NG.png
  countryflags/NI.png
  countryflags/NL.png
  countryflags/NO.png
  countryflags/NP.png
  countryflags/NR.png
  countryflags/NU.png
  countryflags/NZ.png
  countryflags/OM.png
  countryflags/PA.png
  countryflags/PE.png
  countryflags/PF.png
  countryflags/PG.png
  countryflags/PH.png
  countryflags/PK.png
  countryflags/PL.png
  countryflags/PM.png
  countryflags/PN.png
  countryflags/PR.png
  countryflags/PS.png
  countryflags/PT.png
  countryflags/PW.png
  countryflags/PY.png
  countryflags/QA.png
  countryflags/RE.png
  countryflags/RO.png
  countryflags/RS.png
  countryflags/RU.png
  countryflags/RW.png
  countryflags/SA.png
  countryflags/SB.png
  countryflags/SC.png
  countryflags/SD.png
  countryflags/SE.png
  countryflags/SG.png
  countryflags/SH.png
  countryflags/SI.png
  countryflags/SK.png
  countryflags/SL.png
  countryflags/SM.png
  countryflags/SN.png
  countryflags/SO.png
  countryflags/SR.png
  countryflags/SS.png
  countryflags/ST.png
  countryflags/SV.png
  countryflags/SX.png
  countryflags/SY.png
  countryflags/SZ.png
  countryflags/TC.png
  countryflags/TD.png
  countryflags/TF.png
  countryflags/TG.png
  countryflags/TH.png
  countryflags/TJ.png
  countryflags/TK.png
  countryflags/TL.png
  countryflags/TM.png
  countryflags/TN.png
  countryflags/TO.png
  countryflags/TR.png
  countryflags/TT.png
  countryflags/TV.png
  countryflags/TW.png
  countryflags/TZ.png
  countryflags/UA.png
  countryflags/UG.png
  countryflags/US.png
  countryflags/UY.png
  countryflags/UZ.png
  countryflags/VA.png
  countryflags/VC.png
  countryflags/VE.png
  countryflags/VG.png
  countryflags/VI.png
  countryflags/VN.png
  countryflags/VU.png
  countryflags/WF.png
  countryflags/WS.png
  countryflags/XCA.png
  countryflags/XEN.png
  countryflags/XEU.png
  countryflags/XNI.png
  countryflags/XSC.png
  countryflags/XWA.png
  countryflags/YE.png
  countryflags/ZA.png
  countryflags/ZM.png
  countryflags/ZW.png
  countryflags/default.png
  countryflags/index.txt
  debug_font.png
  demo_buttons.png
  demo_buttons2.png
  editor/audio_source.png
  editor/background.png
  editor/basic_freeze.rules
  editor/checker.png
  editor/cursor.png
  editor/ddmax_freeze.rules
  editor/ddnet_tiles.rules
  editor/desert_main.rules
  editor/entities.png
  editor/entities_clear/ddnet.png
  editor/entities_clear/ddrace.png
  editor/entities_clear/fng.png
  editor/entities_clear/race.png
  editor/entities_clear/vanilla.png
  editor/fadeout.rules
  editor/front.png
  editor/generic_clear.rules
  editor/generic_unhookable.rules
  editor/generic_unhookable_0.7.rules
  editor/grass_main.rules
  editor/grass_main_0.7.rules
  editor/jungle_main.rules
  editor/round_tiles.rules
  editor/speed_arrow.png
  editor/speedup.png
  editor/switch.png
  editor/tele.png
  editor/tune.png
  editor/water.rules
  editor/winter_main.rules
  emoticons.png
  file_icons.png
  fonts/DejaVuSansCJKName.ttf
  fonts/DejavuWenQuanYiMicroHei.ttf
  game.png
  gui_buttons.png
  gui_cursor.png
  gui_icons.png
  gui_logo.png
  languages/belarusian.txt
  languages/bosnian.txt
  languages/brazilian_portuguese.txt
  languages/bulgarian.txt
  languages/catalan.txt
  languages/chuvash.txt
  languages/czech.txt
  languages/danish.txt
  languages/dutch.txt
  languages/finnish.txt
  languages/french.txt
  languages/german.txt
  languages/greek.txt
  languages/hungarian.txt
  languages/index.txt
  languages/italian.txt
  languages/japanese.txt
  languages/korean.txt
  languages/kyrgyz.txt
  languages/license.txt
  languages/norwegian.txt
  languages/persian.txt
  languages/polish.txt
  languages/portuguese.txt
  languages/readme.txt
  languages/romanian.txt
  languages/russian.txt
  languages/scripts/readme.txt
  languages/serbian.txt
  languages/simplified_chinese.txt
  languages/slovak.txt
  languages/spanish.txt
  languages/swedish.txt
  languages/traditional_chinese.txt
  languages/turkish.txt
  languages/ukrainian.txt
  mapres/basic_freeze.png
  mapres/bg_cloud1.png
  mapres/bg_cloud2.png
  mapres/bg_cloud3.png
  mapres/ddmax_freeze.png
  mapres/ddnet_start.png
  mapres/ddnet_tiles.png
  mapres/ddnet_walls.png
  mapres/desert_background.png
  mapres/desert_doodads.png
  mapres/desert_main.png
  mapres/desert_mountains.png
  mapres/desert_mountains2.png
  mapres/desert_mountains_new_background.png
  mapres/desert_mountains_new_foreground.png
  mapres/desert_sun.png
  mapres/entities.png
  mapres/entities_ddpp.png
  mapres/fadeout.png
  mapres/fddracepp.png
  mapres/font_teeworlds.png
  mapres/font_teeworlds_alt.png
  mapres/generic_clear.png
  mapres/generic_deathtiles.png
  mapres/generic_lamps.png
  mapres/generic_unhookable.png
  mapres/generic_unhookable_0.7.png
  mapres/grass_doodads.png
  mapres/grass_doodads_0.7.png
  mapres/grass_main.png
  mapres/grass_main_0.7.png
  mapres/jungle_background.png
  mapres/jungle_deathtiles.png
  mapres/jungle_doodads.png
  mapres/jungle_main.png
  mapres/jungle_midground.png
  mapres/jungle_unhookables.png
  mapres/light.png
  mapres/mixed_tiles.png
  mapres/moon.png
  mapres/mountains.png
  mapres/round_tiles.png
  mapres/snow.png
  mapres/snow_mountain.png
  mapres/stars.png
  mapres/sun.png
  mapres/water.png
  mapres/winter_doodads.png
  mapres/winter_main.png
  mapres/winter_mountains.png
  mapres/winter_mountains2.png
  mapres/winter_mountains3.png
  maps/BlmapChill.map
  maps/BlmapChill_versions/BlmapChill_v0004.map
  maps/BlmapChill_versions/BlmapChill_v0052.map
  maps/BlmapChill_versions/BlmapChill_v0059.map
  maps/BlmapChill_versions/BlmapChill_v0103.map
  maps/BlmapChill_versions/BlmapChill_v0104.map
  maps/BlmapChill_versions/BlmapChill_v0105.map
  maps/BlmapChill_versions/BlmapChill_v0106.map
  maps/BlmapChill_versions/BlmapChill_v0107.map
  maps/BlmapChill_versions/BlmapChill_v0108.map
  maps/BlmapChill_versions/BlmapChill_v0109.map
  maps/BlmapChill_versions/BlmapChill_v0110.map
  maps/BlmapChill_versions/BlmapChill_v0111.map
  maps/Blockdale.map
  maps/CBattleGores.map
  maps/ChillBlock5.map
  maps/ChillBlock_fork/ChillBlock5_v002_fork_a_01.map
  maps/ChillBlock_fork/readme.txt
  maps/ChillBlock_old/ChillBlock5_old.map
  maps/ChillBlock_old/ChillBlock5_old10.map
  maps/ChillBlock_old/ChillBlock5_old2.map
  maps/ChillBlock_old/ChillBlock5_old3.map
  maps/ChillBlock_old/ChillBlock5_old4.map
  maps/ChillBlock_old/ChillBlock5_old5.map
  maps/ChillBlock_old/ChillBlock5_old6.map
  maps/ChillBlock_old/ChillBlock5_old7.map
  maps/ChillBlock_old/ChillBlock5_old8.map
  maps/ChillBlock_old/ChillBlock5_old9.map
  maps/ChillBlock_versions/ChillBlock5_v001.map
  maps/ChillBlock_versions/ChillBlock5_v002.map
  maps/ChillBlock_versions/ChillBlock5_v003.map
  maps/ChillBlock_versions/ChillBlock5_v004.map
  maps/ChillBlock_versions/ChillBlock5_v005.map
  maps/ChillBlock_versions/ChillBlock5_v006.map
  maps/ChillBlock_versions/ChillBlock5_v006_christmas.map
  maps/ChillBlock_versions/ChillBlock5_v007.map
  maps/ChillBlock_versions/ChillBlock5_v008.map
  maps/blmapV5_ddpp.map
  maps/ctf5_fngores.map
  maps/ddpp_survival.map
  maps/fly_shy_cry.map
  particles.png
  skins/Ablush\ NeoN.png
  skins/Aoe4leg.png
  skins/Apish\ Coke.png
  skins/BadAnqelMonster.png
  skins/Black\ Phantom\ heinrich5991.png
  skins/Blind\ r0xr.png
  skins/DuMoH.png
  skins/Evil\ Puffi.png
  skins/Flying\ Silex.png
  skins/Hidden\ Assassin.png
  skins/Irradiated\ Sunny.png
  skins/Mobys\ Skull.png
  skins/PaladiN.png
  skins/Red\ Coke.png
  skins/Shadow.png
  skins/Sonic.png
  skins/Straw\ Crytek.png
  skins/Terrorist.png
  skins/Volt.png
  skins/antiantey.png
  skins/aqua.png
  skins/atlas_by_whis.png
  skins/bauer.png
  skins/beast.png
  skins/blacktee.png
  skins/bluekitty.png
  skins/bluestripe.png
  skins/bomb.png
  skins/brownbear.png
  skins/bunny.png
  skins/caesar.png
  skins/cammo.png
  skins/cammostripes.png
  skins/chera.png
  skins/chinese_by_whis.png
  skins/clefairy.png
  skins/coala.png
  skins/default.png
  skins/demonlimekitty.png
  skins/dino.png
  skins/dragon.png
  skins/draw.png
  skins/emo.png
  skins/evil.png
  skins/evilwolfe.png
  skins/fuzzy_coala.png
  skins/ghost.png
  skins/ghostjtj.png
  skins/giraffe.png
  skins/greensward.png
  skins/greyfox.png
  skins/greyfox_2.png
  skins/hammie-chew.png
  skins/hammie-whis.png
  skins/hedgehog.png
  skins/jeet.png
  skins/kintaro_2.png
  skins/kirby.png
  skins/lightbulb.png
  skins/limekitty.png
  skins/m&mred.png
  skins/m&myellow.png
  skins/masterchief.png
  skins/mermydon-coala.png
  skins/mermydon.png
  skins/mike.png
  skins/monstee.png
  skins/mouse.png
  skins/musmann.png
  skins/nanami.png
  skins/nanas.png
  skins/napoleon.png
  skins/nersif.png
  skins/nosey.png
  skins/oldman.png
  skins/oldschool.png
  skins/pbody_by_whis.png
  skins/penguin.png
  skins/pepsi.png
  skins/pikminpurple.png
  skins/pikminwhite.png
  skins/pikminyellow.png
  skins/pinky.png
  skins/random.png
  skins/red_bird.png
  skins/redbopp.png
  skins/redstripe.png
  skins/roman.png
  skins/saddo.png
  skins/savage.png
  skins/t2.png
  skins/tails.png
  skins/tank.png
  skins/tauren.png
  skins/teerasta.png
  skins/teledipsy.png
  skins/telelaalaa.png
  skins/telepo.png
  skins/teletinkywinky.png
  skins/toptri.png
  skins/troll.png
  skins/ts2_contest_skin.png
  skins/tweety.png
  skins/twinbop.png
  skins/twintri.png
  skins/veteran.png
  skins/voodoo_tee.png
  skins/warpaint.png
  skins/wartee.png
  skins/whis.png
  skins/x_ninja.png
)

set_glob(DATA GLOB_RECURSE "frag;json;map;pem;png;rules;ttf;txt;vert;wv" data ${EXPECTED_DATA})

########################################################################
# COPY DATA AND DLLS
########################################################################

foreach(datafile ${DATA})
  file(RELATIVE_PATH OUT ${PROJECT_SOURCE_DIR}/data ${datafile})
  get_filename_component(DESTINATION data/${OUT} PATH)
  file(MAKE_DIRECTORY ${DESTINATION})
  file(COPY ${datafile} DESTINATION ${DESTINATION})
endforeach()
set(COPY_FILES
  ${CURL_COPY_FILES}
  ${FREETYPE_COPY_FILES}
  ${OPUSFILE_COPY_FILES}
  ${SDL2_COPY_FILES}
)
file(COPY ${COPY_FILES} DESTINATION .)

########################################################################
# CODE GENERATION
########################################################################

function(generate_source output_file script_parameter)
  add_custom_command(OUTPUT ${output_file}
    COMMAND ${PYTHON_EXECUTABLE} datasrc/compile.py ${script_parameter}
      > "${PROJECT_BINARY_DIR}/${output_file}"
    DEPENDS
      datasrc/compile.py
      datasrc/content.py
      datasrc/datatypes.py
      datasrc/network.py
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  )
endfunction()

file(MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/src/game/generated/")
execute_process(COMMAND git rev-parse --git-dir
  ERROR_QUIET
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
  OUTPUT_VARIABLE PROJECT_GIT_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE PROJECT_GIT_DIR_ERROR
)
if(NOT PROJECT_GIT_DIR_ERROR)
  set(GIT_REVISION_EXTRA_DEPS
    ${PROJECT_GIT_DIR}/index
    ${PROJECT_GIT_DIR}/logs/HEAD
  )
endif()
add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/src/game/generated/git_revision.cpp
  COMMAND ${PYTHON_EXECUTABLE}
    scripts/git_revision.py
    > ${PROJECT_BINARY_DIR}/src/game/generated/git_revision.cpp
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  DEPENDS
    ${GIT_REVISION_EXTRA_DEPS}
    scripts/git_revision.py
)
generate_source("src/game/generated/protocol.cpp" "network_source")
generate_source("src/game/generated/protocol.h" "network_header")
generate_source("src/game/generated/server_data.cpp" "server_content_source")
generate_source("src/game/generated/server_data.h" "server_content_header")


########################################################################
# SHARED
########################################################################

# Sources
set_src(BASE GLOB_RECURSE src/base
  color.h
  confusables.c
  confusables_data.h
  ddpp_logs.c
  ddpp_logs.h
  detect.h
  hash.c
  hash.h
  hash_bundled.c
  hash_ctxt.h
  hash_libtomcrypt.c
  hash_openssl.c
  math.h
  system.c
  system.h
  system_ddpp.c
  system_ddpp.h
  tl/algorithm.h
  tl/allocator.h
  tl/array.h
  tl/base.h
  tl/range.h
  tl/sorted_array.h
  tl/string.h
  tl/threading.h
  vmath.h
)
set_src(ENGINE_INTERFACE GLOB src/engine
  client.h
  config.h
  console.h
  demo.h
  editor.h
  engine.h
  fetcher.h
  friends.h
  graphics.h
  input.h
  kernel.h
  keys.h
  map.h
  masterserver.h
  message.h
  server.h
  serverbrowser.h
  sound.h
  storage.h
  textrender.h
  updater.h
  uuid.h
)
set_src(ENGINE_SHARED GLOB src/engine/shared
  compression.cpp
  compression.h
  config.cpp
  config.h
  config_variables.h
  config_variables_ddpp.h
  console.cpp
  console.h
  datafile.cpp
  datafile.h
  demo.cpp
  demo.h
  econ.cpp
  econ.h
  engine.cpp
  fifo.cpp
  fifo.h
  filecollection.cpp
  filecollection.h
  global_uuid_manager.cpp
  huffman.cpp
  huffman.h
  jobs.cpp
  jobs.h
  kernel.cpp
  linereader.cpp
  linereader.h
  map.cpp
  mapchecker.cpp
  mapchecker.h
  masterserver.cpp
  memheap.cpp
  memheap.h
  message.h
  netban.cpp
  netban.h
  network.cpp
  network.h
  network_client.cpp
  network_conn.cpp
  network_console.cpp
  network_console_conn.cpp
  network_server.cpp
  packer.cpp
  packer.h
  protocol.h
  protocol_ex.cpp
  protocol_ex.h
  protocol_ex_msgs.h
  ringbuffer.cpp
  ringbuffer.h
  serverbrowser.cpp
  snapshot.cpp
  snapshot.h
  storage.cpp
  storage.h
  uuid_manager.cpp
  uuid_manager.h
  websockets.cpp
  websockets.h
)
set_src(GAME_SHARED GLOB src/game
  collision.cpp
  collision.h
  collision_ddpp.cpp
  ddracecommands.h
  gamecore.cpp
  gamecore.h
  gamecore_ddpp.cpp
  layers.cpp
  layers.h
  localization.cpp
  localization.h
  mapitems.cpp
  mapitems.h
  mapitems_ddpp.h
  teamscore.cpp
  teamscore.h
  tuning.h
  variables.h
  version.h
  version_ddpp.h
  voting.h
)
# A bit hacky, but these are needed to register all the UUIDs, even for stuff
# that doesn't link game.
set(ENGINE_UUID_SHARED
  src/game/generated/protocol.cpp
  src/game/generated/protocol.h
  src/game/generated/server_data.cpp
  src/game/generated/server_data.h
)
foreach(s ${GAME_SHARED})
  if(s MATCHES "mapitems_(ex.cpp|ex.h|ex_types.h)$")
    list(REMOVE_ITEM GAME_SHARED ${s})
  endif()
endforeach()
list(REMOVE_ITEM GAME_SHARED ${ENGINE_UUID_SHARED})
set(GAME_GENERATED_SHARED
  src/game/generated/git_revision.cpp
  src/game/generated/protocol.h
  src/game/generated/server_data.h
)

set(DEPS ${DEP_SQLITE3} ${DEP_JSON} ${DEP_MD5} ${ZLIB_DEP})

# Libraries
set(LIBS
  ${CRYPTO_LIBRARIES}
  ${WEBSOCKETS_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${PLATFORM_LIBS}
  # Add pthreads (on non-Windows) at the end, so that other libraries can depend
  # on it.
  ${CMAKE_THREAD_LIBS_INIT}
  -ldl
)

# Targets
add_library(engine-shared EXCLUDE_FROM_ALL OBJECT ${ENGINE_INTERFACE} ${ENGINE_SHARED} ${ENGINE_UUID_SHARED} ${BASE})
add_library(game-shared EXCLUDE_FROM_ALL OBJECT ${GAME_SHARED} ${GAME_GENERATED_SHARED})
list(APPEND TARGETS_OWN engine-shared game-shared)

########################################################################
# SERVER
########################################################################

# Sources
set_src(ENGINE_SERVER GLOB src/engine/server
  register.cpp
  register.h
  server.cpp
  server.h
  server_ddpp.cpp
  sql_connector.cpp
  sql_connector.h
  sql_server.cpp
  sql_server.h
  sql_string_helpers.cpp
  sql_string_helpers.h
)
set_src(GAME_SERVER GLOB_RECURSE src/game/server
  captcha.cpp
  captcha.h
  db_sqlite3.cpp
  db_sqlite3.h
  ddracechat.cpp
  ddracechat.h
  ddracecommands.cpp
  ddracecommands_ddpp.cpp
  entities/character.cpp
  entities/character.h
  entities/character_ddpp.cpp
  entities/character_dummymodes.cpp
  entities/character_quests.cpp
  entities/character_tiles.cpp
  entities/door.cpp
  entities/door.h
  entities/dragger.cpp
  entities/dragger.h
  entities/drop_pickup.cpp
  entities/drop_pickup.h
  entities/dummy/blmapchill_police.cpp
  entities/dummy/blmapchill_police.h
  entities/dummy/dummybase.cpp
  entities/dummy/dummybase.h
  entities/flag.cpp
  entities/flag.h
  entities/gun.cpp
  entities/gun.h
  entities/homing_missile.cpp
  entities/homing_missile.h
  entities/laser.cpp
  entities/laser.h
  entities/laser_ddpp.cpp
  entities/light.cpp
  entities/light.h
  entities/meteor.cpp
  entities/meteor.h
  entities/pickup.cpp
  entities/pickup.h
  entities/pickup_ddpp.cpp
  entities/plant.cpp
  entities/plant.h
  entities/plasma.cpp
  entities/plasma.h
  entities/plasmabullet.cpp
  entities/plasmabullet.h
  entities/projectile.cpp
  entities/projectile.h
  entities/stable_projectile.cpp
  entities/stable_projectile.h
  entities/weapon.cpp
  entities/weapon.h
  entity.cpp
  entity.h
  eventhandler.cpp
  eventhandler.h
  gamecontext.cpp
  gamecontext.h
  gamecontext_chat.cpp
  gamecontext_ddpp.cpp
  gamecontext_minigames.cpp
  gamecontext_onmessage.cpp
  gamecontext_quests.cpp
  gamecontext_ratelimits.cpp
  gamecontext_sql.cpp
  gamecontext_survival.cpp
  gamecontext_trades.cpp
  gamecontext_votes.cpp
  gamecontroller.cpp
  gamecontroller.h
  gamemodes/DDRace.cpp
  gamemodes/DDRace.h
  gamemodes/ctf.cpp
  gamemodes/gamemode.h
  gameworld.cpp
  gameworld.h
  gameworld_ddpp.cpp
  letters.cpp
  letters.h
  player.cpp
  player.h
  player_ddpp.cpp
  save.cpp
  save.h
  score.h
  score/file_score.cpp
  score/file_score.h
  score/sql_score.cpp
  score/sql_score.h
  teams.cpp
  teams.h
)
set(SERVER_SRC ${ENGINE_SERVER} ${GAME_SERVER})
if(TARGET_OS STREQUAL "windows")
  set(SERVER_ICON "other/icons/teeworlds_srv_cl.rc")
else()
  set(SERVER_ICON)
endif()

# Libraries
set(LIBS_SERVER
  ${LIBS}
  ${MYSQL_LIBRARIES}
  # Add pthreads (on non-Windows) at the end, so that other libraries can depend
  # on it.
  ${CMAKE_THREAD_LIBS_INIT}
)

# Target
set(TARGET_SERVER ${SERVER_EXECUTABLE})
add_executable(${TARGET_SERVER}
  ${DEPS}
  ${SERVER_SRC}
  ${SERVER_ICON}
  $<TARGET_OBJECTS:engine-shared>
  $<TARGET_OBJECTS:game-shared>
)
target_link_libraries(${TARGET_SERVER} ${LIBS_SERVER})
list(APPEND TARGETS_OWN ${TARGET_SERVER})
list(APPEND TARGETS_LINK ${TARGET_SERVER})

if(TARGET_OS AND TARGET_OS STREQUAL "mac")
  set(SERVER_LAUNCHER_SRC src/osxlaunch/server.m)
  set(TARGET_SERVER_LAUNCHER ${TARGET_SERVER}-Launcher)
  add_executable(${TARGET_SERVER_LAUNCHER} ${SERVER_LAUNCHER_SRC})
  target_link_libraries(${TARGET_SERVER_LAUNCHER} ${COCOA})
  list(APPEND TARGETS_OWN ${TARGET_SERVER_LAUNCHER})
  list(APPEND TARGETS_LINK ${TARGET_SERVER_LAUNCHER})
endif()

########################################################################
# VARIOUS TARGETS
########################################################################

set_src(MASTERSRV_SRC GLOB src/mastersrv mastersrv.cpp mastersrv.h)
set_src(TWPING_SRC GLOB src/twping twping.cpp)

set(TARGET_MASTERSRV mastersrv)
set(TARGET_TWPING twping)

add_executable(${TARGET_MASTERSRV} EXCLUDE_FROM_ALL ${MASTERSRV_SRC} $<TARGET_OBJECTS:engine-shared> ${DEPS})
add_executable(${TARGET_TWPING} EXCLUDE_FROM_ALL ${TWPING_SRC} $<TARGET_OBJECTS:engine-shared> ${DEPS})

target_link_libraries(${TARGET_MASTERSRV} ${LIBS})
target_link_libraries(${TARGET_TWPING} ${LIBS})

list(APPEND TARGETS_OWN ${TARGET_MASTERSRV} ${TARGET_TWPING})
list(APPEND TARGETS_LINK ${TARGET_MASTERSRV} ${TARGET_TWPING})

set(TARGETS_TOOLS)
set_src(TOOLS GLOB src/tools
  config_common.h
  config_retrieve.cpp
  config_store.cpp
  confusables.cpp
  crapnet.cpp
  ddpp_map_chiller.cpp
  ddpp_map_gen.cpp
  dilate.cpp
  dummy_map.cpp
  fake_server.cpp
  map_resave.cpp
  map_version.cpp
  packetgen.cpp
  tileset_borderadd.cpp
  tileset_borderfix.cpp
  tileset_borderrem.cpp
  tileset_borderset.cpp
  uuid.cpp
)
foreach(ABS_T ${TOOLS})
  file(RELATIVE_PATH T "${PROJECT_SOURCE_DIR}/src/tools/" ${ABS_T})
  if(T MATCHES "\\.cpp$")
    string(REGEX REPLACE "\\.cpp$" "" TOOL "${T}")
    set(TOOL_DEPS ${DEPS})
    set(TOOL_LIBS ${LIBS})
    if(TOOL MATCHES "^(dilate|map_convert_07|map_optimize|map_extract|map_replace_image|tileset_.*)$")
        list(APPEND TOOL_DEPS ${PNGLITE_DEP})
        list(APPEND TOOL_LIBS ${PNGLITE_LIBRARIES})
        list(APPEND TOOL_INCLUDE_DIRS ${PNGLITE_INCLUDE_DIRS})
    endif()
    if(TOOL MATCHES "^config_")
      list(APPEND EXTRA_TOOL_SRC "src/tools/config_common.h")
    endif()
    set(EXCLUDE_FROM_ALL)
    if(DEV)
      set(EXCLUDE_FROM_ALL EXCLUDE_FROM_ALL)
    endif()
    add_executable(${TOOL} ${EXCLUDE_FROM_ALL}
      ${TOOL_DEPS}
      src/tools/${TOOL}.cpp
      ${EXTRA_TOOL_SRC}
      $<TARGET_OBJECTS:engine-shared>
    )
    target_include_directories(${TOOL} PRIVATE ${TOOL_INCLUDE_DIRS})
    target_link_libraries(${TOOL} ${TOOL_LIBS})
    list(APPEND TARGETS_TOOLS ${TOOL})
  endif()
endforeach()

list(APPEND TARGETS_OWN ${TARGETS_TOOLS})
list(APPEND TARGETS_LINK ${TARGETS_TOOLS})

add_custom_target(tools DEPENDS ${TARGETS_TOOLS})
add_custom_target(everything DEPENDS ${TARGETS_OWN})

########################################################################
# INSTALLATION
########################################################################

function(escape_regex VAR STRING)
  string(REGEX REPLACE "([][^$.+*?|()\\\\])" "\\\\\\1" ESCAPED "${STRING}")
  set(${VAR} ${ESCAPED} PARENT_SCOPE)
endfunction()

function(escape_backslashes VAR STRING)
  string(REGEX REPLACE "\\\\" "\\\\\\\\" ESCAPED "${STRING}")
  set(${VAR} ${ESCAPED} PARENT_SCOPE)
endfunction()

function(max_length VAR)
  set(MAX_LENGTH 0)
  foreach(str ${ARGN})
    string(LENGTH ${str} LENGTH)
    if(LENGTH GREATER MAX_LENGTH)
      set(MAX_LENGTH ${LENGTH})
    endif()
  endforeach()
  set(${VAR} ${MAX_LENGTH} PARENT_SCOPE)
endfunction()

# Tries to generate a list of regex that matches everything except the given
# parameters.
function(regex_inverted VAR)
  max_length(MAX_LENGTH ${ARGN})
  math(EXPR UPPER_BOUND "${MAX_LENGTH}-1")

  set(REMAINING ${ARGN})
  set(RESULT)

  foreach(i RANGE ${UPPER_BOUND})
    set(TEMP ${REMAINING})
    set(REMAINING)
    foreach(str ${TEMP})
      string(LENGTH ${str} LENGTH)
      if(i LESS LENGTH)
        list(APPEND REMAINING ${str})
      endif()
    endforeach()

    set(ADDITIONAL)
    foreach(outer ${REMAINING})
      string(SUBSTRING ${outer} 0 ${i} OUTER_PREFIX)
      set(CHARS "")
      foreach(inner ${REMAINING})
        string(SUBSTRING ${inner} 0 ${i} INNER_PREFIX)
        if(OUTER_PREFIX STREQUAL INNER_PREFIX)
          string(SUBSTRING ${inner} ${i} 1 INNER_NEXT)
          set(CHARS "${CHARS}${INNER_NEXT}")
        endif()
      endforeach()
      escape_regex(OUTER_PREFIX_ESCAPED "${OUTER_PREFIX}")

      list(APPEND ADDITIONAL "${OUTER_PREFIX_ESCAPED}([^${CHARS}]|$)")
    endforeach()
    list(REMOVE_DUPLICATES ADDITIONAL)
    list(APPEND RESULT ${ADDITIONAL})
  endforeach()
  set(${VAR} ${RESULT} PARENT_SCOPE)
endfunction()

set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_GENERATOR TGZ TXZ)
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_STRIP_FILES TRUE)
set(CPACK_COMPONENTS_ALL portable)
set(CPACK_SOURCE_GENERATOR ZIP TGZ TBZ2 TXZ)
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
if(VERSION_PATCH STREQUAL "0")
  set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR})
else()
  set(CPACK_PACKAGE_VERSION ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH})
endif()
set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})

if(TARGET_OS AND TARGET_BITS)
  if(TARGET_OS STREQUAL "windows")
    set(CPACK_SYSTEM_NAME "win${TARGET_BITS}")
    set(CPACK_GENERATOR ZIP)
  elseif(TARGET_OS STREQUAL "linux")
    # Let compiler tell its arch
    # Both gcc and clang support -dumpmachine
    execute_process(
      COMMAND ${CMAKE_C_COMPILER} -dumpmachine
      OUTPUT_VARIABLE ARCHITECTURE_TUPLE
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT ARCHITECTURE_TUPLE)
      # If you're really using a weird compiler, then assume Intel here.
      message(WARNING "Your compiler doesn't support -dumpmachine, this is weird")
      if(TARGET_BITS EQUAL 32)
        set(ARCHITECTURE "x86")
      elseif(TARGET_BITS EQUAL 64)
        set(ARCHITECTURE "x86_64")
      endif()
    else()
      string(REGEX MATCH "^[^-]*" ARCHITECTURE "${ARCHITECTURE_TUPLE}")
      if(ARCHITECTURE MATCHES "i.86")
        set(ARCHITECTURE "x86")
      endif()
    endif()
    set(CPACK_SYSTEM_NAME "linux_${ARCHITECTURE}")
  elseif(TARGET_OS STREQUAL "mac")
    set(CPACK_SYSTEM_NAME "osx")
    set(CPACK_GENERATOR DMG)
  endif()
endif()

set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME})
set(CPACK_ARCHIVE_PORTABLE_FILE_NAME ${CPACK_PACKAGE_FILE_NAME})
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-src)
set(CPACK_SOURCE_FILES
  CMakeLists.txt
  README.md
  autoexec_server.cfg
  cmake/
  data/
  datasrc/
  ddnet-libs/
  license.txt
  other/
  scripts/
  src/
  storage.cfg
)
set(CPACK_SOURCE_IGNORE_FILES
  "\\\\.pyc$"
  "/\\\\.git"
  "/__pycache__/"
)

regex_inverted(CPACK_SOURCE_FILES_INVERTED ${CPACK_SOURCE_FILES})
escape_regex(PROJECT_SOURCE_DIR_ESCAPED ${PROJECT_SOURCE_DIR})

foreach(str ${CPACK_SOURCE_FILES_INVERTED})
  escape_backslashes(STR_ESCAPED "${PROJECT_SOURCE_DIR_ESCAPED}/${str}")
  list(APPEND CPACK_SOURCE_IGNORE_FILES "${STR_ESCAPED}")
endforeach()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME ${PROJECT_NAME})

set(CPACK_TARGETS
  ${TARGET_SERVER}
  config_retrieve
  config_store
)
set(CPACK_DIRS data)
set(CPACK_FILES
  license.txt
  storage.cfg
  autoexec_server.cfg
  ${COPY_FILES}
)
if(TARGET_OS STREQUAL "windows")
  list(APPEND CPACK_FILES other/config_directory.bat)
endif()

if(NOT DEV)
  install(DIRECTORY data DESTINATION share/ddnet COMPONENT data)
  install(TARGETS ${TARGET_SERVER} DESTINATION bin COMPONENT server)
  install(TARGETS ${TARGETS_TOOLS} DESTINATION lib/ddnet COMPONENT tools)
endif()

if(DEV)
  # Don't generate CPack targets.
elseif(CMAKE_VERSION VERSION_LESS 3.6 OR CMAKE_VERSION VERSION_EQUAL 3.6)
  message(WARNING "Cannot create CPack targets, CMake version too old. Use CMake 3.6 or newer.")
else()
  set(EXTRA_ARGS DESTINATION ${CPACK_PACKAGE_FILE_NAME} COMPONENT portable EXCLUDE_FROM_ALL)
  install(TARGETS ${CPACK_TARGETS} ${EXTRA_ARGS})
  install(DIRECTORY ${CPACK_DIRS} ${EXTRA_ARGS})
  install(FILES ${CPACK_FILES} ${EXTRA_ARGS})
endif()

set(PACKAGE_TARGETS)

if(DMGTOOLS_FOUND OR HDIUTIL)
  file(MAKE_DIRECTORY bundle/server/)
  configure_file(other/bundle/server/Info.plist.in bundle/server/Info.plist)

  if(HDIUTIL)
    set(DMG_PARAMS --hdiutil ${HDIUTIL})
  elseif(DMGTOOLS_FOUND)
    set(DMG_PARAMS --dmgtools ${DMG} ${HFSPLUS} ${NEWFS_HFS})
  endif()
  set(DMG_TMPDIR pack_${CPACK_PACKAGE_FILE_NAME}_dmg)
  set(DMG_MKDIRS
    ${TARGET_SERVER}.app
    ${TARGET_SERVER}.app/Contents
    ${TARGET_SERVER}.app/Contents/MacOS
    ${TARGET_SERVER}.app/Contents/Resources
    ${TARGET_SERVER}.app/Contents/Resources/data
    ${TARGET_SERVER}.app/Contents/Resources/data/mapres
  )
  set(DMG_MKDIR_COMMANDS)
  foreach(dir ${DMG_MKDIRS})
    list(APPEND DMG_MKDIR_COMMANDS COMMAND ${CMAKE_COMMAND} -E make_directory ${DMG_TMPDIR}/${dir})
  endforeach()
  add_custom_command(OUTPUT ${CPACK_PACKAGE_FILE_NAME}.dmg
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${DMG_TMPDIR}
    ${DMG_MKDIR_COMMANDS}

    # SERVER
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/data/maps ${DMG_TMPDIR}/${TARGET_SERVER}.app/Contents/Resources/data/maps
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/other/icons/${TARGET_SERVER}.icns ${DMG_TMPDIR}/${TARGET_SERVER}.app/Contents/Resources/
    COMMAND ${CMAKE_COMMAND} -E copy bundle/server/Info.plist ${PROJECT_SOURCE_DIR}/other/bundle/server/PkgInfo ${DMG_TMPDIR}/${TARGET_SERVER}.app/Contents/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARGET_SERVER}> $<TARGET_FILE:${TARGET_SERVER_LAUNCHER}> ${DMG_TMPDIR}/${TARGET_SERVER}.app/Contents/MacOS/

    # DMG
    COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/scripts/dmg.py create ${DMG_PARAMS} ${CPACK_PACKAGE_FILE_NAME}.dmg ${CPACK_PACKAGE_FILE_NAME} ${DMG_TMPDIR}

    DEPENDS
      ${TARGET_SERVER_LAUNCHER}
      ${TARGET_SERVER}
      ${PROJECT_BINARY_DIR}/bundle/server/Info.plist
      data
      other/bundle/server/PkgInfo
      other/icons/${TARGET_SERVER}.icns
      scripts/dmg.py
  )
  add_custom_target(package_dmg DEPENDS ${CPACK_PACKAGE_FILE_NAME}.dmg)
  list(APPEND PACKAGE_TARGETS package_dmg)
endif()

foreach(ext zip tar.gz tar.xz)
  set(TAR_MODE c)
  set(TAR_EXTRA_ARGS)
  string(REPLACE . _ EXT_SLUG ${ext})

  set(TMPDIR pack_${CPACK_PACKAGE_FILE_NAME}_${EXT_SLUG}/${CPACK_PACKAGE_FILE_NAME})

  set(COPY_FILE_COMMANDS)
  set(COPY_DIR_COMMANDS)
  set(COPY_TARGET_COMMANDS)
  set(STRIP_TARGET_COMMANDS)
  foreach(file ${CPACK_FILES})
    list(APPEND COPY_FILE_COMMANDS COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/${file} ${TMPDIR}/)
  endforeach()
  foreach(dir ${CPACK_DIRS})
    list(APPEND COPY_DIR_COMMANDS COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/${dir} ${TMPDIR}/${dir})
  endforeach()
  foreach(target ${CPACK_TARGETS})
    list(APPEND COPY_TARGET_COMMANDS COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${target}> ${TMPDIR}/)
    list(APPEND STRIP_TARGET_COMMANDS COMMAND strip -s ${TMPDIR}/$<TARGET_FILE_NAME:${target}>)
  endforeach()

  if(ext STREQUAL zip)
    set(TAR_EXTRA_ARGS --format=zip)
  elseif(ext STREQUAL tar.gz)
    set(TAR_MODE cz)
  elseif(ext STREQUAL tar.xz)
    set(TAR_MODE cJ)
  endif()
  add_custom_command(OUTPUT ${CPACK_PACKAGE_FILE_NAME}.${ext}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${TMPDIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${TMPDIR}
    ${COPY_FILE_COMMANDS}
    ${COPY_DIR_COMMANDS}
    ${COPY_TARGET_COMMANDS}
    ${STRIP_TARGET_COMMANDS}
    COMMAND ${CMAKE_COMMAND} -E chdir pack_${CPACK_PACKAGE_FILE_NAME}_${EXT_SLUG} ${CMAKE_COMMAND} -E tar ${TAR_MODE} ../${CPACK_PACKAGE_FILE_NAME}.${ext} ${TAR_EXTRA_ARGS} -- ${CPACK_PACKAGE_FILE_NAME}/
    DEPENDS ${CPACK_TARGETS}
  )
  add_custom_target(package_${EXT_SLUG} DEPENDS ${CPACK_PACKAGE_FILE_NAME}.${ext})
  list(APPEND PACKAGE_TARGETS package_${EXT_SLUG})
endforeach()

set(PACKAGE_DEFAULT tar_xz)
if(TARGET_OS STREQUAL "windows")
  set(PACKAGE_DEFAULT zip)
elseif(TARGET_OS STREQUAL "mac")
  set(PACKAGE_DEFAULT dmg)
endif()
add_custom_target(package_default DEPENDS package_${PACKAGE_DEFAULT})

add_custom_target(package_all DEPENDS ${PACKAGE_TARGETS})

# Unset these variables, they might do something in the future of CPack.
unset(CPACK_SOURCE_FILES)
unset(CPACK_SOURCE_FILES_INVERTED)
unset(CPACK_TARGETS)
unset(CPACK_DIRS)
unset(CPACK_FILES)

include(CPack)

########################################################################
# COMPILER-SPECIFICS
########################################################################

# In the future (CMake 3.8.0+), use source_group(TREE ...)
macro(source_group_tree dir)
  file(GLOB ents RELATIVE ${PROJECT_SOURCE_DIR}/${dir} ${PROJECT_SOURCE_DIR}/${dir}/*)
  foreach(ent ${ents})
    if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${dir}/${ent})
      source_group_tree(${dir}/${ent})
    else()
      string(REPLACE "/" "\\" group ${dir})
      source_group(${group} FILES ${PROJECT_SOURCE_DIR}/${dir}/${ent})
    endif()
  endforeach()
endmacro()
source_group_tree(src)

set(TARGETS ${TARGETS_OWN} ${TARGETS_DEP})

foreach(target ${TARGETS})
  if(MSVC)
    target_compile_options(${target} PRIVATE $<$<NOT:${DBG}>:/MT> $<${DBG}:/MTd>) # Use static CRT
    target_compile_options(${target} PRIVATE /MP) # Use multiple cores
    target_compile_options(${target} PRIVATE /EHsc) # Only catch C++ exceptions with catch.
    target_compile_options(${target} PRIVATE /GS) # Protect the stack pointer.
    target_compile_options(${target} PRIVATE /wd4996) # Use of non-_s functions.
  endif()
  if(OUR_FLAGS_LINK)
    target_link_libraries(${target} ${OUR_FLAGS_LINK})
  endif()
  if(OUR_FLAGS)
    target_compile_options(${target} PRIVATE ${OUR_FLAGS})
  endif()
  if(DEFINE_FORTIFY_SOURCE)
    if(MINGW)
      target_compile_definitions(${target} PRIVATE $<$<NOT:$<CONFIG:Debug>>:_FORTIFY_SOURCE=0>) # Currently broken in MinGW, see https://sourceforge.net/p/mingw-w64/discussion/723798/thread/b9d24f041f/
    else()
      target_compile_definitions(${target} PRIVATE $<$<NOT:$<CONFIG:Debug>>:_FORTIFY_SOURCE=2>) # Detect some buffer overflows.
    endif()
  endif()
endforeach()

foreach(target ${TARGETS_LINK})
  if(MSVC)
    set_property(TARGET ${target} APPEND PROPERTY LINK_FLAGS /SAFESEH:NO) # Disable SafeSEH because the shipped libraries don't support it (would cause error LNK2026 otherwise).
  endif()
  if(TARGET_OS STREQUAL "mac")
    target_link_libraries(${target} -stdlib=libc++)
    target_link_libraries(${target} -mmacosx-version-min=10.7)
    # Fixes: Undefined symbols for architecture x86_64:
    #   "___isOSVersionAtLeast", referenced from:
    #       _singleipconnect in libcurl.a(libcurl_la-connect.o)
    target_link_libraries(${target} -Wl,-undefined -Wl,dynamic_lookup)
  endif()
  if((MINGW OR TARGET_OS STREQUAL "linux") AND PREFER_BUNDLED_LIBS)
    # Statically link the standard libraries with on MinGW/Linux so we don't
    # have to ship them as DLLs.
    target_link_libraries(${target} -static-libgcc)
    target_link_libraries(${target} -static-libstdc++)
  endif()
endforeach()

foreach(target ${TARGETS_OWN})
  if((CMAKE_VERSION VERSION_GREATER 3.1 OR CMAKE_VERSION VERSION_EQUAL 3.1) AND NOT TARGET_OS STREQUAL "mac")
    set_property(TARGET ${target} PROPERTY CXX_STANDARD 11)
    set_property(TARGET ${target} PROPERTY CXX_STANDARD_REQUIRED ON)
  endif()

  if(MSVC)
    target_compile_options(${target} PRIVATE /wd4244) # Possible loss of data (float -> int, int -> float, etc.).
    target_compile_options(${target} PRIVATE /wd4267) # Possible loss of data (size_t - int on win64).
    target_compile_options(${target} PRIVATE /wd4800) # Implicit conversion of int to bool.
  endif()
  if(OUR_FLAGS_OWN)
    target_compile_options(${target} PRIVATE ${OUR_FLAGS_OWN})
  endif()
  target_include_directories(${target} PRIVATE ${PROJECT_BINARY_DIR}/src)
  target_include_directories(${target} PRIVATE src)
  target_compile_definitions(${target} PRIVATE $<$<CONFIG:Debug>:CONF_DEBUG>)
  target_include_directories(${target} PRIVATE ${ZLIB_INCLUDE_DIRS})
  target_compile_definitions(${target} PRIVATE GLEW_STATIC)
  if(CRYPTO_FOUND)
    target_compile_definitions(${target} PRIVATE CONF_OPENSSL)
    target_include_directories(${target} PRIVATE ${CRYPTO_INCLUDE_DIRS})
  endif()
  if(WEBSOCKETS)
    target_compile_definitions(${target} PRIVATE CONF_WEBSOCKETS)
    target_include_directories(${target} PRIVATE ${WEBSOCKETS_INCLUDE_DIRS})
  endif()
  if(MYSQL)
    target_compile_definitions(${target} PRIVATE CONF_SQL)
    target_include_directories(${target} PRIVATE ${MYSQL_INCLUDE_DIRS})
  endif()
  if(AUTOUPDATE)
    target_compile_definitions(${target} PRIVATE CONF_AUTOUPDATE)
  endif()
endforeach()

foreach(target ${TARGETS_DEP})
  if(MSVC)
    target_compile_options(${target} PRIVATE /W0)
  endif()
  if(OUR_FLAGS_DEP)
    target_compile_options(${target} PRIVATE ${OUR_FLAGS_DEP})
  endif()
endforeach()
