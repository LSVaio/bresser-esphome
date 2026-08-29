# Bresser El Nino 7-in-1 mit ESPHome

Diese externe ESPHome-Komponente empfaengt die 868-MHz-Funktelegramme einer
Bresser 7-in-1 El Nino direkt mit dem SX1276 eines LILYGO TTGO LoRa32. Die
Messwerte werden ueber die native ESPHome-API in Home Assistant bereitgestellt;
MQTT ist nicht erforderlich.

Getestete Hardware:

- LILYGO TTGO LoRa32 V1.2, klassischer ESP32, SX1276, 4 MB Flash
- Bresser 7-in-1 El Nino auf 868,3 MHz
- ESPHome 2026.8.0

Bereitgestellt werden Temperatur, Luftfeuchtigkeit, Windgeschwindigkeit, Boe,
Windrichtung, Regenzaehler, UV-Index, Beleuchtungsstaerke, Batteriealarm,
Sensor-ID und Funk-RSSI.

## Als GitHub-Library verwenden

```yaml
external_components:
  - source: github://charlyschulte/bresser-esphome@v1.1.0
    components: [bresser_weather]
    refresh: never

bresser_weather:
  pins:
    sck: 5
    miso: 19
    mosi: 27
    cs: 18
    reset: 14
    irq: 26
    dio1: 33
  log_sensor_id: true

  # Optional, nachdem die gewuenschte ID im Log gefunden wurde:
  # filter_sensor_id: 0x00003717

  temperature:
    name: Aussentemperatur
  humidity:
    name: Luftfeuchtigkeit
```

Die vollstaendige Beispielkonfiguration mit allen Entitaeten befindet sich in
`bresser-el-nino.yaml`.

## Sensor-ID zuerst anlernen und spaeter filtern

`filter_sensor_id` ist optional und in der Beispielkonfiguration zunaechst
auskommentiert. Mit `log_sensor_id: true` meldet das Log bei jedem gueltigen
Telegramm beispielsweise:

```text
Empfangene Sensor-ID: 00003717
```

Das USB-Log kann so gelesen werden:

```bash
esphome logs bresser-el-nino.yaml --device /dev/cu.usbserial-0202AE07
```

Wenn die richtige ID bekannt ist, wird sie unter `bresser_weather:` aktiviert:

```yaml
  filter_sensor_id: 0x00003717
```

Danach kann `log_sensor_id` auf `false` gesetzt werden. Die diagnostische
Home-Assistant-Entitaet `Sensor-ID` bleibt davon unberuehrt.

## Funk-Pinbelegung

Alle relevanten SX1276-Pins stehen sichtbar in der YAML. Fuer das getestete
TTGO LoRa32 V1.x/V1.2 gelten:

| Funktion | GPIO |
| --- | ---: |
| SCK | 5 |
| MISO | 19 |
| MOSI | 27 |
| CS/NSS | 18 |
| Reset | 14 |
| IRQ/DIO0 | 26 |
| DIO1 | 33 |

Die USB-Erkennung kann alte LoRa32-Revisionen nicht unterscheiden. Bei einer
anderen Revision muessen der Boardwert und die Funk-Pins in der YAML angepasst
werden. Vor dem Empfang muss eine passende 868-MHz-Antenne angeschlossen sein.

## Bauen und per USB installieren

Zuerst `secrets.yaml.example` als `secrets.yaml` kopieren und sichere Werte
eintragen. Anschliessend:

```bash
python3 -m venv .venv
.venv/bin/pip install "esphome==2026.8.0"
.venv/bin/esphome run bresser-el-nino.yaml \
  --device /dev/cu.usbserial-0202AE07
```

Nach dem ersten Start erscheint nach etwa 15 Sekunden das WLAN
`Bresser Wetter Setup`. Das Passwort steht als `fallback_ap_password` in
`secrets.yaml`. Mit diesem WLAN verbinden, `http://192.168.4.1/` aufrufen und
das eigene 2,4-GHz-WLAN auswaehlen. Alternativ kann WLAN per Improv ueber USB
eingerichtet werden.

Home Assistant findet das Geraet danach normalerweise automatisch unter
**Einstellungen -> Geraete & Dienste**. Falls nicht, dort **Integration
hinzufuegen -> ESPHome** waehlen und den Hostnamen oder die IP-Adresse eingeben.
Den angeforderten Schluessel liefert `api_encryption_key` aus `secrets.yaml`.

## In das ESPHome-Add-on uebernehmen

`bresser-el-nino.yaml` nach `/config/esphome/` kopieren und den lokalen
`external_components`-Block durch den oben gezeigten GitHub-Block ersetzen.
Ausserdem die drei Werte aus der lokalen `secrets.yaml` in
`/config/esphome/secrets.yaml` uebernehmen. Danach kann das Geraet drahtlos per
OTA aktualisiert werden.

## Quellen und Lizenzen

Die Komponente verwendet
[BresserWeatherSensorReceiver 0.41.1](https://github.com/matthias-bs/BresserWeatherSensorReceiver)
und dessen feste
[RadioLib-Abhaengigkeit 7.7.1](https://github.com/jgromes/RadioLib). Die
Versionen sind fuer reproduzierbare Builds festgeschrieben. Der vendorte
Bresser-Quellcode steht unter seiner mitgelieferten MIT-Lizenz; diese
ESPHome-Komponente ebenfalls unter der MIT-Lizenz.
