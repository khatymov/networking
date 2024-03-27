#include "gtest/gtest.h"

#include "my_class_test.h"

using namespace std;
using namespace testing;

TEST(test_class_components, test_my_class)
{
    char* str = "Hello world!";
    MyClass my_class_0(str);

    EXPECT_TRUE(my_class_0.get().compare(string(str)) == 0);

    MyClass my_class_1;
    EXPECT_TRUE(my_class_1.get().empty());

    MyClass my_class_2(std::move(my_class_0));
    EXPECT_TRUE(my_class_2.get().compare(string(str)) == 0);
    EXPECT_TRUE(my_class_0.is_data_null());

    string s("test");
    MyClass tmp(s.c_str());
    MyClass my_class_3(tmp);
    EXPECT_TRUE(my_class_3.get().compare(string(s)) == 0);

    my_class_2 = my_class_3;
    EXPECT_TRUE(my_class_2.get().compare(my_class_3.get()) == 0);
}
