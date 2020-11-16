
idf_component_register(
  SRCS "main.c"
  INCLUDE_DIRS "." "CPC/include" "common" "hardware/include" "NativeCPC/include" "rsc"
  PRIV_REQUIRES CPC hardware NativeCPC)

target_compile_options(${COMPONENT_TARGET} PUBLIC -Os -mlongcalls -Wno-error -Wstrict-aliasing -Wuninitialized -Wno-error=maybe-uninitialized)
