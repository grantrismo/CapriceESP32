#
# Main component makefile.
#
# This Makefile can be left empty. By default, it will take the sources in the
# src/ directory, compile them and link them into lib(subdirectory_name).a
# in the build directory. This behaviour is entirely configurable,
# please read the ESP-IDF documents if you need to do this.
#
COMPONENT_PRIV_INCLUDEDIRS = include
COMPONENT_SRCDIRS = src
CPPFLAGS := -BPS16 -DBPP16 -DLSB_FIRST -DESP32
CFLAGS :=  -O3 -mlongcalls -Wno-error
