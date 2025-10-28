package main

import (
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"os"
	"time"
)

func headers(w http.ResponseWriter, req *http.Request) {
	time.Sleep(50 * time.Millisecond)

	for name, headers := range req.Header {
		for _, h := range headers {
			fmt.Fprintf(w, "%v: %v\n", name, h)
		}
	}
	body, _ := io.ReadAll(req.Body)

	var v any

	json.Unmarshal(body, &v)
	res, _ := json.Marshal(v)

	fmt.Fprintf(w, "%s", string(res))
}

func main() {
	http.HandleFunc("/", headers)
	http.ListenAndServe(os.Args[1], nil)
}
