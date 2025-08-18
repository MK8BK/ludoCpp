#include "controller.h"
#include "model.h"
#include <SDL3/SDL_events.h>

using namespace gamespace;

Controller::Controller() {}

bool Controller::startMainLoop() {
  bool done{false};
  SDL_Event event;
  while (!done) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT)
        done = true;
      else
        model.handleEvent(event);
    }
    model.render();
  }
  return true;
}

Controller::~Controller(){}