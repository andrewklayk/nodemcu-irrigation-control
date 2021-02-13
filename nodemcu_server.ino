#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "RTClib.h"
#include <Wire.h>

RTC_DS1307 rtc;
DateTime now;
//value of humidity sensor in air
#define AIR_VALUE 1024
//value of humidity sensor in water
#define WATER_VALUE 640

//when humidity percent falls lower than this value, start watering
#define CRITICAL_PERCENT 50
//stop watering when reaches this value
#define ENOUGH_PERCENT 80

struct PlantCell {
  // id of cell
  short id;
  // humidity sensor pin
  short hSensorPin;
  // water valve pin
  short waterValve;
  // start watering if below this
  short criticalPercent;
  // stop watering when reach this
  short enoughPercent;
  bool valveIsOpen;
  short humidityPercent;
  bool timeRegulated;

  PlantCell(short _id, short _hs, short _wv, short _tr, short crP, short enP) {
    id = _id;
    hSensorPin = _hs;
    waterValve = _wv;
    timeRegulated = _tr;
    valveIsOpen = false;
    criticalPercent = crP;
    enoughPercent = enP;
  }

  bool ReadHumidity() {
    short h_buff = humidityPercent;
    humidityPercent = map(analogRead(hSensorPin), AIR_VALUE, WATER_VALUE, 0, 100);
    return h_buff != humidityPercent;
  }
};


//const char* ssid = "TP-LINK_3E2D8C";  // SSID
//const char* password = "";  //пароль
const char* ssid = "Florencii 12";  // SSID
const char* password = "0504720119";  //пароль

short cellCount = 2;
PlantCell cells [] = {PlantCell(0, A0, D4, false, 70, 81), PlantCell(1, A0, D7, false, 70, 81)};


