function(insta_patch_engine)
endfunction()

function(insta_patch_server)
    set_src(INSTA_SERVER GLOB_RECURSE src/insta/server
      gamemodes/insta_core/insta_core.cpp
      gamemodes/insta_core/insta_core.h
      skin_info_manager.cpp
      skin_info_manager.h
    )

    set(GAME_SERVER ${GAME_SERVER} PARENT_SCOPE)
    list(APPEND GAME_SERVER ${INSTA_EXTERNAL_GAMEMODES} ${INSTA_SERVER})
    set(GAME_SERVER ${GAME_SERVER} PARENT_SCOPE)
endfunction()
