#include <gtest/gtest.h>
extern "C" {
#include "common.h"
#include "kernel.h"
#include "libapi.h"
}

class bu_Test : public testing::Test {
    static void make_file(const char* path, int len, int val, int inc) {
        FILE* f = fopen(path, "wb");
        assert(f != nullptr);
        auto* d = (unsigned char*)malloc(len);
        assert(d != nullptr);
        for (int i = 0; i < len; i++) {
            d[i] = val;
            val += inc;
        }
        char buf[8192];
        assert(fwrite(buf, 1, sizeof(buf), f) == sizeof(buf));
        assert(fclose(f) == 0);
        free(d);
    }
  protected:
    bu_Test() {}
    ~bu_Test() override = default;
    void SetUp() override {
        mkdir("bu00", 0755);
        mkdir("bu10", 0755);
        make_file("bu00/BASLUS-00000PSYZ00", 8192, 0, 1);
        make_file("bu00/BASLUS-00000PSYZ01", 8192, 0, 2);
    }
    void TearDown() override {
        assert(remove("bu00/BASLUS-00000PSYZ00") == 0);
        assert(remove("bu00/BASLUS-00000PSYZ01") == 0);
        assert(rmdir("bu00") == 0);
        assert(rmdir("bu10") == 0);
    }
};

TEST_F(bu_Test, for_invalid_path) {
    struct DIRENTRY d = {0};
    struct DIRENTRY* pd;
    pd = firstfile((char*)"none:", &d);
    EXPECT_EQ(pd, nullptr);
    EXPECT_STREQ(d.name, "");
    EXPECT_EQ(d.attr, 0);
    EXPECT_EQ(d.size, 0);
}

TEST_F(bu_Test, for_empty_path) {
    struct DIRENTRY d = {0};
    struct DIRENTRY* pd;
    pd = firstfile((char*)"bu10:", &d);
    EXPECT_EQ(pd, nullptr);
    EXPECT_STREQ(d.name, "");
    EXPECT_EQ(d.attr, 0);
    EXPECT_EQ(d.size, 0);
}

TEST_F(bu_Test, for_first_file) {
    struct DIRENTRY d = {0};
    struct DIRENTRY* pd;
    pd = firstfile((char*)"bu00:", &d);
    EXPECT_EQ(pd, &d);
    EXPECT_STREQ(d.name, "BASLUS-00000PSYZ00");
    EXPECT_EQ(d.attr, 80);
    EXPECT_EQ(d.size, 8192);
}

TEST_F(bu_Test, for_multiple_files) {
    struct DIRENTRY d[2];
    struct DIRENTRY* pd;
    pd = nextfile(firstfile((char*)"bu00:", d)+1);
    EXPECT_EQ(pd, &d[1]);
    EXPECT_STREQ(pd->name, "BASLUS-00000PSYZ01");
    EXPECT_EQ(pd->attr, 80);
    EXPECT_EQ(pd->size, 8192);
}

TEST_F(bu_Test, for_end_of_file_list) {
    struct DIRENTRY d[3];
    struct DIRENTRY* pd;
    pd = nextfile(nextfile(firstfile((char*)"bu00:", d)+1)+2);
    EXPECT_EQ(pd, nullptr);
}

TEST_F(bu_Test, open_existing_file) {
    int fd = open("bu00:BASLUS-00000PSYZ00", 1);
    EXPECT_NE(fd, -1);
    close(fd);
}
