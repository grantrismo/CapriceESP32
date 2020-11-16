idf_component_register(SRCS "src/audio.c"
                            "src/display.c"
                            "src/event.c"
                            "src/backlight.c"
                            "src/battery.c"
                            "src/keypad.c"
                            "src/sdcard.c"
                            "src/settings.c"
                            "src/system.c"

                            INCLUDE_DIRS "." "include" "../common" "../CPC/include" "../NativeCPC/include" "../rsc"
                            PRIV_REQUIRES "main" "esp_adc_cal" "fatfs" "nvs_flash" "app_update")

#target_compile_options(${COMPONENT_TARGET} PUBLIC -Os -mlongcalls -Wno-error -Wstrict-aliasing -Wuninitialized -Wno-error=maybe-uninitialized)
