package server

import (
	"io/ioutil"
	"log"
	"os"
	"path"

	"gopkg.in/yaml.v2"
)

// Configuration - Server  Configuration
// Server	 - server  parameters
// Proxies	 - proxies parameters
type Configuration struct {
	Server  Arguments        `yaml:"server"`
	Proxies []ProxyArguments `yaml:"proxies"`
}

func parseYAMLConfiguration(raw []byte) *Configuration {
	config := Configuration{}

	err := yaml.Unmarshal(raw, &config)
	if err != nil {
		log.Fatalf("error: %v", err)
	}

	return &config
}

// ReadYAMLConfiguration - reads server configuration from yaml file
func ReadYAMLConfiguration(filepath string) *Configuration {
	p := path.Clean(filepath)
	if path.Ext(p) != ".yml" {
		log.Fatal("use YAML configuration file")
	}

	f, err := os.Open(path.Clean(filepath))
	if err != nil {
		log.Fatalf("err: %+v", err)
	}

	b, err := ioutil.ReadAll(f)
	if err != nil {
		log.Fatalf("err: %+v", err)
	}

	return parseYAMLConfiguration(b)
}
