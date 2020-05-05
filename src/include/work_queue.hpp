#if not defined(MANDEL_WORK_QUEUE_HPP_)
#define MANDEL_WORK_QUEUE_HPP_

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <tuple>

template <class WorkItem>
class work_queue {
    std::queue<WorkItem> queue_;
    int capacity_;
    int work_done_;
    bool all_done_ = false;

    std::mutex mtx_;

    std::condition_variable space_available_;
    std::condition_variable work_available_;
    std::condition_variable work_was_done_;

  public:

    work_queue(int capacity) : capacity_(capacity) {}

    std::tuple<bool, WorkItem> get_work() {
        std::unique_lock<std::mutex> l(mtx_);

        work_available_.wait(l, [this](){return all_done_ or queue_.size() > 0; });

        if (all_done_) {
            return {false, WorkItem()};
        }

        auto retval = queue_.front();
        queue_.pop();
        work_done_ += 1;

        space_available_.notify_one();
        work_was_done_.notify_one();
        
        return { true, retval };
    }

    int add_work(WorkItem &fp) {

        std::unique_lock<std::mutex> l(mtx_);

        space_available_.wait(l, [this](){
                return queue_.size() < unsigned(capacity_);});

        queue_.push(fp);

        work_available_.notify_one();

        return queue_.size();
    }

    int done_count(int old_count = 0) {
        std::unique_lock<std::mutex> l(mtx_);

        work_was_done_.wait(l, [this, old_count](){
                return work_done_ != old_count; });

        return work_done_;
    }

    void set_all_done() {
        std::unique_lock<std::mutex> l(mtx_);

        all_done_ = true;
        work_available_.notify_one();
    }

};

#endif
