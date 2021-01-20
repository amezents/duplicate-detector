runTests: dupFinder dupFinderTests
	@echo "runs the tests"
	./dupFinderTests

dupFinderTests:
	@echo "builds tests"
	g++ -I. \
	-std=c++17 \
	-isystem /usr/include/gtest/ \
	-pthread \
	libs/libgtest_main.a \
	libs/libgtest.a \
	dupFinderTests.cpp -o dupFinderTests -lcrypto

dupFinder:
	@echo "builds target programm"
	cd example; \
	make run
