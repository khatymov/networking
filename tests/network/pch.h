#pragma once

#include <array>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <thread>
#include <memory>
#include <queue>
#include <cryptopp/aes.h>
#include <cryptopp/osrng.h>
#include <cryptopp/modes.h>
#include <openssl/sha.h>
#include <boost/asio.hpp>
#include <boost/asio/completion_condition.hpp>
#include "spdlog/spdlog.h"
