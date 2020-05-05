#include "work_queue.hpp"


std::tuple<bool, fractal_params> work_queue::get_work() {

    std::unique_lock<std::mutex> l(mtx_);

    work_available_.wait(l, [this](){return all_done_ or queue_.size() > 0; });

    if (all_done_) {
        return {false, fractal_params()};
    }

    auto retval = queue_.front();
    queue_.pop();
    work_done_ += 1;

    space_available_.notify_one();
    work_was_done_.notify_one();
    
    return { true, retval };
}

int work_queue::add_work(fractal_params &fp) {

    std::unique_lock<std::mutex> l(mtx_);

    space_available_.wait(l, [this](){return queue_.size() < unsigned(capacity_);});

    queue_.push(fp);

    work_available_.notify_one();

    return queue_.size();
}

int work_queue::done_count(int old_count) {
    std::unique_lock<std::mutex> l(mtx_);

    work_was_done_.wait(l, [this, old_coun:0
            t](){return work_done_ != old_count; });

    return work_done_;
}

void work_queue::set_all_done() {
    std::unique_lock<std::mutex> l(mtx_);

    all_done_ = true;
    work_available_.notify_one();
}



