#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <iostream.h>
#include <math.h> // Needed for pow() for pitch shifting

#define bool int
#define MOUSE_INT 0x33
#define INIT_MOUSE 0x00
#define SHOW_MOUSE 0x01
#define HIDE_MOUSE 0x02
#define GET_MOUSE_STATUS 0x03
#define VIDEO_ADDRESS 0xB800

// --- UI & Program State ---
enum Screen { SCR_SYNTH, SCR_SETTINGS, SCR_HELP };
Screen current_screen = SCR_SYNTH;

// --- UI Layout Data ---

// Set this to 1 to use the simple ANSI UI, 0 for the box-drawing character UI
bool use_ansi_ui = 0;

// UI for the main synthesizer screen (Box-drawing characters)
const char* ui_layout_box[25] = {
    "[***************^**********^******^****************************************^***]",
    "| Dos-BeepSynth | Settings | Help |                                        | x |",
    "@***************_**********_******_****************************************_***$",
    "|                                                                              |",
    "|                                                                              |",
    "|              `=================~=======================~=====%               |",
    "|              !????   ???   ????|????   ???   ???   ????|?????!               |",
    "|              !????   ???   ????|????   ???   ???   ????|?????!               |",
    "|              !????   ???   ????|????   ???   ???   ????|?????!               |",
    "|              !????   ???   ????|????   ???   ???   ????|?????!               |",
    "|              !????   ???   ????|????   ???   ???   ????|?????!               |",
    "|              !????   ???   ????|????   ???   ???   ????|?????!               |",
    "|              !????   ???   ????|????   ???   ???   ????|?????!               |",
    "|              !?????|?????|?????|?????|?????|?????|?????|?????!               |",
    "|              !?????|?????|?????|?????|?????|?????|?????|?????!               |",
    "|              !?????|?????|?????|?????|?????|?????|?????|?????!               |",
    "|              !?????|?????|?????|?????|?????|?????|?????|?????!               |",
    "|              <=====&=====&=====&=====&=====&=====&=====&=====>               |",
    "|                                                                              |",
    "|                                                                              |",
    "|           [**************^***^***]    [*****************^***^***]            |",
    "|           | Octave:  00  |   |   |    | Transpose:  00  |   |   |            |",
    "|           {**************_***_***}    {*****************_***_***}            |",
    "|                                                                              |",
    "{******************************************************************************}"
};

// UI for the main synthesizer screen (ANSI characters)
const char* ui_layout_ansi[25] = {
    "+---------------+----------+------+----------------------------------------+---+",
    "| Dos-BeepSynth | Settings | Help |                                        | x |",
    "+---------------+----------+------+----------------------------------------+---+",
    "|                                                                              |",
    "|                                                                              |",
    "|              +-----------------+-----------------------+-----+               |",
    "|              |   |   | |   |   |   |   | |   | |   |   |     |               |",
    "|              |   |   | |   |   |   |   | |   | |   |   |     |               |",
    "|              |   |   | |   |   |   |   | |   | |   |   |     |               |",
    "|              |   |   | |   |   |   |   | |   | |   |   |     |               |",
    "|              |   |   | |   |   |   |   | |   | |   |   |     |               |",
    "|              |   |   | |   |   |   |   | |   | |   |   |     |               |",
    "|              |   |___| |___|   |   |___| |___| |___|   |     |               |",
    "|              |     |     |     |     |     |     |     |     |               |",
    "|              |     |     |     |     |     |     |     |     |               |",
    "|              |     |     |     |     |     |     |     |     |               |",
    "|              |     |     |     |     |     |     |     |     |               |",
    "|              +-----+-----+-----+-----+-----+-----+-----+-----+               |",
    "|                                                                              |",
    "|                                                                              |",
    "|           +--------------+---+---+    +-----------------+---+---+            |",
    "|           | Octave:  00  |   |   |    | Transpose:  00  |   |   |            |",
    "|           +--------------+---+---+    +-----------------+---+---+            |",
    "|                                                                              |",
    "+------------------------------------------------------------------------------+"
};

