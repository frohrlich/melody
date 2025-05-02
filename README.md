To use the melody generator (on Debian), please follow these instructions :
- Install the timidity synthesizer :
```
sudo apt install timidity
```
- Compile the program and the midi converter :
```
g++ gener.cc -o gener
gcc asc2mid.c -o asc2mid
```
- Execute the launcher :
```
sudo chmod +x play.sh
./play.sh
```
You should hear a random melody.
