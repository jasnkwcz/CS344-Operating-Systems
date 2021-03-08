#!/bin/bash
./compileall.sh
./dec_server 99999 &
./dec_client ciphertext4 key70000 99999 > plaintext4_a