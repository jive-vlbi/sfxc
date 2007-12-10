SUBDIRS = calc10

prefix=@prefix@
pkg_config_path=@prefix@/lib/pkgconfig

all-local:
	echo installing common library
	make -C common install prefix=${prefix} PKG_CONFIG_PATH=${pkg_config_path}
	echo installing testunit library
	make -C testunit install prefix=${prefix} PKG_CONFIG_PATH=${pkg_config_path}
	echo installing netlib library
	make -C netlib install prefix=${prefix} PKG_CONFIG_PATH=${pkg_config_path}
	echo installing containers library
	make -C containers install prefix=${prefix} PKG_CONFIG_PATH=${pkg_config_path}

clean-local:
	make -C common clean prefix=${prefix} PKG_CONFIG_PATH=${pkg_config_path}
	make -C testunit clean prefix=${prefix} PKG_CONFIG_PATH=${pkg_config_path}
	make -C netlib clean prefix=${prefix} PKG_CONFIG_PATH=${pkg_config_path}
	make -C containers clean prefix=${prefix} PKG_CONFIG_PATH=${pkg_config_path}

distclean-local:
	make -C common distclean prefix=${prefix} PKG_CONFIG_PATH=${pkg_config_path}
	make -C testunit distclean prefix=${prefix} PKG_CONFIG_PATH=${pkg_config_path}
	make -C netlib distclean prefix=${prefix} PKG_CONFIG_PATH=${pkg_config_path}
	make -C containers distclean prefix=${prefix} PKG_CONFIG_PATH=${pkg_config_path}
