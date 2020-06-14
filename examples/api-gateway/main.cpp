#include <aws/lambda-runtime/runtime.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/utils/logging/LogLevel.h>
#include <aws/core/utils/memory/stl/SimpleStringStream.h>
 
using namespace aws::lambda_runtime; 

invocation_response my_handler(invocation_request const& request) 
{
	
	using namespace Aws::Utils::Json;
	
	JsonValue json(request.payload);
	if (!json.WasParseSuccessful()) {
        return invocation_response::failure("Failed to parse input JSON", "InvalidJSON");
    }

	auto v = json.View();
	Aws::SimpleStringStream ss; 
	ss << "Good "; 

	// see precondition remark in README 
	if (v.ValueExists("body")) {
		
		auto body = v.GetString("body"); 
		
		JsonValue body_json(body);
		if (body_json.WasParseSuccessful()) {
			auto body_v = body_json.View();  
			ss << (body_v.ValueExists("time") ? body_v.GetString("time") : ""); 	
		}
	}
	ss << ", "; 

	if (v.ValueExists("queryStringParameters")) {
		auto query_params = v.GetObject("queryStringParameters"); 
		ss << (query_params.ValueExists("name") ? query_params.GetString("name") : "") << " of "; 
		ss << (query_params.ValueExists("city") ? query_params.GetString("city") : "") << ". "; 
	}

	if (v.ValueExists("headers")) {
		auto headers = v.GetObject("headers"); 
		ss << "Happy " << (headers.ValueExists("day") ? headers.GetString("day") : "") << "!"; 
	}	
	
	JsonValue resp; 
	resp.WithString("message", ss.str()); 

	return invocation_response::success(resp.View().WriteCompact(), "applications/json"); 
}

int main() {
	run_handler(my_handler); 
	return 0; 
}
