# Copyright (C) Yan Ruibing

mkdir -p $TCH_OBJS/src/mq

zyre_objs_dir=$TCH_OBJS$tch_regex_dirsep

ZYRE_MAKEFILE=$TCH_OBJS/MQMakefile

echo "creating $ZYRE_MAKEFILE"

cat << END                                                     > $ZYRE_MAKEFILE

CC =	$CC
CFLAGS = $CFLAGS
CPP =	$CPP
LINK =	$LINK

END

zyre_incs=`echo $ZYRE_INC $TCH_OBJS\
    | sed -e "s/  *\([^ ][^ ]*\)/$tch_regex_cont$tch_include_opt\1/g" \
          -e "s/\//$tch_regex_dirsep/g"`

cat << END                                                    >> $ZYRE_MAKEFILE

ZYRE_INCS = $tch_include_opt$zyre_incs

END

zyre_all_srcs="$ZYRE_SRCS"

# the core dependencies and include paths

zyre_deps=`echo $ZYRE_DEPS \
    | sed -e "s/  *\([^ ][^ ]*\)/$tch_regex_cont\1/g" \
          -e "s/\//$tch_regex_dirsep/g"`

zyre_incs=`echo $ZYRE_INC $TCH_OBJS \
    | sed -e "s/  *\([^ ][^ ]*\)/$tch_regex_cont$tch_include_opt\1/g" \
          -e "s/\//$tch_regex_dirsep/g"`

cat << END                                                    >> $ZYRE_MAKEFILE

ZYRE_DEPS = $zyre_deps


ZYRE_INCS = $tch_include_opt$zyre_incs

END

zyre_all_srcs=`echo $zyre_all_srcs | sed -e "s/\//$tch_regex_dirsep/g"`

for zyre_src in $TCH_ADDON_SRCS
do
    zyre_obj="addon/`basename \`dirname $zyre_src\``"

    test -d $TCH_OBJS/$zyre_obj || mkdir -p $TCH_OBJS/$zyre_obj

    zyre_obj=`echo $zyre_obj/\`basename $zyre_src\` \
        | sed -e "s/\//$tch_regex_dirsep/g"`

    zyre_all_srcs="$zyre_all_srcs $zyre_obj"
done

zyre_all_objs=`echo $zyre_all_srcs \
    | sed -e "s#\([^ ]*\.\)cpp#$TCH_OBJS\/\1$tch_objext#g" \
          -e "s#\([^ ]*\.\)cc#$TCH_OBJS\/\1$tch_objext#g" \
          -e "s#\([^ ]*\.\)c#$TCH_OBJS\/\1$tch_objext#g" \
          -e "s#\([^ ]*\.\)S#$TCH_OBJS\/\1$tch_objext#g"`

if test -n "$TCH_RES"; then
   zyre_res=$TCH_RES
else
   zyre_res="$TCH_RC $TCH_ICONS"
   zyre_rcc=`echo $TCH_RCC | sed -e "s/\//$tch_regex_dirsep/g"`
fi

zyre_deps=`echo $zyre_all_objs $zyre_res \
    | sed -e "s/  *\([^ ][^ ]*\)/$tch_regex_cont\1/g" \
          -e "s/\//$tch_regex_dirsep/g"`

zyre_objs=`echo $zyre_all_objs \
    | sed -e "s/  *\([^ ][^ ]*\)/$tch_long_regex_cont\1/g" \
          -e "s/\//$tch_regex_dirsep/g"`

cat << END                                                    >> $ZYRE_MAKEFILE

build:	binary

binary:	$TCH_OBJS${tch_dirsep}zyre.a$tch_binext

$TCH_OBJS${tch_dirsep}zyre.a$tch_binext: $zyre_deps$tch_spacer
	\$(LINK) $tch_long_start$tch_binout$TCH_OBJS${tch_dirsep}zyre.a$tch_binext$tch_long_cont$zyre_objs
	$zyre_rcc
$tch_long_end

modules:

END

if test -n "$TCH_PCH"; then
    zyre_cc="\$(CC) $tch_compile_opt \$(CFLAGS) $tch_use_pch \$(ALL_INCS)"
else
    zyre_cc="\$(CC) $tch_compile_opt \$(CFLAGS) \$(ZYRE_INCS)"
fi

# the core sources

for zyre_src in $ZYRE_SRCS
do
    zyre_src=`echo $zyre_src | sed -e "s/\//$tch_regex_dirsep/g"`
    zyre_obj=`echo $zyre_src \
        | sed -e "s#^\(.*\.\)cpp\\$#$zyre_objs_dir\1$tch_objext#g" \
              -e "s#^\(.*\.\)cc\\$#$zyre_objs_dir\1$tch_objext#g" \
              -e "s#^\(.*\.\)c\\$#$zyre_objs_dir\1$tch_objext#g" \
              -e "s#^\(.*\.\)S\\$#$zyre_objs_dir\1$tch_objext#g"`

    cat << END                                                >> $ZYRE_MAKEFILE

$zyre_obj:	\$(ZYRE_DEPS)$tch_cont$zyre_src
	$zyre_cc$tch_tab$tch_objout$zyre_obj$tch_tab$zyre_src$TCH_AUX

END

done

