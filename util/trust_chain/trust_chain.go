package main

import (
	"crypto/tls"
	"crypto/x509"
	"encoding/pem"
	"fmt"
	"log"
	"net/url"
	"os"
)

func main() {
	if err := run(os.Args); err != nil {
		log.Fatalln(err)
	}
}

func run(osArgs []string) error {
	args := osArgs[1:]
	targets, err := parseArgs(args)
	if err != nil {
		return err
	}

	certs, err := scrapeTargets(targets)
	if err != nil {
		return err
	}

	certs = removeDuplicates(certs)

	if err := outputResult(certs); err != nil {
		return nil
	}

	log.Printf("found %v CA certificates", len(certs))

	return nil
}

func parseArgs(targets []string) ([]*url.URL, error) {
	if len(targets) == 0 {
		return nil, fmt.Errorf("run this program with target URLs as arguments")
	}

	var result []*url.URL
	for _, target := range targets {
		u, err := url.Parse(target)
		if err != nil {
			return nil, err
		}

		if u.Scheme != "https" {
			return nil, fmt.Errorf("all target URLS must be https:// URLs")
		}

		result = append(result, u)
	}

	return result, nil
}

func scrapeTargets(targets []*url.URL) ([]*x509.Certificate, error) {
	var result []*x509.Certificate

	for _, target := range targets {
		hostname := target.Hostname()
		port := target.Port()

		if rootCAs, err := extractRootCAsFrom(hostname, port); err != nil {
			return nil, fmt.Errorf("failed to scrape %v: %w", target, err)
		} else {
			result = append(result, rootCAs...)
		}
	}

	return result, nil
}

func removeDuplicates(certs []*x509.Certificate) []*x509.Certificate {
	var result []*x509.Certificate

	lookup := make(map[string]bool)
	for _, cert := range certs {
		key := string(cert.Signature)

		if _, found := lookup[key]; found {
			continue
		}

		result = append(result, cert)
		lookup[key] = true
	}

	return result
}

func outputResult(certs []*x509.Certificate) error {
	out := os.Stdout

	for _, cert := range certs {
		block := &pem.Block{
			Type:  "CERTIFICATE",
			Bytes: cert.Raw,
		}
		if err := pem.Encode(out, block); err != nil {
			return err
		}
	}

	return nil
}

func extractRootCAsFrom(hostname, port string) ([]*x509.Certificate, error) {
	if port == "" {
		port = "443"
	}

	var rootCAs []*x509.Certificate

	addr := fmt.Sprintf("%v:%v", hostname, port)
	config := &tls.Config{
		VerifyPeerCertificate: rootCaExtractor(&rootCAs),
		ServerName:            hostname,
	}

	if conn, err := tls.Dial("tcp", addr, config); err != nil {
		return nil, fmt.Errorf("cannot connect to %v: %w", addr, err)
	} else {
		_ = conn.Close()
	}

	return rootCAs, nil
}

func rootCaExtractor(roots *[]*x509.Certificate) func(rawCerts [][]byte, verifiedChains [][]*x509.Certificate) error {
	return func(rawCerts [][]byte, verifiedChains [][]*x509.Certificate) error {
		if len(verifiedChains) == 0 {
			return fmt.Errorf("there are no verified chains")
		}

		foundRoot := false
		for i, chain := range verifiedChains {
			for j, certificate := range chain {
				log.Printf("chain#%v cert#%v: %v", i, j, certificate.Subject)
			}

			root := chain[len(chain)-1]
			*roots = append(*roots, root)
			foundRoot = true
		}

		if !foundRoot {
			return fmt.Errorf("no trusted root certificate found")
		}

		return nil
	}
}

func encodeSinglePemBlock(blockType string, data []byte) ([]byte, error) {
	if pemData := pem.EncodeToMemory(&pem.Block{Type: blockType, Bytes: data}); pemData == nil {
		return nil, fmt.Errorf("PEM encoding failed")
	} else {
		return pemData, nil
	}
}
