package main

import (
	"bufio"
	"fmt"
	"log"
	"os"
	"strings"
	"time"

	tgbotapi "github.com/go-telegram-bot-api/telegram-bot-api/v5"
	"github.com/tarm/serial"
)

func main() {
	token, err := os.ReadFile(".tg-token")
	if err != nil {
		panic(err)
	}
	for {
		bot, err := tgbotapi.NewBotAPI(string(token))
		if err != nil {
			fmt.Println(err)
			continue
		}

		bot.Debug = true

		log.Printf("Authorized on account %s", bot.Self.UserName)

		u := tgbotapi.NewUpdate(0)
		u.Timeout = 60

		err = listen(bot)
		if err != nil {
			fmt.Println(err)
		}

		time.Sleep(10 * time.Second)
	}
}

func listen(bot *tgbotapi.BotAPI) error {
	config := &serial.Config{
		Name:        "/dev/cu.usbserial-110",
		Baud:        9600,
		ReadTimeout: 1,
		Size:        8,
	}

	stream, err := serial.OpenPort(config)
	if err != nil {
		return err
	}

	for {
		scanner := bufio.NewScanner(stream)
		for scanner.Scan() {
			raw := scanner.Text()
			fmt.Println(raw)

			if raw == "Ready" || raw == "" {
				continue
			}

			parsed, err := parseMessage(raw)
			if err != nil {
				bot.Send(tgbotapi.NewMessage(101520403, err.Error()))
				continue
			}

			m := fmt.Sprintf("Beacon ID: %s, Counter: %s, Voltage: %s, RSSI: %s, SNR: %s", parsed[0], parsed[1], parsed[2], parsed[3], parsed[4])

			bot.Send(tgbotapi.NewMessage(101520403, m))

		}
		if err := scanner.Err(); err != nil {
			return err
		}
	}
}

func parseMessage(s string) ([]string, error) {
	parts := strings.Split(s, ",")
	if len(parts) != 5 {
		return nil, fmt.Errorf("failed to parse message: %s", s)
	}

	return parts, nil
}
