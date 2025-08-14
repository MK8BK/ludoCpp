#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "model.h"

namespace gamespace {

class Controller {
public:
  bool startMainLoop();

private:
  Game model;

public:
  Controller();
  ~Controller();
};

} // namespace gamespace

#endif
