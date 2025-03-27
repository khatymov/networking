#include <gtest/gtest.h>

#include "pch.h"

int main(int argc, char* argv[]) {
    spdlog::default_logger()->set_pattern("%+ [thread %t]");
    spdlog::default_logger()->set_level(spdlog::level::debug);

    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
