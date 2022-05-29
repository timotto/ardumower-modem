package http_adapter

import (
	"fmt"
	"github.com/ardumower/esp_modem/util/validation/testbed"
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"net/http"
	"strings"
	"time"
)

func ValidationSuite(bed *testbed.Testbed) bool {
	return Describe("HTTP Adapter", func() {
		var tc testClient
		BeforeEach(func() {
			var err error
			tc.DeviceIp, err = bed.AssertWifiStaIp()
			Expect(err).ToNot(HaveOccurred())
		})

		It("allows communication", func() {
			req := tc.ATV()
			res, err := httpClient().Do(req)
			Expect(err).ToNot(HaveOccurred())
			Expect(res).To(HaveHTTPStatus(http.StatusOK))
			Expect(res).To(HaveHTTPBody("V,Ardumower Sunray,1.0.219,1,52,0x4f\r\n"))
		})
	})
}

type testClient struct {
	DeviceIp string
}

func (c testClient) serverUrl(path string) string {
	for strings.HasPrefix(path, "/") {
		path = path[1:]
	}

	return fmt.Sprintf("http://%s/%s", c.DeviceIp, path)
}

func (c testClient) ATV() *http.Request {
	return c.aPostRequest(formatCommand("AT+V"))
}

func formatCommand(cmd string) string {
	return fmt.Sprintf("%v\n", withChecksum(cmd))
}

func withChecksum(cmd string) string {
	var sum byte
	for _, c := range cmd {
		sum += byte(c)
	}

	return fmt.Sprintf("%s,0x%02X", cmd, sum)
}

func (c testClient) aPostRequest(body string) *http.Request {
	req, err := http.NewRequest(http.MethodPost, c.serverUrl("/"), strings.NewReader(body))
	req.Header.Set("content-type", "text/plain")

	Expect(err).ToNot(HaveOccurred())

	return req
}

func httpClient() *http.Client {
	cpy := &http.Client{}
	cpy.Timeout = 5 * time.Second

	return cpy
}
