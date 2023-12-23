package main

import (
	"fmt"
	eventhandlers "lora-beacon/mqtt-multiplexer/event_handlers"
	"os"

	mqtt "github.com/eclipse/paho.mqtt.golang"
	tgbotapi "github.com/go-telegram-bot-api/telegram-bot-api/v5"
	"github.com/rs/zerolog"
	"github.com/rs/zerolog/log"
)

func init() {
	log.Logger = log.Output(zerolog.ConsoleWriter{Out: os.Stderr})
}

var (
	tgClient *tgbotapi.BotAPI
)

func main() {
	token, err := os.ReadFile(".tg-token")
	if err != nil {
		panic(err)
	}

	tgClient, err = tgbotapi.NewBotAPI(string(token))
	if err != nil {
		log.Fatal().Err(err).Msg("Failed to init TG")
	}

	log.Info().Msgf("Authorized on TG account %s", tgClient.Self.UserName)

	handlers := eventhandlers.EventHandler{TgClient: tgClient, KnownDevices: map[string]bool{}}

	var broker = "192.168.1.117"
	var port = 31883
	opts := mqtt.NewClientOptions()
	opts.AddBroker(fmt.Sprintf("tcp://%s:%d", broker, port))
	opts.SetClientID("go_mqtt_client")
	opts.OnConnect = handlers.OnConnectHandler
	opts.OnConnectionLost = handlers.ConnectionLostHandler
	opts.SetAutoReconnect(true)
	opts.SetResumeSubs(true)
	opts.SetCleanSession(false)

	client := mqtt.NewClient(opts)
	token := client.Connect()

	if token.Wait() && token.Error() != nil {
		panic(token.Error())
	}

	subscribe(client, eventhandlers.RawBeaconTopic, handlers.HandleRawEvent)
	subscribe(client, eventhandlers.ParsedTriggerEventsTopic, handlers.HandleTriggerEvent)
	subscribe(client, "homeassistant/binary_sensor/ABCD/state", handlers.HandleDoorStatusEvent)

	select {}
}

func subscribe(client mqtt.Client, topic string, handler mqtt.MessageHandler) {
	token := client.Subscribe(topic, 1, handler)
	token.Wait()
	fmt.Printf("Subscribed to topic %s\n", topic)
}
