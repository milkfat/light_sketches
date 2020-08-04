#ifndef ARDUINO

#ifndef LIGHTS_WEB_SERVER_H
#define LIGHTS_WEB_SERVER_H

#include "web_pages.h"
#include "udp_client.h"

class HTTPS_SERVER {
    
    SimpleWeb::Server<SimpleWeb::HTTPS> server = {"server.crt", "server.key"};
    //SimpleWeb::Server<SimpleWeb::HTTP> server;
    std::thread thread;

  public:

    void stop() {
	  server.stop();
      thread.join();
    }

    void start() {

      using namespace std;
      using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;
    
      server.config.port = 8080;

      server.resource["/lights.html"]["GET"] = [](shared_ptr<HttpsServer::Response> res, shared_ptr<HttpsServer::Request> request) {
        
        std::string asdf = "";
        asdf.append(light_html_head());
                  
        char buff[3072];

        //generate sketch selection dropdown
        asdf.append(generate_sketch_dropdown(buff));

        //generate sketch controls  
        asdf.append(generate_html_controls(buff));


        //the rest of the page
        asdf.append(light_html_tail());
        res->write(asdf);
      };

      server.resource["^/([^/]+)$"]["GET"] = [](shared_ptr<HttpsServer::Response> response, shared_ptr<HttpsServer::Request> request) {
        fstream infile; 
        std::string file_location = "data/";
        file_location.append(request->path_match[1].str());
        infile.open(file_location);
        response->write(infile);
        infile.close();
      };

      server.default_resource["GET"] = [](shared_ptr<HttpsServer::Response> response, shared_ptr<HttpsServer::Request> request) {
        response->write("hello");
        response->write("<br>hello");
      };

      server.on_error = [](shared_ptr<HttpsServer::Request> /*request*/, const SimpleWeb::error_code & /*ec*/) {
        // Handle errors here
        // Note that connection timeouts will also call this handle with ec set to SimpleWeb::errc::operation_canceled
      };

    
      thread = std::thread([this]() {
        server.start();
      });
    


    }
    



};

HTTPS_SERVER https_server;



class WSS_SERVER {
    
    SimpleWeb::SocketServer<SimpleWeb::WSS> server = {"server.crt", "server.key"};
    //SimpleWeb::SocketServer<SimpleWeb::WS> server;
    std::thread thread;

  public:

    void stop() {
	    server.stop();
      thread.join();
    }

    void wssCurrentSketch () {
      char out_message[100] = "{\"current_sketch\":\"";
      uint16_t offset = 19;
      for (int j = 0; j < 20; j++) {
        char c = light_sketches.name()[j];
        if (!c) break;
        out_message[offset] = c;
        offset++;
      }
      out_message[offset] = '"';
      offset++;
      out_message[offset] = '}';
      offset++;
      out_message[offset] = '\0';
      
      for(auto &a_connection : server.get_connections())
        a_connection->send(out_message);
    }

    void wssHTMLControls () {
      char buff[3072];                          
      generate_html_controls(buff);

      for(auto &a_connection : server.get_connections())
        a_connection->send(buff); 

      generate_client_list(buff);
      
      for(auto &a_connection : server.get_connections())
        a_connection->send(buff); 

    }

    void start() {
    
      using namespace std;
      using WssServer = SimpleWeb::SocketServer<SimpleWeb::WSS>;
      //using WssServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

      server.config.port = 8081;

      auto &echo = server.endpoint["^/socket$"];

      

      echo.on_message = [](shared_ptr<WssServer::Connection> connection, shared_ptr<WssServer::InMessage> in_message) {

        streambuf * inbuf = in_message->rdbuf();

        if (inbuf->sgetc() != '{') {
          char out_message[1000];
          
          if (parse_binary(inbuf, out_message)) {

            connection->send(out_message, [](const SimpleWeb::error_code &ec) {
              if(ec) {
                cout << "Server: Error sending message. " <<
                    // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
                    "Error: " << ec << ", error message: " << ec.message() << endl;
              }
            });
          }
          //process binary
        }
        else {
          parse_json(inbuf);
        }
      };

      echo.on_open = [this](shared_ptr<WssServer::Connection> connection) {
        cout << "Server: Opened connection " << connection.get() << endl;
        this->wssCurrentSketch();
        this->wssHTMLControls();
      };

      // See RFC 6455 7.4.1. for status codes
      echo.on_close = [](shared_ptr<WssServer::Connection> connection, int status, const string & /*reason*/) {
        cout << "Server: Closed connection " << connection.get() << " with status code " << status << endl;
      };

      // Can modify handshake response header here if needed
      echo.on_handshake = [](shared_ptr<WssServer::Connection> /*connection*/, SimpleWeb::CaseInsensitiveMultimap & /*response_header*/) {
        return SimpleWeb::StatusCode::information_switching_protocols; // Upgrade to websocket
      };

      // See http://www.boost.org/doc/libs/1_55_0/doc/html/boost_asio/reference.html, Error Codes for error code meanings
      echo.on_error = [](shared_ptr<WssServer::Connection> connection, const SimpleWeb::error_code &ec) {
        cout << "Server: Error in connection " << connection.get() << ". "
          << "Error: " << ec << ", error message: " << ec.message() << endl;
      };

      thread = std::thread([this]() {
        // Start WSS-server
        server.start();
      });

    }


};

