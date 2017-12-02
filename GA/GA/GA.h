#pragma once
const int NTIMES = 1;
const int NSAME = 2;
const int ELITISM = 1;
const int PROTECTIONISM = 2;
const int SELFVARIATION = 4;
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <fstream>
#include <random>
#include <limits>
#include "omp.h"
template<class T>
class GA_Machine {
	
	//Ŀǰ��������
	int iterTimes = 0;
	int iterRemain = 0;

	//������Ŀ����Ⱥ��Ϣ�����Ÿ�����Ϣ
	int indiNums = 0;
	std::vector<T> indi;
	T bestIndividual;

	//����ģʽ��endMode = 1��������N�ν����� = 2 ����N���޸������������
	int endMode = 0;
	int endArgs = 0;

	//�����ʼ����췽ʽ
	double varProb = 0.0;
	int varMode = 0;

	//�����ʼ����淽ʽ
	double crossProb = 0.0;
	int crossMode = 0;

	//��Ӣ���壨ÿ�ε���ǰ�������Ÿ�����
	bool elitism = false;
	//�������壨ÿ�ε���ǰ����������
	bool protectionism = false;
	//�������Ա���
	bool selfVariation = false;

	//��Ӧ�Ⱥ���
	double(*fitness)(T) = nullptr;

	//������
	std::mutex mtx;
	std::thread iterThread;

	bool configLoaded = false;
	bool fitnessLoaded = false;
	bool inited = false;
	bool stoped = true;
	bool needStop = false;

public:
	GA_Machine() = default;
	GA_Machine(GA_Machine&) = default;
	~GA_Machine() = default;

	bool init();
	bool start();
	bool stop();
	bool isStop();
	bool save(const char*);
	bool load(const char*);
	bool set(int, int, double, int, double, int, int, int);
	bool setFitness(double(*fitFunc)(T));
	int getIterTimes();
	std::vector<T>& population();
	T& individual();
	void iterFunc();
};

template<class T>
inline bool GA_Machine<T>::init() {
	//�����ʼ��Ⱥ��Ϣ
	if(inited) {
		return true;
	}
	for(int i = 0; i < indiNums; ++i) {
		indi.emplace_back();
	}
	inited = true;
	return inited;
}

#ifdef _MSC_VER
#pragma optimize("", off)
#elif __GNUC__
#pragma GCC optimize ("-O0")
#endif
template<class T>
inline bool GA_Machine<T>::start() {
	//����GA�������е���
	if (configLoaded && fitnessLoaded && inited && stoped) {
		stoped = false;
		needStop = false;
		if(endMode == NTIMES) {
			iterRemain = endArgs - iterTimes;
		}
		//iterThread = std::thread(&iterFunc);
		std::thread(std::bind(&GA_Machine<T>::iterFunc, this)).detach();
		return true;
	}
	return false;
}

template<class T>
inline bool GA_Machine<T>::stop() {
	//����ֹͣ��־����
	if (!stoped) {
		needStop = true;
		while (!stoped);
		return true;
	}
	return false;
}

template<class T>
inline bool GA_Machine<T>::isStop() {
	return stoped;
}

template<class T>
inline bool GA_Machine<T>::save(const char * outPath) {
	std::fstream outFile(outPath, std::ios::out);
	outFile << iterTimes << std::endl
		<< indiNums << std::endl
		<< endMode << std::endl
		<< endArgs << std::endl
		<< varProb << std::endl
		<< varMode << std::endl
		<< crossProb << std::endl
		<< crossMode << std::endl
		<< elitism << std::endl
		<< protectionism << std::endl
		<< selfVariation << std::endl;
	for(auto x :indi) {
		outFile << x.toString() << std::endl; 
	}
	outFile << bestIndividual.toString() << std::endl;
	outFile.close();
	return true;
}

template<class T>
inline bool GA_Machine<T>::load(const char * inPath) {
	std::fstream inFile(inPath, std::ios::in);
	inFile >> iterTimes >> indiNums >> endMode >> endArgs
		>> varProb >> varMode >> crossProb >> crossMode
		>> elitism >> protectionism >> selfVariation;
	for(int i = 0; i < indiNums; ++i) {
		std::string tmp;
		inFile >> tmp;
		indi.emplace_back(tmp);
	}
	std::string tmp;
	inFile >> tmp;
	bestIndividual = T(tmp);
	
	configLoaded = true;
	fitnessLoaded = false;
	inited = true;
	
	return true;
}

