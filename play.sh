#!/bin/sh
./gener
./asc2mid melody.txt > melody.mid
timidity melody.mid
