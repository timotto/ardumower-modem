package relay

import (
	"fmt"
	"github.com/ardumower/esp_modem/util/validation/testbed"
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/gomega"
	"net/http"
	"strings"
	"sync"
	"time"
)

func ValidationSuite(bed *testbed.Testbed) bool {
	return Describe("Relay Adapter", func() {
		var tc testClient
		BeforeEach(func() {
			var err error
			tc.DeviceIp, err = bed.AssertWifiStaIp()
			Expect(err).ToNot(HaveOccurred())

			tc.RelayUrl, tc.RelayUser, tc.RelayPassword, err = bed.AssertRelay()
			if strings.HasPrefix(tc.RelayUrl, "ws") {
				tc.RelayUrl = strings.Replace(tc.RelayUrl, "ws", "http", 1)
			}
			Expect(err).ToNot(HaveOccurred())
		})

		It("allows communication", func() {
			res, err := httpClient().Do(tc.ATV())
			Expect(err).ToNot(HaveOccurred())
			Expect(res).To(HaveHTTPStatus(http.StatusOK))
			Expect(res).To(HaveHTTPBody("V,Ardumower Sunray,1.0.219,1,52,0x4f\r\n"))
		})

		It("allows consecutive communication", func() {
			for i := 0; i < 10; i++ {
				res, err := httpClient().Do(tc.ATV())
				Expect(err).ToNot(HaveOccurred())
				Expect(res).To(HaveHTTPStatus(http.StatusOK))
				Expect(res).To(HaveHTTPBody("V,Ardumower Sunray,1.0.219,1,52,0x4f\r\n"))
			}
		})

		It("allows parallel communication", func() {
			parallelism := 4
			requestsPerInstance := 10

			errorRates := make([]float64, parallelism)
			wg := &sync.WaitGroup{}
			wg.Add(parallelism)

			for i := 0; i < parallelism; i++ {
				go func(index int) {
					defer GinkgoRecover()
					successfulRequests := float64(0)
					failedRequests := float64(0)
					defer func() {
						total := successfulRequests + failedRequests
						if total > 0 {
							errorRates[index] = failedRequests / total
						}
						wg.Done()
					}()

					for i := 0; i < requestsPerInstance; i++ {
						time.Sleep(100 * time.Millisecond)
						res, err := httpClient().Do(tc.ATV())
						if err != nil || res.StatusCode != http.StatusOK {
							failedRequests++
							continue
						}

						success, err := HaveHTTPBody("V,Ardumower Sunray,1.0.219,1,52,0x4f\r\n").Match(res)
						if err != nil {
							failedRequests++
							continue
						}

						if success {
							successfulRequests++
						} else {
							failedRequests++
						}
					}
				}(i)
			}

			wg.Wait()
			averageErrorRate := float64(0)
			for _, rate := range errorRates {
				averageErrorRate += rate
			}
			averageErrorRate /= float64(parallelism)

			Expect(averageErrorRate).To(BeNumerically("<=", 0.1))
		})
	})
}

type testClient struct {
	DeviceIp      string
	RelayUrl      string
	RelayUser     string
	RelayPassword string
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
	req, err := http.NewRequest(http.MethodPost, c.RelayUrl, strings.NewReader(body))
	req.SetBasicAuth(c.RelayUser, c.RelayPassword)
	req.Header.Set("content-type", "application/x-www-form-urlencoded; charset=UTF-8")
	Expect(err).ToNot(HaveOccurred())

	return req
}

func httpClient() *http.Client {
	cpy := &http.Client{}
	cpy.Timeout = 5 * time.Second

	return cpy
}