// UI for the Settings screen (Box-drawing characters)
const char* ui_settings_box[25] = {
	"[***************^**********^******^****************************************^***]",
	"| Dos BeepSynth | Settings | Help |                                        | x |",
	"@***************_**********_******_****************************************_***$",
	"|                                                                              |",
	"|   Decay Time (ms):          -| 150 |+            [******************]        |",
	"|                                                  |  Close Settings  |        |",
	"|   Show Labels:               | ON  |             {******************}        |",
	"|                                                                              |",
	"|   Default Octave:           -| 00  |+                                        |",
	"|                                                                              |",
	"|   Draw Animations:           | ON  |                                         |",
	"|                                                                              |",
	"|   Use Ansi characters only   | OFF |                                         |",
	"|                                                                              |",
	"|                                                                              |",
	"|                                                                              |",
	"|                                                                              |",
	"|                                                                              |",
	"|                                                                              |",
	"|                                                                              |",
	"|                       [********]   [********]                                |",
	"|                       |  Save  |   | Cancel |                                |",
	"|                       {********}   {********}                                |",
	"|                                                                              |",
	"{******************************************************************************}"
};

// UI for the Settings screen (ANSI characters)
const char* ui_settings_ansi[25] = {
	"+---------------+----------+------+----------------------------------------+---+",
	"| Dos BeepSynth | Settings | Help |                                        | x |",
	"+---------------+----------+------+----------------------------------------+---+",
	"|                                                                              |",
	"|   Decay Time (ms):          -| 150 |+            +------------------+        |",
	"|                                                  |  Close Settings  |        |",
	"|   Show Labels:               | ON  |             +------------------+        |",
	"|                                                                              |",
	"|   Default Octave:           -| 00  |+                                        |",
	"|                                                                              |",
	"|   Draw Animations:           | ON  |                                         |",
	"|                                                                              |",
	"|   Use Ansi characters only   | OFF |                                         |",
	"|                                                                              |",
	"|                                                                              |",
	"|                                                                              |",
	"|                                                                              |",
	"|                                                                              |",
	"|                                                                              |",
	"|                                                                              |",
	"|                       +--------+   +--------+                                |",
	"|                       |  Save  |   | Cancel |                                |",
	"|                       +--------+   +--------+                                |",
	"|                                                                              |",
	"+------------------------------------------------------------------------------+"
};

// UI for the Help screen (Box-drawing characters)
const char* ui_help_box[25] = {
	"[***************^**********^******^****************************************^***]",
	"| Dos BeepSynth | Settings | Help |                                        | x |",
	"@***************_**********_******_****************************************_***$",
	"|                                                                              |",
	"|                              Key Controls Help                               |",
	"|                                                                              |",
	"|   C V B N M , .    :  White keys (C D E F G A B C)                           |",
	"|   F G J K L        :  Black keys (C# D# F# G# A#)                            |",
	"|   /                :  High C (upper octave)                                  |",
	"|   Up/Down Arrow    :  Octave Up/Down (+4 or -4 only)                         |",
    "|   Left/Right Arrow :  Transpose Up/Down (+8 or -8 only)                      |",
    "|   Mouse Click      :  Play notes on piano keys                               |",
	"|   F1               :  This Help Screen                                       |",
	"|   F2               :  Settings Screen                                        |",
	"|   ESC              :  Exit program                                           |",
	"|                                                                              |",
	"|   NOTES:                                                                     |",
	"|   - This synth uses PC speaker.                                              |",
	"|   - Only one note can play at a time.                                        |",
	"|   - UI supports 80x25 text mode only.                                        |",
	"|                                                                              |",
	"|                              [******************]                            |",
	"|                              |    Close Help    |                            |",
	"|                              {******************}                            |",
	"{******************************************************************************}"
};

// UI for the Help screen (ANSI characters)
const char* ui_help_ansi[25] = {
	"+---------------+----------+------+----------------------------------------+---+",
	"| Dos BeepSynth | Settings | Help |                                        | x |",
	"+---------------+----------+------+----------------------------------------+---+",
	"|                                                                              |",
	"|                              Key Controls Help                               |",
	"|                                                                              |",
	"|   C V B N M , .    :  White keys (C D E F G A B C)                           |",
	"|   F G J K L        :  Black keys (C# D# F# G# A#)                            |",
	"|   /                :  High C (upper octave)                                  |",
	"|   Up/Down Arrow    :  Octave Up/Down (+4 or -4 only)                         |",
    "|   Left/Right Arrow :  Transpose Up/Down (+8 or -8 only)                      |",
    "|   Mouse Click      :  Play notes on piano keys                               |",
	"|   F1               :  This Help Screen                                       |",
	"|   F2               :  Settings Screen                                        |",
	"|   ESC              :  Exit program                                           |",
	"|                                                                              |",
	"|   NOTES:                                                                     |",
	"|   - This synth uses PC speaker.                                              |",
	"|   - Only one note can play at a time.                                        |",
	"|   - UI supports 80x25 text mode only.                                        |",
	"|                                                                              |",
	"|                              +------------------+                            |",
	"|                              |    Close Help    |                            |",
	"|                              +------------------+                            |",
	"+------------------------------------------------------------------------------+"
};


