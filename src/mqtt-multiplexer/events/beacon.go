package events

import (
	"time"
)

type BeaconEvent struct {
	ReceivedAT time.Time `json:"received_at"`
	BeaconID   string    `json:"beacon_id"`
	BeaconType uint8     `json:"beacon_type"`
	DataLength uint8     `json:"length"`
	Data       []byte    `json:"data"`
	RSSI       int16     `json:"rssi"`
	SNR        float32   `json:"snr"`
}
