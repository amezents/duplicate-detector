#ifndef DuplicateFinder_h
#define DuplicateFinder_h

#include <string>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <filesystem>
#include <vector>
#include <iterator>

#include <openssl/sha.h>


using namespace std;
using namespace filesystem;
using hash_map = unordered_multimap<size_t, path>;


class DuplicateFinder {

  hash_map filesByContent;
  vector<path> pathesProcessed {};
  vector<path> filesUniqs {};
  vector<path> fileDupes {};
  vector<path> fileHuges {};
  stringstream ss_;
  unsigned long long sizeHuge {0};
  const int K_READ_BUF_SIZE{ 1024 * 16 };

  public:
  string process(const string& dir1,const string& dir2 ) {
    stringstream cmd;
    cmd << "fdupes " << dir1 << " " << dir2 << " > result_1.txt";
    system(cmd.str().c_str());
    stringstream ss;
    ss << std::ifstream("result_1.txt").rdbuf();
    return ss.str();
  }

  size_t hashFromPath(const path &p) {
    ifstream is {p.c_str(), ios::in | ios::binary};
    if (!is) { throw errno; }

    string s;
    is.seekg(0, ios::end);
    s.reserve(is.tellg());
    is.seekg(0, ios::beg);
    s.assign(istreambuf_iterator<char>{is}, {});
    return hash<string>{}(s);
  }

  size_t hashFromPath_1(const path &p) {
    optional<size_t> result {sha256(p)};
    if (!result.has_value()) return 0;
    return result.value();
  }


  bool isHuge(const path& p) {
    return file_size(p) >= sizeHuge;
  }

  void processDirs(const path &dir) {

    for (const auto &entry : recursive_directory_iterator{dir}) {
      const path p {entry.path()};

      if (is_directory(p)) { continue; }
      if (isHuge(p)) { fileHuges.push_back(p); }
      if (is_regular_file(p)) {
        filesByContent.emplace(hashFromPath_1(p), p);
      }
    }
  };

  vector<path> sanityCheckPath(const string& dirs) {
    std::stringstream is {dirs};
    std::vector<string> folders {std::istream_iterator<string>{is},
      std::istream_iterator<string>{}};
    std::vector<path> pathes {};
    for (const auto& dir: folders) {
      try {
        if (exists(dir)) {
            path p{dir};
            pathes.push_back(p);
        }
      }catch(filesystem_error& e) {
          ss_ << e.what() << '\n';
      }catch(std::bad_alloc& e) {
          ss_ << e.what() << '\n';
      }
    }
    return pathes;
  };

  string execute(string dirs,const unsigned long fileSizeLimit=10000) {
    DuplicateFinder::sizeHuge = fileSizeLimit;
    pathesProcessed = sanityCheckPath(dirs);
    if (pathesProcessed.empty()) {
       return "Pathe(s) incorrect or empty\n";
    }

    for (const auto& path: pathesProcessed) 
      processDirs(path);

    buildFilesLists();  
    return "Done\n";
  };
  vector<path> getPathesProcessed() {
    return pathesProcessed;
  };

  bool isDup(const size_t& hash) {
    return filesByContent.count(hash) > 1;
  };

  void buildFilesLists() {
    for (auto [hash, p]: filesByContent) {
      if (isDup(hash)) fileDupes.push_back(p);
      else filesUniqs.push_back(p);
    }
  };

  std::string printFileHuge(bool withHeader = false) {
    stringstream ss {""};
    if (withHeader) ss << "Ignored files which size >= " << sizeHuge << ":\n";
    for (auto p: fileHuges ) ss << "ignored: " << p.c_str() << endl;
    return ss.str();
  };
  std::string printDupes(bool withHeader = false) {
    stringstream ss {""};
    if (withHeader) ss << "Duplicates:\n";
    for (auto p: fileDupes ) ss << p.c_str() << endl;
    return ss.str();
  };
  std::string printUniqs(bool withHeader = false) {
    stringstream ss {""};
    if (withHeader) ss << "Uniques:\n";
    for (auto p: filesUniqs ) ss << p.c_str() << endl;
    return ss.str();
  };
  std::string printFilesProcessed(bool withHeader = false) {
    stringstream ss {""};
    if (withHeader) ss << "All files:\n";
    for (auto [hash, path]: filesByContent ) {
      auto n = filesByContent.count(hash);
      if (n > 1) {
        ss << "dup: " << filesByContent.find(hash)->second.c_str()
           << " == " << path.c_str() << endl;
      }
      if (n == 1) ss << "uniq: " << path.c_str() << endl;
    }
    ss << printFileHuge(true); 
    return ss.str();
  };
  std::string printStatus() {
    return ss_.str();
  };

  vector<path> getFileHuge() {
    return fileHuges;
  };

  vector<path> getFileUniqs() {
    return filesUniqs;
  };

  vector<path> getFileDupes() {
    return fileDupes;
  };

  std::optional<std::size_t> sha256(std::string filename) {
      // Initialize openssl
      SHA256_CTX context;
      if(!SHA256_Init(&context)) return std::nullopt;

      // Read file and update calculated SHA
      char buf[K_READ_BUF_SIZE];
      std::ifstream file(filename, std::ifstream::binary);
      while (file.good()) {
          file.read(buf, sizeof(buf));
          if(!SHA256_Update(&context, buf, file.gcount())) return std::nullopt;
      }

      // Get Final SHA
      unsigned char result[SHA256_DIGEST_LENGTH];
      if(!SHA256_Final(result, &context)) return std::nullopt;

      // Transform byte-array to string
      std::stringstream shastr;
      shastr << std::hex << std::setfill('0');
      for (const auto &byte: result)
          shastr << std::setw(2) << (int)byte;
      char *e;
      return strtoul(shastr.str().c_str(),&e,16);
  }
};



#endif //DuplicateFinder_h