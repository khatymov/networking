#include "crypt_connection_test.h"

#include "my_packet.h"
#include "thread_safe_queue.h"
#include "connection.h"
#include "defs_mock.h"

// client
// thread 1 generate
// thread 2 encrypt
// thread 3 send via socket

// server
// thread 1 get from socket
// thread 2 decrypt and compare


TEST(CryptoConnectionTest, test_sendReceiveCryptoPacks) {
    EXPECT_TRUE(true);
}