#include "stdafx.h"
#include "shabal.h"

// context for 1-dimensional shabal (32bit)
sph_shabal_context global_32;
// context for 4-dimensional shabal (128bit)
mshabal_context global_128;
mshabal_context_fast global_128_fast;
// context for 8-dimensional shabal (256bit)
mshabal256_context global_256;
mshabal256_context_fast global_256_fast;

//AVX2
void procscoop_avx2(unsigned long long const nonce, unsigned long long const n, char const *const data, size_t const acc, const std::string &file_name) {
	char const *cache;
	char sig0[32 + 64 + 32];
	char sig1[32 + 64 + 32];
	char sig2[32 + 64 + 32];
	char sig3[32 + 64 + 32];
	char sig4[32 + 64 + 32];
	char sig5[32 + 64 + 32];
	char sig6[32 + 64 + 32];
	char sig7[32 + 64 + 32];
	char res0[32];
	char res1[32];
	char res2[32];
	char res3[32];
	char res4[32];
	char res5[32];
	char res6[32];
	char res7[32];
	cache = data;
	unsigned long long v;

	memmove(sig0, signature, 32);
	memmove(sig1, signature, 32);
	memmove(sig2, signature, 32);
	memmove(sig3, signature, 32);
	memmove(sig4, signature, 32);
	memmove(sig5, signature, 32);
	memmove(sig6, signature, 32);
	memmove(sig7, signature, 32);
	sig0[96] = -128;
	sig1[96] = -128;
	sig2[96] = -128;
	sig3[96] = -128;
	sig4[96] = -128;
	sig5[96] = -128;
	sig6[96] = -128;
	sig7[96] = -128;
	memset(&sig0[97], 0, 31);
	memset(&sig1[97], 0, 31);
	memset(&sig2[97], 0, 31);
	memset(&sig3[97], 0, 31);
	memset(&sig4[97], 0, 31);
	memset(&sig5[97], 0, 31);
	memset(&sig6[97], 0, 31);
	memset(&sig7[97], 0, 31);


	mshabal256_context x;

	for (v = 0; v<n; v += 8) {
		memmove(&sig0[32], &cache[(v + 0) * 64], 64);
		memmove(&sig1[32], &cache[(v + 1) * 64], 64);
		memmove(&sig2[32], &cache[(v + 2) * 64], 64);
		memmove(&sig3[32], &cache[(v + 3) * 64], 64);
		memmove(&sig4[32], &cache[(v + 4) * 64], 64);
		memmove(&sig5[32], &cache[(v + 5) * 64], 64);
		memmove(&sig6[32], &cache[(v + 6) * 64], 64);
		memmove(&sig7[32], &cache[(v + 7) * 64], 64);


		memcpy(&x, &global_256, sizeof(global_256)); // optimization: mshabal256_init(&x, 256);
		avx2_mshabal(&x, (unsigned char*)sig0, (unsigned char*)sig1, (unsigned char*)sig2, (unsigned char*)sig3, (unsigned char*)sig4, (unsigned char*)sig5, (unsigned char*)sig6, (unsigned char*)sig7, 64 + 32);
		avx2_mshabal_close(&x, 0, 0, 0, 0, 0, 0, 0, 0, 0, res0, res1, res2, res3, res4, res5, res6, res7);

		unsigned long long *wertung = (unsigned long long*)res0;
		unsigned long long *wertung1 = (unsigned long long*)res1;
		unsigned long long *wertung2 = (unsigned long long*)res2;
		unsigned long long *wertung3 = (unsigned long long*)res3;
		unsigned long long *wertung4 = (unsigned long long*)res4;
		unsigned long long *wertung5 = (unsigned long long*)res5;
		unsigned long long *wertung6 = (unsigned long long*)res6;
		unsigned long long *wertung7 = (unsigned long long*)res7;
		unsigned posn = 0;
		if (*wertung1 < *wertung)
		{
			*wertung = *wertung1;
			posn = 1;
		}
		if (*wertung2 < *wertung)
		{
			*wertung = *wertung2;
			posn = 2;
		}
		if (*wertung3 < *wertung)
		{
			*wertung = *wertung3;
			posn = 3;
		}
		if (*wertung4 < *wertung)
		{
			*wertung = *wertung4;
			posn = 4;
		}
		if (*wertung5 < *wertung)
		{
			*wertung = *wertung5;
			posn = 5;
		}
		if (*wertung6 < *wertung)
		{
			*wertung = *wertung6;
			posn = 6;
		}
		if (*wertung7 < *wertung)
		{
			*wertung = *wertung7;
			posn = 7;
		}

		if ((*wertung / baseTarget) <= bests[acc].targetDeadline)
		{
			if (*wertung < bests[acc].best)
			{
				Log("\nfound deadline=");	Log_llu(*wertung / baseTarget); Log(" nonce=");	Log_llu(nonce + v + posn); Log(" for account: "); Log_llu(bests[acc].account_id); Log(" file: "); Log((char*)file_name.c_str());
				EnterCriticalSection(&bestsLock);
				bests[acc].best = *wertung;
				bests[acc].nonce = nonce + v + posn;
				bests[acc].DL = *wertung / baseTarget;
				LeaveCriticalSection(&bestsLock);
				EnterCriticalSection(&sharesLock);
				shares.push_back({ file_name, bests[acc].account_id, bests[acc].best, bests[acc].nonce });
				LeaveCriticalSection(&sharesLock);
				if (use_debug)
				{
					char tbuffer[9];
					_strtime_s(tbuffer);
					bm_wattron(2);
					bm_wprintw("%s [%20llu] found DL:      %9llu\n", tbuffer, bests[acc].account_id, bests[acc].DL, 0);
					bm_wattroff(2);
				}
			}
		}
	}
}

