#include <vector>

#include <gtest/gtest.h>

#include "../common/Communication/liberror.h"
#include "../common/Communication/protocol_util.h"

TEST(ProtocolUtilTest, closedSocketDetectaSendIncompleto) {
    LibError err(0, "socket sent only %d of %d bytes", 4, 10);
    EXPECT_TRUE(ProtocolUtil::closedSocket(err));
}

TEST(ProtocolUtilTest, closedSocketDetectaRecvIncompleto) {
    LibError err(0, "socket received only %d of %d bytes", 2, 8);
    EXPECT_TRUE(ProtocolUtil::closedSocket(err));
}

TEST(ProtocolUtilTest, closedSocketDetectaAcceptFallido) {
    LibError err(0, "socket accept failed");
    EXPECT_TRUE(ProtocolUtil::closedSocket(err));
}

TEST(ProtocolUtilTest, closedSocketNoDetectaErrorAjeno) {
    LibError err(0, "se rompio cualquier otra cosa");
    EXPECT_FALSE(ProtocolUtil::closedSocket(err));
}
