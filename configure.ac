AC_PREREQ([2.68])

AC_INIT([quetoo],[0.1.0],[jay@jaydolan.com])
AC_USE_SYSTEM_EXTENSIONS

AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR(m4)
AC_CONFIG_SRCDIR(src/shared.c)

AC_CANONICAL_TARGET
AC_DEFINE_UNQUOTED(BUILD_HOST,
	"${target_cpu}-${target_os}",
	[Set to the canonical name of the target machine]
)

AC_SUBST(PROGRAM)
AC_SUBST(VERSION)

AC_CHECK_PROG(GIT, git, 1)

if [[ ! -z "${GIT}" ]]; then
	REVISION=`git rev-parse --short HEAD`
else
	REVISION="unversioned"
fi

AC_DEFINE_UNQUOTED(REVISION, "$REVISION", [Git revision])

AM_INIT_AUTOMAKE([foreign])
AM_SILENT_RULES([yes])

dnl ---------------
dnl Check for tools
dnl ---------------

AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_LIBTOOL

dnl --------------------
dnl Check for pkg-config
dnl --------------------

PKG_PROG_PKG_CONFIG

dnl ---------------
dnl Check host type
dnl ---------------

HOST_NAME=""
HOST_CFLAGS=""
HOST_LDFLAGS=""
HOST_LIBS=""
HOST_DEBUG_CFLAGS=""
HOST_DEBUG_LIBS=""
HOST_PROFILE_CFLAGS=""
HOST_PROFILE_LIBS=""
HOST_SO_CFLAGS=""
DEFAULT_PREFIX="/usr/local"

AC_MSG_CHECKING([host type])
case "$host" in
	*-apple-*)
		HOST_NAME="APPLE"
		HOST_CFLAGS="-I/opt/local/include"
		HOST_LDFLAGS="-L/opt/local/lib -headerpad_max_install_names"
		HOST_LIBS="-ldl"
		HOST_DEBUG_LIBS="-rdynamic"
		HOST_SO_CFLAGS="-fPIC -DPIC"
		AC_MSG_RESULT(apple)
		;;
	*bsd*)
		HOST_NAME="BSD"
		HOST_SO_CFLAGS="-fPIC -DPIC"
		AC_MSG_RESULT(bsd)
		;;
	*-*-mingw*)
		HOST_NAME="WINDOWS"
		HOST_LIBS="-lws2_32 -lole32"
		DEFAULT_PREFIX="/"
		AC_MSG_RESULT(windows)
		AC_PATH_PROGS([WINDRES],
			[windres $host-windres],
		)
		if test "x$WINDRES" = "x"; then
			AC_MSG_ERROR([Could not find windres, please install windres])
		fi
		;;
	*)
		HOST_NAME="LINUX"
		HOST_LIBS="-ldl -lm"
		HOST_DEBUG_LIBS="-rdynamic"
		HOST_SO_CFLAGS="-fPIC -DPIC"
		AC_MSG_RESULT(linux)
		;;
esac

AC_SUBST(HOST_NAME)
AC_SUBST(HOST_CFLAGS)
AC_SUBST(HOST_LIBS)
AC_SUBST(HOST_DEBUG_CFLAGS)
AC_SUBST(HOST_DEBUG_LIBS)
AC_SUBST(HOST_PROFILE_CFLAGS)
AC_SUBST(HOST_PROFILE_LIBS)

CFLAGS="-pipe $CFLAGS $HOST_CFLAGS"
AC_SUBST(CFLAGS)

SO_CFLAGS="$SO_CFLAGS $HOST_SO_CFLAGS"
AC_SUBST(SO_CFLAGS)

LDFLAGS="$LDFLAGS $HOST_LDFLAGS"
AC_SUBST(LDFLAGS)

CPPFLAGS="$CFLAGS $CPPFLAGS"
AC_SUBST(CPPFLAGS)

AM_CONDITIONAL([APPLE], [test "x$HOST_NAME" = "xAPPLE"])
AM_CONDITIONAL([BSD], [test "x$HOST_NAME" = "xBSD"])
AM_CONDITIONAL([WINDOWS], [test "x$HOST_NAME" = "xWINDOWS"])
AM_CONDITIONAL([LINUX], [test "x$HOST_NAME" = "xLINUX"])

