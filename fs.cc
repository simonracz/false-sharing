#include <thread>
#include <atomic>
#include <vector>
#include <future>
#include <iostream>
#include <cmath>

using namespace std;

struct Blob {
	// cache line is 64 bytes long
	// atomic_uint64_t ref_count;
	double a;
	double b;
	double c;
	double d;
	double padding[8]; // 8 * 8 = 64
	atomic_uint64_t ref_count;
};

void worker(int id, Blob& blob, promise<double>&& result) {
	double res = 0;
	for (int i = 0; i < 1000000; ++i) {
		++blob.ref_count;
		res = sin(res) + sin(blob.a * id) + sin(blob.b) + cos(blob.c) + sin(blob.d);
		--blob.ref_count;
	}
	result.set_value(res);
}

int main() {
	Blob blob {0.71, 0.2, 0.334, 0.912, {}, 0};
	vector<thread> threads;
	vector<future<double>> futures;
	for (int i = 0; i < 32; ++i) {
		promise<double> p;
		futures.push_back(p.get_future());
		threads.emplace_back(thread(worker, i, ref(blob), move(p)));
	}
	for (auto& t: threads) {
		t.join();
	}
	for (int i = 0; i < 32; ++i) {
		cout << "ID: " << i << ": " << futures[i].get() << "\n";
	}
}
