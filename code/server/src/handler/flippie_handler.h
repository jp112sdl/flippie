#ifndef FLIPPIE_HANDLER_H
#define FLIPPIE_HANDLER_H
#include "../flippie.h"
#include "abstract_handler.h"
#include <ESP8266WebServer.h>

class FlippieHandler : public AbstractHandler {
public:
  FlippieHandler(Flippie* f);
  bool canHandle(HTTPMethod method, String uri) override;
  bool handle(ESP8266WebServer& server, HTTPMethod method, String uri) override;
protected:
  String _uri;
  Flippie* flippie;
};

#endif
