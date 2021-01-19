#include <gmock/gmock.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <optional>
#include <openssl/sha.h>
#include "DuplicateFinder.h"

using namespace std;
namespace fs = std::filesystem;

class DupFinderTest : public ::testing::Test {
  protected:
    DuplicateFinder dFinder;
    string result {};
};

void fileDupeGenerate(string dir,string name, char content) {
  size_t length {0};
  size_t copyMax {0};
  if (name.find("small") != std::string::npos) {length = 10;}
  if (name.find("huge") != std::string::npos) {length = 10000;}
  if (name.find("dup") != std::string::npos) {copyMax = 3;}
  if (name.find("uniq") != std::string::npos) {copyMax = 1;}

  for (size_t copyNumber = 1; copyNumber <= copyMax; copyNumber++) {
    stringstream ss {};
    ss << dir << "/" << name << "_" << content << "_" << copyNumber;
    std::ofstream( ss.str(), ios::out | std::ios::binary) << string(length,content);
  }
};

void populateDirBySmallDup(std::string dir){
  fileDupeGenerate(dir,"small_dup",'a');
  fileDupeGenerate(dir,"small_dup",'b');
  fileDupeGenerate(dir,"small_dup",'c');
};

void populateDirByHugeDup(std::string dir){
  fileDupeGenerate(dir,"huge_dup",'d');
  fileDupeGenerate(dir,"huge_dup",'e');
  fileDupeGenerate(dir,"huge_dup",'f');
};

void populateDirBySmallUniq(std::string dir){
  fileDupeGenerate(dir,"small_uniq",'k');
  fileDupeGenerate(dir,"small_uniq",'l');
  fileDupeGenerate(dir,"small_uniq",'m');
};

void populateDirByHugeUniq(std::string dir){
  fileDupeGenerate(dir,"huge_uniq",'x');
  fileDupeGenerate(dir,"huge_uniq",'y');
  fileDupeGenerate(dir,"huge_uniq",'z');
};


class SmallFilesTest : public DupFinderTest {
  protected:
  void SetUp() override{
    create_directory(path("dir_1"));
    create_directory(path("dir_2"));
    populateDirBySmallDup("dir_1");
    populateDirBySmallDup("dir_2");
    ASSERT_TRUE(exists(path("dir_1")));
    ASSERT_TRUE(exists(path("dir_2")));
    vector<string> smallDupes {
      "small_dup_a_1","small_dup_a_2","small_dup_a_3",
      "small_dup_b_1","small_dup_b_2","small_dup_b_3",
      "small_dup_c_1","small_dup_c_2","small_dup_c_3",
    };

    ASSERT_TRUE(
      all_of(begin(smallDupes),end(smallDupes),
        [](string name){return exists(path("dir_1")/path(name));})
    );
    ASSERT_TRUE(
      all_of(begin(smallDupes),end(smallDupes),
        [](string name){return exists(path("dir_2")/path(name));})
    );

  }
  void TearDown() override{
    remove_all(path("dir_1"));
    remove_all(path("dir_2"));
  }
};

using ::testing::HasSubstr;

TEST_F(SmallFilesTest, FoldersPopulatedBySmallDupes) {
  dFinder.execute("dir_1 dir_2");
  string result {dFinder.printFilesProcessed(true)};
  EXPECT_THAT(result, HasSubstr(" == dir_1/small_dup_a_1"));
  EXPECT_THAT(result, HasSubstr(" == dir_1/small_dup_a_2"));
  EXPECT_THAT(result, HasSubstr(" == dir_1/small_dup_a_3"));
  EXPECT_THAT(result, HasSubstr(" == dir_2/small_dup_a_1"));
  EXPECT_THAT(result, HasSubstr(" == dir_2/small_dup_a_2"));
  EXPECT_THAT(result, HasSubstr(" == dir_2/small_dup_a_3"));
  EXPECT_THAT(result, Not(HasSubstr("uniq")));
  EXPECT_THAT(result, Not(HasSubstr("huge")));
  EXPECT_EQ(dFinder.getFileDupes().size(), 18);
  EXPECT_EQ(dFinder.getFileUniqs().size(), 0);
};



class HugeFilesTest : public DupFinderTest {
  protected:
  void SetUp() override{
    create_directory(path("dir_1"));
    create_directory(path("dir_2"));
    populateDirByHugeUniq("dir_1");
    populateDirByHugeDup("dir_2");
    ASSERT_TRUE(exists(path("dir_1")));
    ASSERT_TRUE(exists(path("dir_2")));
    vector<string> hugeUniqs {
      "huge_uniq_x_1","huge_uniq_y_1","huge_uniq_z_1"
    };
    ASSERT_TRUE(
      all_of(begin(hugeUniqs),end(hugeUniqs),
        [](string name){return exists(path("dir_1")/path(name));})
    );
    vector<string> hugeDupes {
      "huge_dup_d_1","huge_dup_d_2","huge_dup_d_3",
      "huge_dup_e_1","huge_dup_e_2","huge_dup_e_3",
      "huge_dup_f_1","huge_dup_f_2","huge_dup_f_3"
    };
    ASSERT_TRUE(
      all_of(begin(hugeDupes),end(hugeDupes),
        [](string name){return exists(path("dir_2")/path(name));})
    );
  }
  void TearDown() override{
    remove_all(path("dir_1"));
    remove_all(path("dir_2"));
  }
  std::string EmptyList {""};
};


