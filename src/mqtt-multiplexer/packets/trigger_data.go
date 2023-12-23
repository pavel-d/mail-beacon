package packets

import (
	"bytes"
	"encoding/binary"
)

type TriggerData struct {
	TriggerA bool   `json:"trigger_a"`
	TriggerB bool   `json:"trigger_b"`
	TriggerC bool   `json:"trigger_c"`
	Voltage  uint16 `json:"voltage"`
	Sequence uint16 `json:"sequence"`
}

func (t *TriggerData) Unmarshal(data []byte) error {
	buf := bytes.NewReader(data)

	if err := binary.Read(buf, binary.LittleEndian, &t.TriggerA); err != nil {
		return err
	}

	if err := binary.Read(buf, binary.LittleEndian, &t.TriggerB); err != nil {
		return err
	}

	if err := binary.Read(buf, binary.LittleEndian, &t.TriggerC); err != nil {
		return err
	}

	if err := binary.Read(buf, binary.LittleEndian, &t.Voltage); err != nil {
		return err
	}

	if err := binary.Read(buf, binary.LittleEndian, &t.Sequence); err != nil {
		return err
	}

	return nil
}
