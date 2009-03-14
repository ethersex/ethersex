static void
stella_fade_normal (int i)
{
	if (stella_color[i] > stella_fade[i])
		stella_color[i] --;
	else /* stella_color[i] < stella_fade[i] */
		stella_color[i] ++;
}


static void
stella_fade_flashy (int i)
{
	if (stella_color[i] > stella_fade[i])
		stella_color[i] <<= 1;
	
	if (stella_color[i] < stella_fade[i])
		stella_color[i] = stella_fade[i];
}


static struct
{
	void (* p) (int i);
} stella_fade_funcs[FADE_FUNC_LEN] =
{
	{ stella_fade_normal },
	{ stella_fade_flashy },
};
