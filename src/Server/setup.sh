#!/bin/bash

openssl req -x509 -nodes -days 3650 -newkey rsa:2048 \
  -keyout crypto/server.key -out crypto/server.crt \
  -subj "/C=RO/ST=SomeState/L=City/O=Org/OU=Dev/CN=localhost"
