#include <aws/lambda-runtime/runtime.h>
#include <iomanip>
#include <sstream>
#include <algorithm>    // Needed for the "max" function
#include <cmath>
#include <iostream>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/utils/memory/stl/SimpleStringStream.h>

using namespace aws::lambda_runtime;
using namespace Aws::Utils::Json;



// A simple implementation of the Box-Muller algorithm, used to generate
// gaussian random numbers - necessary for the Monte Carlo method below
// Note that C++11 actually provides std::normal_distribution<> in 
// the <random> library, which can be used instead of this function
double gaussian_box_muller() {
  double x = 0.0;
  double y = 0.0;
  double euclid_sq = 0.0;

  // Continue generating two uniform random variables
  // until the square of their "euclidean distance" 
  // is less than unity
  do {
    x = 2.0 * rand() / static_cast<double>(RAND_MAX)-1;
    y = 2.0 * rand() / static_cast<double>(RAND_MAX)-1;
    euclid_sq = x*x + y*y;
  } while (euclid_sq >= 1.0);

  return x*sqrt(-2*log(euclid_sq)/euclid_sq);
}

// Pricing a European vanilla call option with a Monte Carlo method
double monte_carlo_call_price(const int& num_sims, const double& S, const double& K, const double& r, const double& v, const double& T) {
  double S_adjust = S * exp(T*(r-0.5*v*v));
  double S_cur = 0.0;
  double payoff_sum = 0.0;

  for (int i=0; i<num_sims; i++) {
    double gauss_bm = gaussian_box_muller();
    S_cur = S_adjust * exp(sqrt(v*v*T)*gauss_bm);
    payoff_sum += std::max(S_cur - K, 0.0);
  }

  return (payoff_sum / static_cast<double>(num_sims)) * exp(-r*T);
}

static invocation_response my_handler(invocation_request const& request) {

  // parameter list
  int num_sims = 100000;    // Number of simulated asset paths; override with query parameter 'num_sims`
  double S = 100.0;         // Option price
  double K = 100.0;         // Strike price
  double r = 0.05;          // Risk-free rate (5%)
  double v = 0.2;           // Volatility of the underlying (20%)
  double T = 1.0;           // One year until expiry

  // validate input
  if (request.payload.length() > 111111111) {
    return invocation_response::failure("error message here"/*error_message*/, "error type here" /*error_type*/);
  }

  JsonValue json(request.payload);
  if (!json.WasParseSuccessful()) {
    return invocation_response::failure("Failed to parse input JSON", "InvalidJSON");
  }

  auto view = json.View();
  Aws::SimpleStringStream ss;
  ss << "OK ";

  if (view.ValueExists("queryStringParameters")) {
    auto query_params = view.GetObject("queryStringParameters");
    if (query_params.ValueExists("num_sims") && query_params.GetObject("num_sims").IsString()) {
      num_sims = stoi(query_params.GetString("num_sims"));  //override default
    }
  }

  ss << "num_sims=" << std::to_string(num_sims) << ", ";

  double callprice = monte_carlo_call_price(num_sims, S, K, r, v, T);
  ss << "Call price=" << std::to_string(callprice) << " ";
  std::cout << ss.str() << std::endl;

  JsonValue response;
  response.WithString("message", ss.str());
  return invocation_response::success(response.View().WriteCompact(), "application/json");
}

int main()
{
    run_handler(my_handler);
    return 0;
}
