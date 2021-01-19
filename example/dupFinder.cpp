#include "../DuplicateFinder.h"
#include <iostream>

using namespace std;

int main(int argc, char const *argv[])
{
    if (argc != 4) {
        cout << "Usage: " << argv[0] << " <dir_1>" << " <dir_2>" 
             << " <maxFileSize>" << endl;
        return 1;
    }

    stringstream ss {};
    ss << argv[1] << " " << argv[2];
    string dirsList {ss.str()};
    char *e;
    size_t maxFileSize {strtoul(argv[3],&e,10)};
    DuplicateFinder df;

    cout << df.execute( dirsList, maxFileSize);

    cout << df.printFilesProcessed(true);
    return 0;
}
