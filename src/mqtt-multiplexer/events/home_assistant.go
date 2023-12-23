package events

type HomeAssistantAutoDiscovery struct {
	Name              string              `json:"name"`
	DeviceClass       string              `json:"device_class"`
	Icon              string              `json:"icon"`
	UnitOfMeasurement string              `json:"unit_of_measurement"`
	EntityCategory    string              `json:"entity_category"`
	StateTopic        string              `json:"state_topic"`
	UniqueID          string              `json:"unique_id"`
	PayloadOn         string              `json:"payload_on"`
	PayloadOff        string              `json:"payload_off"`
	Device            HomeAssistantDevice `json:"device"`
}
type HomeAssistantDevice struct {
	Identifiers []string `json:"identifiers"`
	Name        string   `json:"name"`
	Mdl         string   `json:"mdl"`
	Mf          string   `json:"mf"`
}