TEST_F(HugeFilesTest, IgnoresHugeWhenSizeLimitIsDefault) {
  dFinder.execute("dir_1 dir_2");
  string result {dFinder.printFilesProcessed(true)};
  EXPECT_THAT(result, HasSubstr("Ignored files which size >= 10000:") );
  EXPECT_THAT(result, HasSubstr("ignored: dir_1/huge_uniq_x_1") );
  EXPECT_THAT(result, HasSubstr("ignored: dir_1/huge_uniq_y_1") );
  EXPECT_THAT(result, HasSubstr("ignored: dir_1/huge_uniq_z_1") );

  EXPECT_THAT(result, HasSubstr("ignored: dir_2/huge_dup_d") );
  EXPECT_THAT(result, HasSubstr("ignored: dir_2/huge_dup_e") );
  EXPECT_THAT(result, HasSubstr("ignored: dir_2/huge_dup_f") );
};


TEST_F(HugeFilesTest, ListHugeUnderSizeLimit) {
  dFinder.execute("dir_1 dir_2", 20000);
  string result {dFinder.printFilesProcessed(true)};
  EXPECT_THAT(result, HasSubstr("Ignored files which size >= 20000:") );
  EXPECT_THAT(result, Not(HasSubstr("ignored: dir_1/huge_uniq_x_1")) );
  EXPECT_THAT(result, Not(HasSubstr("ignored: dir_1/huge_uniq_y_1")) );
  EXPECT_THAT(result, Not(HasSubstr("ignored: dir_1/huge_uniq_z_1")) );
  EXPECT_THAT(result, Not(HasSubstr("ignored: dir_2/huge_dup_d")) );
  EXPECT_THAT(result, Not(HasSubstr("ignored: dir_2/huge_dup_e")) );
  EXPECT_THAT(result, Not(HasSubstr("ignored: dir_2/huge_dup_f")) );
};


class DirsListTest : public DupFinderTest {
  protected:
  void SetUp() override{
    create_directory(path("dir_1"));
    create_directory(path("dir_2"));
  }
  void TearDown() override{
    remove_all(path("dir_1"));
    remove_all(path("dir_2"));
  }
  std::string EmptyList {""};
};

TEST_F(DirsListTest, EmptyDirsStrings) {
  ASSERT_EQ(EmptyList, "");
  ASSERT_NO_THROW(result = dFinder.execute(EmptyList));
  EXPECT_THAT(result, HasSubstr("Pathe(s) incorrect or empty"));
  EXPECT_EQ(dFinder.getPathesProcessed().size(), 0);
};

TEST_F(DirsListTest, NonEmptyDirsStringsWithNonExistedPathes) {
  ASSERT_FALSE(exists(path("folder_1")));
  ASSERT_FALSE(exists(path("folder_2")));
  EXPECT_NO_THROW(dFinder.execute("folder_1 folder_2"));
  EXPECT_EQ(dFinder.getPathesProcessed().size(), 0);
  EXPECT_EQ( dFinder.printStatus(), "");
};

TEST_F(DirsListTest, ProcessesExistingPathesIgnoresNamesWithllegalChars) {
  ASSERT_TRUE(exists(path("dir_1")));
  ASSERT_TRUE(exists(path("dir_2")));
  EXPECT_NO_THROW(dFinder.execute("dir_1 dir_2 dir^.^%3 dir_4"));
  EXPECT_THAT(dFinder.getPathesProcessed()[0].c_str(), HasSubstr("dir_1"));
  EXPECT_THAT(dFinder.getPathesProcessed()[1].c_str(), HasSubstr("dir_2"));
  EXPECT_EQ(dFinder.getPathesProcessed().size(), 2);
};

class SHA256Test : public DupFinderTest {
  protected:
  void SetUp() override{
    create_directory(path("dir_1"));
    populateDirByHugeDup("dir_1");
    populateDirByHugeUniq("dir_1");
  }
  void TearDown() override{
    remove_all(path("dir_1"));
  }

  static const int K_READ_BUF_SIZE{ 1024 * 16 };

  std::optional<std::string> CalcSha256(std::string filename) {
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

      return shastr.str();
  }

  string sha256(string filename) {
      stringstream cmd {""};
      cmd << "openssl dgst -sha256 " << filename;
      cmd << " | cut -d' ' -f2 > dir_1/result.txt";
      system(cmd.str().c_str());
      stringstream ss;
      ss << std::ifstream("dir_1/result.txt").rdbuf();
      return ss.str();  
  }
};

