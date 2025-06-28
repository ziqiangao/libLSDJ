#include <iostream>
#include <sav.h> // Ensure you have the correct path for the header
#include <vio.h> // Same as above
#include <project.h>
#include <instrument.h>
#include <speech.h>
#include <synth.h>
#include <unordered_set>
#include <wave.h>
#include <phrase.h>
#include <chain.h>
#include <groove.h>
#include <table.h>
#include <cstring>


void roll_left_by_one(char *str)
{
    int len = strlen(str);
    if (len <= 1)
        return; // nothing to do

    char first = str[0];
    for (int i = 0; i < len - 1; i++)
    {
        str[i] = str[i + 1];
    }
    str[len - 1] = first;
}

const char *typenumtostring(lsdj_instrument_type_t type)
{
    switch (type)
    {
    case LSDJ_INSTRUMENT_TYPE_PULSE:
        return "PULSE";
        break;
    case LSDJ_INSTRUMENT_TYPE_WAVE:
        return "WAVE";
        break;
    case LSDJ_INSTRUMENT_TYPE_KIT:
        return "KIT";
        break;
    case LSDJ_INSTRUMENT_TYPE_NOISE:
        return "NOISE";
        break;

    default:
        return "";
        break;
    }
}

const char *allophones[] = {
    "AA-", "AE-", "AO-", "AR-", "AW-", "AX-", "AY-",
    "BB1", "BB2", "CH-", "DD1", "DD2", "DH1", "DH2",
    "EH-", "EL-", "ER1", "ER2", "EY-", "FF-", "GG1",
    "GG2", "GG3", "HH1", "HH2", "IH-", "IY-", "JH-",
    "KK1", "KK2", "KK3", "LL-", "MM-", "NG-", "NN1",
    "NN2", "OR-", "OW-", "OY-", "PP-", "RR1", "RR2",
    "SH-", "SS-", "TH-", "TT1", "TT2", "UH-", "UW1",
    "UW2", "VV-", "WH-", "WW-", "XR-", "YR-", "YY1",
    "YY2", "ZH-", "ZZ-"};

const char *commands[] = {
    "-", // LSDJ_COMMAND_NONE = 0
    "B", // LSDJ_COMMAND_A
    "C", // LSDJ_COMMAND_C
    "D", // LSDJ_COMMAND_D
    "E", // LSDJ_COMMAND_E
    "F", // LSDJ_COMMAND_F
    "G", // LSDJ_COMMAND_G
    "H", // LSDJ_COMMAND_H
    "K", // LSDJ_COMMAND_K
    "L", // LSDJ_COMMAND_L
    "M", // LSDJ_COMMAND_M
    "O", // LSDJ_COMMAND_O
    "P", // LSDJ_COMMAND_P
    "R", // LSDJ_COMMAND_R
    "S", // LSDJ_COMMAND_S
    "T", // LSDJ_COMMAND_T
    "V", // LSDJ_COMMAND_V
    "W", // LSDJ_COMMAND_W
    "Z", // LSDJ_COMMAND_Z
    "N", // LSDJ_COMMAND_ARDUINO_BOY_N
    "X", // LSDJ_COMMAND_ARDUINO_BOY_X
    "Q", // LSDJ_COMMAND_ARDUINO_BOY_Q
    "Y", // LSDJ_COMMAND_ARDUINO_BOY_Y
    "A"  // LSDJ_COMMAND_B (added in 7.1.0)
};

void print_chain(uint8_t value)
    {
        if (value == 0xFF)
            printf("--");
        else
            printf("%02X", value);
    }

