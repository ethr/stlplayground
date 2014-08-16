#include <random>
#include <array>
#include <iterator>
#include <fstream>
#include <iostream>
#include <numeric>
#include <cmath>
#include <functional>
#include <chrono>

/**
 * This program demonstrates some nice features of the standard C++ library,
 * including some C++11 features.
 * 
 * Firstly a series of random numbers are generated using utilities in <random>,
 * these are stored in a std::array then written out into a file via an
 * std::ostream_iterator.
 *
 * Next the file is read back in using a std::istream_iterator, and the
 * std::for_each function used to apply a summariser to each number in the file.
 * The istream_iterator does the work of parsing the file, although its unclear
 * how it can cope with anything other than white-space delimited files. A small
 * adapter is required to make the for_each. Again this is overkill and could be
 * done with a standard loop, but I wanted to try out some STL magic. I was
 * previously using std::accumulate() to do a simple summation of the values
 * which worked pretty well.
 *
 */

const size_t NROLLS = 1250001;

// Generate random numbers to file
void generate(const char* fileName) {
  std::ofstream numbersFile(fileName);
  std::ostream_iterator<double> numbersFile_it(numbersFile, " ");
  std::default_random_engine generator(1234);
  std::normal_distribution<double> distribution(5.0, 2.0);
  std::function<double()> rnd = std::bind(distribution, generator);
  std::generate_n(numbersFile_it, NROLLS, rnd);
  numbersFile.flush();
  numbersFile.close();
}

// Calculates mean, variance, std using an online algorithm
struct Summariser {
  Summariser() :
    sum(0.0),
    mean(0.0),
    var(0.0),
    M2(0.0),
    n(0.0)
  {}

  // Call with each data point
  void addObservation(double x) {
    sum += x;
    n = n + 1.0;
    double delta = x - mean;
    mean = mean + delta/n;
    M2 = M2 + delta*(x - mean);
  }

  // Wrapper for addObservation()
  void operator()(double x) {
	  addObservation(x);
  }

  // Call when all data has been observed
  void done() {
    var = M2/(n - 1);
    std = sqrt(var);
  }

  double sum;
  double mean;
  double var;
  double std;
  double M2;
  double n;
};

// Summarise
void summarise(const char* fileName, Summariser* summariser) {
  std::ifstream numbersFile(fileName);
  std::istream_iterator<double> eos;
  std::istream_iterator<double> numbersFile_it(numbersFile);
  // Use C++11 Lambda syntax to create an anonymous function to call
  // addObservation for each value
  // std::for_each(numbersFile_it, eos, [&] (double x) {
  //   summariser->addObservation(x);
  // });
  // Create a delegate to the summariser and use the operator()(double x)
  // method to add observations
  (std::for_each(numbersFile_it, eos, std::ref(*summariser))).get().done();
}

int main() {
  std::chrono::high_resolution_clock::time_point t1 =
	  std::chrono::high_resolution_clock::now();
  generate("numbers.txt");
  std::chrono::high_resolution_clock::time_point t2 =
	  std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> time_span =
	std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
  std::cout << "Generation time: " << time_span.count() << " seconds."
    << std::endl;
  Summariser summariser;
  summarise("numbers.txt", &summariser);
  std::cout << "Sum is: " << summariser.sum << std::endl;
  std::cout << "Mean is: " << summariser.mean << std::endl;
  std::cout << "Var is: " << summariser.var << std::endl;
  std::cout << "Std is: " << summariser.std << std::endl;
  std::chrono::high_resolution_clock::time_point t3 =
	  std::chrono::high_resolution_clock::now();
  time_span = std::chrono::duration_cast<std::chrono::duration<double>>(
      t3 - t2);
  std::cout << "Summariser time: " << time_span.count() << " seconds."
    << std::endl;
  return 0;
}
