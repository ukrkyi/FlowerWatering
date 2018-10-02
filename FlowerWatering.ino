#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "token.h"
#include <Ultrasonic.h>

#define SSID "UCU-Guests"

WiFiClientSecure client;
UniversalTelegramBot bot(TOKEN, client);
Ultrasonic us(4, 16);

bool tryConnect() {
  Serial.begin(9600);
  Serial.println("Trying to connect...");
  WiFi.begin(SSID);
  Serial.print("Waiting for network responce");
  digitalWrite(5, HIGH);
  while (WiFi.status() == WL_IDLE_STATUS ||
         WiFi.status() == WL_DISCONNECTED) { /* Workaround bc library returns WL_DISCONNECTED instead of WL_IDLE_STATUS */
    delay(100);
    digitalWrite(5, !digitalRead(5));
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(5, LOW);
    Serial.print("Connection error: ");
    switch (WiFi.status()) {
      case WL_NO_SHIELD:      Serial.println("No shield");          break;
      case WL_NO_SSID_AVAIL:  Serial.println("Network not found");  break;
      case WL_CONNECT_FAILED: Serial.println("Connection failed");  break;
      case WL_CONNECTION_LOST: Serial.println("Connection lost");    break;
      case WL_DISCONNECTED:   Serial.println("Disconnected");       break;
      default:                Serial.println("Unknown");            break;
    }
    return false;
  }
  digitalWrite(5, HIGH);
  Serial.println("Connected successfully");
  return true;
}

float getFill() {
  us.measure();
  float dist = us.get_cm();
  if (dist > 6)
    return 0;
  else
    return (6 - dist) / 6 * 100; 
}

#define HELP_MESSAGE  "\
Supported commands:\n\n\
/help - Get this help message\n\
/water - Get water reservoir fill percentage"
#define ERROR_MESSAGE "Sorry, I don't understand your query."

bool parseMessage(String chat_id, String message)
{
  if (message.equals("/help")) {
    bot.sendMessage(chat_id, HELP_MESSAGE);
    return true;
  } else if (message.equals("/water")) {
    bot.sendMessage(chat_id, String(getFill()));
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
  while (numNewMessages) {
    for (int i = 0; i < numNewMessages; i++) {
      if (!parseMessage(bot.messages[i].chat_id, bot.messages[i].text)) {
        bot.sendMessage(bot.messages[i].chat_id, ERROR_MESSAGE);
        bot.sendMessage(bot.messages[i].chat_id, HELP_MESSAGE);
      }
    }
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }
}
