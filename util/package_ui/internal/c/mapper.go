package c

import (
	"fmt"
	"strings"
)

func asByteArrayWithNullTerminator(data []byte) string {
	return asByteArray(append(data, 0))
}

func asByteArray(data []byte) string {
	var tmp []string
	for _, val := range data {
		tmp = append(tmp, fmt.Sprintf("0x%02x", val))
	}

	return fmt.Sprintf(`{
  %s
}`, strings.Join(tmp, ","))
}
