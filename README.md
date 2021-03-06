# duplicate-detector
get two folders to find out files\` duplicates and output list of detected dups.

`host:duplicate-detector$./example/dupFinder dir_1 dir_2 20000`
 - dir_1 and dir_2 - folders to process
 - 20000 - files with size greter than this value will not be processed (e.g. analise of huge file consumes much time)

To build dupFinder only execute in shell the command "make" in example folder of repository

`host:duplicate-detector$cd example && make`

To build and run tests for dupFinder execute in shell the command "make" in root folder of repository

`host:duplicate-detector$make`
 - Note: tests use gtest libs which nesessary to build see below
 - output should be something like this:
```
builds target programm
cd example; \
make run
make[1]: Entering directory 'duplicate-detector/example'
g++-9 -I. \
-std=c++17 \
dupFinder.cpp -o dupFinder -lcrypto
make[1]: Leaving directory 'duplicate-detector/example'
runs the tests
./dupFinderTests
Running main() from gmock_main.cc
[==========] Running 10 tests from 5 test cases.
[----------] Global test environment set-up.
[----------] 1 test from SmallFilesTest
[ RUN      ] SmallFilesTest.FoldersPopulatedBySmallDupes
[       OK ] SmallFilesTest.FoldersPopulatedBySmallDupes (3 ms)
[----------] 1 test from SmallFilesTest (3 ms total)

[----------] 2 tests from HugeFilesTest
[ RUN      ] HugeFilesTest.IgnoresHugeWhenSizeLimitIsDefault
[       OK ] HugeFilesTest.IgnoresHugeWhenSizeLimitIsDefault (2 ms)
[ RUN      ] HugeFilesTest.ListHugeUnderSizeLimit
[       OK ] HugeFilesTest.ListHugeUnderSizeLimit (2 ms)
[----------] 2 tests from HugeFilesTest (4 ms total)

[----------] 3 tests from DirsListTest
[ RUN      ] DirsListTest.EmptyDirsStrings
[       OK ] DirsListTest.EmptyDirsStrings (1 ms)
[ RUN      ] DirsListTest.NonEmptyDirsStringsWithNonExistedPathes
[       OK ] DirsListTest.NonEmptyDirsStringsWithNonExistedPathes (7 ms)
[ RUN      ] DirsListTest.ProcessesExistingPathesIgnoresNamesWithllegalChars
[       OK ] DirsListTest.ProcessesExistingPathesIgnoresNamesWithllegalChars (0 ms)
[----------] 3 tests from DirsListTest (8 ms total)

[----------] 1 test from SHA256Test
[ RUN      ] SHA256Test.calcsHashForHugeFiles
[       OK ] SHA256Test.calcsHashForHugeFiles (88 ms)
[----------] 1 test from SHA256Test (89 ms total)

[----------] 3 tests from VacancyTaskTest
[ RUN      ] VacancyTaskTest.complainsWhenFilesSizeLimitAbsent
[       OK ] VacancyTaskTest.complainsWhenFilesSizeLimitAbsent (26 ms)
[ RUN      ] VacancyTaskTest.complainsIfOneDirOnly
[       OK ] VacancyTaskTest.complainsIfOneDirOnly (7 ms)
[ RUN      ] VacancyTaskTest.runsAllFiles
[       OK ] VacancyTaskTest.runsAllFiles (11 ms)
[----------] 3 tests from VacancyTaskTest (45 ms total)

[----------] Global test environment tear-down
[==========] 10 tests from 5 test cases ran. (149 ms total)
[  PASSED  ] 10 tests.
host:duplicate-detector$
```
to build gtest libs from sources do the following
```
cd libs
cmake /usr/src/gtest
make
```
this places libgtets.a and libgtest_main.a in libs dir
 - Note: /usr/src/gtest - folder with src of gtest

to get src of gtest
```
>dpkg -l *gtest*
the ouput will be if gtest is installed
"ii  libgtest-dev:amd6 1.8.0-6       amd64         Google's framework for writing C++ tests"
>dpkg -L libgtest-dev
shows the installation folders

if package is not installed do the following:
sudo apt-get install libgtest-dev
```
