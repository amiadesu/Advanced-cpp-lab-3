#include <iostream>
#include <syncstream>
#include <vector>
#include <map>
#include <thread>
#include <barrier>
#include <atomic>

using namespace std;

struct InitData {
	const int nt = 7;

	const vector<pair<char, int>> totals = {
		{'a', 7}, {'b', 8}, {'c', 8}, {'d', 9},
		{'e', 8}, {'f', 6}, {'g', 5}, {'h', 4},
		{'i', 8}, {'j', 8}
	};

	const vector<string> jobs = {
		"aaaaaaa", // 7a
		"bbccddd", // 2b 2c 3d
		"bbccccd", // 2b 4c 1d
		"bbbbccd", // 4b 2c 1d
		"eeedddd", // 3e 4d
		"eefffgg", // 2e 3f 2g
		"eeefffh", // 3e 3f 1h
		"ggghhhi", // 3g 3h 1i
		"iiiiiii", // 7i
		"jjjjjjj", // 7j
		"j"        // 1j                  
	};
};

void f(char x, int i) {
	osyncstream(cout) << "З набору " << x << " виконано дію " << i << "." << endl;
}

map<char, atomic<int>> init_counters(const InitData& data) {
	map<char, atomic<int>> counters;
	for (auto& [ch, _] : data.totals) {
		counters[ch] = 1;
	}
	return counters;
}

void run_simulation(const InitData& data) {
	auto next_idx = init_counters(data);

	barrier sync(data.nt);

	vector<jthread> workers;
	workers.reserve(data.nt);

	for (int tid = 0; tid < data.nt; ++tid) {
		workers.emplace_back([&, tid] {
			for (const auto& job : data.jobs) {
				if (tid < job.size()) {
					int i = next_idx[job[tid]].fetch_add(1);
					f(job[tid], i);
				}
				sync.arrive_and_wait();
			}
			});
	}
}

int main() {
#ifdef _WIN32
	system("chcp 65001 > nul");
#endif

	InitData data;

	osyncstream(cout) << "Обчислення розпочато." << endl;

	run_simulation(data);

	osyncstream(cout) << "Обчислення завершено." << endl;
}