void setup() {
  static const char * index_html PROGMEM = R"rawliteral(<!DOCTYPE html>
    <html>
      <head>
         <script>
            function createHtmlCell(id, hPin, hVal, isOn, hCrit, wPin) {
                const cellElement = document.createElement('div');
                cellElement.classList.add("cell");
                cellElement.id = "cell" + id;

                const humidityPinRow = document.createElement('div');
                humidityPinRow.classList.add("cell-row");

                const humidityPinPropertyName = document.createElement('div');
                humidityPinPropertyName.classList.add("cell-property-name");
                humidityPinPropertyName.textContent = "Humidity pin:";

                const humidityPinPropertyValue = document.createElement('div');
                humidityPinPropertyValue.classList = "humidity-pin-value cell-property-value";
                humidityPinPropertyValue.textContent = hPin;

                const humidityRow = document.createElement('div');
                humidityRow.classList.add("cell-row");

                const humidityPropertyName = document.createElement('div');
                humidityPropertyName.classList.add("cell-property-name");
                humidityPropertyName.textContent = "Humidity:";

                const humidityPropertyValue = document.createElement('div');
                humidityPropertyValue.classList = "humidity-value cell-property-value";
                humidityPropertyValue.textContent = hVal + "%";

                const isActiveRow = document.createElement('div');
                isActiveRow.classList.add("cell-row");

                const isActivePropertyName = document.createElement('div');
                isActivePropertyName.classList.add("cell-property-name");
                isActivePropertyName.textContent = "Is active:";

                const isActivePropertyValue = document.createElement('div');
                isActivePropertyValue.classList = "is-active-value cell-property-value";
                isActivePropertyValue.textContent = isOn ? 'Yes' : 'No';

                const critHumidityRow = document.createElement('div');
                critHumidityRow.classList.add("cell-row");

                const critHumidityPropertyName = document.createElement('div');
                critHumidityPropertyName.classList.add("cell-property-name");
                critHumidityPropertyName.textContent = "Critical humidity:";

                const critHumidityPropertyValue = document.createElement('div');
                critHumidityPropertyValue.classList = "is-active-value cell-property-value";
                critHumidityPropertyValue.textContent = hCrit + "%";
                            
                humidityPinRow.appendChild(humidityPinPropertyName);
                humidityPinRow.appendChild(humidityPinPropertyValue);
                humidityRow.appendChild(humidityPropertyName);
                humidityRow.appendChild(humidityPropertyValue);
                isActiveRow.appendChild(isActivePropertyName);
                isActiveRow.appendChild(isActivePropertyValue);
                critHumidityRow.appendChild(critHumidityPropertyName);
                critHumidityRow.appendChild(critHumidityPropertyValue);
                    
                cellElement.appendChild(humidityPinRow);
                cellElement.appendChild(humidityRow);
                cellElement.appendChild(critHumidityRow);
                cellElement.appendChild(isActiveRow);

                return cellElement;
            }
            function postCell(cell) {
                const jsonBody = JSON.stringify(cell);
                console.log("Sending POST with: ");
                console.log(jsonBody);
                fetch("/cells", {
                    method: "POST",
                    body: jsonBody
                }).then(response => {
                    console.log("Response: ", response);
                });
            }
            function constructCells() {
                const newCell = {
                    id: 5,
                    hPin: 160,
                    wPin: 212,
                    tReg: false,
                    crP: 70,
                    enP: 81
                };
                postCell(newCell);
                const list = document.querySelector('#cells');
                var cells = [];
                fetch("/cells").then(function(response) {
                    response.text().then(function(text) {
                        json = JSON.parse(text);
                        cells = json.data;
                        cells.forEach(cell => {
                            var currentCell = document.getElementById("cell" + cell.id);
/*                            
                            currentCell.childNode.forEach(function(row) {
                                row.childNodes.forEach(function(secondChildItem) {
                                    if(secondChildItem.className == "humidity-value cell-property-value") {
                                        secondChildItem.textContent = "5";
                                    }
                                })
                            }

                            if(currentCell == null)
*/
                            cellElement = createHtmlCell(cell.id, cell.hPin, cell.hVal, cell.on, cell.hCrit, cell.wPin);
                            if(currentCell == null)
                            {
                                list.appendChild(cellElement);
                            }
                            else
                            {
                                list.replaceChild(cellElement, currentCell);
                            }
                        });
                    });
                });
            }
            
            setInterval(constructCells, 5000);
            
            function updateValues() {
                var cells = document.getElementById("cells").childNodes;
                cells.forEach(function (item) {
                    item.childNodes.forEach(function(childItem) {
                        childItem.childNodes.forEach(function(secondChildItem) {
                            if(secondChildItem.className == "humidity-value cell-property-value") {
                                secondChildItem.textContent = "5";
                            }
                        })
                    })
                    console.log(item.childNodes)
                  });
              }
        </script>
          <style>
            body {
                background-image: url(https://images.adsttc.com/media/images/5f39/ab2b/b357/65d2/c900/0158/large_jpg/rockburger.jpg?1597614883);
                background-color: #b5ccff;
                margin-top: 0px;
                margin-left: 0px;
                margin-right: 0px;
            }
            header {
                background: linear-gradient(rgba(203, 245, 255, 0.9), rgba(0, 204, 255, 0.9));
                text-align: center;
                font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
                font-size: large;
                width: 100%;
                height: auto;
                left: 0px;
                top: 0px;
            }
            .content {
                margin-left: 5px;
                margin-right: 5px;
                font-size: larger;
            }
            .cell-container {
                display: flex;
                flex-direction:row;
                flex-wrap: wrap;
                justify-content:space-evenly;
            }
            .cell {
                background: linear-gradient(rgba(183, 241, 255, 0.8), rgba(0,204,255, 0.8));
                border-radius: 1px;
                margin-top: 5px;
                display: flex;
                flex-direction: column;
                width:27%;
            }
            .cell-row {
                display: flex;
            }
            .cell-property-name{
                margin-right: 5px;
                margin-left: 5px;
            }
            .cell-property-value {
                margin-right: 5px;
                margin-left: 5px;
            }
          </style>
          <title>Control Panel</title>
      </head>
      <body onLoad="constructCells()">
          <header>
              <h2 style="height: fit-content; margin-top: 0px;"> ZPA ANDRIJIVNA</h2>
          </header>
          <div class="content">
              <div id="cells" class="cell-container"></div>
          </div>
      </body>
    </html>)rawliteral";

  Serial.begin(115200);

  delay(100);

  static AsyncWebServer server(80);

  Serial.println("Connecting to ");
  Serial.println(ssid);

  // подключаемся к локальной Wi-Fi сети
  WiFi.begin(ssid, password);

  // проверка подключения Wi-Fi к сети
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");
  Serial.println(WiFi.localIP());

  // Initialize all to HIGH (0)
  for (short i = 0; i < cellCount ; i++)
  {
    pinMode(cells[i].waterValve, OUTPUT);
    digitalWrite(cells[i].waterValve, HIGH);
  }

  // on loading the index page, send the HTML
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });

  // on request, send state updates
  server.on("/cells", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", createUpdatesJson(cellCount, cells));
  });

  // add cells
  /*server.on("/cells", HTTP_POST, [](AsyncWebServerRequest * request) {
    StaticJsonDocument<256> jsonBuffer;
    Serial.print("Got POST request with size: ");
    deserializeJson(jsonBuffer, request->getParam("body", true)->value());
    Serial.println(jsonBuffer.size());
  });*/
  server.on(
    "/cells",
    HTTP_POST,
    [](AsyncWebServerRequest * request){},
    NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
      String res = String((char *)data);
      PlantCell newCell = createCellFromString(res);
      Serial.println(res);

      request->send(200);
  });
  server.begin();

  Wire.begin(D2, D3);
  rtc.begin();

  Serial.println("HTTP server started");
}

