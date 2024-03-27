#include "gtest/gtest.h"

#include "move_test.h"

using namespace std;
using namespace testing;

TEST(test_move_semantics, test_methods)
{
    const char* str_prt0 = "Move0ptr";
    string str0("Move0string");
    Move move0(str_prt0, str0);
    EXPECT_EQ(move0.get_str(), str0);
    EXPECT_STREQ(move0.get_str_prt(), str_prt0);

    const char* str_prt1 = "Move1Ptr";
    string str1("Move1Str");
    auto move1(Move(str_prt1, str1));
    auto move2(std::move(move1));

    Move move3("Move3Ptr", "Move3Str");
    move3 = std::move(move2);
    EXPECT_EQ(move3.get_str(), str1);
    EXPECT_STREQ(move3.get_str_prt(), str_prt1);

    Move move4;
    move4 = move0 + move3;
    EXPECT_EQ(move0.get_str(), str0);
    EXPECT_STREQ(move0.get_str_prt(), str_prt0);
    EXPECT_EQ(move3.get_str(), str1);
    EXPECT_STREQ(move3.get_str_prt(), str_prt1);

    const char* str_prt5 = "Move5Ptr";
    string str5("Move5Str");
    Move move5(str_prt5, str5);
    move5 = move4;
    EXPECT_EQ(move5.get_str(), str0 + str1);
}
