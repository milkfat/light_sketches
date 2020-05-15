#include "text.h"
#include "helper_functions.h"

char * generate_html_controls(char * inbuff) {
  char * p = inbuff;
  *p = '\0';
  for (int i = 0;i < control_variables.avail();i++) {
      if (control_variables.type(i) > 0 && control_variables.type(i) <= 6) {
        int v;
        int vmin;
        int vmax;
        const char * n;
        if (control_variables.read(i, v, vmin, vmax, n) ) {
            //range sliders
            mystrcat(p, n);
            mystrcat(p, ": <input type='range' min='");

            p+=sprintf(p, "%d", vmin);
            mystrcat(p, "' max='");
            p+=sprintf(p, "%d", vmax);
            mystrcat(p, "' value='");
            p+=sprintf(p, "%d", v);
            mystrcat(p, "' style='width:90%;margin: auto;display: block;' class='slider' id='");
            p+=sprintf(p, "%d", i);
            mystrcat(p, "' oninput='am(\"");
            p+=sprintf(p, "%d", i);
            mystrcat(p, "\",\"");
            p+=sprintf(p, "%d", i);
            mystrcat(p, "\")'>\n");
        }
      } else if (control_variables.type(i) == 7) {
        //color controls
        CRGB rgb;
        const char * n;
        if (control_variables.read(i, rgb, n) ) {
            mystrcat(p, n);
            mystrcat(p, ": <input type='color'");
            mystrcat(p, " value='#");
            char hex[7];
            snprintf(hex, 7, "%02X%02X%02X", rgb.r, rgb.g, rgb.b );
            mystrcat(p, hex);
            mystrcat(p, "' id='");
            p+=sprintf(p, "%d", i);
            mystrcat(p, "' oninput='am(\"");
            p+=sprintf(p, "%d", i);
            mystrcat(p, "\",\"");
            p+=sprintf(p, "%d", i);
            mystrcat(p, "\")'><br>\n");
        }
      } else if (control_variables.type(i) == 8) {
        //checkbox
        bool v;
        const char * n;

        if (control_variables.read(i, v, n)) {
            mystrcat(p, n);
            mystrcat(p, ": <input type='checkbox'");
            mystrcat(p, " id='");
            p+=sprintf(p, "%d", i);
            mystrcat(p, "' oninput='amBoolean(\"");
            p+=sprintf(p, "%d", i);
            mystrcat(p, "\",\"");
            p+=sprintf(p, "%d", i);
            mystrcat(p, "\")'");
            if (v) {
              mystrcat(p, " checked");
            }
            mystrcat(p, "><br>\n");
        }

      } else if (control_variables.type(i) == 9) {
        //momentary button
        bool v;
        const char * n;

        if (control_variables.read(i, v, n)) {
            mystrcat(p, "<button type=\"button\"");
            mystrcat(p, " id='");
            p+=sprintf(p, "%d", i);
            mystrcat(p, "' onmousedown='amInline(\"");
            p+=sprintf(p, "%d", i);
            mystrcat(p, "\",\"1\"); return false;'");
            mystrcat(p, " onmouseup='amInline(\"");
            p+=sprintf(p, "%d", i);
            mystrcat(p, "\",\"0\"); return false;'");
            mystrcat(p, "' ontouchstart='amInline(\"");
            p+=sprintf(p, "%d", i);
            mystrcat(p, "\",\"1\"); return false;'");
            mystrcat(p, " ontouchend='amInline(\"");
            p+=sprintf(p, "%d", i);
            mystrcat(p, "\",\"0\"); return false;'");
            mystrcat(p, ">");
            mystrcat(p, n);
            mystrcat(p, "</button><br>\n");
        }
      } else if (control_variables.type(i) == 10) {
        //string
        char * c;
        const char * n;

        if (control_variables.read(i, c, n)) {
            mystrcat(p, "<textarea cols=24 rows=2 id='");
            p+=sprintf(p, "%d", i);
            mystrcat(p, "' placeholder='type your message' oninput='am(\"");
            p+=sprintf(p, "%d", i);
            mystrcat(p, "\",\"");
            p+=sprintf(p, "%d", i);
            mystrcat(p, "\")'>");
            mystrcat(p, c);
            mystrcat(p, "</textarea><br>\n");
        }

      }
  }
  *p = '\0';
  return inbuff;
}


