To use the melody generator (on Linux), please follow these instructions :
1) Install the timidity synthesizer :
  sudo apt install timidity
2) Compile the program and the midi converter :
  g++ gener.cc -o gener
  gcc asc2mid.c -o asc2mid
3) Make the launcher executable :
  sudo chmod +x play.sh
4) Execute the launcher :
  ./play.sh
You should hear a random melody.
