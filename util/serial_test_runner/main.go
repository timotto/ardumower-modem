package main

import (
	"io"
	"log"
	"os"
	"sync"

	"github.com/tarm/serial"

	"github.com/ardumower/esp_modem/util/serial_test_runner/internal/aunit"
	"github.com/ardumower/esp_modem/util/serial_test_runner/pkg/readline"
)

func main() {
	r := newSingleRun()
	a := aunit.NewAdapter(r)
	rl := readline.NewReadLine("\r\n", a.WriteLine)

	c := &serial.Config{Name: portName(), Baud: 115200}
	s, err := serial.OpenPort(c)
	if err != nil {
		log.Fatal(err)
	}

	a.SetReturnChannel(s)

	go copyLoop(s, rl, os.Stdout)

	r.Wait()

	res := r.Result()
	if !res.Success() {
		if res.Crash {
			log.Printf("pipe into trace: %s", res.Backtrace)
		}
		os.Exit(1)
	}
}

func portName() string {
	if len(os.Args) != 2 {
		log.Fatalf("Usage: %s <port-name> - eg COM45 or /dev/ttyUSB8", os.Args[0])
	}

	return os.Args[1]
}

func copyLoop(in io.Reader, out, tee io.Writer) {
	buf := make([]byte, 128)
	for {

		n, err := in.Read(buf)
		if err != nil {
			log.Fatal(err)
		}
		tee.Write(buf[:n])
		out.Write(buf[:n])
	}
}

type singleRun struct {
	wg *sync.WaitGroup

	result aunit.Stats
}

func newSingleRun() *singleRun {
	wg := &sync.WaitGroup{}
	wg.Add(1)

	return &singleRun{
		wg: wg,
	}
}

func (r *singleRun) Result() aunit.Stats {
	return r.result
}

func (r *singleRun) OnStart(s aunit.Stats) {
	r.result = s
	log.Println("single-run::started")
	r.wg.Done()
}

func (r *singleRun) OnFinish(s aunit.Stats) {
	r.result = s
	if r.result.Success() {
		log.Println("single-run::finished::success")
	} else {
		log.Println("single-run::finished::failure")
	}
	r.wg.Done()
}

func (r *singleRun) OnCrash(s aunit.Stats) {
	r.result = s
	log.Println("single-run::crash")
	r.wg.Done()
}

func (r *singleRun) Wait() {
	r.wg.Wait()
}