//AVX2
void procscoop_avx2_fast(unsigned long long const nonce, unsigned long long const n, char const *const data, size_t const acc, const std::string &file_name) {
	char const *cache;
	char sig0[32];
	char end0[32];
	char res0[32];
	char res1[32];
	char res2[32];
	char res3[32];
	char res4[32];
	char res5[32];
	char res6[32];
	char res7[32];
	cache = data;
	unsigned long long v;

	memmove(sig0, signature, 32);
	end0[0] = -128;
	memset(&end0[1], 0, 31);

	mshabal256_context_fast x;
	mshabal256_context_fast x2;
	memcpy(&x2, &global_256_fast, sizeof(global_256_fast)); // local copy of global fast contex
	
	//prepare shabal inputs
	union {
		mshabal_u32 words[64 * MSHABAL256_FACTOR];
		__m256i data[16];
	} u1,u2;

	for (int j = 0; j < 64 * MSHABAL256_FACTOR / 2; j += 4 * MSHABAL256_FACTOR) {
		size_t o = j / MSHABAL256_FACTOR;
		u1.words[j + 0] = *(mshabal_u32 *)(sig0 + o);
		u1.words[j + 1] = *(mshabal_u32 *)(sig0 + o);
		u1.words[j + 2] = *(mshabal_u32 *)(sig0 + o);
		u1.words[j + 3] = *(mshabal_u32 *)(sig0 + o);
		u1.words[j + 4] = *(mshabal_u32 *)(sig0 + o);
		u1.words[j + 5] = *(mshabal_u32 *)(sig0 + o);
		u1.words[j + 6] = *(mshabal_u32 *)(sig0 + o);
		u1.words[j + 7] = *(mshabal_u32 *)(sig0 + o);
		u2.words[j + 0 + 64] = *(mshabal_u32 *)(end0 + o);
		u2.words[j + 1 + 64] = *(mshabal_u32 *)(end0 + o);
		u2.words[j + 2 + 64] = *(mshabal_u32 *)(end0 + o);
		u2.words[j + 3 + 64] = *(mshabal_u32 *)(end0 + o);
		u2.words[j + 4 + 64] = *(mshabal_u32 *)(end0 + o);
		u2.words[j + 5 + 64] = *(mshabal_u32 *)(end0 + o);
		u2.words[j + 6 + 64] = *(mshabal_u32 *)(end0 + o);
		u2.words[j + 7 + 64] = *(mshabal_u32 *)(end0 + o);
	}

	for (v = 0; v<n; v += 8) {
		//Inititialise Shabal
		memcpy(&x, &x2, sizeof(x2)); // optimization: mshabal256_init(&x, 256);

		//Load and shuffle data 
		//NB: this can be further optimised by preshuffling plot files depending on SIMD length and use avx2 memcpy
		//Did not find a away yet to completely avoid memcpys

		for (int j = 0; j < 64 * MSHABAL256_FACTOR / 2; j += 4 * MSHABAL256_FACTOR) {
			size_t o = j / MSHABAL256_FACTOR;
			u1.words[j + 0 + 64] = *(mshabal_u32 *)(&cache[(v + 0) * 64] + o);
			u1.words[j + 1 + 64] = *(mshabal_u32 *)(&cache[(v + 1) * 64] + o);
			u1.words[j + 2 + 64] = *(mshabal_u32 *)(&cache[(v + 2) * 64] + o);
			u1.words[j + 3 + 64] = *(mshabal_u32 *)(&cache[(v + 3) * 64] + o);
			u1.words[j + 4 + 64] = *(mshabal_u32 *)(&cache[(v + 4) * 64] + o);
			u1.words[j + 5 + 64] = *(mshabal_u32 *)(&cache[(v + 5) * 64] + o);
			u1.words[j + 6 + 64] = *(mshabal_u32 *)(&cache[(v + 6) * 64] + o);
			u1.words[j + 7 + 64] = *(mshabal_u32 *)(&cache[(v + 7) * 64] + o);
			u2.words[j + 0] = *(mshabal_u32 *)(&cache[(v + 0) * 64 + 32] + o);
			u2.words[j + 1] = *(mshabal_u32 *)(&cache[(v + 1) * 64 + 32] + o);
			u2.words[j + 2] = *(mshabal_u32 *)(&cache[(v + 2) * 64 + 32] + o);
			u2.words[j + 3] = *(mshabal_u32 *)(&cache[(v + 3) * 64 + 32] + o);
			u2.words[j + 4] = *(mshabal_u32 *)(&cache[(v + 4) * 64 + 32] + o);
			u2.words[j + 5] = *(mshabal_u32 *)(&cache[(v + 5) * 64 + 32] + o);
			u2.words[j + 6] = *(mshabal_u32 *)(&cache[(v + 6) * 64 + 32] + o);
			u2.words[j + 7] = *(mshabal_u32 *)(&cache[(v + 7) * 64 + 32] + o);
		}

		avx2_mshabal_openclose_fast(&x, &u1, &u2, res0, res1, res2, res3, res4, res5, res6, res7,0);

		unsigned long long *wertung = (unsigned long long*)res0;
		unsigned long long *wertung1 = (unsigned long long*)res1;
		unsigned long long *wertung2 = (unsigned long long*)res2;
		unsigned long long *wertung3 = (unsigned long long*)res3;
		unsigned long long *wertung4 = (unsigned long long*)res4;
		unsigned long long *wertung5 = (unsigned long long*)res5;
		unsigned long long *wertung6 = (unsigned long long*)res6;
		unsigned long long *wertung7 = (unsigned long long*)res7;
		unsigned posn = 0;
		if (*wertung1 < *wertung)
		{
			*wertung = *wertung1;
			posn = 1;
		}
		if (*wertung2 < *wertung)
		{
			*wertung = *wertung2;
			posn = 2;
		}
		if (*wertung3 < *wertung)
		{
			*wertung = *wertung3;
			posn = 3;
		}
		if (*wertung4 < *wertung)
		{
			*wertung = *wertung4;
			posn = 4;
		}
		if (*wertung5 < *wertung)
		{
			*wertung = *wertung5;
			posn = 5;
		}
		if (*wertung6 < *wertung)
		{
			*wertung = *wertung6;
			posn = 6;
		}
		if (*wertung7 < *wertung)
		{
			*wertung = *wertung7;
			posn = 7;
		}

		if ((*wertung / baseTarget) <= bests[acc].targetDeadline)
		{
			if (*wertung < bests[acc].best)
			{
				Log("\nfound deadline=");	Log_llu(*wertung / baseTarget); Log(" nonce=");	Log_llu(nonce + v + posn); Log(" for account: "); Log_llu(bests[acc].account_id); Log(" file: "); Log((char*)file_name.c_str());
				EnterCriticalSection(&bestsLock);
				bests[acc].best = *wertung;
				bests[acc].nonce = nonce + v + posn;
				bests[acc].DL = *wertung / baseTarget;
				LeaveCriticalSection(&bestsLock);
				EnterCriticalSection(&sharesLock);
				shares.push_back({ file_name, bests[acc].account_id, bests[acc].best, bests[acc].nonce });
				LeaveCriticalSection(&sharesLock);
				if (use_debug)
				{
					char tbuffer[9];
					_strtime_s(tbuffer);
					bm_wattron(2);
					bm_wprintw("%s [%20llu] found DL:      %9llu\n", tbuffer, bests[acc].account_id, bests[acc].DL, 0);
					bm_wattroff(2);
				}
			}
		}
	}
}

