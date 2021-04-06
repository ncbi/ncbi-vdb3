#!/bin/bash

CERT="cert.crt"
KEY="private.key"

openssl req -x509 -sha256 -nodes -days 365 -newkey rsa:4096 -keyout $KEY -out $CERT
