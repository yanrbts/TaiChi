# Copyright (C) Yan Ruibing

help=no

TCH_BUILD=

CC=${CC:-cc}
CPP=
TCH_OBJS=objs

TCH_DEBUG=NO
TCH_ADDON_SRCS=
TCH_ADDON_DEPS=
TCH_CC_OPT=
TCH_LD_OPT=
CPU=NO
TCH_LIB=" -lpthread -lzmq -lczmq -Wl,-rpath,./lib -L./lib/ -lzyre"

TCH_PLATFORM=

opt=

for option
do
    opt="$opt `echo $option | sed -e \"s/\(--[^=]*=\)\(.* .*\)/\1'\2'/\"`"

    case "$option" in
        -*=*) value=`echo "$option" | sed -e 's/[-_a-zA-Z0-9]*=//'` ;;
           *) value="" ;;
    esac

    case "$option" in
        --help)                          help=yes                   ;;
        --with-cpu-opt=*)                CPU="$value"               ;;
        --with-debug)                    TCH_DEBUG=YES              ;;
        *)
            echo "$0: error: invalid option \"$option\""
            exit 1
        ;;
    esac
done

TCH_CONFIGURE="$opt"

if [ $help = yes ]; then

cat << END

  --help                             print this message
  --with-cpu-opt=CPU                 build for the specified CPU, valid values:
                                     pentium, pentiumpro, pentium3, pentium4,
                                     athlon, opteron, sparc32, sparc64, ppc64
  --with-debug                       enable debug logging

END

    exit 1
fi