//AVX
void procscoop_avx(unsigned long long const nonce, unsigned long long const n, char const *const data, size_t const acc, const std::string &file_name) {
	char const *cache;
	char sig0[32 + 64 + 32];
	char sig1[32 + 64 + 32];
	char sig2[32 + 64 + 32];
	char sig3[32 + 64 + 32];
	char res0[32];
	char res1[32];
	char res2[32];
	char res3[32];
	cache = data;
	unsigned posn;

	memcpy(sig0, signature, 32);
	memcpy(sig1, signature, 32);
	memcpy(sig2, signature, 32);
	memcpy(sig3, signature, 32);
	sig0[96] = -128;
	sig1[96] = -128;
	sig2[96] = -128;
	sig3[96] = -128;
	memset(&sig0[97], 0, 31);
	memset(&sig1[97], 0, 31);
	memset(&sig2[97], 0, 31);
	memset(&sig3[97], 0, 31);

	mshabal_context z;

	for (unsigned long long v = 0; v < n; v += 4)
	{
		memcpy(&sig0[32], &cache[(v + 0) * 64], 64);
		memcpy(&sig1[32], &cache[(v + 1) * 64], 64);
		memcpy(&sig2[32], &cache[(v + 2) * 64], 64);
		memcpy(&sig3[32], &cache[(v + 3) * 64], 64);

		memcpy(&z, &global_128, sizeof(global_128)); // optimization: avx1_mshabal_init(&x, 256);
		avx1_mshabal(&z, (const unsigned char*)sig0, (const unsigned char*)sig1, (const unsigned char*)sig2, (const unsigned char*)sig3, 64 + 32);
		avx1_mshabal_close(&z, 0, 0, 0, 0, 0, res0, res1, res2, res3);

		unsigned long long *wertung = (unsigned long long*)res0;
		unsigned long long *wertung1 = (unsigned long long*)res1;
		unsigned long long *wertung2 = (unsigned long long*)res2;
		unsigned long long *wertung3 = (unsigned long long*)res3;
		posn = 0;
		if (*wertung1 < *wertung)
		{
			*wertung = *wertung1;
			posn = 1;
		}
		if (*wertung2 < *wertung)
		{
			*wertung = *wertung2;
			posn = 2;
		}
		if (*wertung3 < *wertung)
		{
			*wertung = *wertung3;
			posn = 3;
		}


		if ((*wertung / baseTarget) <= bests[acc].targetDeadline)
		{
			if (*wertung < bests[acc].best)
			{
				Log("\nfound deadline=");	Log_llu(*wertung / baseTarget); Log(" nonce=");	Log_llu(nonce + v + posn); Log(" for account: "); Log_llu(bests[acc].account_id); Log(" file: "); Log((char*)file_name.c_str());
				EnterCriticalSection(&bestsLock);
				bests[acc].best = *wertung;
				bests[acc].nonce = nonce + v + posn;
				bests[acc].DL = *wertung / baseTarget;
				LeaveCriticalSection(&bestsLock);
				EnterCriticalSection(&sharesLock);
				shares.push_back({ file_name, bests[acc].account_id, bests[acc].best, bests[acc].nonce });
				LeaveCriticalSection(&sharesLock);
				if (use_debug)
				{
					char tbuffer[9];
					_strtime_s(tbuffer);
					bm_wattron(2);
					bm_wprintw("%s [%20llu] found DL:      %9llu\n", tbuffer, bests[acc].account_id, bests[acc].DL, 0);
					bm_wattroff(2);
				}
			}
		}
	}
}