void loop() {
  static long currentMillis;
  static int interval = 5000;
  //if *interval* milliseconds have passed (instead of delay):
  if (millis() - currentMillis >= interval)
  {
    now = rtc.now();
    /*Serial.print(now.hour());
      Serial.print(":");
      Serial.print(now.minute());
      Serial.print(":");
      Serial.println(now.second());*/
    //read humidity values
    for (short i = 0; i < cellCount; i++) {
      if (cells[i].ReadHumidity()) {
        printHumidity(i, cells[i].humidityPercent);
      }
    }
    for (short i = 0; i < cellCount; i++)
    {
      //if the cell isnt time regulated OR it's watering time
      if (cells[i].timeRegulated == false || (cells[i].timeRegulated /*&& TimeIsBetween(mHourOn, mMinuteOn, rtc.Hours, rtc.minutes, mHourOff, mMinuteOff) || TimeIsBetween(eHourOn, eMinuteOn, rtc.Hours, rtc.minutes, eHourOff, eMinuteOff)*/))
      {
        //if soil moisture too low and valve is closed, open valve
        if (!cells[i].valveIsOpen && cells[i].humidityPercent <= cells[i].criticalPercent) {
          // INVERSIJA
          Serial.print("Opened ");
          Serial.print(i);
          Serial.print(" with humidity ");
          Serial.println(cells[i].humidityPercent);
          digitalWrite(cells[i].waterValve, LOW);
          cells[i].valveIsOpen = true;
        }
      }
      //if valve open and moisture is high enough, close valve
      if (cells[i].valveIsOpen && (cells[i].humidityPercent >= cells[i].enoughPercent)) {
        // INVERSIJA
        Serial.print("Closed ");
        Serial.print(i);
        Serial.print(" with humidity ");
        Serial.println(cells[i].humidityPercent);
        digitalWrite(cells[i].waterValve, HIGH);
        cells[i].valveIsOpen = false;
      }
    }
    currentMillis = millis();
  }
}

bool TimeIsBetween(int &aHour, int &aMinute, int &bhour, int &bminute, int &chour, int &cminute) {
  //TEST
  return true;
  //return TimeIsEarlier(aHour, aMinute, bhour, bminute) && TimeIsEarlier(bhour, bminute, chour, cminute);
}

bool TimeIsEarlier(int &aHour, int &aMinute, int &bhour, int &bminute) {
  return (aHour < bhour || (bhour == bhour && aMinute < bminute));
}

void printHumidity(short &i, short &value) {
  Serial.print(i);
  Serial.print(": ");
  Serial.print(value);
  Serial.println("%");
  /*int pos1 = 0;
    int pos2 = 0;
    switch (i) {
    case 0:
      pos1 = pos2 = 0;
      break;
    case 1:
      pos1 = 6;
      pos2 = 0;
      break;
    case 2:
      pos1 = 0;
      pos2 = 1;
      break;
    case 3:
      pos1 = 6;
      pos2 = 1;
      break;
    }
    //lcd.setCursor(pos1, pos2);
    //lcd.print(i + 1);
    //lcd.print(":");
    //lcd.print(value);
    //lcd.print("% ");*/
}

PlantCell createCellFromString(String& json) {
  StaticJsonDocument<200> doc;
  deserializeJson(doc, json);
  return PlantCell(doc["id"], doc["hPin"], doc["wPin"], doc["tReg"], doc["crP"], doc["enP"]);
}

char * createUpdatesJson(short &cellCount, PlantCell* cells) {
  DynamicJsonDocument doc(2048);
  JsonArray data = doc.createNestedArray("data");
  for (short i = 0; i < cellCount; i++)
  {
    JsonObject cell = data.createNestedObject();
    cell["id"] = cells[i].id;
    cell["hCrit"] = cells[i].criticalPercent;
    cell["hEn"] = cells[i].enoughPercent;
    cell["hPin"] = cells[i].hSensorPin;
    cell["hVal"] = cells[i].humidityPercent;
    cell["wPin"] = cells[i].waterValve;
    cell["on"] = cells[i].valveIsOpen;
  }
  char res[2048];
  serializeJson(doc, res);
  return res;
}
