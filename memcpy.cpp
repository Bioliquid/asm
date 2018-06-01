#include <iostream>
#include <cstring>
#include <limits>
#include <emmintrin.h>
using namespace std;

namespace lib {
	void memcpy_by_one(unsigned long d, unsigned long s, size_t n) {
		for (size_t i = 0; i < n; ++i) {
			__asm__ volatile(
				"movb %1,   %%al;"
				"movb %%al, %0;"
				: "=m"(*(unsigned long*)d)
				: "m"(*(unsigned long*)s)
				: "al", "memory"
				);
			++d;
			++s;
		}
	}

	void memcpy_by_eight(unsigned long d, unsigned long s, size_t n) {
		for (size_t i = 0; i < n; i += 8) {
			__asm__ volatile(
				"movq %1, %%rax;"
				"movq %%rax, %0;"
				: "=m"(*(unsigned long*)d)
				: "m"(*(unsigned long*)s)
				: "rax", "memory"
				);
			s += 8;
			d += 8;
		}
	}

	void memcpy_by_sixteen(unsigned long d, unsigned long s, size_t n) {
		__m128i tmp;
		for (size_t i = 0; i < n; i += 16) {
			__asm__ volatile(
				"movdqu %2, %1;"
				"movdqu %1, %0;"
				: "=m"(*(unsigned long*)d), "=x"(tmp)
				: "m"(*(unsigned long*)s)
				: "memory"
				);
			s += 16;
			d += 16;
		}
	}

	void memcpy_by_sixteen_aligned(unsigned long d, unsigned long s, size_t n) {
		size_t offset = 16 - d % 16;
		if (offset != 16) {
			memcpy_by_one(d, s, offset);
			n -= offset;
			d += offset;
			s += offset;
		}
		size_t tail = n % 16;
		n -= tail;
		if (n >= 16) {
			memcpy_by_sixteen(d, s, n);
		}
		if (tail > 0) {
			d += n;
			s += n;
			memcpy_by_one(d, s, tail);
		}
	}

	void memcpy(void *dest, void const *src, size_t len, char const *v) {
		if (strcmp(v, "16a") == 0) {
			memcpy_by_sixteen_aligned(reinterpret_cast<unsigned long>(dest), reinterpret_cast<unsigned long>(src), len);
		}
		if (strcmp(v, "16") == 0) {
			memcpy_by_sixteen(reinterpret_cast<unsigned long>(dest), reinterpret_cast<unsigned long>(src), len);
		}
		if (strcmp(v, "8") == 0) {
			memcpy_by_eight(reinterpret_cast<unsigned long>(dest), reinterpret_cast<unsigned long>(src), len);
		}
		if (strcmp(v, "1") == 0) {
			memcpy_by_one(reinterpret_cast<unsigned long>(dest), reinterpret_cast<unsigned long>(src), len);
		}
	}
}

int main(int argc, char **argv) {
	int n = rand() % 1000 + 1000;
	int *a = new int[n];
	int *b = new int[n];
	int *c = new int[n];
	int *d = new int[n];
	int *e = new int[n];

	for (int i = 0; i < n; ++i) {
		a[i] = rand() % numeric_limits<int>::max();
	}

	lib::memcpy(b, a, sizeof(*a) * n, "16a");
	lib::memcpy(c, a, sizeof(*a) * n, "16");
	lib::memcpy(d, a, sizeof(*a) * n, "8");
	lib::memcpy(e, a, sizeof(*a) * n, "1");

	bool passed_b = true;
	bool passed_c = true;
	bool passed_d = true;
	bool passed_e = true;

	for (int i = 0; i < n; ++i) {
		cout << b[i] << ' ' << c[i] << ' ' << d[i] << ' ' << e[i] << endl;
		if (b[i] != a[i]) {
			passed_b = false;
		}
		if (c[i] != a[i]) {
			passed_c = false;
		}
		if (d[i] != a[i]) {
			passed_d = false;
		}
		if (e[i] != a[i]) {
			passed_e = false;
		}
	}
	cout << "Passed memcpy_16a: " << (passed_b ? "yes" : "no") << endl;
	cout << "Passed memcpy_16:  " << (passed_c ? "yes" : "no") << endl;
	cout << "Passed memcpy_8:   " << (passed_d ? "yes" : "no") << endl;
	cout << "Passed memcpy_1:   " << (passed_e ? "yes" : "no") << endl;
}