GamePay Core integration/staging tree
=====================================

[![Build Status](https://travis-ci.org/gamepay-project/gamepay.svg?branch=master)](https://travis-ci.org/gamepay-project/gamepay)

https://gamepay.org

What is GamePay?
----------------

GamePay is an experimental digital currency that enables instant payments to
anyone, anywhere in the world. GamePay uses peer-to-peer technology to operate
with no central authority: managing transactions and issuing money are carried
out collectively by the network. GamePay Core is the name of open source
software which enables the use of this currency.

For more information, as well as an immediately useable, binary version of
the GamePay Core software, see [https://gamepay.org](https://gamepay.org).

Install GamePay
-------
git clone https://github.com/Globaleliteee/GamePay
GAMEPAY_ROOT=$(pwd)/GamePay
BDB_PREFIX="${GAMEPAY_ROOT}/db4"
mkdir -p $BDB_PREFIX
wget 'http://download.oracle.com/berkeley-db/db-4.8.30.NC.tar.gz'
echo '12edc0df75bf9abd7f82f821795bcee50f42cb2e5f76a6a281b85732798364ef  db-4.8.30.NC.tar.gz' | sha256sum -c

tar -xzvf db-4.8.30.NC.tar.gz
cd db-4.8.30.NC/build_unix/
find . -type f \( -name "*.cpp" -o -name "*.h" \) -exec sed -i 's/__atomic_compare_exchange/my_atomic_compare_exchange/g' {} +
../dist/configure --enable-cxx --disable-shared --with-pic --prefix=$BDB_PREFIX
make install

cd $GAMEPAY_ROOT
./autogen.sh
./configure LDFLAGS="-L${BDB_PREFIX}/lib/" CPPFLAGS="-I${BDB_PREFIX}/include/" CXXFLAGS="--param ggc-min-expand=1 --param ggc-min-heapsize=32768" --enable-upnp-default --without-gui

./configure LDFLAGS="-L${BDB_PREFIX}/lib/" CPPFLAGS="-I${BDB_PREFIX}/include/" CXXFLAGS="--param ggc-min-expand=1 --param ggc-min-heapsize=32768" --enable-upnp-default --with-gui=qt5
make -j$(nproc)
make install
cd src
strip gamepayd

License
-------

GamePay Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is regularly built (see `doc/build-*.md` for instructions) and tested, but it is not guaranteed to be
completely stable. [Tags](https://github.com/gamepay-project/gamepay/tags) are created
regularly from release branches to indicate new official, stable release versions of GamePay Core.

The https://github.com/gamepay-project/gui repository is used exclusively for the
development of the GUI. Its master branch is identical in all monotree
repositories. Release branches and tags do not exist, so please do not fork
that repository unless it is for development reasons.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md)
and useful hints for developers can be found in [doc/developer-notes.md](doc/developer-notes.md).

The developer [mailing list](https://groups.google.com/forum/#!forum/gamepay-dev)
should be used to discuss complicated or controversial changes before working
on a patch set.

Developer IRC can be found on Freenode at #gamepay-dev.

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](src/test/README.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`. Further details on running
and extending unit tests can be found in [/src/test/README.md](/src/test/README.md).

There are also [regression and integration tests](/test), written
in Python, that are run automatically on the build server.
These tests can be run (if the [test dependencies](/test) are installed) with: `test/functional/test_runner.py`

The Travis CI system makes sure that every pull request is built for Windows, Linux, and macOS, and that unit/sanity tests are run automatically.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

Translations
------------

We only accept translation fixes that are submitted through [Bitcoin Core's Transifex page](https://www.transifex.com/projects/p/bitcoin/).
Translations are converted to GamePay periodically.

Translations are periodically pulled from Transifex and merged into the git repository. See the
[translation process](doc/translation_process.md) for details on how this works.

**Important**: We do not accept translation changes as GitHub pull requests because the next
pull from Transifex would automatically overwrite them again.
