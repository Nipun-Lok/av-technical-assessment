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

TEST(Task2, TaskGenerator) {
    std::atomic<bool> shutdown_flag{false};

    ThreadSafeQueue<std::unique_ptr<ITask>> task_queue;
    TaskGenerator generator(task_queue, shutdown_flag);
    std::thread generator_thread(&TaskGenerator::run, &generator);

    std::this_thread::sleep_for(std::chrono::seconds(10));

    shutdown_flag = true;

    generator_thread.join();

    int expectedSize{10};
    EXPECT_EQ(task_queue.size(), expectedSize);
}

TEST(Task2, TaskProcessor) {
    std::atomic<bool> shutdown_flag{false};

    ThreadSafeQueue<std::unique_ptr<ITask>> task_queue;
    ThreadSafeQueue<std::unique_ptr<ITask>> processed_queue;

    TaskGenerator generator(task_queue, shutdown_flag);
    TaskProcessor processor(task_queue, processed_queue, shutdown_flag);
    std::thread generator_thread(&TaskGenerator::run, &generator);
    std::thread processor_thread(&TaskProcessor::run, &processor);

    std::this_thread::sleep_for(std::chrono::seconds(10));

    shutdown_flag = true;

    generator_thread.join();
    processor_thread.join();

    int expectedTaskSize{0};
    int expectedProcessedSize{10};
    EXPECT_EQ(task_queue.size(), expectedTaskSize);
    EXPECT_EQ(processed_queue.size(), expectedProcessedSize);
}

TEST(Task2, PacketTransmitter) {
    std::atomic<bool> shutdown_flag{false};

    ThreadSafeQueue<std::unique_ptr<ITask>> task_queue;
    ThreadSafeQueue<std::unique_ptr<ITask>> processed_queue;

    TaskGenerator generator(task_queue, shutdown_flag);
    TaskProcessor processor(task_queue, processed_queue, shutdown_flag);
    PacketTransmitter transmitter(processed_queue, shutdown_flag);

    std::thread generator_thread(&TaskGenerator::run, &generator);
    std::thread processor_thread(&TaskProcessor::run, &processor);
    std::thread transmitter_thread(&PacketTransmitter::run, &transmitter, std::ref(std::cout));

    std::this_thread::sleep_for(std::chrono::seconds(10));

    shutdown_flag = true;

    generator_thread.join();
    processor_thread.join();
    transmitter_thread.join();

    int expectedTaskSize{0};
    int expectedProcessedSize{0};
    EXPECT_EQ(task_queue.size(), expectedTaskSize);
    EXPECT_EQ(processed_queue.size(), expectedProcessedSize);
}

TEST(Task3, PacketTransmitter) {
    std::atomic<bool> shutdown_flag{false};
    ThreadSafeQueue<std::unique_ptr<ITask>> test_queue;
    PacketTransmitter transmitter(test_queue, shutdown_flag);
    
    // make simple task
    std::unique_ptr<ITask> simpleTask{std::make_unique<SimpleTask>(10.0f)};
    simpleTask->process();

    // make complex task
    std::vector<int> test{1,2,3,4};
    std::unique_ptr<ITask> complexTask{std::make_unique<ComplexTask>(test)};
    complexTask->process();

    // output string stream
    std::stringstream out{};

    // simple task test
    transmitter.transmit(simpleTask, out);
    std::string expected{"Packet: 0x00 0x41 0xa0 0x00 0x00"};
    std::string result{out.str().substr(0, expected.length())};

    EXPECT_EQ(expected, result);
    
    out.str("");
    // complex task test
    transmitter.transmit(complexTask, out);

    expected = "Packet: 0x40 0x00 0x00 0x00 0x0a";
    result = out.str().substr(0, expected.length());

    EXPECT_EQ(expected, result);
}