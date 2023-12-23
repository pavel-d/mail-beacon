package eventhandlers

import (
	"fmt"

	mqtt "github.com/eclipse/paho.mqtt.golang"
	tgbotapi "github.com/go-telegram-bot-api/telegram-bot-api/v5"
)

const (
	RawBeaconTopic           = "whisper_link/beacons/raw_events"
	ParsedTriggerEventsTopic = "whisper_link/beacons/parsed_events/trigger"

	HomeAssistantStateTopicFormat = "homeassistant/%s/%s/state"
	HomeAssistantDiscoveryTopic   = "homeassistant/%s/%s/config"
)

func stateTopic(deviceType, deviceName string) string {
	return fmt.Sprintf(HomeAssistantStateTopicFormat, deviceType, deviceName)
}

func autoDiscoveryTopic(deviceType, deviceName string) string {
	return fmt.Sprintf(HomeAssistantDiscoveryTopic, deviceType, deviceName)
}

type EventHandler struct {
	TgClient *tgbotapi.BotAPI

	KnownDevices map[string]bool
}

func (e *EventHandler) OnConnectHandler(client mqtt.Client) {
	fmt.Println("Connected")
}

func (e *EventHandler) ConnectionLostHandler(client mqtt.Client, err error) {
	fmt.Printf("Connect lost: %v\n", err)
}
