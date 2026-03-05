#include <iostream>
#include <vector>
#include <thread>
#include <memory>
#include <iomanip>
#include <atomic>

template<typename T>
class ThreadSafeQueue {
private:
    // Implement this
public:
    void push(T value) {
        // Implement this
    }
    T pop() {
        // Implement this
        return nullptr;
    }
    size_t size() {
        // Implement this
        return 0;
    }
    // A non-blocking pop for graceful shutdown
    T pop_for_shutdown() {
        // Implement this
        return nullptr;
    }
};

// Class Representation of a Task
class ITask {
public:
    virtual ~ITask() = default;
    virtual void process() = 0;
    virtual float getProcessedValue() const = 0;
    virtual uint8_t getTaskType() const = 0;
};

class SimpleTask : public ITask {
private:
    float val_;
public:
    explicit SimpleTask(float val): val_{val} {}
    void process() override {val_ *= 2.0f;}
    float getProcessedValue() const override {return val_;}
    uint8_t getTaskType() const override {return 0;}
};

class ComplexTask : public ITask {
private:
    // You can define the members as per your requirement
    std::vector<int> nums_;
    int sum_{0};
public:
    explicit ComplexTask(std::vector<int> nums): nums_{nums} {}
    void process() override {
        for (const auto& num: nums_) sum_ += num;
    }
    float getProcessedValue() const override {return static_cast<float>(sum_);}
    uint8_t getTaskType() const override {return 1;}
};


class TaskGenerator {
private:
    ThreadSafeQueue<std::unique_ptr<ITask>>& task_queue_;
    std::atomic<bool>& shutdown_;
public:
    TaskGenerator(ThreadSafeQueue<std::unique_ptr<ITask>>& queue, std::atomic<bool>& shutdown)
        : task_queue_(queue), shutdown_(shutdown) {}
    void run() {
        // Implement the task generation loop with a shutdown check
    }
};

class TaskProcessor {
private:
    ThreadSafeQueue<std::unique_ptr<ITask>>& task_queue_;
    ThreadSafeQueue<std::unique_ptr<ITask>>& processed_queue_;
    std::atomic<bool>& shutdown_;
public:
    TaskProcessor(ThreadSafeQueue<std::unique_ptr<ITask>>& t_queue, ThreadSafeQueue<std::unique_ptr<ITask>>& p_queue, std::atomic<bool>& shutdown)
        : task_queue_(t_queue), processed_queue_(p_queue), shutdown_(shutdown) {}
    void run() {
        // Implement the data processing loop with a shutdown check
    }
};

class PacketTransmitter {
private:
    ThreadSafeQueue<std::unique_ptr<ITask>>& processed_queue_;
    std::atomic<bool>& shutdown_;
public:
    PacketTransmitter(ThreadSafeQueue<std::unique_ptr<ITask>>& queue, std::atomic<bool>& shutdown)
        : processed_queue_(queue), shutdown_(shutdown) {}
    void run() {
        // Implement the data transmission (bitpacking) loop with a shutdown check
    }
    void transmit(const std::unique_ptr<ITask>& data, std::ostream& os) {
        uint8_t buffer[8] = {0};
        
        // Bitpacking logic
        // Implement the bitpacking logic here

        // Print the buffer in hex format for verification
        os << "Packet: ";
        for (int i = 0; i < 8; ++i) {
            os << "0x" << std::setw(2) << std::setfill('0') << std::hex << (int)buffer[i] << " ";
        }
        os << std::dec << std::endl;
    }
};

#ifndef TESTING
int main() {
    std::cout << "Starting the data generation pipeline" << std::endl;

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

    std::cout << "Data Gen pipeline Finished." << std::endl;

    return 0;
}
#endif