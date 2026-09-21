#include "test_framework.h"
#include "util/debug_log.h"
#include <iostream>
#include <sstream>

TEST(debug_log_formats_messages_with_tag) {
    const std::string formatted = DebugLog::format("Eink", "partial update");
    ASSERT_STR_EQ("[DEBUG][Eink] partial update", formatted);
}

TEST(debug_log_formats_messages_with_different_tags) {
    const std::string formatted = DebugLog::format("MTProto", "DH key derived");
    ASSERT_STR_EQ("[DEBUG][MTProto] DH key derived", formatted);
}

TEST(debug_log_macro_writes_to_clog_or_is_noop) {
    std::ostringstream capture_stream;
    std::streambuf* original_clog = std::clog.rdbuf(capture_stream.rdbuf());

    DEBUG_LOG("TestTag", "test message");

    std::clog.rdbuf(original_clog);
#ifdef DEBUG
    ASSERT_STR_EQ("[DEBUG][TestTag] test message\n", capture_stream.str());
#else
    ASSERT_STR_EQ("", capture_stream.str());
#endif
}
