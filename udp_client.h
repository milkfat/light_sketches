#ifndef LIGHTS_UDP_CLIENT_H
#define LIGHTS_UDP_CLIENT_H

#define NUM_UDP_CLIENTS 20

struct UDP_CLIENT {
    bool active = false;
    uint32_t address;
    uint16_t port;
    uint32_t last_seen;
    char name[16];

  };

class UDP_CLIENTS {

  UDP_CLIENT clients[NUM_UDP_CLIENTS];

  public:

    //adds a client to our list of known clients
  void add_client(uint32_t address, uint16_t port, const char * name, size_t size) {
      int available_client = -1;
      for (int i = 0; i < NUM_UDP_CLIENTS; i++) {
        if (clients[i].active && clients[i].address == address) {
          clients[i].last_seen = millis();
          return;
        }
        if (!clients[i].active) available_client = i;
      }
      if (available_client != -1) {
        clients[available_client].address = address;
        clients[available_client].port = port;
        clients[available_client].active = true;
        clients[available_client].last_seen = millis();
        for (int i = 0; i < size; i++) {
          clients[available_client].name[i] = name[i];
          clients[available_client].name[i+1] = 0;
        }
      }
    }
    
    UDP_CLIENT& operator[] (int index)
    {
        return clients[index];
    }
};

UDP_CLIENTS udp_clients;



#endif