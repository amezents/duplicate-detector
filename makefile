runTests: dupFinder dupFinderTests
	@echo "runs the tests"
	./dupFinderTests

dupFinderTests:
	@echo "builds tests"
	g++-9 -I. \
	-std=c++17 \
	-isystem /usr/include/gtest/ \
	-pthread \
	/usr/local/lib/libgmock_main.a \
	/usr/local/lib/libgmock.a \
	dupFinderTests.cpp -o dupFinderTests -lcrypto

dupFinder:
	@echo "builds target programm"
	cd example; \
	make run
#	pwd; \
#	g++-9 -I. \
#	-std=c++17 \
#	dupFinder.cpp -o dupFinder -lcrypto