const char * light_html_head() {
  return (const char *)
#ifdef ARDUINO
  F(
#endif
      R""""(
       
       <html><head>
       <title>LIGHTS!</title>
       <meta name='viewport' content='width=device-width, initial-scale=1.0'>
       <style>
       body { -webkit-appearance: none;font-size:32px;}
       button,input[type='text'] { -webkit-appearance: none;font-size:32px;width:100%;}
       select { font-size:32px; }
       input[type='range'] { width:100%;transform: scaleY(2); }
       input[type='checkbox'] {transform: scale(2);}
       html { touch-action: manipulation;}
       </style>
       <script>
       function sf(sketch,flag) {
           var http = new XMLHttpRequest();
           http.open('POST', 'setsketch', true);
           http.setRequestHeader('Content-type','application/x-www-form-urlencoded');
           var params = 'sketch=' + sketch + '&flag=' + flag;
           http.send(params);
       }

       document.onkeydown = checkKeyDown;
       document.onkeyup = checkKeyUp;
       keyList = {};

       function checkKeyDown(e) {
         if(keyList[e.keyCode]) {
           return;
         } else {
           keyList[e.keyCode] = true;
         }
         checkKey(e, "k");
       }

       function checkKeyUp(e) {
         keyList[e.keyCode] = false;
         checkKey(e, "K");
       }
       
       function checkKey(e, name) {
           e = e || window.event;
           //up
           if (e.keyCode == '38') {
             amInline(name,"38")
           }
           //down
           else if (e.keyCode == '40') {
             amInline(name,"40")
           }
           //left
           else if (e.keyCode == '37') {
             amInline(name,"37")
           }
           //right
           else if (e.keyCode == '39') {
             amInline(name,"39")
           }
       }

       </script></head><body>
       <form id="myForm">
       <button type="button" onmousedown='amInline("e","e"); return false;'>NEXT EFFECT!</button><br />
       <button type="button" onmousedown='amInline("a","a"); return false;'>RESET SKETCH!</button><br />
       <button type="button" onmousedown='amInline("r","r"); return false;'>SYSTEM RESTART!</button><br />
       <select id="n" size="5" name="n" onchange='am("n","n"); return false;'>
      )""""

#ifdef ARDUINO
  )
#endif
      ;
}





