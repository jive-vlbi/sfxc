# I hate jsoncpp.... this is a nightmare to install
#

export PKG_CONFIG_PATH=$PWD/lib/localusr/lib/pkgconfig:$PKG_CONFIG_PATH

echo $PKG_CONFIG_PATH
aclocal && autoheader && autoconf && automake --add-missing && ./configure
