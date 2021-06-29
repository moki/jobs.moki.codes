package server

import (
	"fmt"
	"log"
	"net/http"
	"path"

	"github.com/evanw/esbuild/pkg/api"
)

const ssemsg string = "event: reload\ndata: succ\n\n"

const webRootPath string = "/"

// Arguments - server configuration
type Arguments struct {
	Host        string `yaml:"host"`
	Port        uint16 `yaml:"port"`
	SourceDir   string `yaml:"source_dir"`
	BuildDir    string `yaml:"build_dir"`
	JSEntry     string `yaml:"js_entry"`
	JSTarget    string `yaml:"js_target"`
	CSSEntry    string `yaml:"css_entry"`
	HTMLEntry   string `yaml:"html_entry"`
	HTMLTarget  string `yaml:"html_target"`
	JSXFactory  string `yaml:"jsx_factory"`
	JSXFragment string `yaml:"jsx_fragment"`
}

// DevServer - development server
type DevServer struct {
	HTTPServer    *http.Server
	notifications chan struct{}
	handleRebuild func(api.BuildResult)
	configuration *Arguments
}

// New - creates new server instance
func New(configuration *Arguments) *DevServer {
	host := configuration.Host
	port := configuration.Port

	mux := http.NewServeMux()

	server := &DevServer{
		HTTPServer: &http.Server{
			Addr:    fmt.Sprintf("%v:%v", host, port),
			Handler: mux,
		},
		configuration: configuration,
	}

	return server
}

// AddProxy - adds proxy target for the server
func (s *DevServer) AddProxy(configuration *ProxyArguments) {
	mux := s.HTTPServer.Handler.(*http.ServeMux)

	proxy := NewProxy(configuration)

	for _, route := range configuration.Routes {
		mux.HandleFunc(route, proxy.ServeHTTP)
	}
}

func handleRebuildFactory(notifications *chan struct{}) func(api.BuildResult) {
	return func(result api.BuildResult) {
		nerr := len(result.Errors)
		if nerr == 0 {
			goto warnings
		}

		fmt.Printf("errors: %d\n", len(result.Errors))
		for _, m := range result.Errors {
			file := m.Location.File
			line := m.Location.Line
			msg := m.Text

			fmt.Printf("%+v:%+v %+v\n", file, line, msg)
		}

	warnings:

		nwar := len(result.Warnings)
		if nwar == 0 {
			goto notify
		}

		fmt.Printf("warnings: %d\n", len(result.Warnings))
		for _, m := range result.Warnings {
			file := m.Location.File
			line := m.Location.Line
			msg := m.Text

			fmt.Printf("%+v:%+v %+v\n", file, line, msg)
		}

	notify:
		*notifications <- struct{}{}
	}
}

func (s *DevServer) configureHotReload() {
	mux := s.HTTPServer.Handler.(*http.ServeMux)
	s.notifications = make(chan struct{})

	s.handleRebuild = handleRebuildFactory(&s.notifications)

	htmlResPath := path.Join(s.configuration.SourceDir, s.configuration.HTMLEntry)

	mux.HandleFunc(webRootPath, func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, htmlResPath)
	})

	jsReqPath := path.Join(webRootPath, s.configuration.JSTarget)

	jsResPath := path.Join(s.configuration.BuildDir, s.configuration.JSTarget)

	mux.HandleFunc(jsReqPath, func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, jsResPath)
	})

	cssReqPath := path.Join(webRootPath, s.configuration.CSSEntry)

	cssResPath := path.Join(s.configuration.BuildDir, s.configuration.CSSEntry)

	mux.HandleFunc(cssReqPath, func(w http.ResponseWriter, r *http.Request) {
		http.ServeFile(w, r, cssResPath)
	})

	jsSourcePath := path.Join(s.configuration.SourceDir, s.configuration.JSEntry)

	api.Build(api.BuildOptions{
		Bundle:      true,
		EntryPoints: []string{jsSourcePath},
		Outfile:     jsResPath,
		JSXFactory:  s.configuration.JSXFactory,
		JSXFragment: s.configuration.JSXFragment,
		Watch: &api.WatchMode{
			OnRebuild: s.handleRebuild,
		},
		Write:  true,
		Loader: map[string]api.Loader{".ttf": api.LoaderDataURL},
	})

	mux.HandleFunc("/events", func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Content-Type", "text/event-stream")
		w.Header().Set("Cache-Control", "no-cache")
		w.Header().Set("Connection", "keep-alive")

		for {
			select {
			case <-s.notifications:
				fmt.Fprintf(w, ssemsg)

				if f, ok := w.(http.Flusher); ok {
					f.Flush()
				}
			case <-r.Context().Done():
				return
			}
		}
	})
}

// Listen - makes server to start listening
func (s *DevServer) Listen() error {
	s.configureHotReload()

	log.Printf("Listening %v\n", s.HTTPServer.Addr)

	return s.HTTPServer.ListenAndServe()
}
