#include <aws/lambda-runtime/runtime.h>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/utils/memory/stl/SimpleStringStream.h>

using namespace aws::lambda_runtime;
using namespace Aws::Utils::Json;

// Pricing a European vanilla call option with a Monte Carlo method
double monte_carlo_call_price(const int& num_sims, const double& S, const double& K, const double& r, const double& v, const double& T) {
  double s_adjust = S * exp(T*(r-0.5*v*v));
  double s_cur = 0.0;
  double payoff_sum = 0.0;

  std::random_device rd {};
  std::mt19937 prng {rd()};
  std::normal_distribution<> d {0, 1};

  for (int i=0; i<num_sims; i++) {
    s_cur = s_adjust * exp(sqrt(v*v*T)*d(prng));
    payoff_sum += std::max(s_cur - K, 0.0);
  }

  return (payoff_sum / static_cast<double>(num_sims)) * exp(-r*T);
}

static invocation_response my_handler(invocation_request const& request) {

  // parameter list
  int num_sims = 100000;    // Number of simulated asset paths; override with query parameter 'num_sims`
  double S = 100.0;         // Stock price
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
