int strnicmp (const char *s1, const char *s2, int maxlen)
{
	char c1, c2;
	do {
		c1=*s1++;
		if (c1>='a' && c1<='z') c1&=0xDF;
		c2=*s2++;
		if (c2>='a' && c2<='z') c2&=0xDF;
		if (c1==c2) maxlen--;
	} while (maxlen && c1 && c2 && c1==c2);
	if ((!c1 && !c2) || !maxlen) return 0;
	if ((!c1 && c2) || (c1<c2)) return -1;
	return 1;
}