//SSE4
void procscoop_sse4(unsigned long long const nonce, unsigned long long const n, char const *const data, size_t const acc, const std::string &file_name) {
	char const *cache;
	char sig0[32 + 64 + 32];
	char sig1[32 + 64 + 32];
	char sig2[32 + 64 + 32];
	char sig3[32 + 64 + 32];
	char res0[32];
	char res1[32];
	char res2[32];
	char res3[32];
	cache = data;
	unsigned posn;

	memcpy(sig0, signature, 32);
	memcpy(sig1, signature, 32);
	memcpy(sig2, signature, 32);
	memcpy(sig3, signature, 32);
	sig0[96] = -128;
	sig1[96] = -128;
	sig2[96] = -128;
	sig3[96] = -128;
	memset(&sig0[97], 0, 31);
	memset(&sig1[97], 0, 31);
	memset(&sig2[97], 0, 31);
	memset(&sig3[97], 0, 31);
	mshabal_context z;

	for (unsigned long long v = 0; v < n; v += 4)
	{
		memcpy(&sig0[32], &cache[(v + 0) * 64], 64);
		memcpy(&sig1[32], &cache[(v + 1) * 64], 64);
		memcpy(&sig2[32], &cache[(v + 2) * 64], 64);
		memcpy(&sig3[32], &cache[(v + 3) * 64], 64);

		memcpy(&z, &global_128, sizeof(global_128)); // optimization: avx1_mshabal_init(&x, 256);
		sse4_mshabal(&z, (const unsigned char*)sig0, (const unsigned char*)sig1, (const unsigned char*)sig2, (const unsigned char*)sig3, 64 + 32);
		sse4_mshabal_close(&z, 0, 0, 0, 0, 0, res0, res1, res2, res3);

		unsigned long long *wertung = (unsigned long long*)res0;
		unsigned long long *wertung1 = (unsigned long long*)res1;
		unsigned long long *wertung2 = (unsigned long long*)res2;
		unsigned long long *wertung3 = (unsigned long long*)res3;
		posn = 0;
		if (*wertung1 < *wertung)
		{
			*wertung = *wertung1;
			posn = 1;
		}
		if (*wertung2 < *wertung)
		{
			*wertung = *wertung2;
			posn = 2;
		}
		if (*wertung3 < *wertung)
		{
			*wertung = *wertung3;
			posn = 3;
		}


		if ((*wertung / baseTarget) <= bests[acc].targetDeadline)
		{
			if (*wertung < bests[acc].best)
			{
				Log("\nfound deadline=");	Log_llu(*wertung / baseTarget); Log(" nonce=");	Log_llu(nonce + v + posn); Log(" for account: "); Log_llu(bests[acc].account_id); Log(" file: "); Log((char*)file_name.c_str());
				EnterCriticalSection(&bestsLock);
				bests[acc].best = *wertung;
				bests[acc].nonce = nonce + v + posn;
				bests[acc].DL = *wertung / baseTarget;
				LeaveCriticalSection(&bestsLock);
				EnterCriticalSection(&sharesLock);
				shares.push_back({ file_name, bests[acc].account_id, bests[acc].best, bests[acc].nonce });
				LeaveCriticalSection(&sharesLock);
				if (use_debug)
				{
					char tbuffer[9];
					_strtime_s(tbuffer);
					bm_wattron(2);
					bm_wprintw("%s [%20llu] found DL:      %9llu\n", tbuffer, bests[acc].account_id, bests[acc].DL, 0);
					bm_wattroff(2);
				}
			}
		}
	}
}