dnl --------------
dnl Check for cURL
dnl --------------

PKG_CHECK_MODULES([CURL], [libcurl])

dnl -----------------
dnl Check for curses
dnl -----------------

PKG_CHECK_MODULES([CURSES], [ncurses >= 5.0],, [
	AC_CHECK_HEADER(curses.h,
		CURSES_LIBS="-lpdcurses",
		[AC_MSG_ERROR([Could not find curses.h, please install curses])]
	)
])

dnl --------------
dnl Check for GLib
dnl --------------
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.0.0])

dnl -----------------
dnl Check for libxml2
dnl -----------------

PKG_CHECK_MODULES(XML, libxml-2.0 >= 2.4)

dnl --------------------------------------
dnl Check for SDL2, SDL2_image, SDL2_mixer
dnl --------------------------------------

PKG_CHECK_MODULES([SDL2], [sdl2 >= 2.0.0 SDL2_image SDL2_mixer])

dnl ------------------------
dnl Check for ObjectivelyMVC
dnl ------------------------

PKG_CHECK_MODULES([OBJECTIVELYMVC], [ObjectivelyMVC])

dnl -------------------
dnl Check for PhysicsFS
dnl -------------------

AC_CHECK_HEADER(physfs.h,
	PHYSFS_LIBS="-lphysfs -lz",
	[AC_MSG_ERROR([Could not find physfs.h, please install PhysFS])]
)
AC_SUBST(PHYSFS_LIBS)

dnl -----------------------------------
dnl Sort out OpenGL an flags and libraries
dnl -----------------------------------

HOST_OPENGL_CFLAGS=""
HOST_OPENGL_LIBS=""

case "$host" in
	*-apple-*)
		HOST_OPENGL_LIBS="-framework OpenGL"
		;;
	*-mingw*)
		HOST_OPENGL_LIBS="-lopengl32"
		;;
	*)
		HOST_OPENGL_LIBS="-lGL"
		;;
esac

AC_SUBST(HOST_OPENGL_CFLAGS)
AC_SUBST(HOST_OPENGL_LIBS)

OPENGL_CFLAGS="$OPENGL_CFLAGS $HOST_OPENGL_CFLAGS"
AC_SUBST(OPENGL_CFLAGS)

OPENGL_LIBS="$OPENGL_LIBS $HOST_OPENGL_LIBS"
AC_SUBST(OPENGL_LIBS)

dnl -------------------------------
dnl Check for execinfo.h (optional)
dnl -------------------------------

AC_CHECK_HEADER(execinfo.h,
	AC_DEFINE(HAVE_EXECINFO, 1,
		[Define to 1 if you have the <execinfo.h> header file.]
	),
)

dnl --------------------------
dnl Check for MySQL (optional)
dnl --------------------------

HAVE_MYSQL=no

AC_ARG_WITH(mysql,
	AS_HELP_STRING([--with-mysql],
		[use MySQL (relies on mysql_config)]
	)
)

AC_PATH_PROGS(MYSQL_CONFIG, mysql_config mysql_config5)

if test "x${with_mysql}" != xno -a "x${MYSQL_CONFIG}" != x; then
	MYSQL_CFLAGS=`$MYSQL_CONFIG --cflags`

	save_CPPFLAGS="$CPPFLAGS"
	CPPFLAGS="$CPPFLAGS $MYSQL_CFLAGS"

	AC_CHECK_HEADER(mysql.h,
		HAVE_MYSQL=yes
		AC_DEFINE(HAVE_MYSQL, 1,
			[Define to 1 if you have the mysql.h header file.]
		)
		MYSQL_LIBS=`$MYSQL_CONFIG --libs`,
		HAVE_MYSQL=no
	)

	CPPFLAGS="$save_CPPFLAGS"
fi
AC_SUBST(MYSQL_CFLAGS)
AC_SUBST(MYSQL_LIBS)

dnl ---------------------------------
dnl Check which game modules to build
dnl ---------------------------------

AC_MSG_CHECKING(which game modules to build)

GAME_MODULES="default"

AC_ARG_WITH(games,
    AS_HELP_STRING([--with-games='default ...'],
		[build specified game modules]
	)
)