const char * light_html_tail() {
  return (const char *)
#ifdef ARDUINO
  F(
#endif
      R""""(
       </p>
       </form>
           <p id = 'status'>Not connected</p>
           <p>Clients connected: <span id = 'clients'>
              </span></p>
           <ul id = 'log'></ul>
       
         </article>
       <script>
                      connected = document.getElementById("connected");
                      log = document.getElementById("log");
                      form = document.getElementById("myForm");
                      state = document.getElementById("status");
                      
                      if (window.WebSocket === undefined) {
                         state.innerHTML = "sockets not supported";
                         state.className = "fail";
                      }else {
                          if (typeof String.prototype.startsWith != "function") {
                             String.prototype.startsWith = function (str) {
                                return this.indexOf(str) == 0;
                             };
                          }
        
                        window.addEventListener("load", onLoad, false);
                     }
                      webshocketattempt = 0;
                      function onLoad() {
                         if (webshocketattempt > 1) {
                           return;
                         }
                         if (webshocketattempt == 0) {
                           websocket = new WebSocket(((window.location.protocol === "https:") ? "wss://" : "ws://") + window.location.hostname + "/socket");
                         } else {
                           websocket = new WebSocket(((window.location.protocol === "https:") ? "wss://" : "ws://") + window.location.hostname + ":8081/socket");
                         }
                         webshocketattempt++;
                         websocket.onopen = function(evt) { onOpen(evt) };
                         websocket.onclose = function(evt) { onClose(evt) };
                         websocket.onmessage = function(evt) { onMessage(evt) };
                         websocket.onerror = function(evt) { onError(evt) };
                         setInterval(wsSendTimer, 33);
                      }
        
                      function onOpen(evt) {
                         state.className = "success";
                         state.innerHTML = "Connected to server";
                      }
        
                      function onClose(evt) {
                         state.className = "fail";
                         state.innerHTML = "Not connected. Attempting to reconnect...";
                         //connected.innerHTML = "0";
                          // Try to reconnect in 5 seconds
                         websocket = null;
                         setTimeout(function(){onLoad()}, 500);
                      }
        
        
                      function onMessage(evt) {
                         var message = evt.data;
        
                         log.innerHTML = '<li class = "message">' + 
                        message + "</li>" + log.innerHTML;
                        var elem = document.querySelector('li:nth-child(1)');
                        if (elem) {
                         elem.parentNode.removeChild(elem);
                        }
                        try {
                           var json_stuff = JSON.parse(evt.data);
                           for (x in json_stuff) {
                             x2 = document.getElementById(x);
                             if (typeof json_stuff[x] == typeof true){
                                // variable is a boolean
                               x2.checked = json_stuff[x];
                             } else if (x == 'clients') {
                               x2.innerHTML = '';
                               for (y in json_stuff[x]) {
                               x2.innerHTML += '<br />' + json_stuff[x][y][0] + '.' + json_stuff[x][y][1] + '.' + json_stuff[x][y][2] + '.' + json_stuff[x][y][3] + ': ' + json_stuff[x][y][4];
                               }
                             } else {
                                // variable is not a boolean
                               x2.value = json_stuff[x];
                             }
                           }
                         }
                         catch {
                           document.getElementById('s').innerHTML = evt.data;
                         }
                     }
        
                      function onError(evt) {
                         websocket = null;
                         state.className = "fail";
                         state.innerHTML = "Communication error";
                      }

                      var message_buffer = {};
                      var ws_send_now = 1;
                      function wsSendTimer() {
                        ws_send_now = 1;
                        wsSend();
                      }
                      function wsSend() {
                        if(!ws_send_now) return;
                        var cnt = 0;
                        var comma = '';
                        var ws_message = '{';
                        for (jkey in message_buffer) {
                          ws_message += comma + '"' + jkey + '":"' + message_buffer[jkey] + '"';
                          cnt++;
                          if (cnt > 0) {
                            comma = ',';
                          }
                        }
                        if (cnt > 0) {
                          ws_message += '}';
                          websocket.send(ws_message);
                          message_buffer = {};
                          ws_send_now = 0;
                        }
                      }
        
                      function am(jkey,jid) {
                         message_buffer[jkey] = document.getElementById(jid).value;
                         wsSend();
                      }
        
                      function amInline(jkey,jval) {
                         message_buffer[jkey] = jval;
                         wsSend();
                      }
        
                      function amBoolean(jkey,jid) {
                         var val = document.getElementById(jid).checked+0;
                         message_buffer[jkey] = val;
                         wsSend();
                      }
                   </script>
        </body>
        </html>
      )""""     
#ifdef ARDUINO
  )
#endif
;
}



char * generate_sketch_dropdown(char * inbuff) {
    char * p = inbuff;
    *p = '\0';
    int temp_s = 0;
    const char* n;
    while ((n = light_sketches.names(temp_s))) {
      mystrcat(p,"<option value=\"");
      mystrcat(p,n);
      mystrcat(p,"\"");
      if (temp_s == light_sketches.current_sketch()) {
        mystrcat(p," selected");
      }
      mystrcat(p,">");
      mystrcat(p,n);
      mystrcat(p,"</option>\n");
      temp_s++;
    }
    mystrcat(p,"</select><br />\n");
    mystrcat(p,"<p id = 's'>\n");
    *p = '\0';
    return inbuff;
}




