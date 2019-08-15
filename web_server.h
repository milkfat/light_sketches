#ifndef ARDUINO

#ifndef LIGHTS_WEB_SERVER_H
#define LIGHTS_WEB_SERVER_H

class HTTPS_SERVER {
    
    SimpleWeb::Server<SimpleWeb::HTTPS> server = {"server.crt", "server.key"};
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

      server.resource["^/draw$"]["GET"] = [](shared_ptr<HttpsServer::Response> response, shared_ptr<HttpsServer::Request> request) {
        fstream infile; 
        infile.open("draw.html");
        response->write(infile);
        infile.close();
      };

      server.default_resource["GET"] = [](shared_ptr<HttpsServer::Response> response, shared_ptr<HttpsServer::Request> request) {
        response->write("hello");
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
    std::thread thread;

  public:

    void stop() {
	    server.stop();
      thread.join();
    }

    void start() {
    
      using namespace std;
      using WssServer = SimpleWeb::SocketServer<SimpleWeb::WSS>;

      server.config.port = 8081;

      auto &echo = server.endpoint["^/socket$"];
    

      echo.on_message = [](shared_ptr<WssServer::Connection> connection, shared_ptr<WssServer::InMessage> in_message) {
        

        if (in_message->peek() != '{') {
          
          //treat our message as binary
          char c;
          while (in_message->get(c)) {        
            switch (c) {
              
              case 'l':
                {
                  //read cursor location
                  uint8_t offset = 0;
                  int x;
                  int y;
                  int x1;
                  int y1;
                  uint8_t p;
                  uint8_t id;

                  x = (int)in_message->get();
                  y = (int)in_message->get();
                  x1 = (int)in_message->get();
                  y1 = (int)in_message->get();
                  p = (int)in_message->get();
                  id = (int)in_message->get();
                  button2_down = true;

                  if (drawing_enabled) {
                      draw_line_fine(temp_canvas, (x+offset)*256, y*256, (x1+offset)*256, y1*256, 255, 255, p);
                  }
              
                  update_pointer(x+offset,y,x1+offset,y1,p,id);
                }
                break;
              
              case 'e':
                //read cursor lifts (end)
                _remove_pointer( (int)in_message->get() );
                button2_down = false;
                break;

              case 'a':
                //read orientation data from device (angles)
                in_message->read((char*)&rotation_alpha, 4);
                in_message->read((char*)&rotation_beta, 4);
                in_message->read((char*)&rotation_gamma, 4);
                break;

              case 'd':
                drawing_enabled = (int)in_message->get();
                break;

            }
          }

          //process binary
        }
        else {
          try {
            ptree pt;
            read_json(*in_message, pt);
            std::ostringstream buf; 
            write_json (buf, pt, false);
            std::cout << buf.str();
            int cnt = 0;
            


            

            try {
                next_sketch = pt.get<int>("skalt");
            } catch (const exception &e) {
            }
            
            try {
                spacebar = pt.get<int>("sknext");
            } catch (const exception &e) {
            }

            
            //clear canvas
            try {
                if (pt.get<int>("cc")) {
                for (int i = 0; i < NUM_LEDS; i++) {
                    temp_canvas[i] = CRGB::Black;
                }
                }
            } catch (const exception &e) {
            }

            
            
            } catch (const exception &e) {
            }
        }
      };

      echo.on_open = [](shared_ptr<WssServer::Connection> connection) {
        cout << "Server: Opened connection " << connection.get() << endl;
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

#endif

#endif