if test "x${with_games}" != x; then
    GAME_MODULES="${with_games}"
fi

AC_SUBST(GAME_MODULES)
AC_MSG_RESULT($GAME_MODULES)

dnl ---------------------------------
dnl Check which ai modules to build
dnl ---------------------------------

AC_MSG_CHECKING(which ai modules to build)

AI_MODULES="default"

AC_ARG_WITH(ai,
	AS_HELP_STRING([--with-ai='default ...'],
		[build specified ai modules]
	)
)

if test "x${with_ai}" != x; then
	AI_MODULES="${with_ai}"
fi

AC_SUBST(AI_MODULES)
AC_MSG_RESULT($AI_MODULES)

dnl --------------------------
dnl Check which tools to build
dnl --------------------------

AC_MSG_CHECKING(which tools to build)

TOOLS="quemap quetoo-master quetoo-update"

AC_ARG_WITH(tools,
	AS_HELP_STRING([--with-tools='quemap quetoo-master quetoo-update...'],
		[build specified tools]
	)
)

if test "x${with_tools}" != x; then
	TOOLS="${with_tools}"
fi

AC_SUBST(TOOLS)
AC_MSG_RESULT($TOOLS)

dnl ---------------------------------
dnl Check whether to build unit tests
dnl ---------------------------------

BUILD_TESTS=no

AC_ARG_WITH(tests,
	AS_HELP_STRING([--with-tests],
		[build unit tests]
	)
)

AC_MSG_CHECKING(whether to build unit tests)

if test "x${with_tests}" = xyes; then
	BUILD_TESTS=yes
	PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])
fi
AC_MSG_RESULT($BUILD_TESTS)

AM_CONDITIONAL(BUILD_TESTS, test "x$BUILD_TESTS" = xyes)

dnl -----------------
dnl Compiler warnings
dnl -----------------

AC_MSG_CHECKING(whether to abort on compiler warnings)

WARN_CFLAGS="-Wall -Wextra -Wno-unused-parameter -Wno-unknown-pragmas"

AC_ARG_ENABLE(warn,
	AS_HELP_STRING(
		[--enable-warn], [abort on compiler warnings]
	),
	AC_MSG_RESULT(yes)
	WARN_CFLAGS="-Werror $WARN_CFLAGS",
	AC_MSG_RESULT(no)
)
AC_SUBST(WARN_CFLAGS)

dnl ----------------------
dnl Compiler optimizations
dnl ----------------------

AC_MSG_CHECKING(whether to enable compiler optimizations)

OPT_CFLAGS=""

AC_ARG_ENABLE(optimizations,
	AS_HELP_STRING(
		[--enable-optimizations=none|default|expensive], [enable compiler optimizations]
	),
	AC_MSG_RESULT($enable_optimizations)
	if test "x${enable_optimizations}" = xnone; then
		OPT_CFLAGS="-O0 $OPT_CFLAGS"
	elif test "x${enable_optimizations}" = xexpensive; then
		OPT_CFLAGS="-O3 $OPT_CFLAGS"
	fi,
	AC_MSG_RESULT(default)
)
AC_SUBST(OPT_CFLAGS)

dnl ---------------------
dnl Debugging information
dnl ---------------------

AC_MSG_CHECKING(whether to include debugging information)

DEBUG_CFLAGS=""
DEBUG_LIBS=""

AC_ARG_ENABLE(debug,
	AS_HELP_STRING(
		[--enable-debug], [include debugging information]
	),
	AC_MSG_RESULT(yes)
	DEBUG_CFLAGS="-g $DEBUG_CFLAGS $HOST_DEBUG_CFLAGS"
	DEBUG_LIBS="$DEBUG_LIBS $HOST_DEBUG_LIBS",
	AC_MSG_RESULT(no)
)
AC_SUBST(DEBUG_CFLAGS)
AC_SUBST(DEBUG_LIBS)

dnl ---------------------
dnl Profiling information
dnl ---------------------

AC_MSG_CHECKING(whether to include profiling information)

PROFILE_CFLAGS=""
PROFILE_LIBS=""