// --- Global Settings & Variables ---

// Base frequencies for C5-C6, used as a reference for pitch shifting
float base_notes[13] = {523.25,554.37,587.33,622.25,659.25,698.46,739.99,783.99,830.61,880.00,932.33,987.77,1046.5};
// Active note frequencies, modified by octave and transpose
float notes[13];

// Main settings
int octave = 0;
int transpose = 0;
int video_mode = 7; // 80x25 monochrome text mode
bool key_labels = 1;
int decay_time = 150;
bool draw_key_animations = 1;
int default_octave = 0;

// Temporary settings for use on the settings screen
int temp_decay_time;
bool temp_key_labels;
bool temp_draw_key_animations;
int temp_default_octave;
bool temp_use_ansi_ui;

// --- Core Functions ---

void setTextMode(int mode) {
	 union REGS inregs, outregs;
	 inregs.h.ah = 0x00;
	 inregs.h.al = mode;
	 int86(0x10, &inregs, &outregs);
}

void setCharacterColr(int row, int column, char character, unsigned char color) {
    unsigned char far* screenBuffer = (unsigned char far*)0xB8000000;
    int offset = (row - 1) * 160 + (column - 1) * 2;
    screenBuffer[offset] = character;
    screenBuffer[offset + 1] = color;
}

void setCharacter(int row, int column, char character) {
    unsigned char far* screenBuffer = (unsigned char far*)0xB8000000;
    int offset = (row - 1) * 160 + (column - 1) * 2;
    screenBuffer[offset] = character;
}

void writeString(int row, int col, const char* str) {
	for(int i = 0; str[i] != '\0'; i++) {
		setCharacter(row, col + i, str[i]);
	}
}

// --- Mouse Functions ---
void show_mouse();
void hide_mouse();

// --- UI Drawing Functions ---

void drawOnAnimations(int note){
	if (!draw_key_animations) return;
	char keyChar = 219;
	unsigned char keyColor = use_ansi_ui ? 0x22 : 0xF2;

	if(video_mode == 7){
		if(note == 0){ setCharacterColr(17,19,keyChar,keyColor); } //c
		else if(note == 1){ setCharacterColr(17,25,keyChar,keyColor); } //d
		else if(note == 2){ setCharacterColr(17,31,keyChar,keyColor); } //e
		else if(note == 3){ setCharacterColr(17,37,keyChar,keyColor); } //f
		else if(note == 4){ setCharacterColr(17,43,keyChar,keyColor); } //g
		else if(note == 5){ setCharacterColr(17,49,keyChar,keyColor); } //a
		else if(note == 6){ setCharacterColr(17,55,keyChar,keyColor); } //b
		else if(note == 7){ setCharacterColr(17,61,keyChar,keyColor); } //c6
		else if(note == 8){ setCharacterColr(13,22,keyChar,keyColor); } //c#
		else if(note == 9){ setCharacterColr(13,28,keyChar,keyColor); } //d#
		else if(note == 10){ setCharacterColr(13,40,keyChar,keyColor); } //f#
		else if(note == 11){ setCharacterColr(13,46,keyChar,keyColor); } //g#
		else if(note == 12){ setCharacterColr(13,52,keyChar,keyColor); } //a#
	}
}

