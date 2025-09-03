package main

import (
    "fmt"
    "net/http"
    "os"
    "encoding/base64"
    "io"
)

func headers(w http.ResponseWriter, req *http.Request) {
    for name, headers := range req.Header {
        for _, h := range headers {
            fmt.Fprintf(w, "%v: %v\n", name, h)
        }
    }
    body, _ := io.ReadAll(req.Body)
    fmt.Fprintf(w, "%s", base64.URLEncoding.EncodeToString(body))
}

func main() {
    http.HandleFunc("/", headers)
    http.ListenAndServe(os.Args[1], nil)
}
