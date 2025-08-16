#include "controller.h"
#include "model.h"
#include <SDL3/SDL_events.h>
#include <iostream>
// #include <chrono>

using namespace gamespace;
// using namespace std::literals;

Controller::Controller() {}

bool Controller::startMainLoop() {
  bool done{false};
  SDL_Event event;
  // std::chrono::time_point start = std::chrono::system_clock::now();
  while (!done) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT)
        done = true;
      else
        model.handleEvent(event);
    }
    // if (std::chrono::system_clock::now() - start > 5s)
    //   return true;
    model.render();
  }
  return true;
}

Controller::~Controller(){}