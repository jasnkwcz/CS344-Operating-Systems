#!/bin/bash
echo "testing"
./compileall.sh
./enc_server 99998 &
./enc_client plaintext4 key70000 99998 > ciphertext4
echo "enc test result:"
wc -m ciphertext4
sleep 5s
./dec_server 99999 &
./dec_client ciphertext4 key70000 99999 > plaintext4_a
wc -m plaintext4_a