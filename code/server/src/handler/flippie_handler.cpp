#include "flippie_handler.h"

FlippieHandler::FlippieHandler(Flippie *f) { flippie = f; }

bool FlippieHandler::handle(ESP8266WebServer &server, HTTPMethod method,
                            String uri) {
  if (!canHandle(method, uri)) {
    return false;
  }

  debug_printf("Handle %s '%s?%s' in FlippieHandler...",
               method == HTTP_GET ? "GET" : "POST", uri.c_str(),
               argsToString(server).c_str());

  if (method == HTTP_GET) {
    char *tmp = new char[16];
    if (server.hasArg("shiftregister")) {
      server.send(200, "application/json",
                  flippie->shift_register_as_json_short_string());
    } else if (server.hasArg("json")) {
      server.send(200, "application/json", flippie->shift_register_as_json());
    } else if (server.hasArg("led_A")) {
      server.send(200, "application/json", flippie->led_A_on ? "1" : "0");
    } else if (server.hasArg("led_B")) {
      server.send(200, "application/json", flippie->led_B_on ? "1" : "0");
    } else if (server.hasArg("led_C")) {
      server.send(200, "application/json", flippie->led_C_on ? "1" : "0");
    } else if (server.hasArg("address")) {
      sprintf(tmp, "%u\0", flippie->get_address());
      server.send(200, "application/json", String(tmp));
    } else if (server.hasArg("column")) {
      sprintf(tmp, "%u\0", flippie->get_column());
      server.send(200, "application/json", String(tmp));
    } else if (server.hasArg("d")) {
      server.send(200, "application/json", flippie->get_d() ? "true" : "false");
    } else if (server.hasArg("row_set")) {
      sprintf(tmp, "%u\0", flippie->get_row_set());
      server.send(200, "application/json", String(tmp));
    } else if (server.hasArg("row_rst")) {
      sprintf(tmp, "%u\0", flippie->get_row_rst());
      server.send(200, "application/json", String(tmp));
    } else {
      server.send(405, "application/json", "\"Unknown get command\"");
    }
    free(tmp);
  } else if (method == HTTP_POST) {
    bool json_verb = true;
    if (server.hasArg("task")) {
      if (server.arg("task").equals("clear")) {
        flippie->clear();
      } else if (server.arg("task").equals("fill")) {
        flippie->fill();
      } else if (server.arg("task").equals("inverse")) {
        flippie->inverse();
      } else if (server.arg("task").equals("magnetize")) {
        flippie->magnetize(
            server.hasArg("repeats") ? server.arg("repeats").toInt() : 1);
      } else if (server.arg("task").equals("clear-shiftregister")) {
        flippie->clear_shift_register(true);
      } else if (server.arg("task").equals("fill-shiftregister")) {
        flippie->fill_shift_register(true);
      } else {
        Serial.printf("Unknown task '%s'\n", server.arg("task").c_str());
        server.send(405, "application/json", "\"Unknown task\"");
      }
    } else {
      unsigned char x = 0;
      if (server.hasArg("led_mode")) {
        flippie->config->led_mode =
            server.arg("led_mode").charAt(0) == '1' ? 1 : 0;
        goto send_message;
      }
      if (server.hasArg("verbose")) {
        flippie->config->verbose = server.arg("verbose").charAt(0) == '1';
        goto send_message;
      }
      if (server.hasArg("led_A")) {
        flippie->led_A_on = server.arg("led_A").charAt(0) == '1' ? true : false;
      }
      if (server.hasArg("led_B")) {
        flippie->led_B_on = server.arg("led_B").charAt(0) == '1' ? true : false;
      }
      if (server.hasArg("led_C")) {
        if (flippie->config->led_mode == LED_MODE_NONE) {
          flippie->led_C_on =
              server.arg("led_C").charAt(0) == '1' ? true : false;
        }
      }
      if (server.hasArg("clear") && server.arg("clear").charAt(0) == '1') {
        flippie->clear_shift_register(false);
        goto send_message;
      }
      if (server.hasArg("address")) {
        x = server.arg("address").toInt();
        if (x >= 0 && x <= BROSE_MAX_ADDR) {
          flippie->set_address(x);
        }
      }
      if (server.hasArg("column")) {
        x = server.arg("column").toInt();
        if (x >= 0 && x <= FP2800A_MAX_COLUMNS) {
          flippie->set_column(x);
        }
      }
      if (server.hasArg("d")) {
        flippie->set_d(server.arg("d").charAt(0) == '1' ? 1 : 0);
      }
      if (server.hasArg("row_set")) {
        x = server.arg("row_set").toInt();
        if (x >= 0 && x <= BROSE_MAX_ROWS) {
          flippie->set_row_set(x);
        }
      }
      if (server.hasArg("row_rst")) {
        x = server.arg("row_rst").toInt();
        if (x >= 0 && x <= BROSE_MAX_ROWS) {
          flippie->set_row_rst(x);
        }
      }
      flippie->fire_shift_register(
          server.hasArg("enable") && server.arg("enable").charAt(0) == '1',
          server.hasArg("persistant") &&
              server.arg("persistant").charAt(0) == '1');
    }
  send_message:
    server.send(200, "application/json", json_verb ? "true" : "false");
  }
  if (flippie->config->verbose)
    Serial.println("DONE.");
  return true;
}

bool FlippieHandler::canHandle(HTTPMethod method, String uri) {
  if (uri == "/flippie" && (method == HTTP_GET || method == HTTP_POST)) {
    return true;
  }
  return false;
}
