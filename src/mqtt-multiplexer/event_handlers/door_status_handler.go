package eventhandlers

import (
	mqtt "github.com/eclipse/paho.mqtt.golang"
	tgbotapi "github.com/go-telegram-bot-api/telegram-bot-api/v5"
	"github.com/rs/zerolog/log"
)

func (e *EventHandler) HandleDoorStatusEvent(client mqtt.Client, msg mqtt.Message) {
	log.Info().Msg("Sending telegram message")

	text := "✅ Door closed"
	if string(msg.Payload()) == "1" {
		text = "❗️ Door open"
	}

	tgMsg := tgbotapi.NewMessage(101520403, text)
	tgMsg.ParseMode = tgbotapi.ModeMarkdownV2

	e.TgClient.Send(tgMsg)
}
