package eventhandlers

import (
	"bytes"
	"encoding/json"
	"fmt"
	"lora-beacon/mqtt-multiplexer/events"
	"lora-beacon/mqtt-multiplexer/packets"
	"os"
	"strconv"

	mqtt "github.com/eclipse/paho.mqtt.golang"
	"github.com/rs/zerolog/log"
)

func (e *EventHandler) HandleTriggerEvent(client mqtt.Client, msg mqtt.Message) {
	log.Info().Msg("Received trigger message")

	file, err := os.OpenFile("events.log", os.O_APPEND|os.O_CREATE|os.O_WRONLY, 0644)
	if err != nil {
		log.Error().Err(err).Msg("Error opening file")
		return
	}
	defer file.Close()

	// Write some data to the file.
	_, err = file.Write(msg.Payload())
	if err != nil {
		log.Error().Err(err).Msg("Error writing to file")
	}

	_, err = file.Write([]byte("\n"))
	if err != nil {
		log.Error().Err(err).Msg("Error writing to file")
	}

	var beaconEvent events.BeaconEvent
	err = json.Unmarshal(msg.Payload(), &beaconEvent)
	if err != nil {
		log.Error().Err(err).Msg("Failed to unmarshal beacon packet")
	}

	var triggerData packets.TriggerData
	err = json.Unmarshal(beaconEvent.Data, &triggerData)
	if err != nil {
		log.Error().Err(err).Msg("Failed to unmarshal trigger data")
	}

	publishTriggerState(client, beaconEvent, triggerData)

	if e.KnownDevices[beaconEvent.BeaconID] {
		return
	}

	err = publishAutoDiscovery(client, beaconEvent, triggerData)
	if err != nil {
		log.Warn().Err(err).Msg("Failed to publish auto-discovery event")
	}

	e.KnownDevices[beaconEvent.BeaconID] = true
}

func publishAutoDiscovery(client mqtt.Client, beaconEvent events.BeaconEvent, triggerData packets.TriggerData) error {
	device := events.HomeAssistantDevice{
		Identifiers: []string{"mailbox"},
		Name:        "Mailbox",
		Mdl:         "Beacon",
		Mf:          "WhisperLink",
	}

	evt := events.HomeAssistantAutoDiscovery{
		// Name:        "Mailbox",
		DeviceClass:    "door",
		Icon:           "mdi:mailbox",
		StateTopic:     stateTopic("binary_sensor", beaconEvent.BeaconID),
		UniqueID:       beaconEvent.BeaconID,
		PayloadOn:      "1",
		PayloadOff:     "0",
		Device:         device,
		EntityCategory: "config",
	}
	evtJson, err := json.Marshal(evt)
	if err != nil {
		return err
	}

	client.Publish(autoDiscoveryTopic("binary_sensor", beaconEvent.BeaconID), 1, true, evtJson)

	evt = events.HomeAssistantAutoDiscovery{
		Name:              "Mailbox",
		DeviceClass:       "signal_strength",
		Icon:              "mdi:signal",
		StateTopic:        stateTopic("sensor", fmt.Sprintf("%s/%s", beaconEvent.BeaconID, "rssi")),
		UniqueID:          fmt.Sprintf("%s_%s", beaconEvent.BeaconID, "rssi"),
		EntityCategory:    "diagnostic",
		UnitOfMeasurement: "dBm",
		Device:            device,
	}
	evtJson, err = json.Marshal(evt)
	if err != nil {
		return err
	}

	client.Publish(autoDiscoveryTopic("sensor", fmt.Sprintf("%s/%s", beaconEvent.BeaconID, "rssi")), 1, true, evtJson)

	evt = events.HomeAssistantAutoDiscovery{
		Name:              "Mailbox",
		DeviceClass:       "battery",
		Icon:              "mdi:battery",
		StateTopic:        stateTopic("sensor", fmt.Sprintf("%s/%s", beaconEvent.BeaconID, "batt")),
		UniqueID:          fmt.Sprintf("%s_%s", beaconEvent.BeaconID, "batt"),
		EntityCategory:    "diagnostic",
		UnitOfMeasurement: "mV",
		Device:            device,
	}
	evtJson, err = json.Marshal(evt)
	if err != nil {
		return err
	}

	client.Publish(autoDiscoveryTopic("sensor", fmt.Sprintf("%s/%s", beaconEvent.BeaconID, "batt")), 1, true, evtJson)

	return nil
}

func publishTriggerState(client mqtt.Client, beaconEvent events.BeaconEvent, triggerData packets.TriggerData) {
	val := "0"
	if triggerData.TriggerB {
		val = "1"
	}

	client.Publish(
		stateTopic("binary_sensor", beaconEvent.BeaconID),
		1,
		true,
		val,
	)

	client.Publish(
		stateTopic("sensor", fmt.Sprintf("%s/%s", beaconEvent.BeaconID, "rssi")),
		1,
		true,
		strconv.Itoa(int(beaconEvent.RSSI)),
	)

	client.Publish(
		stateTopic("sensor", fmt.Sprintf("%s/%s", beaconEvent.BeaconID, "batt")),
		1,
		false,
		strconv.Itoa(int(triggerData.Voltage)),
	)
}

func formatJSON(inputJSON []byte) (string, error) {
	var out bytes.Buffer
	err := json.Indent(&out, inputJSON, "", "\t")
	if err != nil {
		return "", err
	}
	return out.String(), nil
}
