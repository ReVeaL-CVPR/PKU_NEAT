#define _CRT_SECURE_NO_WARNINGS
#include "GA.h"
//#include <cstring>
#include <random>
#include <iostream>
#include <cmath>
using namespace std;
random_device rd;
class test{
public:
	int nums[10];

	test() {
		for(int i = 0; i < 10; ++i) {
			nums[i] = rd() % 20 - 10;
		}
	}

	pair<test, test> cross(test b, int arg) {
		int point = rd() % 10;
		test a(*this);
		for(int i = 0; i < point; ++i) {
			swap(a.nums[i], b.nums[i]);
		}
		return make_pair(a, b);
	}

	void variation(int) {
		int point = rd() % 10;
		nums[point] = rd() % 20 - 10;
	}

	string toString() {
		char tmp[100] = "";
		for(int i = 0; i < 10; ++i) {
			sprintf(tmp, "%s%d", tmp, nums[i]);
		}
		return tmp;
	}
	int getSum() {
		int sum = 0;
		for(int i = 0; i < 10; ++i) {
			sum += nums[i];
		}
		return sum;
	}
};
class test2 {
public:
	double nums[10];

	test2() {
		for (int i = 0; i < 10; ++i) {
			nums[i] = static_cast<double>(rd()) / static_cast<double>(numeric_limits<unsigned>::max()) - 0.5;
		}
	}

	pair<test2, test2> cross(test2 b, int arg) {
		int point = rd() % 10;
		test2 a(*this);
		for (int i = 0; i < point; ++i) {
			swap(a.nums[i], b.nums[i]);
		}
		return make_pair(a, b);
	}

	void variation(int) {
		int point = rd() % 10;
		nums[point] = static_cast<double>(rd()) / static_cast<double>(numeric_limits<unsigned>::max()) - 0.5;;
	}

	string toString() {
		char tmp[100] = "";
		for (int i = 0; i < 10; ++i) {
			sprintf(tmp, "%s%lf", tmp, nums[i]);
		}
		return tmp;
	}
	double getSum() {
		double sum = 0;
		for (int i = 0; i < 10; ++i) {
			sum += nums[i];
		}
		return sum;
	}
};
double quad(test a) {
	int tmp = a.getSum();
	return 10000 - tmp * tmp;
}
double getSin(test2 a) {
	double tmp = a.getSum();
	return fabs(cos(tmp));
}
int main() {
	GA_Machine<test2> m;
	m.set(20, 1, 0.8, 1, 0.3, 1, 100000, ELITISM);
	m.init();
	m.setFitness(getSin);
	m.start();
	while(!m.isStop());
	auto ans = m.individual();
	cout << ans.getSum();
	return 0;
}