//ALL CPUs
void procscoop_sph(const unsigned long long nonce, const unsigned long long n, char const *const data, const size_t acc, const std::string &file_name) {
	char const *cache;
	char sig[32 + 64];
	cache = data;
	char res[32];
	memcpy_s(sig, sizeof(sig), signature, sizeof(char) * 32);

	sph_shabal_context x;
	for (unsigned long long v = 0; v < n; v++)
	{
		memcpy_s(&sig[32], sizeof(sig) - 32, &cache[v * 64], sizeof(char) * 64);

		memcpy(&x, &global_32, sizeof(global_32)); // optimization: sph_shabal256_init(&x);
		sph_shabal256(&x, (const unsigned char*)sig, 64 + 32);
		sph_shabal256_close(&x, res);

		unsigned long long *wertung = (unsigned long long*)res;

		if ((*wertung / baseTarget) <= bests[acc].targetDeadline)
		{
			if (*wertung < bests[acc].best)
			{
				Log("\nfound deadline=");	Log_llu(*wertung / baseTarget); Log(" nonce=");	Log_llu(nonce + v); Log(" for account: "); Log_llu(bests[acc].account_id); Log(" file: "); Log((char*)file_name.c_str());
				EnterCriticalSection(&bestsLock);
				bests[acc].best = *wertung;
				bests[acc].nonce = nonce + v;
				bests[acc].DL = *wertung / baseTarget;
				LeaveCriticalSection(&bestsLock);
				EnterCriticalSection(&sharesLock);
				shares.push_back({ file_name, bests[acc].account_id, bests[acc].best, bests[acc].nonce });
				LeaveCriticalSection(&sharesLock);
				if (use_debug)
				{
					char tbuffer[9];
					_strtime_s(tbuffer);
					bm_wattron(2);
					bm_wprintw("%s [%20llu] found DL:      %9llu\n", tbuffer, bests[acc].account_id, bests[acc].DL, 0);
					bm_wattroff(2);
				}
			}
		}
	}
}


