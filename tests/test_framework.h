#ifndef KINDLE_TEST_FRAMEWORK_H
#define KINDLE_TEST_FRAMEWORK_H

#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class TestFailure : public std::runtime_error {
public:
    explicit TestFailure(const std::string& message) : std::runtime_error(message) {}
};

struct TestCase {
    std::string name;
    std::function<void()> function;
};

class TestRegistry {
public:
    static TestRegistry& instance() {
        static TestRegistry registry;
        return registry;
    }

    void add(const std::string& name, const std::function<void()>& function) {
        tests_.push_back({name, function});
    }

    int run() const {
        int failures = 0;
        for (const TestCase& test : tests_) {
            failures += runTest(test);
        }
        std::cout << tests_.size() - static_cast<std::size_t>(failures)
                  << " passed, " << failures << " failed\n";
        return failures;
    }

private:
    int runTest(const TestCase& test) const {
        try {
            test.function();
            std::cout << "[PASS] " << test.name << '\n';
            return 0;
        } catch (const std::exception& error) {
            std::cout << "[FAIL] " << test.name << ": " << error.what() << '\n';
            return 1;
        } catch (...) {
            std::cout << "[FAIL] " << test.name << ": unknown exception\n";
            return 1;
        }
    }

    std::vector<TestCase> tests_;
};

class TestRegistrar {
public:
    TestRegistrar(const std::string& name, const std::function<void()>& function) {
        TestRegistry::instance().add(name, function);
    }
};

inline int runAllTests() {
    return TestRegistry::instance().run();
}

#define TEST(name) \
    static void name(); \
    static TestRegistrar registrar_##name(#name, name); \
    static void name()

#define ASSERT_TRUE(condition)                                                       \
    do {                                                                             \
        if (!(condition)) {                                                          \
            throw TestFailure(std::string("Expected true: ") + #condition);         \
        }                                                                            \
    } while (false)

#define ASSERT_FALSE(condition)                                                      \
    do {                                                                             \
        if (condition) {                                                             \
            throw TestFailure(std::string("Expected false: ") + #condition);        \
        }                                                                            \
    } while (false)

#define ASSERT_EQ(expected, actual)                                                  \
    do {                                                                             \
        const auto expected_value = (expected);                                      \
        const auto actual_value = (actual);                                          \
        if (!(expected_value == actual_value)) {                                    \
            std::ostringstream assertion_message;                                    \
            assertion_message << "Expected equality: " << #expected << " and "      \
                              << #actual;                                            \
            throw TestFailure(assertion_message.str());                              \
        }                                                                            \
    } while (false)

#define ASSERT_STR_EQ(expected, actual)                                               \
    do {                                                                             \
        const std::string expected_value = (expected);                               \
        const std::string actual_value = (actual);                                   \
        if (expected_value != actual_value) {                                        \
            throw TestFailure(std::string("Expected strings equal: ") + #expected +  \
                              " and " + #actual);                                   \
        }                                                                            \
    } while (false)

#define RUN_ALL_TESTS() runAllTests()

#endif