WSS_SERVER wss_server;

void wss_send() {
	wss_server.wssHTMLControls();
}






class UDP_SERVER
{

  boost::asio::ip::udp::socket socket_;
  boost::asio::ip::udp::socket socket2_;
  boost::asio::ip::udp::endpoint remote_endpoint_;
  boost::asio::ip::udp::endpoint remote_endpoint2_;
  boost::system::error_code err_; 
  boost::system::error_code err2_; 
  char recv_buffer_[1472];
  char recv_buffer2_[1472];

public:
  UDP_SERVER(boost::asio::io_service& io_service)
    : socket_(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 24322))
    , socket2_(io_service, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 24323))
  {
    start_receive();
    start_receive2();
  }

private:
  void start_receive()
  {
    socket_.async_receive_from(
        boost::asio::buffer(recv_buffer_), remote_endpoint_,
        boost::bind(&UDP_SERVER::handle_receive, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }



    
  void start_receive2()
  {
    socket2_.async_receive_from(
        boost::asio::buffer(recv_buffer2_), remote_endpoint2_,
        boost::bind(&UDP_SERVER::handle_receive2, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }

  //handles received UDP packets
  void handle_receive(const boost::system::error_code& error,
      std::size_t bytes_transferred)
  {
    if (!error || error == boost::asio::error::message_size)
    {
      // for (int i = 0; i < bytes_transferred; i++) {
      //   std::cout << recv_buffer_[i];
      // }
      //std::cout << " " << bytes_transferred << " from " << remote_endpoint_ << "\n";
      if (memcmp("LIGHTSKETCH", recv_buffer_, 11) == 0) {
        udp_clients.add_client(remote_endpoint_.address().to_v4().to_uint(), remote_endpoint_.port(), &recv_buffer_[11], bytes_transferred-11);
      }
      send(recv_buffer_, bytes_transferred);
      start_receive();
    }
  }

  //handles received UDP packets
  void handle_receive2(const boost::system::error_code& error,
      std::size_t bytes_transferred)
  {
    if (!error || error == boost::asio::error::message_size)
    {
      if (image_buffer) {
        static int current_image_buffer = 0;
        memcpy(image_buffer[current_image_buffer++], recv_buffer2_, bytes_transferred);
        if (current_image_buffer == NUM_IMAGE_BUFFERS || bytes_transferred != 1472) {
          current_image_buffer = 0;
        }
      }
      start_receive2();
    }
  }
  

  //sends a message to all known clients
  void send(const char* buffer, size_t size/*, const char* ip_address, const uint16_t port*/) {
    for (int i = 0; i < 10; i++) {
      socket_.async_send_to(boost::asio::buffer(buffer, size), boost::asio::ip::udp::endpoint(boost::asio::ip::address_v4(udp_clients[i].address), udp_clients[i].port),
        boost::bind(&UDP_SERVER::handle_send, this));
    }
  }

  void handle_send()
  {
  }

};
/*
				//send RGB data over UDP
				
				//buffer for image data and PPM header
				char my_buffer[8+(NUM_LEDS-1)*3];
				
				//PPM container header
				for (int i = 0; i < 9; i++) {
					my_buffer[i] = "P6 4 4 1 "[i];
				}

				//add image data to buffer
				for (int i = 0; i < NUM_LEDS-1; i++) {
					int j = 9+i*3;
					my_buffer[j+0]=leds[(NUM_LEDS-2)-i].r;
					my_buffer[j+1]=leds[(NUM_LEDS-2)-i].g;
					my_buffer[j+2]=leds[(NUM_LEDS-2)-i].b;
				}
				int buffer_position = 0;
				int bytes_remaining = sizeof(my_buffer);
				//send image data over UDP with max frame size of 1472
				while (bytes_remaining > 1472) {
					socket.send_to(boost::asio::buffer(&my_buffer[buffer_position], 1472), remote_endpoint, 0, err);
					buffer_position += 1472;
					bytes_remaining -= 1472;
					//std::cout << "Bytes sent: 1472\n";
				}
				
				if (bytes_remaining > 0) {
					socket.send_to(boost::asio::buffer(&my_buffer[buffer_position], bytes_remaining), remote_endpoint, 0, err);
					//std::cout << "Bytes sent: " << bytes_remaining << "\n";
				}
				*/
#endif

#endif