template<class T>
inline bool GA_Machine<T>::set(int indiNum, int crossMod, double crossPro,
	int varMod, double varPro, int endMod, int endArg, int flags){
	indiNums = indiNum;
	crossMode = crossMod;
	crossProb = crossPro;
	varMode = varMod;
	varProb = varPro;
	endMode = endMod;
	endArgs = endArg;

	elitism = flags & ELITISM;
	protectionism = flags & PROTECTIONISM;
	selfVariation = flags & SELFVARIATION;

	configLoaded = true;
	return true;
}

template<class T>
inline bool GA_Machine<T>::setFitness(double(*fitFunc)(T)) {
	fitness = fitFunc;
	fitnessLoaded = true;
	return true;
}

template<class T>
inline int GA_Machine<T>::getIterTimes() {
	return iterTimes;
}

template<class T>
inline std::vector<T>& GA_Machine<T>::population() {
	return indi;
}

template<class T>
inline T & GA_Machine<T>::individual() {
	return bestIndividual;
}

template<class T>
inline void GA_Machine<T>::iterFunc() {
	while (1) {
		if ((endMode == NTIMES && iterRemain == 0) || needStop) {
			stoped = true;
			needStop = false;
			return;
		}
		if (endMode == NTIMES)
			--iterRemain;
		iterTimes++;
		std::vector<double> fitnessNum(indiNums, 0.0);
		std::vector<T> tmpIndi((indiNums / 2) * 2, T());
		std::vector<double> prob(indiNums, 0.0);
		double totalFit = 0.0;
		#pragma omp parallel
		{
			#pragma omp for reduction(+:totalFit)
			for (int i = 0; i < indiNums; ++i) {
				//���м�����Ӧ��
				fitnessNum[i] = fitness(indi[i]);
				totalFit += fitnessNum[i];
			}

			//���ÿ���ĸ�������
			#pragma omp single
			{
				int bestIndex = 0;
				double bestFit = fitnessNum[0];
				for (int i = 1; i < indiNums; ++i) {
					if(fitnessNum[i] > bestFit) {
						bestIndex = i;
						bestFit = fitnessNum[i];
					}
					fitnessNum[i] += fitnessNum[i - 1];
				}
				bestIndividual = indi[bestIndex];
				//std::cout << bestIndividual.getSum() << std::endl;
			}
			#pragma omp for
			for (int i = 0; i < indiNums; ++i) {
				prob[i] = fitnessNum[i] / totalFit;
			}

			//���̷�ѡ�����������ӽ�
			#pragma omp for
			for (int i = 0; i < indiNums / 2; ++i) {
				unsigned int rdNum1, rdNum2, cross, var;
				std::random_device rd;
				rdNum1 = rd();
				rdNum2 = rd();
				cross = rd();
				var = rd();
				double pro1 = static_cast<double>(rdNum1) / static_cast<double>(std::numeric_limits<unsigned>::max());
				double pro2 = static_cast<double>(rdNum2) / static_cast<double>(std::numeric_limits<unsigned>::max());
				double crossPro = static_cast<double>(cross) / static_cast<double>(std::numeric_limits<unsigned>::max());
				double varPro = static_cast<double>(var) / static_cast<double>(std::numeric_limits<unsigned>::max());

				int flag1 = false, flag2 = false;
				int indi1, indi2;
				for (int j = 0; j < indiNums && !(flag1 && flag2); ++j) {
					if (!flag1 && prob[j] > pro1) {
						indi1 = j;
						flag1 = true;
					}
					if (!flag2 && prob[j] > pro2) {
						indi2 = j;
						flag2 = true;
					}
				}
				T new1, new2;
				if (crossPro > crossProb) {
					//�������ӽ�
					new1 = indi[indi1];
					new2 = indi[indi2];
				}
				else {
					//�����ӽ�
					auto indiPair = indi[indi1].cross(indi[indi2], crossMode);
					new1 = indiPair.first;
					new2 = indiPair.second;
				}
				if (varPro > varProb) {
					//�����б���
				}
				else {
					//���б���
					new1.variation(varMode);
					new2.variation(varMode);
				}
				tmpIndi[2 * i] = new1;
				tmpIndi[2 * i + 1] = new2;
			}
		}
		if (elitism) {
			//����õļ�����Ⱥ
			std::random_device rd;
			int point = rd() % tmpIndi.size();
			tmpIndi[point] = bestIndividual;
		}
		if (protectionism) {
			//����ļ�����Ⱥ
		}
		indi = tmpIndi;
		indiNums = indi.size();
	}
}
