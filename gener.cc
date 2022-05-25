#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdlib>

using namespace std;

string note(int);
string writeNote(int chan, int pitch, int temps, int duree, int octave=0); // write a note in text file to be converted to midi
int scale(const vector<int> chord, int step, int fond=0); // calculates note with scale and step
vector<int> constrChord(const vector<int> scale, int step, int njazz=0); // extract chord from scale
int myrandom(int i);

/* SOME SCALES AND CHORDS */
const vector<int> major = {0,2,4,5,7,9,11};
const vector<int> mineureHarm = {0,2,3,5,7,8,11};
const vector<int> mineureNat = {0,2,3,5,7,8,10};
const vector<int> pentaMin = {0,3,5,7,10};
const vector<int> blues = {0,3,5,6,7,10};
const vector<int> Maj = {0,4,7};
const vector<int> Min = {0,3,7};
const vector<int> Acc7 = {0,4,7,10};
const vector<int> Maj7 = {0,4,7,11};
const vector<int> Min7 = {0,3,7,10};

int main() {
	default_random_engine generator;
	generator.seed(time(NULL));
	srand(unsigned(time(0)));
	
	vector<int> scaly = mineureNat; // Choose scale
	int fonda = 0; // root
	int njazz = 0; // 0 = triad, 1=7th, 2=7th+9th etc
	int pm = 4, sizeBar = 4; // time signature pm/sizeBar
	int bar=sizeBar*pm;
	int harm = bar; // chord change frequency
	
	int t=0,dur=0,vers=4*bar,bloc=4*vers;
	vector<int> chord=constrChord(scaly,0,njazz),futurChord,prevchord=chord,useChord,versBox1,versBox2,patty=chord;
	vector<vector<int>> chordBox;
	int aco=0,intScale=0,leadnote=0,prevLeadNote=0,pitch=0,step=0,
		ve=0,tauxJazz=0,tauxCounter=0,tauxPhrase=0,tauxRepet=0,
		phra=0,phraseTaille=0,bassPitch=0,accPitch=0,middle=0;
	bool gamRel=0,prevoi=0,phrase=0;
	
	discrete_distribution<int> distribHarm;
	if(scaly == major) distribHarm = discrete_distribution<int>({1,1,1,1,1,1,0}); // don't play the 7th step diminished chord
	else distribHarm = discrete_distribution<int>({1,0,1,1,1,1,1}); // same principle for minor scale
	distribHarm(generator);
	
	/* TIME */
	uniform_int_distribution<int> distribProb(0,10);
	distribProb(generator);
	//initializer_list<double> listTemps = {2,10,6,4,2,2,1};
	//initializer_list<double> listTempsJazz = {2,10,6,0,0,0,0};
	initializer_list<double> listTemps = {0,double(distribProb(generator)),double(distribProb(generator)),
										  double(distribProb(generator)),double(distribProb(generator)),
										  double(distribProb(generator)),double(distribProb(generator)),
										  double(distribProb(generator))};
	initializer_list<double> listTempsJazz = {0,double(distribProb(generator)),double(distribProb(generator)),
											  double(distribProb(generator)),double(distribProb(generator)),
											  0,0,0}; // don't want jazz notes to be too long 
	discrete_distribution<int> useDistribTemps;
	
	ofstream infout("info.txt");
	
	infout << "listTemps : ";
	for(auto num = listTemps.begin(); num != listTemps.end(); ++num) {
		infout << *num << ' ';
	}
	infout << endl;
	
	infout << "listTempsJazz : ";
	for(auto num = listTempsJazz.begin(); num != listTempsJazz.end(); ++num) {
		infout << *num << ' ';
	}
	infout << endl;
		
	
	/* PITCH */
	//~ initializer_list<double> listPitch {1,1,1,1,1,1,1};  // 2nd (including)
	//~ initializer_list<double> listPitchJazz {1,1,1,1,1,1,1,
											//~ 1,1,1,1,1,1,1,1}; // same for scale
	
	initializer_list<double> listPitch {double(distribProb(generator)),double(distribProb(generator)),
										double(distribProb(generator)),double(distribProb(generator)),
										double(distribProb(generator)),double(distribProb(generator))};
	initializer_list<double> listPitchJazz {double(distribProb(generator)),double(distribProb(generator)),
										    double(distribProb(generator)),double(distribProb(generator)),
										    double(distribProb(generator)),double(distribProb(generator)),
										    double(distribProb(generator)),double(distribProb(generator)),
										    double(distribProb(generator)),double(distribProb(generator)),
										    double(distribProb(generator)),double(distribProb(generator)),
										    double(distribProb(generator)),double(distribProb(generator)),
										    double(distribProb(generator))};
	
	discrete_distribution<int> useDistribPitch;
	
	infout << "listPitch : ";
	for(auto num = listPitch.begin(); num != listPitch.end(); ++num) {
		infout << *num << ' ';
	}
	infout << endl;
	
	infout << "listPitchJazz : ";
	for(auto num = listPitchJazz.begin(); num != listPitchJazz.end(); ++num) {
		infout << *num << ' ';
	}
	infout << endl;
	
	uniform_int_distribution<int> distribCent(1,100); // various use
	distribCent(generator);
	uniform_int_distribution<int> distribJazz(1,100); // probability to play outside chord
	distribJazz(generator);
	uniform_int_distribution<int> distribPhrase(3,5); // phrases size
	distribPhrase(generator);
	
	if(scaly==major) fonda = 0; // C major
	else fonda = -3; // A minor
	tauxJazz = distribJazz(generator); // 70
	tauxCounter = 20; // 50
	tauxPhrase = 40; // 40
	tauxRepet = 70; // 70

	infout << "tauxJazz : " << tauxJazz << endl;

	infout.close();
	
	vector<vector<int>> leadTabl;
	vector<vector<int>> bassTabl;
	vector<vector<int>> accTabl;
	vector<int> currentNote;
	
	int songLength = 1000;
	
	while(t < songLength) {
		if(t%harm==0) {
			if(t!=0) chord = futurChord;
			step = distribHarm(generator);
			futurChord = constrChord(scaly, step, njazz);
		}
		
		if(t>=dur) { // LEAD
			if ((t-phraseTaille)%harm < t%harm) phrase = 0;
			dur = 0;
			if(distribCent(generator)<=tauxJazz) { // jazz (all scale)
				useChord = scaly;
				useDistribTemps = discrete_distribution<int>(listTempsJazz);
				useDistribTemps(generator);
			}
			else { // normal (stay in chord)
				useChord = chord;
				useDistribTemps = discrete_distribution<int>(listTemps);
				useDistribTemps(generator);
			}
			
			if(!phrase && (distribCent(generator)<=tauxPhrase)) { // NEW PHRASE
				phraseTaille = distribPhrase(generator);
				if(int(leadTabl.size()) >= phraseTaille) { // only for beginning
					phrase=1;
				}
				phra = 0;
			}
			// NOTE CALCULATION
			if(t%2==0 || (t%2!=0 && distribCent(generator)<=tauxCounter)) { // play off-beat or not
				dur = useDistribTemps(generator);
				if(useChord==chord) {
					if(prevchord==scaly) pitch = (pitch-step)*(3+njazz)/7;
					useDistribPitch = discrete_distribution<int>(listPitch);
					useDistribPitch(generator);
					middle = listPitch.size()/2;
					pitch += useDistribPitch(generator) - middle; // centered on zero
					if(pitch-step>=10) pitch -= 2;
					else if (pitch-step<=-15) pitch += 2;
				}
				else if(useChord==scaly) {
					if(prevchord==chord) pitch = pitch*7/(3+njazz)+step;
					useDistribPitch = discrete_distribution<int>(listPitchJazz);
					useDistribPitch(generator);
					middle = listPitchJazz.size()/2;
					pitch += useDistribPitch(generator) - middle;
					if(pitch>=20) pitch -= 5;
					else if (pitch<=-30) pitch += 5;
				}
				
				leadnote = scale(useChord, pitch, fonda);
				prevchord = useChord;
			}
			else dur = 0;
			
			//cout << "phra=" << phra << " phrase=" << phrase << " phraseTaille=" << phraseTaille << endl;
			
			if(phrase && (phra < phraseTaille)) {
				//if(phra==0 && (phraseTaille%2 != 0)) dur = 0;
				int rInd = leadTabl.size()-phraseTaille;
				if(distribCent(generator)<=tauxRepet) {
					leadnote = leadTabl[rInd][1];
				}
				dur = leadTabl[rInd+1][0] - leadTabl[rInd][0];
				++phra;
			}
			
			else if(phrase && phra==phraseTaille) phrase = 0;
			
			if(leadnote<=-20) leadnote += 12;
			else if(leadnote>=20) leadnote -= 12;
			
			currentNote = vector<int>({t,leadnote});
			if(dur!=0) leadTabl.push_back(currentNote);
			
			dur += t;
			prevLeadNote = leadnote;
			
		}
		if(t%harm==0) { // BASS
			bassPitch = scale(chord, 0, fonda);
			currentNote = vector<int>({t,bassPitch});
			bassTabl.push_back(currentNote);
		}
		if(t%(pm/2)==0) { // ACCOMP
			aco++;
			if(t%bar==0) aco = 0;
			accPitch = scale(chord, aco%sizeBar, fonda);
			currentNote = vector<int>({t,accPitch});
			accTabl.push_back(currentNote);
		}
		++t;
	}
	
	cout << "end" << endl;
	
	ofstream out("melody.txt");
	
	out << "format=0 tracks=1 division=96" << endl << endl
		<< "BA    1   CR         0   TR  0   CH 16   Text type 3: \"Test 1\"" << endl
		<< "BA    1   CR         0   TR  0   CH 16   Text type 2: \"Felix Rohrlich\"" << endl
		<< "BA    1   CR         0   TR  0   CH 16   Tempo 200" << endl
		<< "BA    1   CR         0   TR  0   CH 16   Time signature 4/4, clocks/mtick 24, crotchets/32ndnote 8" << endl
		<< "BA    1   CR         0   TR  0   CH  1   ST &B0 &79 &00" << endl
		<< "BA    1   CR         0   TR  0   CH  1   ST &B0 &00 &00" << endl
		<< "BA    1   CR         0   TR  0   CH  1   ST &B0 &20 &00" << endl
		
		<< "BA    1   CR         0   TR  0   CH  1   Instrument 34" << endl
		<< "BA    1   CR         0   TR  0   CH  2   Instrument 25" << endl
		<< "BA    1   CR         0   TR  0   CH  3   Instrument 1" << endl
		
		<< "BA    1   CR         0   TR  0   CH  1   Channel volume 80" << endl
		<< "BA    1   CR         0   TR  0   CH  2   Channel volume 40" << endl
		<< "BA    1   CR         0   TR  0   CH  3   Channel volume 80" << endl
		
		<< "BA    1   CR         0   TR  0   CH  1   ST &B0 &0A &40" << endl
		<< "BA    1   CR         0   TR  0   CH  1   ST &B0 &5B &50" << endl
		<< "BA    1   CR         0   TR  0   CH  1   ST &B0 &5D &00" << endl;
	
	int tp=0; // time play
	int ilead=0,ibass=0,iacc=0,gap=0; // progress, note duration
	
	for (tp=0;tp<songLength/2;++tp) {
		if(leadTabl[ilead][0]==tp) {
			gap = leadTabl[ilead+1][0] - leadTabl[ilead][0];
			out << writeNote(3, leadTabl[ilead][1], tp, gap, 1) << endl; // LEAD
			++ilead;
		}
		if(bassTabl[ibass][0]==tp) {
			gap = bassTabl[ibass+1][0] - bassTabl[ibass][0];
			out << writeNote(1, bassTabl[ibass][1], tp, gap, -2) << endl; // BASS
			++ibass;
		}
		if(accTabl[iacc][0]==tp) {
			gap = accTabl[iacc+1][0] - accTabl[iacc][0];
			out << writeNote(2, accTabl[iacc][1], tp, gap, 0) << endl; // ACCOMP
			++iacc;
		}
	}

	out << "BA    10000   CR         0   TR  0   CH 16   End of track" << endl;
	out.close();
	
	ifstream infin("info.txt");
	cout << infin.rdbuf();
	infin.close();
	
	
	/* !!!! TODO !!!! */
	// - Vraie boucle vector
	// - Revoir accords (+ vector de chords)
	// - varie frequence changement accords
	// - mathsons
	// - script copier
	// - pas forcement I debut
	
}