//AVX
void procscoop_avx_fast(unsigned long long const nonce, unsigned long long const n, char const *const data, size_t const acc, const std::string &file_name) {
	char const *cache;
	char sig0[32];
	char end0[32];
	char res0[32];
	char res1[32];
	char res2[32];
	char res3[32];
	cache = data;
	unsigned long long v;

	memmove(sig0, signature, 32);
	end0[0] = -128;
	memset(&end0[1], 0, 31);

	mshabal_context_fast z, z2;
	memcpy(&z2, &global_128_fast, sizeof(global_128_fast)); // local copy of global fast context

    //prepare shabal inputs
	union {
		mshabal_u32 words[64];
		__m128i data[16];
	} u1, u2;

	for (int j = 0; j < 64 / 2; j += 4 ) {
		size_t o = j ;
		u1.words[j + 0] = *(mshabal_u32 *)(sig0 + o);
		u1.words[j + 1] = *(mshabal_u32 *)(sig0 + o);
		u1.words[j + 2] = *(mshabal_u32 *)(sig0 + o);
		u1.words[j + 3] = *(mshabal_u32 *)(sig0 + o);
		u2.words[j + 0 + 32] = *(mshabal_u32 *)(end0 + o);
		u2.words[j + 1 + 32] = *(mshabal_u32 *)(end0 + o);
		u2.words[j + 2 + 32] = *(mshabal_u32 *)(end0 + o);
		u2.words[j + 3 + 32] = *(mshabal_u32 *)(end0 + o);
	}

	for (v = 0; v<n; v += 4) {
		// initialise shaba^l
		memcpy(&z, &z2, sizeof(z2)); // optimization: mshabal256_init(&x, 256);

		// load and shuffle data 
		// NB: this can be further optimised by preshuffling plot files depending on SIMD length and use avx2 memcpy
		// did not find a away yet to completely avoid memcpys

		for (int j = 0; j < 64 / 2; j += 4) {
			size_t o = j;
			u1.words[j + 0 + 32] = *(mshabal_u32 *)(&cache[(v + 0) * 64] + o);
			u1.words[j + 1 + 32] = *(mshabal_u32 *)(&cache[(v + 1) * 64] + o);
			u1.words[j + 2 + 32] = *(mshabal_u32 *)(&cache[(v + 2) * 64] + o);
			u1.words[j + 3 + 32] = *(mshabal_u32 *)(&cache[(v + 3) * 64] + o);
			u2.words[j + 0] = *(mshabal_u32 *)(&cache[(v + 0) * 64 + 32] + o);
			u2.words[j + 1] = *(mshabal_u32 *)(&cache[(v + 1) * 64 + 32] + o);
			u2.words[j + 2] = *(mshabal_u32 *)(&cache[(v + 2) * 64 + 32] + o);
			u2.words[j + 3] = *(mshabal_u32 *)(&cache[(v + 3) * 64 + 32] + o);
		}

		avx1_mshabal_openclose_fast(&z, &u1, &u2, res0, res1, res2, res3, 0);

		unsigned long long *wertung = (unsigned long long*)res0;
		unsigned long long *wertung1 = (unsigned long long*)res1;
		unsigned long long *wertung2 = (unsigned long long*)res2;
		unsigned long long *wertung3 = (unsigned long long*)res3;
		unsigned posn = 0;
		if (*wertung1 < *wertung)
		{
			*wertung = *wertung1;
			posn = 1;
		}
		if (*wertung2 < *wertung)
		{
			*wertung = *wertung2;
			posn = 2;
		}
		if (*wertung3 < *wertung)
		{
			*wertung = *wertung3;
			posn = 3;
		}

		if ((*wertung / baseTarget) <= bests[acc].targetDeadline)
		{
			if (*wertung < bests[acc].best)
			{
				Log("\nfound deadline=");	Log_llu(*wertung / baseTarget); Log(" nonce=");	Log_llu(nonce + v + posn); Log(" for account: "); Log_llu(bests[acc].account_id); Log(" file: "); Log((char*)file_name.c_str());
				EnterCriticalSection(&bestsLock);
				bests[acc].best = *wertung;
				bests[acc].nonce = nonce + v + posn;
				bests[acc].DL = *wertung / baseTarget;
				LeaveCriticalSection(&bestsLock);
				EnterCriticalSection(&sharesLock);
				shares.push_back({ file_name, bests[acc].account_id, bests[acc].best, bests[acc].nonce });
				LeaveCriticalSection(&sharesLock);
				if (use_debug)
				{
					char tbuffer[9];
					_strtime_s(tbuffer);
					bm_wattron(2);
					bm_wprintw("%s [%20llu] found DL:      %9llu\n", tbuffer, bests[acc].account_id, bests[acc].DL, 0);
					bm_wattroff(2);
				}
			}
		}
	}
}