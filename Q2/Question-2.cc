#include <iostream>
#include <vector>
#include <thread>
#include <memory>
#include <iomanip>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <cstring>
#include <ctime>

template<typename T>
class ThreadSafeQueue {
private:
    std::mutex mx_;
    std::condition_variable cv_;
    std::queue<T> queue_;

    /** 
     * @brief Tries to pop element off front of queue
     * @return popped off element or
     * @return nullptr if queue empty.
     */
    T pop_() {
        if (queue_.empty()) return nullptr;
        T value{std::move(queue_.front())};
        queue_.pop();
        return value;
    }

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

    /**
     * @brief Pops element off front of queue. Pauses thread if queue empty or 
     * shutdown and rechecks every 50ms. On wake will still run and return as 
     * normal.
     * @param shutdown atomic flag to indicate shutdown process
     * @return popped element or nullptr if empty.
     */
    T pop(std::atomic<bool>& shutdown) {
        std::unique_lock<std::mutex> lock(mx_);
        // if queue empty pause thread and check again occasionally
        while (queue_.empty() && !shutdown.load()) {
            cv_.wait_for(lock, std::chrono::milliseconds(50));
        }
        return pop_();
    }

    // /**
    //  * @return size of queue
    //  */
    size_t size() {
        std::lock_guard<std::mutex> lock(mx_);
        return queue_.size();
    }

    /**
     * @brief A non-blocking pop for graceful shutdown
     */ 
    T pop_for_shutdown() {
        std::lock_guard<std::mutex> lock(mx_);
        return pop_();
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

    /**
     * @brief Computes sum of elements in vector
     */
    void process() override {
        sum_ = 0;
        for (const auto& num: nums_) sum_ += num;
    }

    /**
     * @return post processed sum of elements in vector as float
     */
    float getProcessedValue() const override {return static_cast<float>(sum_);}

    /**
     * @return 1 for ComplexTask
     */
    uint8_t getTaskType() const override {return 1;}
};


class TaskGenerator {
private:
    ThreadSafeQueue<std::unique_ptr<ITask>>& task_queue_;
    std::atomic<bool>& shutdown_;
public:
    TaskGenerator(ThreadSafeQueue<std::unique_ptr<ITask>>& queue, std::atomic<bool>& shutdown)
        : task_queue_(queue), shutdown_(shutdown) {}
    
    /**
     * @brief Adds tasks to threadsafe queue
     */
    void run() {
        std::unique_ptr<ITask> task;
        // alternate between simple and complex tasks 10 times or until
        // shutdown flag is set
        for (int count{10}; !shutdown_.load() && count; count--) {
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

    /**
     * @brief Pops task from task queue, process and pushes to processed queue
     */
    void run() {
        std::unique_ptr<ITask> task;
        // runs until shutdown flag is set or task queue is empty
        while (!shutdown_.load()) {
            task = task_queue_.pop(shutdown_);
            if (task) {
                task->process();
                processed_queue_.push(std::move(task));
            }
        }
        // runs until task queue is empty
        while ((task = task_queue_.pop_for_shutdown())) {
            task->process();
            processed_queue_.push(std::move(task));
        }
    }
};

class PacketTransmitter {
private:
    ThreadSafeQueue<std::unique_ptr<ITask>>& processed_queue_;
    std::atomic<bool>& shutdown_;
public:
    PacketTransmitter(ThreadSafeQueue<std::unique_ptr<ITask>>& queue, std::atomic<bool>& shutdown)
        : processed_queue_(queue), shutdown_(shutdown) {}

    /**
     * @brief Sends data to ostream os and pops task off processed queue
     */
    void run(std::ostream& os) {
        std::unique_ptr<ITask> task;
        while (!shutdown_.load() && (task = processed_queue_.pop(shutdown_))) {
            transmit(task, os);
        }
        // fall through and exit if shutdown
        while ((task = processed_queue_.pop_for_shutdown())) {
            transmit(task, os);
        }
    }

    /**
     * @brief transmits data to os ostream
     * @param data unique pointer to task
     * @param os output stream
     */
    void transmit(const std::unique_ptr<ITask>& data, std::ostream& os) {
        int currentTime = std::time(nullptr);
        uint8_t buffer[8] = {0};

        // write task type
        buffer[0] = (data->getTaskType()) ? 0x40:0;

        // check task type through bitmask at bit 6 and copy processed value
        // in big endian 
        int val{};
        if (!(buffer[0] & 0x40)) {
            float rawFloat = data->getProcessedValue();
            // copy bits to integer for bitwise operations
            std::memcpy(&val, &rawFloat, sizeof(float));
        } else {
            // cast to int to write as integer representation
            val = static_cast<int>(data->getProcessedValue());
        }
        
        // reverse to big endian
        val = val >> 24 
            | ((val >> 8) & 0xFF00)
            | ((val << 8) & 0xFF0000)
            | ((val << 24) & 0xFF000000);
        std::memcpy(&buffer[1], &val, sizeof(int));

        // copy least significant 3 bytes. currentTime is little endian -> convert
        buffer[5] = (currentTime >> 16) & 0xFF;
        buffer[6] = (currentTime >> 8) & 0xFF;
        buffer[7] = currentTime & 0xFF;

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