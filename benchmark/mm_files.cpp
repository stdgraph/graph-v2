#include "mm_files.hpp"

using std::filesystem::path;
using std::vector;

path        gap = path(BENCHMARK_DATA_DIR) / "GAP";
bench_files gap_road(gap, "GAP-road", "GAP-road.mtx", "GAP-road.sorted.mtx", "GAP-road_sources.mtx");
bench_files gap_twitter(gap, "GAP-twitter", "GAP-twitter.mtx", "GAP-twitter.sorted.mtx", "GAP-twitter_sources.mtx");
bench_files gap_web(gap, "GAP-web", "GAP-web.mtx", "GAP-web.sorted.mtx", "GAP-web_sources.mtx");
bench_files gap_kron(gap, "GAP-kron", "GAP-kron.mtx", "GAP-kron.sorted.mtx", "GAP-kron_sources.mtx");
bench_files gap_urand(gap, "GAP-urand", "GAP-urand.mtx", "GAP-urand.sorted.mtx", "GAP-urand_sources.mtx");


path        g2bench = path(BENCHMARK_DATA_DIR) / "g2bench";
bench_files g2bench_chesapeake(g2bench, "", "chesapeake.mtx", "chesapeake.sorted.mtx", "sources.mtx");
bench_files g2bench_bips98_606(g2bench, "", "bips98_606.mtx", "bips98_606.sorted.mtx", "sources.mtx");
// sources.mtx is a generic set of sources that can be used with either g2bench graph

vector<bench_files> datasets = {gap_road, gap_twitter, gap_web, gap_kron, gap_urand, g2bench_chesapeake, g2bench_bips98_606};
