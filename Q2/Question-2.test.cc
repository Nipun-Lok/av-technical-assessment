#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include <sstream>
#include <iomanip>
#include <cstring>
#define TESTING
// We include the implementation file to get access to the classes.
// In a real project, you would have separate header files.
#include "Question-2.cc"

// Sample test case for SimpleTask processing
TEST(Task1, SimpleTaskProcessing) {
    float initialValue = 12.5f;
    SimpleTask task(initialValue);

    task.process();

    float expectedValue = 25.0f;
    EXPECT_FLOAT_EQ(task.getProcessedValue(), expectedValue);
    EXPECT_EQ(task.getTaskType(), 0);
}

TEST(Task1, ComplexTaskProcessing) {
    std::vector<int> nums{{1,2,3,4}};
    ComplexTask task(nums);
    
    task.process();

    int expectedSum{10};
    EXPECT_EQ(task.getProcessedValue(), expectedSum);
    EXPECT_EQ(task.getTaskType(), 1);
}