#include <thread>
#include <future>
#include <iostream>
#include <mutex>
#include <chrono>

/**
 * This program uses std::async to start a bunch of tasks, which simply sleep
 * the current thread for a number of milliseconds.
 *
 * Using launch::async, the sleepfor function is called immediately on a new
 * thread
 * Using launch::deferred, the sleepfor function is called only when the get()
 * method on the future<> object is returned.
 *
 * The future<> is the object returned from std::async to allow the job to be
 * tracked.
 *
 * Example output:
 *
 * Main thread id 5212
 * Launching asynchronously
 * sleepfor(200) called from thread id 5020
 * sleepfor(1000) called from thread id 4512
 * sleepfor(300) called from thread id 5424
 * sleepfor(600) called from thread id 5448
 * sleepfor(250) called from thread id 744
 * sleepfor(400) called from thread id 5664
 * Done sleeping 200
 * Done sleeping 250
 * Done sleeping 300
 * Done sleeping 400
 * Done sleeping 600
 * Done sleeping 1000
 * Now with deferred
 * sleepfor(200) called from thread id 5212
 * Done sleeping 200
 * sleepfor(1000) called from thread id 5212
 * Done sleeping 1000
 * sleepfor(300) called from thread id 5212
 * Done sleeping 300
 * sleepfor(400) called from thread id 5212
 * Done sleeping 400
 * sleepfor(600) called from thread id 5212
 * Done sleeping 600
 * sleepfor(250) called from thread id 5212
 * Done sleeping 250
 */

std::mutex mtx;

bool sleepfor(int x) {
  {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "sleepfor(" << x << ") called from thread id "
      << std::this_thread::get_id() << std::endl;
  }
  std::chrono::milliseconds dura(x);
  std::this_thread::sleep_for(dura);
  {
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Done sleeping " << x << std::endl;
  }
  return true;
}

template <std::launch Policy>
void launchTasks() {
  std::vector<std::future<bool>> tasks;
  tasks.push_back(std::async(Policy, sleepfor, 200));
  tasks.push_back(std::async(Policy, sleepfor, 1000));
  tasks.push_back(std::async(Policy, sleepfor, 300));
  tasks.push_back(std::async(Policy, sleepfor, 400));
  tasks.push_back(std::async(Policy, sleepfor, 600));
  tasks.push_back(std::async(Policy, sleepfor, 250));
  std::for_each(tasks.begin(), tasks.end(),
    [] (std::future<bool>& task) {
    task.get();
  });
}

int main() {
  std::cout << "Main thread id " << std::this_thread::get_id() << std::endl;
  std::cout << "Launching asynchronously" << std::endl;
  launchTasks<std::launch::async>();
  std::cout << "Now with deferred" << std::endl;
  launchTasks<std::launch::deferred>();
}
