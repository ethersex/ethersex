#ifndef PWM_TETRIS_H
#define PWM_TETRIS_H

/*
 * The adapted melody is in the public domain.
 */
const struct notes_duration_t tetris_notes[] PROGMEM =
{ 
    {C,VIERTEL}, {D,VIERTEL}, {E,VIERTEL}, {E,VIERTEL},
    {G,HALB},{G,HALB},
    {A,VIERTEL}, {A,VIERTEL}, {A,VIERTEL}, {A,VIERTEL},
    {G,DREIVIERTEL},{p,VIERTEL}, {p,  1}, {p,  1},
    {A,VIERTEL}, {A,VIERTEL}, {A,VIERTEL}, {A,VIERTEL},
    {G,DREIVIERTEL},{p,VIERTEL}, {p,  1}, {p,  1},
    {F,VIERTEL}, {F,VIERTEL}, {F,VIERTEL}, {F,VIERTEL},
    {E,HALB},{E,HALB},
    {D,VIERTEL}, {D,VIERTEL}, {D,VIERTEL}, {D,VIERTEL},
    {C,DREIVIERTEL},{p,VIERTEL}, {p,  1}, {p,  1}
};

#endif /* PWM_TETRIS_H */
