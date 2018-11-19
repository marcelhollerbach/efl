#!/bin/sh

set -e
. .ci/travis.sh
if [ "$1" != "release-ready" ] ; then
  exit 0
fi
travis_fold distcheck "make distcheck"
if [ "$BUILDSYSTEM" = "ninja" ] ; then
if [ "$DISTRO" != "" ] ; then
    docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 --env CC="ccache gcc" \
      --env CXX="ccache g++" \
      --env CFLAGS="-fdirectives-only" --env CXXFLAGS="-fdirectives-only" \
      --env LD="ld.gold" $(cat $HOME/cid) dbus-launch ninja -C build dist || \
      (sudo cat efl-*/_build/sub/src/test-suite.log; false)
  else
    export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"
    ninja -C build dist
  fi
else
  if [ "$DISTRO" != "" ] ; then
    docker exec --env MAKEFLAGS="-j5 -rR" --env EIO_MONITOR_POLL=1 --env CC="ccache gcc" \
      --env CXX="ccache g++" \
      --env CFLAGS="-fdirectives-only" --env CXXFLAGS="-fdirectives-only" \
      --env LD="ld.gold" $(cat $HOME/cid) bash -c .ci/distcheck.sh || \
      (sudo cat efl-*/_build/sub/src/test-suite.log; false)
  else
    export PATH="/usr/local/opt/ccache/libexec:$(brew --prefix gettext)/bin:$PATH"
    make
  fi
fi
travis_endfold distcheck
