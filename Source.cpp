#include "cpprest/containerstream.h"
#include "cpprest/filestream.h"
#include "cpprest/http_client.h"
#include "cpprest/json.h"
#include "cpprest/producerconsumerstream.h"
#include <iostream>
#include <sstream>
#include <conio.h>

using namespace ::pplx;
using namespace utility;
using namespace concurrency::streams;

using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace web::json;

void DisplayJSONValue(json::value v)
{
	if (!v.is_null())
	{
	
		for (auto iter = v.as_object().cbegin(); iter != v.as_object().cend(); ++iter)
		{
			//// It is necessary to make sure that you get the value as const reference
			//// in order to avoid copying the whole JSON value recursively (too expensive for nested objects)
			//const json::value &key = iter->first;
			//const json::value &value = iter->second;

			//if (value.is_object() || value.is_array())
			//{
			//	// We have an object with children or an array
			//	if ((!key.is_null()) && (key.is_string()))
			//	{
			//		std::wcout << L"Parent: " << key.as_string() << std::endl;
			//	}
			//	// Loop over each element in the object by calling DisplayJSONValue
			//	DisplayJSONValue(value);
			//	if ((!key.is_null()) && (key.is_string()))
			//	{
			//		std::wcout << L"End of Parent: " << key.as_string() << std::endl;
			//	}
			//}
			//else
			//{
			//	// Always display the value as a string
			//	std::wcout << L"Key: " << key.as_string() << L", Value: " << value.to_string() << std::endl;
			//}
		}
	}
}

pplx::task<void> HTTPGetAsync(std::wstring pa1th)
{
	// Create http_client to send the request.
	http_client client(U("https://graph.facebook.com/v2.1/815784245224772/"));

	// Manually build up an HTTP request with a header that specifies the content type and the request URI
	http_request request(methods::GET);
	request.headers().set_content_type(L"application/json");
	//request.set_request_uri(U("groups"));


	// Build request URI and start the request.
	uri_builder builder(U("/comments"));
	builder.append_query(U("access_token"), U("EAACEdEose0cBAPUWu1QSo1Pu3YWXg7hRHaI6XueOZAgms0YH7asTKuuGX2cmtFfpoUSC7KjEI6AZALL6EUpTadVgfQ4r4WhxtJcDBQim0QPFZBpzcCEf3NP4ihAH3g6fO6Jw59UrgcwC54mhLyZCkl6scZCafgqqp6K6FbU4FQgZDZD"));
	//return client.request(methods::GET, builder.to_string());

	// Make an HTTP GET request and asynchronously process the response
	return client
		.request(request)
		// The following code executes when the response is available
		.then([](http_response response) -> pplx::task<json::value>
	{
		std::wostringstream stream;
		stream.str(std::wstring());
		stream << L"Content type: " << response.headers().content_type() << std::endl;
		stream << L"Error code: " << response.status_code() << ": "<< response.reason_phrase() <<std::endl;
		stream << L"Content length: " << response.headers().content_length() << L"bytes" << std::endl;
		stream << L"Data: " << response.body() << L"bytes" << std::endl;
		std::wcout << stream.str();

		// If the status is OK extract the body of the response into a JSON value
		if (response.status_code() == status_codes::OK)
		{
			return response.extract_json();
		}
		else
		{
			// return an empty JSON value
			return pplx::task_from_result(json::value());
		}
		// Continue when the JSON value is available
	}).then([](pplx::task<json::value> previousTask)
	{
		// Get the JSON value from the task and call the DisplayJSONValue method
		try
		{
			json::value const & value = previousTask.get();
			DisplayJSONValue(value);
		}
		catch (http_exception const & e)
		{
			std::wcout << e.what() << std::endl;
		}
	});
}

#ifdef _MS_WINDOWS
int wmain(int argc, wchar_t *args[])
#else
int main(int argc, char *args[])
#endif
{
	auto fileStream = std::make_shared<ostream>();

	// Open stream to output file.
	pplx::task<void> requestTask = fstream::open_ostream(U("results.html")).then([=](ostream outFile)
	{
		*fileStream = outFile;

		// Create http_client to send the request.
		http_client client(U("https://graph.facebook.com/v2.1/815784245224772/")); //id_facebook_post = 815784245224772, replace by yourself.

		// Build request URI and start the request.
		uri_builder builder(U("/comments"));
		builder.append_query(U("access_token"), U("adVgfQ4r4WhxtJcDBQim0QPFZBpzcCEf3NP4ihAH3g6fO6Jw59UrgcwC54mhLyZCkl6scZCafgqqp6K6FbU4FQgZDZD"));//replace by your token:
		//get it at : https://developers.facebook.com/tools/explorer
		return client.request(methods::GET, builder.to_string());
	})

		// Handle response headers arriving.
		.then([=](http_response response)
	{
		printf("Received response status code:%u\n", response.status_code());

		/*if(!jsonObject.isNull("paging")) {
                                JSONObject paging = jsonObject.getJSONObject("paging");
                                JSONObject cursors = paging.getJSONObject("cursors");
                                if (!cursors.isNull("after"))
                                    afterString[0] = cursors.getString("after");
                                else
                                    noData[0] = true;*/

		// Write response body into the file.
		return response.body().read_to_end(fileStream->streambuf());


	})

		// Close the file stream.
		.then([=](size_t)
	{
		return fileStream->close();
	});

	// Wait for all the outstanding I/O to complete and handle any exceptions
	try
	{
		requestTask.wait();
	}
	catch (const std::exception &e)
	{
		printf("Error exception:%s\n", e.what());
	}

	return 0;
}