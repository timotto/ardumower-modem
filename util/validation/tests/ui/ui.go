package ui

import (
	"fmt"
	"io/ioutil"
	"net/http"
	"strings"
	"sync"
	"time"

	"github.com/ardumower/esp_modem/util/validation/testbed"
	. "github.com/onsi/ginkgo"
	. "github.com/onsi/ginkgo/extensions/table"
	. "github.com/onsi/gomega"
)

func UiValidationSuite(bed *testbed.Testbed) bool {
	return Describe("Webserver with auth", func() {
		var tc testClient
		BeforeEach(func() {
			var err error
			tc.DeviceIp, err = bed.AssertWifiStaIp()
			Expect(err).ToNot(HaveOccurred())

			tc.Username, tc.Password, err = bed.AssertWebWithCredentials()
			Expect(err).ToNot(HaveOccurred())
		})

		DescribeTable("URLs that require authentication", func(path string) {
			c := httpClient()

			ExpectResponseStatus(c, tc.aGetRequest(path), http.StatusUnauthorized)

			ExpectResponseStatus(c, tc.anAuthenticatedGetRequest(path), http.StatusOK)
		}, entriesFor("/", "/build/bundle.js", "/build/bundle.css", "/api/modem/settings")...)

		DescribeTable("URLs that do not require authentication", func(path string) {
			c := httpClient()

			ExpectResponseStatus(c, tc.aGetRequest(path), http.StatusOK)
		}, entriesFor("/api/modem/info")...)

		Describe("Asset serving performance", func() {
			It("Responds at /", func() {
				res, err := httpClient().Do(tc.anAuthenticatedGetRequest("/"))
				Expect(err).ToNot(HaveOccurred())
				Expect(res.StatusCode).To(Equal(http.StatusOK))
			})

			// AsyncTCP/ESPAsyncWebServer is unstable with parallel requests
			XIt("serves parallel requests consecutively", func() {
				urls := []string{"/", "/build/bundle.js", "/build/bundle.css", "/api/modem/settings"}
				parallel := 2
				count, res := tc.RunParallelRequests(hammerTimeDuration, parallel, urls)
				Expect(res).To(Equal(NoErrors()))
				Expect(count).To(BeNumerically(">=", float64(parallel)*CalculateExpectedRequestCount(hammerTimeDuration, 600)))
			})

			It("serves small asset consecutively", func() {
				count, res := tc.RunConsecutiveRequests(hammerTimeDuration, "/index.html")
				Expect(ErrorCount(res)).To(BeNumerically("<=", 2))
				Expect(count).To(BeNumerically(">=", CalculateExpectedRequestCount(hammerTimeDuration, 500)))
			})

			It("serves big asset consecutively", func() {
				count, res := tc.RunConsecutiveRequests(hammerTimeDuration, "/build/bundle.js")
				Expect(ErrorCount(res)).To(BeNumerically("<=", 2))
				Expect(count).To(BeNumerically(">=", CalculateExpectedRequestCount(hammerTimeDuration, 800)))
			})
		})
	})
}

const (
	hammerTimeDuration = 5 * time.Second
)

type (
	CollectedErrors struct {
		Errors    []error
		BadStatus []int
	}
)

func NoErrors() CollectedErrors { return CollectedErrors{} }

func ErrorCount(c CollectedErrors) int {
	return len(c.Errors) + len(c.BadStatus)
}

func (c CollectedErrors) Add(o CollectedErrors) CollectedErrors {
	return CollectedErrors{Errors: append(c.Errors, o.Errors...), BadStatus: append(c.BadStatus, o.BadStatus...)}
}

type testClient struct {
	DeviceIp string
	Username string
	Password string
}

func (c testClient) serverUrl(path string) string {
	for strings.HasPrefix(path, "/") {
		path = path[1:]
	}

	return fmt.Sprintf("http://%s/%s", c.DeviceIp, path)
}

func (c testClient) aGetRequest(path string) *http.Request {
	req, err := http.NewRequest(http.MethodGet, c.serverUrl(path), nil)
	Expect(err).ToNot(HaveOccurred())

	return req
}

func (c testClient) anAuthenticatedGetRequest(path string) *http.Request {
	req := c.aGetRequest(path)
	req.SetBasicAuth(c.Username, c.Password)

	return req
}

func (c testClient) RunConsecutiveRequests(duration time.Duration, path string) (int, CollectedErrors) {
	result := CollectedErrors{}
	timeout := time.Now().Add(duration)
	count := 0

	for time.Now().Before(timeout) {
		count++

		res, err := httpClient().Do(c.anAuthenticatedGetRequest(path))
		if err != nil {
			result.Errors = append(result.Errors, err)
			continue
		}

		if res.StatusCode != http.StatusOK {
			result.BadStatus = append(result.BadStatus, res.StatusCode)
			continue
		}
	}

	return count, result
}

func (c testClient) RunParallelRequests(duration time.Duration, parallel int, paths []string) (int, CollectedErrors) {
	results := make([]CollectedErrors, parallel)
	timeout := time.Now().Add(duration)
	counts := make([]int, parallel)

	wg := &sync.WaitGroup{}

	var worker = func(instanceIndex int) {
		defer wg.Done()
		for {
			for _, path := range paths {
				if time.Now().After(timeout) {
					return
				}

				res, err := httpClient().Do(c.anAuthenticatedGetRequest(path))
				switch {
				case err != nil:
					results[instanceIndex].Errors = append(results[instanceIndex].Errors, err)
				case res.StatusCode != http.StatusOK:
					results[instanceIndex].BadStatus = append(results[instanceIndex].BadStatus, res.StatusCode)
				default:
					counts[instanceIndex]++
				}
			}
		}
	}

	wg.Add(parallel)
	for i := 0; i < parallel; i++ {
		go worker(i)
	}
	wg.Wait()

	count := 0
	for _, c := range counts {
		count += c
	}
	result := CollectedErrors{}
	for _, r := range results {
		result = result.Add(r)
	}

	return count, result
}

func CalculateExpectedRequestCount(testDuration time.Duration, estimatedAverageResponseTimeInMilliseconds float64) float64 {
	return testDuration.Seconds() / (estimatedAverageResponseTimeInMilliseconds / 1000.0)
}

func entriesFor(path ...string) []TableEntry {
	var result []TableEntry
	for _, p := range path {
		result = append(result, Entry(p, p))
	}
	return result
}

func ExpectResponseStatus(c *http.Client, req *http.Request, status int) {
	res, err := c.Do(req)
	Expect(err).ToNot(HaveOccurred())
	defer res.Body.Close()
	b, err := ioutil.ReadAll(res.Body)
	bodystr := string(b)
	bodylen := len(bodystr)
	if bodylen > 20 {
		bodylen = 20
	}
	desc := res.Status + ", Body(" + fmt.Sprintf("%v", len(b)) + " byte): [" + bodystr[:bodylen] + "]" + ", URL: [" + req.URL.String() + "]"
	Expect(err).ToNot(HaveOccurred(), desc)
	Expect(res.StatusCode).To(Equal(status), desc)
}

func httpClient() *http.Client {
	cpy := &http.Client{}
	cpy.Timeout = 3 * time.Second

	return cpy
}