std::unordered_set<int> usedsynths = {};
std::unordered_set<int> usedwaves = {};
std::unordered_set<int> usedtables = {};

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file> > <output_file>\n";
        return 1;
    }

    const char *inputFile = argv[1];

    // Allocate memory for the sav object
    lsdj_sav_t *sav = nullptr;

    // Read the .sav file into the allocated object
    if (lsdj_sav_read_from_file(inputFile, &sav, nullptr) != 0)
    {
        std::cerr << "Failed to read input file: " << inputFile << "\n";
        return 1;
    }

    // Correct way to print the active project index
    int activeProjectIndex = lsdj_sav_get_active_project_index(sav);
    printf("Active project index: %d\n", activeProjectIndex);

    // Retrieve the active project
    auto project = lsdj_sav_get_project(sav, activeProjectIndex);

    // Get the name of the project and its length
    const char *projectName = lsdj_project_get_name(project);
    size_t projectNameLength = lsdj_project_get_name_length(project);

    // Print the project name (ensuring it's properly null-terminated)
    if (projectName && projectNameLength > 0)
    {
        printf("Project name: %.*s\n", static_cast<int>(projectNameLength), projectName);
    }
    else
    {
        printf("Project name: <Unnamed>\n");
    }

    auto song = lsdj_project_get_song(project);

    printf("Version: %i\n", lsdj_project_get_version(project));
    printf("Tempo: %i\n", lsdj_song_get_tempo(song));
    printf("Clone Mode: %s\n", lsdj_song_get_clone_mode(song) ? "SLIM" : "DEEP");
    printf("Transpose: %d\n", (int8_t)lsdj_song_get_transposition(song));
    printf("Font: %i\n", lsdj_song_get_font(song));
    printf("Theme: %i\n", lsdj_song_get_color_palette(song));

    printf("\n");

    printf("Instruments:");
    for (int i = 0; i < LSDJ_INSTRUMENT_COUNT - 1; i++)
    {
        if (lsdj_instrument_is_allocated(song, i))
        {
            printf("\n  Instrument #%02X \"%s\":\n", i, lsdj_instrument_get_name(song, i));
            auto type = lsdj_instrument_get_type(song, i);
            printf("    Type: %s\n", typenumtostring(type));
            auto panning = lsdj_instrument_get_panning(song, i);
            lsdj_wave_play_mode_t playmode = LSDJ_INSTRUMENT_WAVE_PLAY_MANUAL;
            switch (type)
            {
            case LSDJ_INSTRUMENT_TYPE_PULSE:
                printf("    Envelope: %X,%X/%X,%X/%X,%X\n", lsdj_instrument_adsr_get_initial_level(song, i), lsdj_instrument_adsr_get_attack_speed(song, i), lsdj_instrument_adsr_get_attack_level(song, i), lsdj_instrument_adsr_get_decay_speed(song, i), lsdj_instrument_adsr_get_sustain_level(song, i), lsdj_instrument_adsr_get_release_speed(song, i));
                printf("    Duty: %i\n", lsdj_instrument_pulse_get_pulse_width(song, i));
                printf("    Output: %s%s\n", panning & 1 ? "L" : "", panning & 2 ? "R" : "");
                printf("    Vibrato: %i, %i, %i\n", lsdj_instrument_get_plv_speed(song, i), lsdj_instrument_get_vibrato_shape(song, i), lsdj_instrument_get_vibrato_direction(song, i));
                printf("    Transpose: %s\n", lsdj_instrument_get_transpose(song, i) ? "Yes" : "No");
                printf("    Length: %i\n", lsdj_instrument_pulse_get_length(song, i));
                printf("    CMD/Rate: %i\n", lsdj_instrument_get_command_rate(song, i));
                if (lsdj_instrument_is_table_enabled(song, i))
                {
                    int t = lsdj_instrument_get_table(song, i);
                    printf("    Table Mode: %i\n", lsdj_instrument_get_table_mode(song, i));
                    printf("    Table: %X\n", t);
                    usedtables.insert(t);
                }
                printf("    PU2 Transpose: %d\n", (int8_t)lsdj_instrument_pulse_get_pulse2_tune(song, i));
                printf("    Finetune: %X\n", lsdj_instrument_pulse_get_finetune(song, i));
                break;
            case LSDJ_INSTRUMENT_TYPE_WAVE:
                printf("    Volume: %i\n", lsdj_instrument_wave_get_volume(song, i));
                printf("    Output: %s%s\n", panning & 1 ? "L" : "", panning & 2 ? "R" : "");
                printf("    Vibrato: %i, %i, %i\n", lsdj_instrument_get_plv_speed(song, i), lsdj_instrument_get_vibrato_shape(song, i), lsdj_instrument_get_vibrato_direction(song, i));
                printf("    Transpose: %s\n", lsdj_instrument_get_transpose(song, i) ? "Yes" : "No");
                printf("    CMD/Rate: %i\n", lsdj_instrument_get_command_rate(song, i));
                if (lsdj_instrument_is_table_enabled(song, i))
                {
                    int t = lsdj_instrument_get_table(song, i);
                    printf("    Table Mode: %i\n", lsdj_instrument_get_table_mode(song, i));
                    printf("    Table: %X\n", t);
                    usedtables.insert(t);
                }
                playmode = lsdj_instrument_wave_get_play_mode(song, i);
                printf("    Play Mode: %i\n", playmode);
                if (playmode == LSDJ_INSTRUMENT_WAVE_PLAY_MANUAL)
                {
                    auto used = lsdj_instrument_wave_get_wave(song, i);
                    usedwaves.insert(used);
                    printf("    Wave: %i\n", used);
                }
                else
                {
                    auto used = lsdj_instrument_wave_get_synth(song, i);
                    usedsynths.insert(used);
                    for (uint8_t k = 0;k<16;k++) {
                        usedwaves.insert((used<<4)+k);
                    }
                    printf("    Synth: %i\n", used);
                    printf("    Speed: %i\n", lsdj_instrument_wave_get_speed(song, i));
                    printf("    Length: %i\n", lsdj_instrument_wave_get_length(song, i));
                }
                break;
            case LSDJ_INSTRUMENT_TYPE_KIT:
                printf("    Volume: %i\n", lsdj_instrument_wave_get_volume(song, i));
                printf("    Output: %s%s\n", panning & 1 ? "L" : "", panning & 2 ? "R" : "");
                printf("    Vibrato: %i, %i, %i\n", lsdj_instrument_get_plv_speed(song, i), lsdj_instrument_get_vibrato_shape(song, i), lsdj_instrument_get_vibrato_direction(song, i));
                printf("    Pitch: %d\n", (int8_t)lsdj_instrument_kit_get_pitch(song, i));
                if (lsdj_instrument_is_table_enabled(song, i))
                {
                    int t = lsdj_instrument_get_table(song, i);
                    printf("    Table Mode: %i\n", lsdj_instrument_get_table_mode(song, i));
                    printf("    Table: %X\n", t);
                    usedtables.insert(t);
                }
                printf("    Kit: %X/%X\n", lsdj_instrument_kit_get_kit1(song, i), lsdj_instrument_kit_get_kit2(song, i));
                printf("    Offset: %X/%X\n", lsdj_instrument_kit_get_offset1(song, i), lsdj_instrument_kit_get_offset2(song, i));
                printf("    Length: %X/%X\n", lsdj_instrument_kit_get_length1(song, i), lsdj_instrument_kit_get_length2(song, i));
                printf("    Loop Mode: %i/%i\n", lsdj_instrument_kit_get_loop1(song, i), lsdj_instrument_kit_get_loop2(song, i));
                printf("    Half Speed: %s\n", lsdj_instrument_kit_get_half_speed(song, i) ? "Yes" : "No");
                printf("    Clipping Mode: %i\n", lsdj_instrument_kit_get_distortion_mode(song, i));
                break;
            case LSDJ_INSTRUMENT_TYPE_NOISE:
                printf("    Envelope: %X,%X/%X,%X/%X,%X\n", lsdj_instrument_adsr_get_initial_level(song, i), lsdj_instrument_adsr_get_attack_speed(song, i), lsdj_instrument_adsr_get_attack_level(song, i), lsdj_instrument_adsr_get_decay_speed(song, i), lsdj_instrument_adsr_get_sustain_level(song, i), lsdj_instrument_adsr_get_release_speed(song, i));
                printf("    Output: %s%s\n", panning & 1 ? "L" : "", panning & 2 ? "R" : "");
                printf("    Vibrato: %i, %i\n", lsdj_instrument_get_vibrato_shape(song, i), lsdj_instrument_get_vibrato_direction(song, i));
                printf("    Pitch: %s\n", lsdj_instrument_noise_get_stability(song, i) ? "STABLE" : "FREE");
                printf("    Length: %i\n", lsdj_instrument_noise_get_length(song, i));
                printf("    CMD/Rate: %i\n", lsdj_instrument_get_command_rate(song, i));
                if (lsdj_instrument_is_table_enabled(song, i))
                {
                    int t = lsdj_instrument_get_table(song, i);
                    printf("    Table Mode: %i\n", lsdj_instrument_get_table_mode(song, i));
                    printf("    Table: %X\n", t);
                    usedtables.insert(t);
                }
                printf("    Transpose: %s\n", lsdj_instrument_get_transpose(song, i) ? "Yes" : "No");
                break;
            default:
                break;
            }
        }
    }

    if (lsdj_instrument_is_allocated(song, 0x40))
    {
        printf("\nSpeech:\n");
        for (int i = 0; i < LSDJ_SPEECH_WORD_COUNT; i++)
        {
            if (lsdj_speech_get_word_allophone(song, i, 1) != LSDJ_SPEECH_WORD_NO_ALLOPHONE_VALUE)
            {
                printf("  Word %.3s:\n", lsdj_speech_get_word_name(song, i) - i);
                int j = 0;
                while (true)
                {
                    if (lsdj_speech_get_word_allophone(song, i, j) == LSDJ_SPEECH_WORD_NO_ALLOPHONE_VALUE)
                        break;
                    printf("    %X|%s|%02X\n", j >> 1, allophones[(lsdj_speech_get_word_allophone(song, i, j)) - 1], lsdj_speech_get_word_allophone(song, i, j + 1));
                    j += 2;
                }
            }
        }
    }

    if (!usedsynths.empty())
        printf("\nSynths:\n");
    for (int i = 0; i < LSDJ_SYNTH_COUNT; i++)
    {
        if (usedsynths.count(i))
        {
            printf("  Synth %X:\n", i);
            printf("    Waveform: %i\n", lsdj_synth_get_waveform(song, i));
            printf("    Filter: %i\n", lsdj_synth_get_filter(song, i));
            printf("    Distortion: %i\n", lsdj_synth_get_distortion(song, i));
            printf("    Phase Compression: %i\n", lsdj_synth_get_phase_compression(song, i));

            printf("    Volume: %02X/%02X\n", lsdj_synth_get_volume_start(song, i), lsdj_synth_get_volume_end(song, i));
            printf("    Cutoff: %02X/%02X\n", lsdj_synth_get_cutoff_start(song, i), lsdj_synth_get_cutoff_end(song, i));
            printf("    Resonance: %X/%X\n", lsdj_synth_get_resonance_start(song, i), lsdj_synth_get_resonance_end(song, i));
            printf("    VShift: %02X/%02X\n", lsdj_synth_get_vshift_start(song, i), lsdj_synth_get_vshift_end(song, i));
            printf("    Limit: %02X/%02X\n", lsdj_synth_get_limit_start(song, i), lsdj_synth_get_limit_end(song, i));
            printf("    Phase: %02X/%02X\n", lsdj_synth_get_phase_start(song, i), lsdj_synth_get_phase_end(song, i));
        }
    }

    if (!usedwaves.empty())
        printf("\nWaves:\n");

    for (int i = 0; i < LSDJ_WAVE_COUNT; i++)
    {
        if (usedwaves.count(i))
        {
            printf("  Wave %02X:\n    ", i);

            uint8_t *data = lsdj_wave_get_bytes(song, i);

            // First build the string
            char waveStr[33]; // 32 hex chars + null terminator
            for (int j = 0; j < 16; j++)
            {
                uint8_t byte = data[j];
                uint8_t high = 0xF - ((byte & 0xF0) >> 4);
                uint8_t low = 0xF - (byte & 0x0F);
                waveStr[j * 2] = "0123456789ABCDEF"[high];
                waveStr[j * 2 + 1] = "0123456789ABCDEF"[low];
            }
            waveStr[32] = '\0';

            // Roll string left by one character
            char first = waveStr[0];
            memmove(waveStr, waveStr + 1, 31); // shift left 31 characters
            waveStr[31] = first;
            waveStr[32] = '\0';

            printf("%s\n\n", waveStr);
        }
    }

    printf("\nPhrases:\n");
    for (int i = 0; i < LSDJ_PHRASE_COUNT; i++)
    {
        if (lsdj_phrase_is_allocated(song, i))
        {
            printf("  Phrase %02X:\n", i);
            for (int j = 0; j < LSDJ_PHRASE_LENGTH; j++)
            {
                printf("    ");
                auto note = lsdj_phrase_get_note(song, i, j);
                auto instrument = lsdj_phrase_get_instrument(song, i, j);
                auto command = lsdj_phrase_get_command(song, i, j);
                if (note == LSDJ_PHRASE_NO_NOTE)
                    printf("--|");
                else
                    printf("%02X|", note);
                if (instrument == LSDJ_PHRASE_NO_INSTRUMENT)
                    printf("--|");
                else
                    printf("%02X|", instrument);
                if (command == LSDJ_COMMAND_NONE)
                    printf("---\n");
                else
                    printf("%s%02X\n", commands[command], lsdj_phrase_get_command_value(song, i, j));
            }
            printf("\n");
        }
    }

    printf("\nChains:\n");
    for (int i = 0; i < LSDJ_CHAIN_COUNT; i++)
    {
        if (lsdj_chain_is_allocated(song, i))
        {
            printf("  Chain %02X:\n", i);
            for (int j = 0; j < LSDJ_CHAIN_LENGTH; j++)
            {
                printf("    ");
                auto phrase = lsdj_chain_get_phrase(song, i, j);
                if (phrase == LSDJ_CHAIN_NO_PHRASE)
                    break;
                else
                    printf("%02X|", phrase);
                printf("%02X\n", lsdj_chain_get_transposition(song, i, j));
            }
            printf("\n");
        }
    }

    

    printf("\nSong:\n");
    for (int i = 0; i < 192; i++)
    {
        uint8_t p1 = lsdj_row_get_chain(song, i, LSDJ_CHANNEL_PULSE1);
        uint8_t p2 = lsdj_row_get_chain(song, i, LSDJ_CHANNEL_PULSE2);
        uint8_t w = lsdj_row_get_chain(song, i, LSDJ_CHANNEL_WAVE);
        uint8_t n = lsdj_row_get_chain(song, i, LSDJ_CHANNEL_NOISE);

        // Stop if all are 0xFF
        if (p1 == 0xFF && p2 == 0xFF && w == 0xFF && n == 0xFF)
            break;

        printf("  ");
        print_chain(p1);
        printf("|");
        print_chain(p2);
        printf("|");
        print_chain(w);
        printf("|");
        print_chain(n);
        printf("\n");
    }


    if (lsdj_groove_get_step(song,0,0) != 6 && lsdj_groove_get_step(song,0,1) != 6) printf("\nGrooves:\n");
    for (int i = 0; i < LSDJ_GROOVE_COUNT; i++)
    {
        if (lsdj_groove_get_step(song,i,0) != 6 && lsdj_groove_get_step(song,i,1) != 6)
        {
            printf("  Groove %02X:\n", i);
            for (int j = 0; j < LSDJ_GROOVE_LENGTH; j++)
            {
                printf("    ");
                auto groove = lsdj_groove_get_step(song, i, j);
                if (groove == LSDJ_GROOVE_NO_VALUE)
                    break;
                else
                    printf("%02X\n", groove);
            }
            printf("\n");
        }
    }


    if (!usedtables.empty()) printf("\nTables:\n");
    for (int i = 0; i < LSDJ_TABLE_COUNT; i++)
    {
        if (usedtables.count(i))
        {
            printf("  Table %02X:\n", i);
            for (int j = 0; j < LSDJ_TABLE_LENGTH; j++)
            {
                printf("    ");

                printf("%02X|",lsdj_table_get_envelope(song,i,j));
                printf("%02X|",lsdj_table_get_transposition(song,i,j));

                auto command1 = lsdj_table_get_command1(song,i,j);
                auto command2 = lsdj_table_get_command2(song,i,j);
                if (command1 == LSDJ_COMMAND_NONE)
                    printf("---|");
                else
                    printf("%s%02X|", commands[command1], lsdj_table_get_command1_value(song, i, j));
                if (command2 == LSDJ_COMMAND_NONE)
                    printf("---\n");
                else
                    printf("%s%02X\n", commands[command2], lsdj_table_get_command2_value(song, i, j));
            }
            printf("\n");
        }
    }

    // Clean up
    lsdj_sav_free(sav);

    return 0;
}
