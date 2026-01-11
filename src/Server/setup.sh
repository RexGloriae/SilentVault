#!/bin/bash

openssl req -x509 -nodes -days 3650 -newkey rsa:2048 \
  -keyout crypto/server.key -out crypto/server.crt \
  -subj "/C=RO/ST=Buc/L=Buc/O=MTA/OU=RexGloriae/CN=127.0.0.1"
