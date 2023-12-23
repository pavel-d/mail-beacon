package packets

import (
	"bytes"
	"encoding/binary"
	"fmt"
)

const (
	BeaconTypeTrigger = 0x01

	PacketHeaderSize   = 4
	PacketMetadataSize = 2 + 4 // RSSI (int16) + SNR (float)
)

type BeaconPacket struct {
	BeaconID   uint16
	BeaconType uint8
	DataLength uint8
	Data       []byte
	RSSI       int16
	SNR        float32
}

func (p *BeaconPacket) BeaconIDString() string {
	return fmt.Sprintf("%X", p.BeaconID)
}

func (p *BeaconPacket) Unmarshal(data []byte) error {
	buf := bytes.NewReader(data)

	if err := binary.Read(buf, binary.LittleEndian, &p.BeaconID); err != nil {
		return err
	}

	if err := binary.Read(buf, binary.LittleEndian, &p.BeaconType); err != nil {
		return err
	}

	if err := binary.Read(buf, binary.LittleEndian, &p.DataLength); err != nil {
		return err
	}

	if int(PacketHeaderSize+p.DataLength+PacketMetadataSize) != len(data) {
		return fmt.Errorf("unexpected data length: %d, packets size: %d", p.DataLength, len(data))
	}

	p.Data = make([]byte, p.DataLength)

	if err := binary.Read(buf, binary.LittleEndian, &p.Data); err != nil {
		return err
	}

	if err := binary.Read(buf, binary.LittleEndian, &p.RSSI); err != nil {
		return err
	}

	if err := binary.Read(buf, binary.LittleEndian, &p.SNR); err != nil {
		return err
	}

	return nil
}