void parse_json(std::streambuf * inbuf) {

  // Get the input message
  char buff[128] = {'\0'};
  char * p = buff;
  while ( inbuf->sgetc() != EOF )
  {
    char c = inbuf->sbumpc();
    if (c == '"' && inbuf->sgetc() != EOF)
    //possible key, read it and look for value
    {
      c = inbuf->sbumpc();
      //next sketch
      if (c == 'n' && inbuf->sbumpc() == '"' && inbuf->sbumpc() == ':' && inbuf->sbumpc() == '"') {
        while ( inbuf->sgetc() != EOF && inbuf->sgetc() != '"') {
          c = inbuf->sbumpc();
          *p++ = c;
        }
        light_sketches.set_sketch(buff);
      }

      //next effect
      if (c == 'e' && inbuf->sbumpc() == '"' && inbuf->sbumpc() == ':' && inbuf->sbumpc() == '"') {
        light_sketches.next_effect();
        // std::string res;
        // for (int y=0; y<MATRIX_HEIGHT; y++) {
        //   for (int x = 0; x < MATRIX_WIDTH; x++) {
        //     res += String(x).c_str();
        //     res += ",";
        //     res += String(y).c_str();
        //     res += "=";
        //     res += String(XY(x,y)).c_str();
        //     res += "<br />";
        //   }
        // }
        // for(int i = 0; i < MAX_CLIENTS; i++) {
        //     if (activeClients2[i] != nullptr) {
        //       activeClients2[i]->send(res, SEND_TYPE_TEXT);
        //     }
        //   }
      }

      //reset sketch
      if (c == 'a' && inbuf->sbumpc() == '"' && inbuf->sbumpc() == ':' && inbuf->sbumpc() == '"') {
        light_sketches.reset();
      }

      //restart ESP32
      if (c == 'r' && inbuf->sbumpc() == '"' && inbuf->sbumpc() == ':' && inbuf->sbumpc() == '"') {
#ifdef ESP32
        ESP.restart();
#endif
      }

      if (c == 'k' && inbuf->sbumpc() == '"' && inbuf->sbumpc() == ':' && inbuf->sbumpc() == '"') {
        //read keydown value
        int num = 0;
        while ( inbuf->sgetc() != EOF && inbuf->sgetc() != '"') {
          c = inbuf->sbumpc();
          *p++ = c;
          num*=10;
          num+=c-48;
        }
        control_variables.key_down(num);
      }

      if (c == 'K' && inbuf->sbumpc() == '"' && inbuf->sbumpc() == ':' && inbuf->sbumpc() == '"') {
        //read keyup value
        int num = 0;
        while ( inbuf->sgetc() != EOF && inbuf->sgetc() != '"') {
          c = inbuf->sbumpc();
          *p++ = c;
          num*=10;
          num+=c-48;
        }
        control_variables.key_up(num);
      }

      //control variables
      if (c >= 48 && c <=57) {
        int cpos = c-48;
        c = inbuf->sgetc();
        if (c >= 48 && c <=57) {
          cpos*=10;
          cpos+=c-48;
          inbuf->sbumpc();
        }
        if (inbuf->sbumpc() == '"' && inbuf->sbumpc() == ':' && inbuf->sbumpc() == '"') {
          int type = control_variables.type(cpos);
          if (type > 0 && type <= 6) {
            //integer datatypes
            int num = 0;
            while ( inbuf->sgetc() != EOF && inbuf->sgetc() != '"') {
              c = inbuf->sbumpc();
              *p++ = c;
              num*=10;
              num+=c-48;
            }
            control_variables.set(cpos, num);
          } else if (type == 7 && inbuf->sbumpc() == '#') {
            //read hex value into RGB
            CRGB rgb;
            uint8_t* ptr = &rgb.r;
            int cnt = 0;
            //clear buffer
            p = buff;
            *p = '\0';
            //read two characters and convert hex to decimal
            while ( inbuf->sgetc() != EOF && inbuf->sgetc() != '"') {
              c = inbuf->sbumpc();
              *p++ = c;
              cnt++;
              if (cnt == 2) {
                *ptr = strtol(buff, 0, 16); //hex to decimal
                ptr++;
                cnt = 0;
                //clear buffer
                p = buff;
                *p = '\0';
              }
            }
            control_variables.set(cpos, rgb);
          } else if (type == 8 || type == 9) {
            //boolean datatypes
            int num = 0;
            while ( inbuf->sgetc() != EOF && inbuf->sgetc() != '"') {
              c = inbuf->sbumpc();
              *p++ = c;
              num*=10;
              num+=c-48;
            }
            bool boo = num;
            control_variables.set(cpos, boo);
          } else if (type == 10) {
            //string datatypes
            char string[50];
            char * c = string;
            while ( inbuf->sgetc() != EOF && inbuf->sgetc() != '"') {
              *c++ = inbuf->sbumpc();
            }
            *c = '\0';
            control_variables.set(cpos, string);
          }
        }
      }
    }
  }
  //Serial.println(msg.c_str());


  //strcpy((char*)canvas, msg.c_str());

}