TEST_F(SHA256Test, calcsHashForHugeFiles) {
  ASSERT_TRUE(exists(path("dir_1/huge_dup_d_1")));
  ASSERT_TRUE(exists(path("dir_1/huge_dup_d_2")));
  ASSERT_TRUE(exists(path("dir_1/huge_dup_d_3")));
  ASSERT_TRUE(exists(path("dir_1/huge_uniq_x_1")));
  ASSERT_TRUE(exists(path("dir_1/huge_uniq_y_1")));
  ASSERT_TRUE(exists(path("dir_1/huge_uniq_z_1")));

  string res_1 {""};
  optional<string> res_2 {};
  string filename{""};

  filename = "dir_1/huge_dup_d_1";
  res_1 = sha256(filename);
  res_2 = CalcSha256(filename);
  EXPECT_EQ(res_1, res_2.value()+"\n");

  filename = "dir_1/huge_dup_d_2";
  res_1 = sha256(filename);
  res_2 = CalcSha256(filename);
  EXPECT_EQ(res_1, res_2.value()+"\n");

  filename = "dir_1/huge_dup_d_3";
  res_1 = sha256(filename);
  res_2 = CalcSha256(filename);
  EXPECT_EQ(res_1, res_2.value()+"\n");

  filename = "dir_1/huge_uniq_x_1";
  res_1 = sha256(filename);
  res_2 = CalcSha256(filename);
  EXPECT_EQ(res_1, res_2.value()+"\n");
};


class VacancyTaskTest : public SmallFilesTest {
  protected:
  void SetUp() override{
    SmallFilesTest::SetUp();
    ASSERT_TRUE(exists(path("dir_1")));
    ASSERT_TRUE(exists(path("dir_2")));
    populateDirByHugeDup("dir_1");
    populateDirByHugeDup("dir_2");
    vector<string> hudeDupes {
      "huge_dup_d_1","huge_dup_d_2","huge_dup_d_3",
      "huge_dup_e_1","huge_dup_e_2","huge_dup_e_3",
      "huge_dup_f_1","huge_dup_f_2","huge_dup_f_3",
    };

    ASSERT_TRUE(
      all_of(begin(hudeDupes),end(hudeDupes),
        [](string name){return exists(path("dir_1")/path(name));})
    );
    ASSERT_TRUE(
      all_of(begin(hudeDupes),end(hudeDupes),
        [](string name){return exists(path("dir_2")/path(name));})
    );
    vector<string> hudeUniqs {
      "huge_uniq_x_1","huge_uniq_y_1","huge_uniq_z_1"
    };
    populateDirByHugeUniq("dir_2");
    ASSERT_TRUE(
      all_of(begin(hudeUniqs),end(hudeUniqs),
        [](string name){return exists(path("dir_2")/path(name));})
    );    
    vector<string> smallUniqs {
      "small_uniq_k_1","small_uniq_l_1","small_uniq_m_1"
    };
    populateDirBySmallUniq("dir_2");
    ASSERT_TRUE(
      all_of(begin(smallUniqs),end(smallUniqs),
        [](string name){return exists(path("dir_2")/path(name));})
    );    

  }
  void TearDown() override{
    remove_all(path("dir_1"));
    remove_all(path("dir_2"));
  }

  string run_system(string cmd, string outputFile) {
    string command {cmd + ">" + outputFile};
    system(command.c_str());
    stringstream ss;
    ss << std::ifstream(outputFile).rdbuf();
    return ss.str();
  }
};

TEST_F(VacancyTaskTest, complainsWhenFilesSizeLimitAbsent) {
  result = run_system("./example/dupFinder dir_1 dir_2", "dir_1/result.txt");
  EXPECT_THAT(result, HasSubstr("Usage: ./example/dupFinder <dir_1> <dir_2> <maxFileSize>") );
};

TEST_F(VacancyTaskTest, complainsIfOneDirOnly) {
  result = run_system("./example/dupFinder dir_1 10000", "dir_1/result.txt");
  EXPECT_THAT(result, HasSubstr("Usage: ./example/dupFinder <dir_1> <dir_2> <maxFileSize>") );
};

TEST_F(VacancyTaskTest, /*DISABLED_*/runsAllFiles) {
  result = run_system("./example/dupFinder dir_1 dir_2 10000", "dir_1/result.txt");
  EXPECT_THAT(result, HasSubstr("Ignored files which size >= 10000:") );
  EXPECT_THAT(result, HasSubstr("ignored: dir_2/huge_uniq_x_1") );
  EXPECT_THAT(result, HasSubstr("ignored: dir_2/huge_uniq_y_1") );
  EXPECT_THAT(result, HasSubstr("ignored: dir_2/huge_uniq_z_1") );

  EXPECT_THAT(result, HasSubstr("ignored: dir_1/huge_dup_d") );
  EXPECT_THAT(result, HasSubstr("ignored: dir_1/huge_dup_e") );
  EXPECT_THAT(result, HasSubstr("ignored: dir_1/huge_dup_f") );
  EXPECT_THAT(result, HasSubstr("ignored: dir_2/huge_dup_d") );
  EXPECT_THAT(result, HasSubstr("ignored: dir_2/huge_dup_e") );
  EXPECT_THAT(result, HasSubstr("ignored: dir_2/huge_dup_f") );

};