string note(int x) {
	string noty("");
	int xcalc;
	
	if (x<0) xcalc = (x-12*(x/12-1))%12;
	else xcalc = x%12;
	
	switch (xcalc) {
		case 0:
			noty = "C";
			break;
		case 1:
			noty = "C#";
			break;
		case 2:
			noty = "D";
			break;
		case 3:
			noty = "D#";
			break;
		case 4:
			noty = "E";
			break;
		case 5:
			noty = "F";
			break;
		case 6:
			noty = "F#";
			break;
		case 7:
			noty = "G";
			break;
		case 8:
			noty = "G#";
			break;
		case 9:
			noty = "A";
			break;
		case 10:
			noty = "A#";
			break;
		case 11:
			noty = "B";
			break;
	}
	int octy = x/12 + (x%12 == 0);
	while(octy <= 0 && x < 0) {
		noty += "-";
		octy++;
	}
	octy = x/12;
	while(octy > 0) {
		noty += "'";
		octy--;
	}
	return noty;
}

int scale(const vector<int> chord, int step, int fond) {
	int niveau, xcalg, taille = chord.size();
	niveau = 12*((step/taille)-(step<0)+(step%taille==0 && step < 0));
	if (step<0) xcalg = (step-taille*(step/taille-1))%taille;
	else xcalg = step%taille;
	return niveau + fond + chord[xcalg];
}