void drawOffAnimations(int note){
	if (!draw_key_animations) return;
	if(video_mode == 7){
		if(note >= 0 && note <= 7){
			char keyChar = use_ansi_ui ? ' ' : 219;
			unsigned char offColor = 0x07;
			if(note == 0){ setCharacterColr(17,19,keyChar,offColor); }
			else if(note == 1){ setCharacterColr(17,25,keyChar,offColor); }
			else if(note == 2){ setCharacterColr(17,31,keyChar,offColor); }
			else if(note == 3){ setCharacterColr(17,37,keyChar,offColor); }
			else if(note == 4){ setCharacterColr(17,43,keyChar,offColor); }
			else if(note == 5){ setCharacterColr(17,49,keyChar,offColor); }
			else if(note == 6){ setCharacterColr(17,55,keyChar,offColor); }
			else if(note == 7){ setCharacterColr(17,61,keyChar,offColor); }
		}
		else if (note >= 8 && note <= 12) {
			char keyChar = use_ansi_ui ? '_' : 219;
			unsigned char offColor = use_ansi_ui ? 0x07 : 0x00;
			if(note == 8){ setCharacterColr(13,22,keyChar,offColor); }
			else if(note == 9){ setCharacterColr(13,28,keyChar,offColor); }
			else if(note == 10){ setCharacterColr(13,40,keyChar,offColor); }
			else if(note == 11){ setCharacterColr(13,46,keyChar,offColor); }
			else if(note == 12){ setCharacterColr(13,52,keyChar,offColor); }
		}
	}
}

void drawKeyLabels(){
	if (!key_labels) {
		for(int i=1; i<=80; i++) { setCharacter(16, i, ' '); }
		return;
	}
	if(use_ansi_ui == 0){
		setCharacterColr(16,19,'C',0x70); setCharacterColr(16,25,'D',0x70);
		setCharacterColr(16,31,'E',0x70); setCharacterColr(16,37,'F',0x70);
		setCharacterColr(16,43,'G',0x70); setCharacterColr(16,49,'A',0x70);
		setCharacterColr(16,55,'B',0x70); setCharacterColr(16,61,'C',0x70);
	} else {
		setCharacter(16,19,'C'); setCharacter(16,25,'D'); setCharacter(16,31,'E');
		setCharacter(16,37,'F'); setCharacter(16,43,'G'); setCharacter(16,49,'A');
		setCharacter(16,55,'B'); setCharacter(16,61,'C');
	}
	setCharacter(11,22,'C'); setCharacter(12,22,'#');
	setCharacter(11,28,'D'); setCharacter(12,28,'#');
	setCharacter(11,40,'F'); setCharacter(12,40,'#');
	setCharacter(11,46,'G'); setCharacter(12,46,'#');
	setCharacter(11,52,'A'); setCharacter(12,52,'#');
}

void drawUIFromLayout(const char** layout, bool is_ansi) {
    char keyChar = is_ansi ? '#' : 219;
    for (int row = 0; row < 25; ++row) {
        for (int col = 0; col < 80; ++col) {
            char ch = layout[row][col];
            if (ch == '\0') break;
            char charToDraw = ch;
            if (!is_ansi) {
                switch(ch) {
                    case '|': charToDraw = 179; break;
                    case '*': charToDraw = 196; break;
                    case '[': charToDraw = 218; break;
                    case ']': charToDraw = 191; break;
                    case '{': charToDraw = 192; break;
                    case '}': charToDraw = 217; break;
                    case '^': charToDraw = 194; break;
                    case '_': charToDraw = 193; break;
                    case '@': charToDraw = 195; break;
                    case '$': charToDraw = 180; break;
                    case 'X': charToDraw = 197; break;
                    case '!': charToDraw = 186; break;
                    case '=': charToDraw = 205; break;
                    case '~': charToDraw = 209; break;
                    case '`': charToDraw = 201; break;
                    case '%': charToDraw = 187; break;
                    case '<': charToDraw = 200; break;
                    case '&': charToDraw = 207; break;
                    case '>': charToDraw = 188; break;
                    case '?': charToDraw = keyChar; break;
                }
            } else {
                if (ch == '?') charToDraw = keyChar;
            }
            setCharacter(row + 1, col + 1, charToDraw);
        }
    }
}

void updateSynthDisplayValues() {
    char buffer[4];
    sprintf(buffer, "%+02d", octave);
    setCharacter(22, 24, buffer[0]);
    setCharacter(22, 25, buffer[1]);
    sprintf(buffer, "%+02d", transpose);
    setCharacter(22, 55, buffer[0]);
    setCharacter(22, 56, buffer[1]);
}

void updateSettingsDisplay() {
	char buffer[5];
	sprintf(buffer, "%-3d", temp_decay_time);
	writeString(5, 34, buffer);
	writeString(7, 34, temp_key_labels ? "ON " : "OFF");
	sprintf(buffer, "%+02d", temp_default_octave);
	writeString(9, 34, buffer);
	writeString(11, 34, temp_draw_key_animations ? "ON " : "OFF");
	writeString(13, 34, temp_use_ansi_ui ? "ON " : "OFF");
}

