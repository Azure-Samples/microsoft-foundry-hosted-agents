FROM golang:1.26-alpine AS build

WORKDIR /src
COPY go.mod go.sum ./
RUN go mod download

COPY . .
RUN CGO_ENABLED=0 GOOS=linux GOARCH=amd64 go build -trimpath -ldflags="-s -w" -o /out/agent .

FROM alpine:3.23

COPY --from=build /etc/ssl/certs/ca-certificates.crt /etc/ssl/certs/ca-certificates.crt
COPY --from=build /out/agent /app/agent

RUN mkdir -p /home/session && chmod 1777 /tmp

WORKDIR /app
EXPOSE 8088

ENTRYPOINT ["/app/agent"]
