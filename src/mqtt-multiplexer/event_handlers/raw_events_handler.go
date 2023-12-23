package eventhandlers

import (
	"encoding/json"
	"fmt"
	"lora-beacon/mqtt-multiplexer/events"
	"lora-beacon/mqtt-multiplexer/packets"
	"time"

	mqtt "github.com/eclipse/paho.mqtt.golang"
	"github.com/rs/zerolog/log"
)

func (e *EventHandler) HandleRawEvent(client mqtt.Client, msg mqtt.Message) {
	raw := msg.Payload()
	// fmt.Printf("%s", hex.Dump(raw))

	p := &packets.BeaconPacket{}
	err := p.Unmarshal(raw)
	if err != nil {
		fmt.Printf("Failed to unmarshal packet: %w\n", err)
		return
	}

	log.Info().Str("beacon_id", p.BeaconIDString()).
		Str("beacon_type", fmt.Sprintf("%X", p.BeaconType)).
		Int("len", int(p.DataLength)).Msg("Received beacon message")

	switch p.BeaconType {
	case packets.BeaconTypeTrigger:
		triggerData := &packets.TriggerData{}
		err := triggerData.Unmarshal(p.Data)
		if err != nil {
			log.Error().Err(err).Msg("Failed to unmarshal trigger data")
			return
		}

		dataJson, err := json.Marshal(triggerData)
		if err != nil {
			log.Error().Err(err).Msg("Failed to marshal trigger data")
			return
		}

		eventData := &events.BeaconEvent{
			ReceivedAT: time.Now(),
			BeaconID:   p.BeaconIDString(),
			BeaconType: p.BeaconType,
			DataLength: p.DataLength,
			Data:       dataJson,
			RSSI:       p.RSSI,
			SNR:        p.SNR,
		}

		log.Debug().Interface("data", eventData).Msg("Debug")

		payload, err := json.Marshal(eventData)
		if err != nil {
			log.Error().Err(err).Msg("Failed to marshal event data")
			return
		}

		client.Publish(ParsedTriggerEventsTopic, 1, false, payload)
	default:
		log.Warn().Str("beacon_type", fmt.Sprintf("%X", p.BeaconType)).Msg("Unexpected beacon type")
	}
}