AC_ARG_ENABLE(profile,
	AS_HELP_STRING(
		[--enable-profile], [include profiling information]
	),
	AC_MSG_RESULT(yes)
	PROFILE_CFLAGS="-pg $HOST_PROFILE_CFLAGS"
	PROFILE_LIBS="$HOST_PROFILE_LIBS",
	AC_MSG_RESULT(no)
)
AC_SUBST(PROFILE_CFLAGS)
AC_SUBST(PROFILE_LIBS)

dnl ----------------------------------------
dnl Consolidate our core flags and libraries
dnl ----------------------------------------

BASE_CFLAGS=$(echo $CFLAGS $OPT_CFLAGS $DEBUG_CFLAGS $PROFILE_CFLAGS $WARN_CFLAGS)
BASE_LIBS=$(echo $HOST_LIBS $DEBUG_LIBS $PROFILE_LIBS $WARN_LIBS)
BASE_LDFLAGS=$(echo $LDFLAGS)

AC_SUBST(BASE_CFLAGS)
AC_SUBST(BASE_LIBS)
AC_SUBST(BASE_LDFLAGS)

dnl ----------------------
dnl Fill in path variables
dnl ----------------------
test "$prefix" = "NONE" && prefix=$DEFAULT_PREFIX
test "$exec_prefix" = "NONE" && exec_prefix=$prefix

if test "x${prefix}" = "x/"; then
	eval BINDIR="/bin"
	eval PKGLIBDIR="/lib"
	eval PKGDATADIR="/share"
else
	eval BINDIR="$prefix/bin"
	eval PKGLIBDIR="$prefix/lib/$PACKAGE"
	eval PKGDATADIR="$prefix/share/$PACKAGE"
fi

AC_DEFINE_UNQUOTED(PKGLIBDIR, "$PKGLIBDIR",
	[Define to path containing the shared modules.]
)
AC_SUBST(PKGLIBDIR)

AC_DEFINE_UNQUOTED(PKGDATADIR, "$PKGDATADIR",
	[Define to path containing the game data.]
)
AC_SUBST(PKGDATADIR)

AM_CONDITIONAL(BUILD_CLIENT, test "x$BUILD_CLIENT" = xyes)
AM_CONDITIONAL(BUILD_MASTER, test "x$BUILD_MASTER" = xyes)

dnl -----------------------------------------------------------
dnl Configuraton tests complete -- provide a summary of results
dnl -----------------------------------------------------------

AC_CONFIG_FILES([
	Makefile
	deps/Makefile
	deps/minizip/Makefile
	src/Makefile
	src/ai/Makefile
	src/ai/default/Makefile
	src/collision/Makefile
	src/cgame/Makefile
	src/cgame/default/Makefile
	src/cgame/default/viewcontrollers/Makefile
	src/cgame/default/views/Makefile
	src/client/Makefile
	src/client/renderer/Makefile
	src/client/renderer/shaders/Makefile
	src/client/sound/Makefile
	src/client/ui/Makefile
	src/client/ui/renderers/Makefile
	src/client/ui/viewcontrollers/Makefile
	src/client/ui/views/Makefile
	src/game/Makefile
	src/game/default/Makefile
	src/main/Makefile
	src/net/Makefile
	src/server/Makefile
	src/tests/Makefile
	src/tools/Makefile
	src/tools/quemap/Makefile
	src/tools/quetoo-master/Makefile
	src/tools/quetoo-update/Makefile
])

AC_OUTPUT

AC_MSG_RESULT([
Quetoo $VERSION: Automatic configuration OK.

  Configuration summary:
    Platform: .......... $host
    Compiler: .......... $CC
    C flags: ........... $BASE_CFLAGS
    Libraries: ......... $BASE_LIBS
    Linking: ........... $BASE_LDFLAGS

    Game modules: ...... $GAME_MODULES
    AI modules: ........ $AI_MODULES
    Tools: ............. $TOOLS

    Options:
      MySQL: ........... $HAVE_MYSQL
      Unit tests: ...... $BUILD_TESTS

  Installation directories:

    Program: ........... $BINDIR
    Libraries: ......... $PKGLIBDIR
    Data: .............. $PKGDATADIR

Type 'make' to build Quetoo. Type 'make install' to install it.
Type 'make rsync-data' to install or update the game data.
])
