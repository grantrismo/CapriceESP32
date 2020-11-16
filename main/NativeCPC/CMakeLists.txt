idf_component_register(SRCS "src/Native_DiskSave.c"
                            "src/Native_DiskReadCatalogue.c"
                            "src/Native_AutoStart.c"
                            "src/Native_WinCopyScreen.c"
                            "src/Native_SoundCallback.c"
                            "src/Native_DiskLoad.c"
                            "src/Native_DiskFormat.c"
                            "src/Native_DiskEject.c"
                            "src/Native_CPCStop.c"
                            "src/Native_CPCSetColor.c"
                            "src/Native_CPCReset.c"
                            "src/Native_CPCStart.c"
                            "src/Native_CPCExecute.c"

                            INCLUDE_DIRS "." "include" "../common" "../hardware/include" "../CPC/include" "../rsc")

target_compile_options(${COMPONENT_TARGET} PUBLIC -Ofast -mlongcalls -Wno-error -Wstrict-aliasing -Wuninitialized -Wno-error=maybe-uninitialized)
