#pragma once

#include <array>
#include <boost/asio.hpp>
#include <boost/asio/completion_condition.hpp>
#include <boost/asio/ip/address.hpp>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
#include <thread>

#include <openssl/sha.h>

#include "cryptopp/aes.h"
#include "cryptopp/modes.h"
#include "cryptopp/osrng.h"
#include "spdlog/spdlog.h"
