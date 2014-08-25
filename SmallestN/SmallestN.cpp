#include <algorithm>
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <random>
#include <iterator>
#include <chrono>
#include <memory>
#include <list>
#include <iomanip>

/**
 * This program presents two methods for tracking the N smallest numbers in a
 * stream of numbers. The first method uses a std::vector to store N number in
 * sorted order. The second version uses a linked list.
 *
 * Example output:
 *            Quantity       findNSmallest      findNSmallest2
 *               100           0.0010001                    0
 *              1000           0.0100005            0.0030002
 *             10000            0.101006            0.0230013
 *            100000            0.980056             0.211012
 *           1000000             9.59255              2.09012
 *
 * The std::vector<> version has to constantly re-allocate its buffer of
 * numbers, causing a big slow down in performance. Version 2 has limited memory
 * operations.
 *
 * Both algorithms work in O(N) time but the first version is nearly 5x slower
 * due to the memory allocations.
 */

class Timer {
  public:

    void start() {
      t1_ =std::chrono::high_resolution_clock::now();
    }

    void stop() {
      t2_ =std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> time_span =
        std::chrono::duration_cast<std::chrono::duration<double>>(t2_ - t1_);
      time_ = time_span.count();
    }

    double time() const {
      return time_;
    }

  private:
    double time_;
    std::chrono::high_resolution_clock::time_point t1_;
    std::chrono::high_resolution_clock::time_point t2_;
};

class ListNode {
  public:
    ListNode(double value, ListNode* next = NULL) :
      value_(value),
      next_(next) {}

    ~ListNode() {
      if (next_ != NULL) {
        delete next_;
      }
    }

    ListNode* next() {
      return next_;
    }

    double value() const {
      return value_;
    }

    void join(ListNode* other) {
      next_ = other;
    }

  private:
    double value_;
    ListNode* next_;
};

template <class T, class V>
void findNSmallest(T first, T last, size_t n, V oit) {
  std::vector<double> buffer(n, std::numeric_limits<double>::max());
  buffer.reserve(n + 1);
  for (; first != last; ++first) {
    double tmp = *first;
    auto it = std::lower_bound(buffer.begin(), buffer.end(), tmp);
    buffer.insert(it, tmp);
    buffer.resize(n);
  }
  std::copy(buffer.begin(), buffer.end(), oit);
}

template <class T, class V>
void findNSmallest2(T first, T last, size_t n, V oit) {
  std::auto_ptr<ListNode> head;
  std::vector<double> buffer(n, std::numeric_limits<double>::max());
  for (; first != last; ++first) {
    double tmp = *first;
    if (head.get() == NULL) {
      head.reset(new ListNode(tmp));
    }
    else {
      // Insertion into correct place
      ListNode* currNode = head.get();
      ListNode* prevNode = NULL;
      for (;;) {
        if (tmp < currNode->value()) {
          // Insert
          if (prevNode == NULL) {
            // Inserts at start
            head.release();
            head.reset(new ListNode(tmp, currNode));
          }
          else {
            // Insert part way through
            ListNode* newnode = new ListNode(tmp, currNode);
            prevNode->join(newnode);
          }
          break;
        }
        if (currNode->next() == NULL) {
          // Reached end of list
          ListNode* newnode = new ListNode(tmp);
          currNode->join(newnode);
          break;
        } else {
          prevNode = currNode;
          currNode = currNode->next();
        }
      }
    }
    // Trim list to size
    ListNode* currNode = head.get();
    for (size_t i = 0; currNode != NULL && i < n; ++i) {
      if (i == n - 1) {
        ListNode* tmp = currNode->next();
        currNode->join(NULL);
        delete tmp;
      }
      currNode = currNode->next();
    }
  }
  for (ListNode* currNode = head.get(); currNode != NULL;
      currNode = currNode->next()) {
    *oit = currNode->value();
  }
}

template <class Fn>
double harness(const std::vector<double>& input, size_t N, Fn func) {
  std::vector<double> output(N);
  Timer timer;
  timer.start();
  func(input.begin(), input.end(), N, output.begin());
  timer.stop();
  return timer.time();
}

int main() {
  const static size_t N = 10;
  const size_t COL_WIDTH = 20;
  std::cout << std::setw(COL_WIDTH) << "Quantity";
  std::cout << std::setw(COL_WIDTH) << "findNSmallest";
  std::cout << std::setw(COL_WIDTH) << "findNSmallest2";
  std::cout << std::endl;
  size_t quant = 100;
  for (size_t i = 0; i < 10; ++i) {
    std::default_random_engine generator(1234);
    std::normal_distribution<double> distribution(5.0, 2.0);
    std::function<double()> rnd = std::bind(distribution, generator);
    std::vector<double> input(quant);
    std::generate_n(input.begin(), input.size(), rnd);
    std::cout << std::setw(COL_WIDTH) << quant;
    std::cout << std::setw(COL_WIDTH) << harness(input, N,
        findNSmallest<std::vector<double>::const_iterator,
        std::vector<double>::iterator>) << " ";
    std::cout << std::setw(COL_WIDTH) << harness(input, N,
        findNSmallest2<std::vector<double>::const_iterator,
        std::vector<double>::iterator>) << " ";
    std::cout << std::endl;
    quant *= 10;
  }
  return 0;
}