void redrawScreen() {
	hide_mouse();
	clrscr();
	const char** layout;
	bool is_ansi_mode;
	switch(current_screen) {
		case SCR_SYNTH:
			is_ansi_mode = use_ansi_ui;
			layout = is_ansi_mode ? ui_layout_ansi : ui_layout_box;
			drawUIFromLayout(layout, is_ansi_mode);
			setCharacter(22, 65, '>'); setCharacter(22, 34, '>');
			setCharacter(22, 61, '<'); setCharacter(22, 30, '<');
			updateSynthDisplayValues();
			drawKeyLabels();
			break;
		case SCR_SETTINGS:
			is_ansi_mode = temp_use_ansi_ui;
			layout = is_ansi_mode ? ui_settings_ansi : ui_settings_box;
			drawUIFromLayout(layout, is_ansi_mode);
			updateSettingsDisplay();
			break;
		case SCR_HELP:
			is_ansi_mode = use_ansi_ui;
			layout = is_ansi_mode ? ui_help_ansi : ui_help_box;
			drawUIFromLayout(layout, is_ansi_mode);
			break;
	}
	show_mouse();
	gotoxy(1,1);
}

void applyPitchShift() {
    int total_semitone_shift = (octave * 12) + transpose;
    float pitch_factor = pow(2.0, total_semitone_shift / 12.0);
    for (int i = 0; i < 13; i++) {
        notes[i] = base_notes[i] * pitch_factor;
    }
}


// --- Mouse and Program Initialization ---

void show_mouse() {
	union REGS regs;
	regs.x.ax = SHOW_MOUSE;
	int86(MOUSE_INT, &regs, &regs);
}

void hide_mouse() {
	union REGS regs;
	regs.x.ax = HIDE_MOUSE;
	int86(MOUSE_INT, &regs, &regs);
}

int init_mouse(){
	union REGS regs;
	regs.x.ax = INIT_MOUSE;
	int86(MOUSE_INT, &regs, &regs);
	show_mouse();
	return 0xFFFF == regs.x.ax;
}

void get_mouse(int *x, int *y, int *left, int *right){
	union REGS regs;
	regs.x.ax = GET_MOUSE_STATUS;
	int86(MOUSE_INT, &regs, &regs);
	*x = regs.x.cx / 8;
	*y = regs.x.dx / 8;
	*left = regs.x.bx & 0x1;
	*right = regs.x.bx & 0x2;
}

void enterSettingsScreen() {
	// Copy current settings to temp variables to allow for cancellation
	temp_decay_time = decay_time;
	temp_key_labels = key_labels;
	temp_draw_key_animations = draw_key_animations;
	temp_default_octave = default_octave;
	temp_use_ansi_ui = use_ansi_ui;
	current_screen = SCR_SETTINGS;
	redrawScreen();
}

void saveSettings() {
	// Copy temp variables back to main settings
	decay_time = temp_decay_time;
	key_labels = temp_key_labels;
	draw_key_animations = temp_draw_key_animations;
	default_octave = temp_default_octave;
	octave = default_octave; // Reset current octave to new default
	use_ansi_ui = temp_use_ansi_ui;
	applyPitchShift(); // Recalculate notes based on new octave
}


// --- Main Input Loop ---

