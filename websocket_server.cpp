#include <iostream>
#include <string>

#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"


typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

typedef server::message_ptr message_ptr;

void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg);

std::string parse_type(rapidjson::Document&);
rapidjson::Document get_payload(rapidjson::Document&);
float parse_a(rapidjson::Document&);
float parse_b(rapidjson::Document&);

std::string create_response(std::string, std::string);


int main()
{
	// Create server
	server my_server;

	try
	{
		// Initialize Asio
		std::cout << "Initializing ASIO" << std::endl;
		my_server.init_asio();

		// Register our message handler
		std::cout << "Binding message handler" << std::endl;
		my_server.set_message_handler(bind(&on_message, &my_server, ::_1, ::_2));

		// Listen on port 8080
		int port = 8080;
		std::cout << "Listening on port " << port << std::endl;
		my_server.listen(port);

		// Start accept loop
		std::cout << "Starting accept loop" << std::endl;
		my_server.start_accept();

		// Start the ASIO io_service run loop
		std::cout << "Running ASIO IO" << std::endl;
		my_server.run();
	}
	catch (websocketpp::exception const & e)
	{
		std::cout << "Websocketpp exception in main()" << std::endl;
		std::cout << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "Other exception in main()" << std::endl;
	}
}


// Handle request from client
// - Parse message received from client
// - Perform the requested mathematical operation
// - Send result in a response message to client
void on_message(server* s, websocketpp::connection_hdl hdl, message_ptr msg)
{
	std::cout << "on_message() was called with connection handle: " << hdl.lock().get() << std::endl;
	std::cout << "Message: " << msg->get_payload() << std::endl;
	std::cout << "Opcode: " << msg->get_opcode() << std::endl;

	if (msg->get_payload() == "stop-listening")
	{
		// Close connection
		s->pause_reading(hdl);
		s->close(hdl, websocketpp::close::status::normal, "");
		std::cout << "Connection Closed" << std::endl;

		s->stop_listening();
		std::cout << "No longer listening" << std::endl;

		return;
	}

	// Parse "type", "a", and "b" from the message
	std::cout << "Parsing message" << std::endl;
	rapidjson::Document message;
	message.Parse(msg->get_payload().c_str());

	std::string type = parse_type(message);
	std::cout << "type: " << type << std::endl;

	std::cout << "Parsing payload" << std::endl;
	rapidjson::Document payload = get_payload(message);

	float a = parse_a(payload);
	std::cout << "a: " << a << std::endl;

	float b = parse_b(payload);
	std::cout << "b: " << b << std::endl;

	std::string response_type;
	std::string response_payload;

	// Perform the appropriate operation
	if (type.compare("add") == 0)
	{
		response_type = "sum";
		response_payload = std::to_string(a + b);
	}
	else if (type.compare("subtract") == 0)
	{
		response_type = "difference";
		response_payload = std::to_string(a - b);
	}
	else
	{
		std::cout << "Invalid operation" << std::endl;
		abort();
	}

	// Create the message to be sent back to the client
	std::string response = create_response(response_type, response_payload);

	try
	{
		std::cout << "Responding with " << response << std::endl;
		s->send(hdl, response.c_str(), msg->get_opcode());
	}
	catch (websocketpp::exception const & e)
	{
		std::cout << "Response failed because: " << "(" << e.what() << ")" << std::endl;
	}
}


// Parse and return string "type" from JSON document
std::string parse_type(rapidjson::Document& doc)
{
	try
	{
		assert(doc.HasMember("type"));
		assert(doc["type"].IsString());

		return doc["type"].GetString();
	}
	catch (...)
	{
		std::cout << "Exception in parse_type()" << std::endl;
		abort();
	}
}


// Pull out and return "payload" object from JSON document
rapidjson::Document get_payload(rapidjson::Document& message_doc)
{
	try
	{
		assert(message_doc.HasMember("payload"));
		rapidjson::Value& payload_value = message_doc["payload"];

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		payload_value.Accept(writer);
		std::string payload_str = buffer.GetString();

		rapidjson::Document payload_doc;
		payload_doc.Parse(payload_str.c_str());

		return payload_doc;
	}
	catch (...)
	{
		std::cout << "Exception in get_payload()" << std::endl;
		abort();
	}
}


// Parse and return int "a" from JSON document
float parse_a(rapidjson::Document& doc)
{
	try
	{
		assert(doc.HasMember("a"));
		assert(doc["a"].IsNumber());

		return doc["a"].GetFloat();
	}
	catch (...)
	{
		std::cout << "Exception in parse_a()" << std::endl;
		abort();
	}
}


// Parse and return int "b" from JSON document
float parse_b(rapidjson::Document& doc)
{
	try
	{
		assert(doc.HasMember("b"));
		assert(doc["b"].IsNumber());

		return doc["b"].GetFloat();
	}
	catch (...)
	{
		std::cout << "Exception in parse_b()" << std::endl;
		abort();
	}
}


// Create the response message to be sent to the client
std::string create_response(std::string type, std::string payload)
{
	std::string response = "";

	response.append("{\"type\":\"");
	response.append(type);
	response.append("\",\"payload\":\"");
	response.append(payload);
	response.append("\"}");

	return response;
}
