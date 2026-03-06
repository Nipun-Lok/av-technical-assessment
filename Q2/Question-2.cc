#include <iostream>
#include <vector>
#include <thread>
#include <memory>
#include <iomanip>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>

template<typename T>
class ThreadSafeQueue {
private:
    std::mutex mx_;
    std::condition_variable cv_;
    std::queue<T> queue_;

public:
    /**
     * @brief Pushes value into queue. Notifies next paused thread when completed
     * @param value Element to be added to queue
     */
    void push(T value) {
        {
            std::lock_guard<std::mutex> lock(mx_);
            queue_.push(std::move(value));
        }
        cv_.notify_one();
    }
    T pop() {
        // Implement this
        return nullptr;
    }
    size_t size() {
        std::lock_guard<std::mutex> lock(mx_);
        return queue_.size();
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

    /**
     * @brief Doubles the stored value
     */
    void process() override {val_ *= 2.0f;}

    /**
     * @return stored value
     */
    float getProcessedValue() const override {return val_;}

    /**
     * @return 0 for SimpleTask
     */
    uint8_t getTaskType() const override {return 0;}
};

class ComplexTask : public ITask {
private:
    std::vector<int> nums_;
    int sum_{0};
public:
    explicit ComplexTask(std::vector<int> nums): nums_{nums} {}
    void process() override {
        sum_ = 0;
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
        // do not run if shutting down
        if (shutdown_.load()) return;
        std::unique_ptr<ITask> task;
        // alternate between simple and complex tasks 10 times
        for (int count{10}; !shutdown_.load() && count ; count--) {
            if (count % 2) {
                // simple tasks use task_queue_ size as preprocessed value
                task = std::make_unique<SimpleTask>(static_cast<float>(task_queue_.size()));
            } else {
                task = std::make_unique<ComplexTask>(std::vector<int>({1,2,3,4}));
            }
            task_queue_.push(std::move(task));
        }
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