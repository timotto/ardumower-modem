package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"
)

func main() {
	rd := bufio.NewReader(os.Stdin)
	for {
		fmt.Printf("> ")
		line := must(rd.ReadString('\n'))
		line = strings.TrimSuffix(line, "\n")
		fmt.Printf("checksum: 0x%02x\n", computeChecksum(line))
	}
}

func computeChecksum(text string) byte {
	result := byte(0)
	for _, b := range []byte(text) {
		result += b
	}
	return result
}

func must[T any](res T, err error) T {
	if err != nil {
		panic(err.Error())
	}
	return res
}