void mainLoop(){
	char kc = 0;
	int mx, my, left, right;
	bool left_button_prev_state = 0;
	bool running = 1;

	while(running){
		get_mouse(&mx, &my, &left, &right);

		// --- Global Mouse Handling (Top Bar) ---
		if (left && !left_button_prev_state) {
			if (my == 1) { // Top bar clicked
				if (mx >= 76 && mx <= 78) { running = 0; } // Close button
				else if (mx >= 17 && mx <= 26) { // Settings button
					if (current_screen != SCR_SETTINGS) enterSettingsScreen();
				} else if (mx >= 28 && mx <= 33) { // Help button
					if (current_screen != SCR_HELP) {
						current_screen = SCR_HELP;
						redrawScreen();
					}
				}
			}
		}

		// --- Screen-specific Input Handling ---
		switch(current_screen) {
			case SCR_SYNTH:
				if(kbhit()){
					kc = getch();
					if (kc == 0) { // Extended key (e.g., F-keys, Arrow keys)
						kc = getch(); // Get the scan code
						switch(kc) {
							case 59: // F1
								current_screen = SCR_HELP;
								redrawScreen();
								break;
							case 60: // F2
								enterSettingsScreen();
								break;
							case 72: // Up Arrow
								if (octave < 4) {
									octave++;
									applyPitchShift();
									updateSynthDisplayValues();
								}
								break;
							case 80: // Down Arrow
								if (octave > -4) {
									octave--;
									applyPitchShift();
									updateSynthDisplayValues();
								}
								break;
							case 75: // Left Arrow
								if (transpose > -8) {
									transpose--;
									applyPitchShift();
									updateSynthDisplayValues();
								}
								break;
							case 77: // Right Arrow
								if (transpose < 8) {
									transpose++;
									applyPitchShift();
									updateSynthDisplayValues();
								}
								break;
						}
					} else { // Normal key
						if (kc == 27) { // ESC
							running = 0;
						}
						// Note playing logic
						switch(kc){
							case 'c': sound(notes[0]); drawOnAnimations(0); delay(decay_time); nosound(); drawOffAnimations(0); break;
							case 'v': sound(notes[2]); drawOnAnimations(1); delay(decay_time); nosound(); drawOffAnimations(1); break;
							case 'b': sound(notes[4]); drawOnAnimations(2); delay(decay_time); nosound(); drawOffAnimations(2); break;
							case 'n': sound(notes[5]); drawOnAnimations(3); delay(decay_time); nosound(); drawOffAnimations(3); break;
							case 'm': sound(notes[7]); drawOnAnimations(4); delay(decay_time); nosound(); drawOffAnimations(4); break;
							case ',': sound(notes[9]); drawOnAnimations(5); delay(decay_time); nosound(); drawOffAnimations(5); break;
							case '.': sound(notes[11]); drawOnAnimations(6); delay(decay_time); nosound(); drawOffAnimations(6); break;
							case 'f': sound(notes[1]); drawOnAnimations(8); delay(decay_time); nosound(); drawOffAnimations(8); break;
							case 'g': sound(notes[3]); drawOnAnimations(9); delay(decay_time); nosound(); drawOffAnimations(9); break;
							case 'j': sound(notes[6]); drawOnAnimations(10); delay(decay_time); nosound(); drawOffAnimations(10); break;
							case 'k': sound(notes[8]); drawOnAnimations(11); delay(decay_time); nosound(); drawOffAnimations(11); break;
							case 'l': sound(notes[10]); drawOnAnimations(12); delay(decay_time); nosound(); drawOffAnimations(12); break;
							case '/': sound(notes[12]); drawOnAnimations(7); delay(decay_time); nosound(); drawOffAnimations(7); break;
							default: nosound(); break;
						}
					}
				}
				if (left && !left_button_prev_state) {
					bool changed = 0;
					if (my == 21) { // Octave/Transpose buttons
						if (mx >= 28 && mx <= 30) { // Octave down
							if (octave > -4) { octave--; changed = 1; }
						}
						if (mx >= 32 && mx <= 34) { // Octave up
							if (octave < 4) { octave++; changed = 1; }
						}
						if (mx >= 59 && mx <= 61) { // Transpose down
							if (transpose > -8) { transpose--; changed = 1; }
						}
						if (mx >= 63 && mx <= 65) { // Transpose up
							if (transpose < 8) { transpose++; changed = 1; }
						}
					}
					if(changed) {
						applyPitchShift();
						updateSynthDisplayValues();
						delay(100);
					}

                    // Piano key clicks
                    int sound_note = -1;
                    int anim_note = -1;

                    // Check for white key clicks
                    if (my >= 13 && my <= 16) {
                        if (mx >= 16 && mx <= 20)      { sound_note = 0; anim_note = 0; } // C
                        else if (mx >= 22 && mx <= 26) { sound_note = 2; anim_note = 1; } // D
                        else if (mx >= 28 && mx <= 32) { sound_note = 4; anim_note = 2; } // E
                        else if (mx >= 34 && mx <= 38) { sound_note = 5; anim_note = 3; } // F
                        else if (mx >= 40 && mx <= 44) { sound_note = 7; anim_note = 4; } // G
                        else if (mx >= 46 && mx <= 50) { sound_note = 9; anim_note = 5; } // A
                        else if (mx >= 52 && mx <= 56) { sound_note = 11; anim_note = 6; } // B
                        else if (mx >= 58 && mx <= 62) { sound_note = 12; anim_note = 7; } // C6
                    }
                    // Check for black key clicks and white key clicks but inline with black keys
                    else if (my >= 6 && my <= 12) {
                        if (mx >= 20 && mx <= 22)      { sound_note = 1; anim_note = 8; } // C#
                        else if (mx >= 26 && mx <= 28) { sound_note = 3; anim_note = 9; } // D#
                        else if (mx >= 38 && mx <= 40) { sound_note = 6; anim_note = 10; } // F#
                        else if (mx >= 44 && mx <= 46) { sound_note = 8; anim_note = 11; } // G#
                        else if (mx >= 50 && mx <= 52) { sound_note = 10; anim_note = 12; } // A#
						else if (mx >= 16 && mx <= 19) { sound_note = 0; anim_note = 0; } // C
						else if (mx >= 23 && mx <= 25) { sound_note = 2; anim_note = 1; } // D
						else if (mx >= 28 && mx <= 32) { sound_note = 4; anim_note = 2; } // E
						else if (mx >= 34 && mx <= 38) { sound_note = 5; anim_note = 3; } // F
						else if (mx >= 40 && mx <= 44) { sound_note = 7; anim_note = 4; } // G
						else if (mx >= 46 && mx <= 50) { sound_note = 9; anim_note = 5; } // A
						else if (mx >= 52 && mx <= 56) { sound_note = 11; anim_note = 6; } // B
						else if (mx >= 58 && mx <= 62) { sound_note = 12; anim_note = 7; } // C6
                    }

                    if (sound_note != -1) {
                        sound(notes[sound_note]);
                        drawOnAnimations(anim_note);
                        delay(decay_time);
                        nosound();
                        drawOffAnimations(anim_note);
                    }
				}
				break;

			case SCR_SETTINGS:
				if (kbhit() && getch() == 27) running = 0;
				if (left && !left_button_prev_state) {
					bool settings_changed = 0;
					
					// Decay Time Controls
					if (my == 4) {
						if (mx == 30) { // Minus button
							if (temp_decay_time > 100) temp_decay_time -= 10;
							settings_changed = 1;
						} else if (mx == 38) { // Plus button
							if (temp_decay_time < 900) temp_decay_time += 10;
							settings_changed = 1;
						} else if (mx >= 32 && mx <= 36) { // Click on value to reset
							temp_decay_time = 150;
							settings_changed = 1;
						}
					}
					
					// Default Octave Controls
					if (my == 8) {
						if (mx == 30) { // Minus button
							if (temp_default_octave > -4) temp_default_octave--;
							settings_changed = 1;
						} else if (mx == 38) { // Plus button
							if (temp_default_octave < 4) temp_default_octave++;
							settings_changed = 1;
						} else if (mx >= 32 && mx <= 36) { // Click on value to reset
							temp_default_octave = 0;
							settings_changed = 1;
						}
					}
					
					// Toggle Controls
					if (my == 6 && mx >= 33 && mx <= 37) { temp_key_labels = !temp_key_labels; settings_changed = 1; }
					if (my == 10 && mx >= 33 && mx <= 37) { temp_draw_key_animations = !temp_draw_key_animations; settings_changed = 1; }
					if (my == 12 && mx >= 33 && mx <= 37) {
						temp_use_ansi_ui = !temp_use_ansi_ui;
						redrawScreen();
					}
					
					// Buttons
					if (my == 5 && (mx >= 51 && mx <= 68)) { // Close Settings
						current_screen = SCR_SYNTH;
						redrawScreen();
					} else if (my == 21) {
						if (mx >= 25 && mx <= 34) { // Save
							saveSettings();
							current_screen = SCR_SYNTH;
							redrawScreen();
						} else if (mx >= 39 && mx <= 48) { // Cancel
							current_screen = SCR_SYNTH;
							redrawScreen();
						}
					}
					
					if (settings_changed) updateSettingsDisplay();
				}
				break;

			case SCR_HELP:
				if (kbhit() && getch() == 27) { running = 0; }
				if (left && !left_button_prev_state) {
					if (my == 22 && (mx >= 32 && mx <= 49)) { // Close Help button
						current_screen = SCR_SYNTH;
						redrawScreen();
					}
				}
				break;
		}
		left_button_prev_state = left;
		delay(10);
	}
}

int main() {
	clrscr();
	octave = default_octave;
	applyPitchShift();
	setTextMode(3);
    redrawScreen();
	init_mouse();
	mainLoop();
	nosound();
	clrscr();
	setTextMode(3);
	return 0;
}
