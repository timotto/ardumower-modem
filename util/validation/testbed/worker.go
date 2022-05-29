package testbed

import (
	"bufio"
	"fmt"
	"os"
	"sync"

	"github.com/ardumower/esp_modem/util/validation/pkg/readline"
	"github.com/tarm/serial"
)

func (b *Testbed) worker(s *serial.Port) {
	reader := bufio.NewReader(s)

	line := readline.NewReadLine("\r\n", b.handleLineFromModem)

	wg := &sync.WaitGroup{}
	wg.Add(2)

	go func() {
		defer wg.Done()
		defer close(b.out)

		buffer := make([]byte, 1024)

		for {
			if !b.started {
				_, _ = reader.Read(buffer[:1])
				continue
			}

			n, err := reader.Read(buffer)
			if err != nil {
				fmt.Printf("failed to read from serial port: %v\n", err.Error())
				return
			}

			data := buffer[:n]
			_, _ = os.Stdout.Write(data)
			if _, err := line.Write(data); err != nil {
				fmt.Printf("failed to echo from serial port: %v\n", err.Error())
				return
			}
		}
	}()

	go func() {
		defer wg.Done()
		defer s.Close()
		for data := range b.out {
			if n, err := s.Write(data); err != nil {
				fmt.Printf("write to serial failed after %v bytes with %v\n", n, err.Error())
				return
			} else if n != len(data) {
				fmt.Printf("short write to serial %v bytes isntead of %v\n", n, len(data))
				return
			}
		}
		fmt.Println("serial output go routine finished")
	}()

	wg.Wait()
	fmt.Println("TestBed worker finished")
}
