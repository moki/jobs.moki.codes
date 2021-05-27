package server

import (
	"fmt"
	"log"
	"net/http/httputil"
	"net/url"
)

// ProxyArguments - proxy configuration
type ProxyArguments struct {
	Protocol string   `yaml:"protocol"`
	Host     string   `yaml:"host"`
	Port     uint16   `yaml:"port"`
	Path     string   `yaml:"path"`
	Routes   []string `yaml:"routes"`
}

// NewProxy - creates proxy instance
func NewProxy(configuration *ProxyArguments) *httputil.ReverseProxy {
	proto := configuration.Protocol
	host := configuration.Host
	port := configuration.Port
	path := configuration.Path

	purl := fmt.Sprintf("%v://%v:%v%v", proto, host, port, path)

	target, err := url.Parse(purl)
	if err != nil {
		log.Fatalf("err: %+v", err)
	}

	return httputil.NewSingleHostReverseProxy(target)
}