vector<int> constrChord(const vector<int> scale, int step, int njazz) {
	vector<int> chord;
	int niveau, stepCalc, xcalg, taille = scale.size();
	for(int i = 0 ; i <= 2+njazz ; ++i) {
		stepCalc = step + 2*i;
		niveau = 12*((stepCalc/taille)-(stepCalc<0)+(stepCalc%taille==0 && stepCalc < 0));
		if (stepCalc<0) xcalg = (stepCalc-taille*(stepCalc/taille-1))%taille;
		else xcalg = stepCalc%taille;
		chord.push_back(scale[xcalg] + niveau);
	}
	return chord;
}

string writeNote(int chan, int pitch, int temps, int duree, int octave) {
	int pitchCalc = pitch+octave*12;
	string texNote("");
	texNote += "BA    1   CR";
	if(temps == 1) texNote += "       1/2";
	else if(temps%2 != 0) texNote += "     " + to_string(temps/2) + "+1/2";
	else texNote += "         " + to_string(temps/2);
	texNote += "   TR  0   CH  " + to_string(chan) + "   NT  " + note(pitchCalc);
	if(duree == 1) texNote += "            1/2";
	else if(duree%2 != 0) texNote += "          " + to_string(duree/2) + "+1/2";
	else texNote += "              " + to_string(duree/2);
	
	return texNote;
}

int myrandom (int i) {return std::rand()%i;}
