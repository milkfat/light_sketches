//getLocalTime function from ESP32 ... and Windows?
void getLocalTime(struct tm * &timeinfo) {
      time_t rawtime;
      time (&rawtime);
      timeinfo = localtime (&rawtime);
}

//emulate the Arduino millis() function
//returns the number of milliseconds from the start of execution
auto time_start = std::chrono::high_resolution_clock::now();

uint64_t millis() {
    auto time_now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = time_now - time_start;

    return elapsed.count()*1000; 
}

uint64_t micros() {
    auto time_now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = time_now - time_start;

    return elapsed.count()*1000000; 
}

auto& ps_malloc = malloc;

int random(int a, int b) {
  int d = b - a;
  if (d == 0) {
      return a;
  } else {
    return (rand() % d) + a;
  }
}

int random(int a) {
  return rand() % a;
}

auto sq = [](auto num) {
    return num*num;
};
