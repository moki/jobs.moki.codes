package main

import (
	"log"
	"os"

	"github.com/moki/jobs.moki.codes/service/frontend/pkg/server"
)

const nargs int = 2

func readArguments() string {
	if len(os.Args) != nargs {
		return "config/development.yml"
	}

	return os.Args[1]
}

func main() {
	configFilepath := readArguments()

	config := server.ReadYAMLConfiguration(configFilepath)

	server := server.New(&config.Server)

	for i := range config.Proxies {
		server.AddProxy(&config.Proxies[i])
	}

	err := server.Listen()
	if err != nil {
		log.Fatalf("err: %+v", err)
	}
}
