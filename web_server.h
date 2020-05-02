#ifndef ARDUINO

#ifndef LIGHTS_WEB_SERVER_H
#define LIGHTS_WEB_SERVER_H

#include "web_pages.h"

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
                  
        char buff[2048];
        char * p = buff;
        *p = '\0';

        //generate sketch selection dropdown
        generate_sketch_dropdown(p);

        //generate sketch controls  
        generate_html_controls(p);

        asdf.append(buff);

        //the rest of the page
        asdf.append(light_html_tail());
        res->write(asdf);
      };

      server.resource["^/([^/]+)$"]["GET"] = [](shared_ptr<HttpsServer::Response> response, shared_ptr<HttpsServer::Request> request) {
        fstream infile; 
        infile.open(request->path_match[1].str());
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
      char buff[2048];
      char * p = buff; 
      *p = '\0';                               
      generate_html_controls(p);

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
          parse_javascript(inbuf);
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

#endif

#endif