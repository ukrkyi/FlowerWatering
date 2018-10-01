#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "token.h"

#define SSID "UCU-Guests"

WiFiClientSecure client;
UniversalTelegramBot bot(TOKEN, client);

bool tryConnect() {
  WiFi.begin(SSID);
  digitalWrite(5, LOW);
  do {
    delay(500);
    digitalWrite(5, !digitalRead(5));
  } while (WiFi.status() == WL_IDLE_STATUS);
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(5, LOW);
    return false;
  }
  digitalWrite(5, HIGH);
  return true;
}

#define HELP_MESSAGE  "\
Supported commands:\n\n\
/help - Get this help message"
#define ERROR_MESSAGE "Sorry, I don't understand your query."

bool parseMessage(String chat_id, String message)
{
  if (message.equals("/help")) {
    bot.sendMessage(chat_id, HELP_MESSAGE);
    return true;
  }
  return false;
}

void setup() {
  pinMode(5, OUTPUT);
  tryConnect();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    if (!tryConnect())
      return;
  }
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while(numNewMessages) {
    for (int i=0; i<numNewMessages; i++) {
      if (!parseMessage(bot.messages[i].chat_id, bot.messages[i].text)){
        bot.sendMessage(bot.messages[i].chat_id, ERROR_MESSAGE);
        bot.sendMessage(bot.messages[i].chat_id, HELP_MESSAGE);
      }
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}
