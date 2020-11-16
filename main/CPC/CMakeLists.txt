idf_component_register(SRCS "src/Prefs.c"
                            "src/gbuf.c"
                            "src/CPC.c"
                            "src/vfsfile.c"
                            "src/Files.c"
                            "src/Keyboard.c"
                            "src/Sound.c"
                            "src/ttgui.c"
                            "src/Routines.c"
                            "src/e_exp.c"
                            "src/vkb_layout.c"
                            "src/s_rint.c"
                    INCLUDE_DIRS "." "include" "../common" "../hardware/include" "../NativeCPC/include" "../rsc"
                    PRIV_REQUIRES hardware NativeCPC)

target_compile_options(${COMPONENT_TARGET} PUBLIC -Ofast -mlongcalls -Wno-error -Wstrict-aliasing -Wuninitialized -Wno-error=maybe-uninitialized)
