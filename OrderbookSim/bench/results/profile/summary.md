# command: ./build/orderbook_bench --reps 30 --out <scratch> & sleep 3; sample <pid> 8 1 -mayDie -file sample_report.txt
# then: python3 OrderbookSim/bench/profile_summary.py sample_report.txt
# binary: Release build at 881c98c (bench.cpp and book code identical at 432ef48); machine shared, 1-min load average about 25

main-thread samples: 4084; inside the order book's public calls: 3353

| frame (inclusive; prefix match) | samples | share of book calls | share of main thread |
|---|---:|---:|---:|
| `Orderbook::addOrder(OrderType` | 2941 | 87.7% | 72.0% |
| `Orderbook::CancelOrder(unsigned long long)` | 412 | 12.3% | 10.1% |
| `Orderbook::MatchOrders()` | 1525 | 45.5% | 37.3% |
| `OrderDetailHistory::addOrderToHistory` | 631 | 18.8% | 15.5% |
| `OrderDetailHistory::removeMatchedOrder` | 118 | 3.5% | 2.9% |
| `OrderDetailHistory::addOrderToPurchaseHistory` | 83 | 2.5% | 2.0% |
| `OrderDetailHistory::updateNeutralNetwork` | 899 | 26.8% | 22.0% |
| `NeuralNetwork::Run` | 149 | 4.4% | 3.6% |
| `localtime_r` | 313 | 9.3% | 7.7% |
| `std::chrono::system_clock::now()` | 250 | 7.5% | 6.1% |
| `std::__tree` | 39 | 1.2% | 1.0% |
| `std::__hash_table` | 1012 | 30.2% | 24.8% |
| `operator new(unsigned long)` | 541 | 16.1% | 13.2% |
| `_free` | 198 | 5.9% | 4.8% |
| `mach_absolute_time` | 332 | 9.9% | 8.1% |
