package aunit

import (
	"fmt"
	"io"
	"log"
	"regexp"
	"strconv"
	"time"
)

type Adapter struct {
	l Listener
	w io.Writer

	stats Stats
}

type Listener interface {
	OnStart(s Stats)
	OnFinish(s Stats)
	OnCrash(s Stats)
}

type Stats struct {
	Duration  time.Duration
	Total     int
	Passed    int
	Failed    int
	Skipped   int
	Timeout   int
	Crash     bool
	Backtrace string
}

var (
	delayRegexp     = regexp.MustCompile("^press RETURN to start without delay$")
	startRegexp     = regexp.MustCompile("^TestRunner started on ([0-9]+) test\\(s\\).$")
	durationRegexp  = regexp.MustCompile("^TestRunner duration: ([0-9.]+) seconds.$")
	summaryRegexp   = regexp.MustCompile("^TestRunner summary: ([0-9]*) passed, ([0-9]*) failed, ([0-9]*) skipped, ([0-9]*) timed out, out of ([0-9]*) test")
	backtraceRegexp = regexp.MustCompile("^Backtrace: 0x[0-9a-f]*")
)

func NewAdapter(l Listener) *Adapter {
	return &Adapter{
		l: l,
	}
}

func (a *Adapter) SetReturnChannel(w io.Writer) {
	a.w = w
}

func (a *Adapter) WriteLine(line string) {
	switch {
	case a.checkDelay(line):
		break
	case a.checkStart(line):
		a.l.OnStart(a.stats)
	case a.checkDuration(line):
		break
	case a.checkSummary(line):
		a.l.OnFinish(a.stats)
	case a.checkBacktrace(line):
		a.l.OnCrash(a.stats)
	}
}

func (a *Adapter) checkDelay(line string) bool {
	if !delayRegexp.MatchString(line) {
		return false
	}

	if a.w != nil {
		_, _ = a.w.Write([]byte("\r\n"))
	}

	return true
}

func (a *Adapter) checkStart(line string) bool {
	if m := startRegexp.FindAllString(line, -1); m == nil || len(m) < 1 || len(m[0]) < 2 {
		return false
	} else if total, err := strconv.ParseInt(string(m[0][1]), 10, 32); err != nil {
		log.Printf("aunit.adapter.check-start.parse-total %v", err.Error())
		return false
	} else {
		a.stats.Clear()
		a.stats.Total = int(total)
		return true
	}
}

func (a *Adapter) checkDuration(line string) bool {
	if m := durationRegexp.FindAllStringSubmatch(line, -1); m == nil || len(m) < 1 || len(m[0]) < 2 {
		return false
	} else if duration, err := time.ParseDuration(fmt.Sprintf("%ss", string(m[0][1]))); err != nil {
		log.Printf("aunit.adapter.check-duration.parse-duration %v", err.Error())
		return false
	} else {
		a.stats.Duration = duration
		return true
	}
}

func (a *Adapter) checkSummary(line string) bool {
	// "TestRunner summary: 17 passed, 1 failed, 0 skipped, 0 timed out, out of 18 test(s)."
	if m := summaryRegexp.FindAllStringSubmatch(line, -1); m == nil || len(m) < 1 || len(m[0]) < 6 {
		return false
	} else {
		var values []int
		for i, s := range m[0][1:] {
			if p, err := strconv.ParseInt(s, 10, 32); err != nil {
				log.Printf("aunit.adapter.check-summary.parse-index-%d %v", i, err.Error())
				return false
			} else {
				values = append(values, int(p))
			}
		}

		a.stats.Passed = values[0]
		a.stats.Failed = values[1]
		a.stats.Skipped = values[2]
		a.stats.Timeout = values[3]
		a.stats.Total = values[4]

		return true
	}
}

func (a *Adapter) checkBacktrace(line string) bool {
	if m := backtraceRegexp.MatchString(line); m {
		a.stats.Backtrace = line
		a.stats.Crash = true

		return true
	} else {
		return false
	}
}

func (s *Stats) Clear() {
	s.Duration = time.Duration(0)
	s.Total = 0
	s.Passed = 0
	s.Skipped = 0
	s.Failed = 0
	s.Timeout = 0
	s.Crash = false
	s.Backtrace = ""
}

func (s Stats) Success() bool {
	return !s.Crash && s.Total == s.Passed
}