static int parse_binary(std::streambuf * inbuf, char * out_message) {

          uint8_t p = 0;

          uint16_t offset = 0;

          //treat our message as binary
          char c;
          while (inbuf->sgetc() != EOF) {
            c = inbuf->sbumpc();        
            switch (c) {
              
              case 'l':
                {
                  //read cursor location
                  uint8_t offset = 0;
                  int16_t x;
                  int16_t y;
                  int16_t x1;
                  int16_t y1;
                  int16_t p;
                  int16_t id;

                  inbuf->sgetn((char*)&x, 2);
                  inbuf->sgetn((char*)&y, 2);
                  inbuf->sgetn((char*)&x1, 2);
                  inbuf->sgetn((char*)&y1, 2);
                  inbuf->sgetn((char*)&p, 2);
                  inbuf->sgetn((char*)&id, 2);
                  button2_down = true;

                  if (drawing_enabled && temp_canvas != nullptr) {
                      CRGB * old_screen_buffer = led_screen.screen_buffer;
                      led_screen.screen_buffer = temp_canvas;    
                      draw_line_fine(led_screen, (x+offset)*256, y*256, (x1+offset)*256, y1*256, 255, 255, p);
                      led_screen.screen_buffer = old_screen_buffer;    
                  }
              
                  update_pointer(x+offset,y,x1+offset,y1,p,id);
                }
                break;
              
              case 'e':
                //read cursor lifts (end)
                _remove_pointer( (int)inbuf->sbumpc() );
                button2_down = false;
                break;

              case 'a':
                //read orientation data from device (angles)
                inbuf->sgetn((char*)&led_screen.rotation_alpha, 4);
                inbuf->sgetn((char*)&led_screen.rotation_beta, 4);
                inbuf->sgetn((char*)&led_screen.rotation_gamma, 4);
                break;

              case 'd':
                drawing_enabled = (int)inbuf->sbumpc();
                break;

              case 'n':
					      spacebar=(int)inbuf->sbumpc();
                break;

              case 'v':
					      next_sketch=(int)inbuf->sbumpc();
                break;

              case 'z':
                p=0;
                while (inbuf->sgetc() != EOF && inbuf->sgetc() != '\0' && p < 20) {
                  next_sketch_name[p]=inbuf->sbumpc();
                  p++;
                }
                next_sketch_name[p] = '\0';
                break;

              case 'c': //send configuration
                {
                  strcpy(out_message, "{\"light_sketches\":[");
                  offset = 19;
                  for (int i = 0; i < MAX_NUMBER_OF_LIGHT_SKETCHES; i++) {
                    out_message[offset] = '"';
                    offset++;
                    if (light_sketches.names(i) != nullptr) {
                      for (int j = 0; j < 20; j++) {
                        if (light_sketches.names(i)[j]) {
                          out_message[offset] = light_sketches.names(i)[j];
                          offset++;
                        }
                      }
                    }
                    out_message[offset] = '"';
                    offset++;
                    out_message[offset] = ',';
                    offset++;
                  }
                  offset--;
                  out_message[offset] = ']';
                  offset++;
                  out_message[offset] = '}';
                  offset++;
                  out_message[offset] = '\0';
                }
                break;

            }
          }
          return offset;
    }